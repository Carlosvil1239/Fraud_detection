//
// Created by Carlos Villacañas Iglesias.
//

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <optional>

#include "../model/MarkovModel.hpp"

// Simple report to debug training quickly.
struct TrainingReport {
    std::uint64_t lines_read = 0;
    std::uint64_t transitions_counted = 0;
    std::uint64_t unknown_states_skipped = 0;
    std::uint64_t entities_seen = 0;
};

struct TrainingOptions {
    // Laplace smoothing: probability uses (count + alpha).
    // alpha = 0.0 means no smoothing.
    double alpha = 0.0;

    // If true and a row has no outgoing transitions, use uniform probabilities.
    bool uniform_if_dead_end = true;

};

class MarkovTrainer {
public:
    // Trains a Markov model from a dataset.
    // dataset_path: path to credit-card.dat
    static MarkovModel train_from_dataset(
            const std::string& dataset_path,
            const TrainingOptions& opt,
            TrainingReport* out_report = nullptr
    );
};
