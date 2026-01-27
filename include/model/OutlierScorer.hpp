//
// Created by Carlos Villacañas.
//

#pragma once

#include <cstddef>
#include <vector>

#include "MarkovModel.hpp"

namespace fd::model {

enum class ScoreType {
    MISS_PROBABILITY,
    MISS_RATE
};

double score_sequence(const MarkovModel& model, const std::vector<std::size_t>& state_idx, ScoreType type);

}
