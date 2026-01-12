//
// Created by Carlos on 15/12/2025.
//

#include "../../include/util/Stats.hpp"

#include <algorithm>
#include <numeric>

static double percentile_sorted_ms(const std::vector<std::uint64_t>& sorted_ns, double p) {
    if (sorted_ns.empty()) return 0.0;
    // nearest-rank
    const double rank = (p / 100.0) * (sorted_ns.size() - 1);
    const std::size_t idx = static_cast<std::size_t>(rank + 0.5);
    const std::uint64_t ns = sorted_ns[std::min(idx, sorted_ns.size() - 1)];
    return static_cast<double>(ns) / 1e6;
}

LatencyStats compute_latency_stats_ms(std::vector<std::uint64_t> latencies_ns) {
    LatencyStats s;
    if (latencies_ns.empty()) return s;

    const double sum = std::accumulate(latencies_ns.begin(), latencies_ns.end(), 0.0);
    s.mean_ms = (sum / static_cast<double>(latencies_ns.size())) / 1e6;

    std::sort(latencies_ns.begin(), latencies_ns.end());
    s.p50_ms = percentile_sorted_ms(latencies_ns, 50.0);
    s.p95_ms = percentile_sorted_ms(latencies_ns, 95.0);
    s.p99_ms = percentile_sorted_ms(latencies_ns, 99.0);
    return s;
}
