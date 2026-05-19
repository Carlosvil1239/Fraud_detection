//
// Created by Carlos Villacañas.
//

#pragma once

#include <vector>
#include <cstdint>

namespace fd::util {

// Stores latency summary values in milliseconds.
struct LatencyStats {
    double mean_ms = 0.0;
    double p50_ms = 0.0;
    double p95_ms = 0.0;
    double p99_ms = 0.0;
};

// Computes latency statistics from nanosecond values.
LatencyStats compute_latency_stats_ms(std::vector<std::uint64_t> latencies_ns);

}
