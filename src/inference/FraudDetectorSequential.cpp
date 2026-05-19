//
// Created by Carlos Villacañas.
//

#include "inference/FraudDetectorSequential.hpp"

#include "io/DatasetReader.hpp"
#include "inference/Event.hpp"
#include "inference/MarkovPredictor.hpp"
#include "stats/Stats.hpp"
#include "stats/Time.hpp"

#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

namespace fd::detection::sequential {

// Runs one sequential detection pass and builds the final statistics.
fd::detection::DetectionResult detect_from_dataset(
        std::string_view dataset_path,
        const fd::model::MarkovModel& model,
        const fd::detection::DetectionOptions& opt
) {
    auto records = fd::io::read_credit_card_dataset(dataset_path);

    fd::pipeline::MarkovPredictor predictor(
            model,
            opt.window,
            opt.state_position,
            opt.score_type,
            opt.threshold
    );

    std::size_t outliers = 0;
    std::vector<std::uint64_t> latencies_ns;
    std::uint64_t total_events = 0;

    for (auto&[key, record] : records) {
        fd::pipeline::Event event;
        event.key = key;
        event.record = std::move(record);
        event.ts_ns = fd::util::now_ns();

        total_events++;

        if (predictor.process(event)) {
            outliers++;
            latencies_ns.push_back(fd::util::now_ns() - event.ts_ns);
        }
    }

    fd::detection::DetectionResult result;
    result.total_events = total_events;
    result.outliers = outliers;
    result.latency_ms = fd::util::compute_latency_stats_ms(std::move(latencies_ns));
    result.predictor_stats = predictor.stats();
    return result;
}

}