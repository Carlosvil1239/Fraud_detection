//
// Created by Carlos Villacañas.
//

#pragma once

#include <cstddef>
#include <vector>

#include "../training/MarkovModel.hpp"

namespace fd::model {

// Chooses how the anomaly score is calculated from the transitions.
enum class ScoreType {
    MISS_PROBABILITY,
    MISS_RATE
};

// Scores a sequence of states using the probabilities stored in the model.
double score_sequence(const MarkovModel& model, const std::vector<std::size_t>& state_idx, ScoreType type);

}
