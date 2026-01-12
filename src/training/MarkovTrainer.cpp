//
// Created by Carlos Villacañas Iglesias.
//
#include "../../include/training/MarkovTrainer.hpp"


#include <fstream>
#include <unordered_map>
#include <stdexcept>
#include <cstdint>

// Dynamic state table used during training.
struct StateTable {
    std::vector<std::string> states;
    std::unordered_map<std::string, std::size_t> index;

    std::size_t ensure_state(const std::string& s) {
        auto it = index.find(s);
        if (it != index.end()) return it->second;
        const std::size_t id = states.size();
        states.push_back(s);
        index.emplace(states.back(), id);
        return id;
    }
};

MarkovModel MarkovTrainer::train_from_dataset(
        const std::string& dataset_path,
        const TrainingOptions& opt,
        TrainingReport* out_report
) {
    TrainingReport rep;

    std::ifstream in(dataset_path);
    if (!in) {
        throw std::runtime_error("Cannot open dataset file: " + dataset_path);
    }

    StateTable table;

    // counts is a row-major matrix (n*n) of transition counts.
    std::vector<std::uint64_t> counts;

    // Keep last state per entity in streaming order.
    std::unordered_map<std::string, std::size_t> last_state_by_entity;
    last_state_by_entity.reserve(1 << 16);

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        rep.lines_read++;

        const std::size_t first_comma = line.find(',');
        if (first_comma == std::string::npos) continue;

        const std::size_t last_comma = line.rfind(',');
        if (last_comma == std::string::npos || last_comma <= first_comma) continue;

        const std::string entity_id = line.substr(0, first_comma);
        const std::string state     = line.substr(last_comma + 1); // "MNL"

        if (state.empty()) continue;


        // Map state -> index (dynamic: add if new).
        const std::size_t old_n = table.states.size();
        const std::size_t curr_idx = table.ensure_state(state);
        const std::size_t new_n = table.states.size();

        // If a new state appears, we must expand the counts matrix from old_n^2 to new_n^2.
        if (new_n != old_n) {
            std::vector<std::uint64_t> new_counts(new_n * new_n, 0ULL);

            // Copy previous counts into the top-left old_n x old_n block.
            for (std::size_t i = 0; i < old_n; ++i) {
                for (std::size_t j = 0; j < old_n; ++j) {
                    new_counts[i * new_n + j] = counts[i * old_n + j];
                }
            }

            counts.swap(new_counts);
        }

        // Count transition for this entity if we have a previous state.
        auto it_prev = last_state_by_entity.find(entity_id);
        if (it_prev != last_state_by_entity.end()) {
            const std::size_t prev_idx = it_prev->second;
            const std::size_t n = table.states.size();
            counts[prev_idx * n + curr_idx] += 1ULL;
            rep.transitions_counted++;
        } else {
            rep.entities_seen++;
        }

        last_state_by_entity[entity_id] = curr_idx;
    }

    const std::size_t n = table.states.size();
    if (n == 0) {
        throw std::runtime_error("No states were found during training.");
    }

    // If we never had any state expansion (edge case), counts may still be empty.
    if (counts.empty()) {
        counts.assign(n * n, 0ULL);
    }

    // Convert counts -> probabilities with optional smoothing.
    std::vector<double> probs(n * n, 0.0);

    for (std::size_t i = 0; i < n; ++i) {
        double row_sum = 0.0;

        for (std::size_t j = 0; j < n; ++j) {
            row_sum += static_cast<double>(counts[i * n + j]) + opt.alpha;
        }

        if (row_sum <= 0.0) {
            if (opt.uniform_if_dead_end) {
                const double u = 1.0 / static_cast<double>(n);
                for (std::size_t j = 0; j < n; ++j) probs[i * n + j] = u;
            } else {
                for (std::size_t j = 0; j < n; ++j) probs[i * n + j] = 0.0;
            }
            continue;
        }

        for (std::size_t j = 0; j < n; ++j) {
            const double num = static_cast<double>(counts[i * n + j]) + opt.alpha;
            probs[i * n + j] = num / row_sum;
        }
    }

    if (out_report) *out_report = rep;
    return {table.states, probs};
}
