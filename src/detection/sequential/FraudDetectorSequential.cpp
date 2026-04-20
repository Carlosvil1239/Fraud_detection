//
// Created by Carlos Villacañas.
//

#include "../../../include/detection/sequential/FraudDetectorSequential.hpp"

#include "../../../include/io/DatasetReader.hpp"
#include "../../../include/pipeline/Event.hpp"
#include "../../../include/pipeline/MarkovPredictor.hpp"
#include "../../../include/util/Stats.hpp"
#include "../../../include/util/Time.hpp"

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

    for (auto& record : records) {
        fd::pipeline::Event event;
        event.key = record.key;
        event.record = std::move(record.record);
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