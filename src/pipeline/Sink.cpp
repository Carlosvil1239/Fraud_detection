//
// Created by Carlos Villacañas Iglesias.
//

#include "../../include/pipeline/Sink.hpp"
#include "../../include/util/Time.hpp"


void Sink::consume(const Event& e) {
    outliers_++;
    const std::uint64_t lat = now_ns() - e.ts_ns;
    latencies_.push_back(lat);
}

LatencyStats Sink::final_stats_ms() const {
    return compute_latency_stats_ms(latencies_);
}
