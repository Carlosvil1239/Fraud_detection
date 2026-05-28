//
// Created by Carlos Villacañas.
//

#include "training/MarkovTrainerTBB.hpp"
#include "io/DatasetReader.hpp"
#include "training/StateExtractor.hpp"

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/enumerable_thread_specific.h>
#include <oneapi/tbb/parallel_for.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

// Keeps the discovered states and their numeric ids.
struct StateTable {
    std::vector<std::string> states;
    std::unordered_map<std::string, std::size_t> index;

    std::size_t ensure_state(const std::string& state) {
        if (const auto it = index.find(state); it != index.end()) {
            return it->second;
        }

        const std::size_t new_index = states.size();
        states.push_back(state);
        index.emplace(states.back(), new_index);
        return new_index;
    }
};

// Stores transition counts in a flat matrix.
struct TransitionAccumulator {
    explicit TransitionAccumulator(std::size_t matrix_size = 0)
        : counts(matrix_size, 0ULL) {
    }

    void add_transition(std::size_t from, std::size_t to, std::size_t state_count) {
        counts[from * state_count + to] += 1ULL;
    }

    void merge_from(const TransitionAccumulator& other) {
        if (counts.size() != other.counts.size()) {
            throw std::runtime_error("TransitionAccumulator: incompatible accumulator sizes.");
        }

        for (std::size_t i = 0; i < counts.size(); ++i) {
            counts[i] += other.counts[i];
        }
    }

    std::vector<std::uint64_t> counts;
};

// Saves the first and last state of an entity inside one block.
struct ChunkBoundaryInfo {
    std::uint32_t first_state = 0U;
    std::uint32_t last_state = 0U;
};

// Stores the boundary information found for one block.
struct BlockResult {
    std::unordered_map<std::uint32_t, ChunkBoundaryInfo> boundaries;
};

// Reads several complete blocks so TBB can process them together.
std::vector<std::string> read_block_batch(
    std::ifstream& in,
    std::size_t target_bytes,
    const std::size_t max_blocks,
    std::string& carry
) {
    std::vector<std::string> blocks;
    blocks.reserve(max_blocks);

    for (std::size_t i = 0; i < max_blocks; ++i) {
        std::string block;
        if (!fd::io::read_next_block(in, target_bytes, carry, block)) {
            break;
        }
        blocks.push_back(std::move(block));
    }

    return blocks;
}

// Chooses a small batch size based on the number of hardware threads.
std::size_t choose_batch_block_count() {
    const unsigned int hw = std::max(1u, std::thread::hardware_concurrency());
    return std::max<std::size_t>(1, static_cast<std::size_t>(hw) * 2ULL);
}

}

