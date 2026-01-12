//
// Created by Carlos Villacañas Iglesias.
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

struct Event {
    std::size_t key = 0;
    std::string record;
    std::uint64_t ts_ns = 0; // timestamp assigned by Source
    double score = 0.0;      // filled by Predictor if outlier
};
