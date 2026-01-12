//
// Created by Carlos Villacañas Iglesias.
//

#pragma once

#include <vector>
#include <cstddef>
#include "MarkovModel.hpp"



class OutlierScorer {
public:
    static double score_sequence(
            const MarkovModel& model,
            const std::vector<std::size_t>& state_idx
    );
};