namespace fd::training::parallel {

fd::model::MarkovModel train_from_dataset_tbb(
    std::string_view dataset_path,
    int state_position,
    const fd::training::TrainingOptions& opt,
    fd::training::TrainingReport* out_report
) {
    TrainingReport rep;

    StateTable state_table;

    // Map entity ids to integers once, so later arrays can use compact indexes.
    std::deque<std::string> entity_storage;
    std::unordered_map<std::string_view, std::uint32_t> entity_to_id;
    entity_to_id.reserve(1 << 16);

    std::size_t valid_records = 0;

    // First pass: discover states and assign integer ids to entities.
    {
        std::ifstream in{std::string(dataset_path), std::ios::binary};
        if (!in) {
            throw std::runtime_error(
                std::string("Cannot open dataset file: ") + std::string(dataset_path)
            );
        }

        std::string carry;
        while (true) {
            std::string block;
            if (!fd::io::read_next_block(in, fd::io::kTargetBlockBytes, carry, block)) {
                break;
            }

            fd::io::for_each_record_in_buffer(
                block,
                [&](bool has_comma, std::string_view entity_id, std::string_view record) {
                    rep.lines_read++;

                    if (!has_comma) {
                        return;
                    }

                    const std::string state =
                        fd::training::extract_state_from_record(record, state_position);

                    if (state.empty()) {
                        return;
                    }

                    state_table.ensure_state(state);

                    if (!entity_to_id.contains(entity_id)) {
                        const auto new_id =
                            static_cast<std::uint32_t>(entity_to_id.size());
                        entity_storage.emplace_back(entity_id);
                        entity_to_id.emplace(std::string_view(entity_storage.back()), new_id);
                    }

                    ++valid_records;
                }
            );
        }
    }

    rep.entities_seen = entity_to_id.size();
    rep.transitions_counted =
        (valid_records >= rep.entities_seen) ? (valid_records - rep.entities_seen) : 0;

    const std::size_t n = state_table.states.size();
    if (n == 0) {
        throw std::runtime_error("No states were found during training.");
    }

    const std::size_t entity_count = entity_to_id.size();
    constexpr std::uint32_t invalid_state = std::numeric_limits<std::uint32_t>::max();

    // Each worker counts transitions locally and the totals are merged later.
    oneapi::tbb::enumerable_thread_specific<TransitionAccumulator> tls(
        [n] { return TransitionAccumulator(n * n); }
    );

    TransitionAccumulator boundary_accumulator(n * n);
    std::vector<std::uint32_t> carry_last_state(entity_count, invalid_state);

    const std::size_t batch_blocks = choose_batch_block_count();

    // Second pass: count transitions inside blocks in parallel.
    {
        std::ifstream in{std::string(dataset_path), std::ios::binary};
        if (!in) {
            throw std::runtime_error(
                std::string("Cannot open dataset file: ") + std::string(dataset_path)
            );
        }

        std::string carry;

        while (true) {
            std::vector<std::string> blocks =
                read_block_batch(in, fd::io::kTargetBlockBytes, batch_blocks, carry);

            if (blocks.empty()) {
                break;
            }

            std::vector<BlockResult> results(blocks.size());

            const std::size_t estimated_entities_per_block =
                std::max<std::size_t>(
                    1,
                    entity_count / std::max<std::size_t>(1, blocks.size())
                );

            for (auto& result : results) {
                result.boundaries.reserve(estimated_entities_per_block);
            }

            oneapi::tbb::parallel_for(
                oneapi::tbb::blocked_range<std::size_t>(0, blocks.size(), 1),
                [&](const oneapi::tbb::blocked_range<std::size_t>& range) {
                    auto& local_acc = tls.local();

                    for (std::size_t block_idx = range.begin(); block_idx != range.end(); ++block_idx) {
                        auto& result = results[block_idx];
                        const std::string& block = blocks[block_idx];

                        fd::io::for_each_record_in_buffer(
                            block,
                            [&](bool has_comma, std::string_view entity_id, std::string_view record) {
                                if (!has_comma) {
                                    return;
                                }

                                const std::string state =
                                    fd::training::extract_state_from_record(record, state_position);

                                if (state.empty()) {
                                    return;
                                }

                                const auto entity_it = entity_to_id.find(entity_id);
                                if (entity_it == entity_to_id.end()) {
                                    throw std::runtime_error(
                                        "Entity discovered in counting pass was missing from the entity table."
                                    );
                                }

                                const auto state_it = state_table.index.find(state);
                                if (state_it == state_table.index.end()) {
                                    throw std::runtime_error(
                                        "State discovered in counting pass was missing from the state table."
                                    );
                                }

                                const std::uint32_t entity_num = entity_it->second;
                                const auto state_num =
                                    static_cast<std::uint32_t>(state_it->second);

                                const auto [it, inserted] =
                                    result.boundaries.try_emplace(
                                        entity_num,
                                        ChunkBoundaryInfo{state_num, state_num}
                                    );

                                if (!inserted) {
                                    local_acc.add_transition(it->second.last_state, state_num, n);
                                    it->second.last_state = state_num;
                                }
                            }
                        );
                    }
                }
            );

            // Add transitions that cross block boundaries in file order.
            for (auto &[boundaries] : results) {
                for (const auto& [entity_num, info] : boundaries) {
                    if (const std::uint32_t prev_state = carry_last_state[entity_num]; prev_state != invalid_state) {
                        boundary_accumulator.add_transition(prev_state, info.first_state, n);
                    }
                    carry_last_state[entity_num] = info.last_state;
                }
            }
        }
    }

    TransitionAccumulator accumulated(n * n);
    for (const auto& local : tls) {
        accumulated.merge_from(local);
    }
    accumulated.merge_from(boundary_accumulator);

    std::vector<double> probs(n * n, 0.0);

    // Convert transition counts into probability rows.
    oneapi::tbb::parallel_for(
        oneapi::tbb::blocked_range<std::size_t>(0, n, 64),
        [&](const oneapi::tbb::blocked_range<std::size_t>& range) {
            for (std::size_t i = range.begin(); i != range.end(); ++i) {
                double row_sum = 0.0;

                for (std::size_t j = 0; j < n; ++j) {
                    row_sum += static_cast<double>(accumulated.counts[i * n + j]) + opt.alpha;
                }

                if (row_sum <= 0.0) {
                    if (opt.uniform_if_dead_end) {
                        const double uniform_value = 1.0 / static_cast<double>(n);
                        for (std::size_t j = 0; j < n; ++j) {
                            probs[i * n + j] = uniform_value;
                        }
                    } else {
                        for (std::size_t j = 0; j < n; ++j) {
                            probs[i * n + j] = 0.0;
                        }
                    }
                    continue;
                }

                for (std::size_t j = 0; j < n; ++j) {
                    const double numerator =
                        static_cast<double>(accumulated.counts[i * n + j]) + opt.alpha;
                    probs[i * n + j] = numerator / row_sum;
                }
            }
        }
    );

    if (out_report) {
        *out_report = rep;
    }

    return {state_table.states, probs};
}

}
