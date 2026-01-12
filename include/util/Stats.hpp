//
// Created by Carlos Villacañas Iglesias.
//

#pragma once

#include <vector>
#include <cstdint>

struct LatencyStats {
    double mean_ms = 0.0;
    double p50_ms = 0.0;
    double p95_ms = 0.0;
    double p99_ms = 0.0;
};

// Computes stats from latencies (ns). It sorts internally.
LatencyStats compute_latency_stats_ms(std::vector<std::uint64_t> latencies_ns);
