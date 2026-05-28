//
// Created by Carlos Villacañas.
//

#include "stats/Stats.hpp"

#include <algorithm>
#include <numeric>
#include <cmath>

// Reads a percentile from an already sorted nanosecond vector and returns ms.
static double percentile_sorted_ms(const std::vector<std::uint64_t>& sorted_ns, double p) {
    if (sorted_ns.empty()) return 0.0;

    // Use the nearest index in the sorted vector.
    const auto last_index = static_cast<double>(sorted_ns.size() - 1);
    const double rank = (p / 100.0) * last_index;
    const auto idx = static_cast<std::size_t>(std::lround(rank));

    const std::uint64_t ns = sorted_ns[idx];
    return static_cast<double>(ns) / 1e6;
}
namespace fd::util {

// Sorts the samples and computes mean, p50, p95 and p99 in milliseconds.
LatencyStats compute_latency_stats_ms(std::vector<std::uint64_t> latencies_ns) {
    LatencyStats s;
    if (latencies_ns.empty()) return s;

    const double sum = std::accumulate(latencies_ns.begin(), latencies_ns.end(), 0.0);
    s.mean_ms = (sum / static_cast<double>(latencies_ns.size())) / 1e6;

    std::ranges::sort(latencies_ns);
    s.p50_ms = percentile_sorted_ms(latencies_ns, 50.0);
    s.p95_ms = percentile_sorted_ms(latencies_ns, 95.0);
    s.p99_ms = percentile_sorted_ms(latencies_ns, 99.0);
    return s;
}

}
