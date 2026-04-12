//
// Created by Carlos Villacañas.
//

#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>

#include "Event.hpp"
#include "../util/Stats.hpp"

namespace fd::pipeline {

// Receives detected outliers and stores their latency values.
class Sink {
public:
    Sink() = default;

    void consume(const Event& e);

    [[nodiscard]] std::size_t outliers() const { return outliers_; }
    [[nodiscard]] fd::util::LatencyStats final_stats_ms() const;

private:
    std::size_t outliers_ = 0;
    std::vector<std::uint64_t> latencies_;
};

}
