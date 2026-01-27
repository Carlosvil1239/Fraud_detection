//
// Created by Carlos Villacañas.
//

#include "../../include/pipeline/Sink.hpp"
#include "../../include/util/Time.hpp"


namespace fd::pipeline {

void Sink::consume(const Event& e) {
    outliers_++;
    const std::uint64_t lat = fd::util::now_ns() - e.ts_ns;
    latencies_.push_back(lat);
}

fd::util::LatencyStats Sink::final_stats_ms() const {
    return fd::util::compute_latency_stats_ms(latencies_);
}

}
