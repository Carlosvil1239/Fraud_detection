//
// Created by Carlos Villacañas.
//

#include "training/MarkovTrainerSequential.hpp"
#include "io/DatasetReader.hpp"
#include "training/StateExtractor.hpp"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

// Keeps the discovered states and their numeric ids.
struct StateTable {
    std::vector<std::string> states;
    std::unordered_map<std::string, std::size_t> index;

    std::size_t ensure_state(const std::string& s) {
        const auto it = index.find(s);
        if (it != index.end()) {
            return it->second;
        }

        const std::size_t id = states.size();
        states.push_back(s);
        index.emplace(states.back(), id);
        return id;
    }
};

}

namespace fd::training::sequential {

fd::model::MarkovModel train_from_dataset(
    std::string_view dataset_path,
    int state_position,
    const fd::training::TrainingOptions& opt,
    fd::training::TrainingReport* out_report
) {
    fd::training::TrainingReport rep;

    StateTable table;
    std::unordered_set<std::string> valid_entities;
    valid_entities.reserve(1 << 16);

    std::size_t valid_records = 0;

    // First pass: discover all states and valid entities.
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

                    const std::string state = fd::training::extract_state_from_record(record, state_position);

                    if (state.empty()) {
                        return;
                    }

                    table.ensure_state(state);
                    valid_entities.emplace(entity_id);
                    ++valid_records;
                }
            );
        }
    }

    rep.entities_seen = valid_entities.size();
    rep.transitions_counted =
        (valid_records >= rep.entities_seen) ? (valid_records - rep.entities_seen) : 0;

    const std::size_t n = table.states.size();
    if (n == 0) {
        throw std::runtime_error("No states were found during training.");
    }

    std::vector<std::uint64_t> counts(n * n, 0ULL);
    std::unordered_map<std::string, std::size_t> last_state_by_entity;
    last_state_by_entity.reserve(valid_entities.size() * 2U);

    // Second pass: count transitions between consecutive states per entity.
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
                    if (!has_comma) {
                        return;
                    }

                    const std::string state =
                        fd::training::extract_state_from_record(std::string(record), state_position);

                    if (state.empty()) {
                        return;
                    }

                    const auto state_it = table.index.find(state);
                    if (state_it == table.index.end()) {
                        throw std::runtime_error(
                            "State discovered in counting pass was missing from the state table."
                        );
                    }

                    const std::size_t curr_idx = state_it->second;
                    const std::string entity_key(entity_id);

                    const auto prev_it = last_state_by_entity.find(entity_key);
                    if (prev_it != last_state_by_entity.end()) {
                        counts[prev_it->second * n + curr_idx] += 1ULL;
                    }

                    last_state_by_entity[entity_key] = curr_idx;
                }
            );
        }
    }

    std::vector<double> probs(n * n, 0.0);

    // Convert transition counts into probability rows.
    for (std::size_t i = 0; i < n; ++i) {
        double row_sum = 0.0;

        for (std::size_t j = 0; j < n; ++j) {
            row_sum += static_cast<double>(counts[i * n + j]) + opt.alpha;
        }

        if (row_sum <= 0.0) {
            if (opt.uniform_if_dead_end) {
                const double u = 1.0 / static_cast<double>(n);
                for (std::size_t j = 0; j < n; ++j) {
                    probs[i * n + j] = u;
                }
            } else {
                for (std::size_t j = 0; j < n; ++j) {
                    probs[i * n + j] = 0.0;
                }
            }
            continue;
        }

        for (std::size_t j = 0; j < n; ++j) {
            const double num = static_cast<double>(counts[i * n + j]) + opt.alpha;
            probs[i * n + j] = num / row_sum;
        }
    }

    if (out_report) {
        *out_report = rep;
    }

    return {table.states, probs};
}

}
