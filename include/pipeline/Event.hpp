//
// Created by Carlos Villacañas.
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace fd::pipeline {

// Holds one dataset record while it goes through the detection pipeline.
struct Event {
    std::size_t key = 0;
    std::string record;
    std::uint64_t ts_ns = 0; // timestamp assigned before processing
    double score = 0.0;      // score set by the predictor
};

}
