//
// Created by Carlos Villacañas.
//

#include "../../../include/detection/parallel/FraudDetectorTBB.hpp"

#include "../../../include/io/PartitionedDatasetReader.hpp"
#include "../../../include/pipeline/Event.hpp"
#include "../../../include/pipeline/MarkovPredictor.hpp"
#include "../../../include/util/Stats.hpp"
#include "../../../include/util/Time.hpp"

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/info.h>
#include <oneapi/tbb/parallel_for.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <string_view>
#include <utility>
#include <vector>

namespace {

struct DetectionAccumulator {
    std::uint64_t total_events = 0;
    std::size_t outliers = 0;
    fd::pipeline::PredictorStats predictor_stats;
    std::vector<std::uint64_t> latencies_ns;

    void merge_from(DetectionAccumulator&& other) {
        total_events += other.total_events;
        outliers += other.outliers;

        predictor_stats.events += other.predictor_stats.events;
        predictor_stats.windows_full += other.predictor_stats.windows_full;
        predictor_stats.scored += other.predictor_stats.scored;
        predictor_stats.state_fail += other.predictor_stats.state_fail;
        predictor_stats.sum_score += other.predictor_stats.sum_score;
        predictor_stats.max_score = std::max(
                predictor_stats.max_score,
                other.predictor_stats.max_score
        );

        latencies_ns.insert(
                latencies_ns.end(),
                std::make_move_iterator(other.latencies_ns.begin()),
                std::make_move_iterator(other.latencies_ns.end())
        );
    }
};

// Processes one partition while keeping the local order of its keys.
DetectionAccumulator run_partition(
        std::vector<fd::io::RawRecord>& records,
        const fd::model::MarkovModel& model,
        const fd::detection::DetectionOptions& opt
) {
    fd::pipeline::MarkovPredictor predictor(
            model,
            opt.window,
            opt.state_position,
            opt.score_type,
            opt.threshold
    );

    DetectionAccumulator acc;

    for (auto& record : records) {
        fd::pipeline::Event event;
        event.key = record.key;
        event.record = std::move(record.record);
        event.ts_ns = fd::util::now_ns();

        acc.total_events++;

        if (predictor.process(event)) {
            acc.outliers++;
            acc.latencies_ns.push_back(fd::util::now_ns() - event.ts_ns);
        }
    }

    acc.predictor_stats = predictor.stats();
    return acc;
}

}

namespace fd::detection::parallel {

// Runs detection with partitioned parsing and parallel partition processing.
fd::detection::DetectionResult detect_from_dataset_tbb(
        std::string_view dataset_path,
        const fd::model::MarkovModel& model,
        const fd::detection::DetectionOptions& opt
) {
    const std::size_t partitions = std::max<std::size_t>(
            1,
            oneapi::tbb::info::default_concurrency()
    );

    auto partitioned_records =
            fd::io::read_credit_card_dataset_partitioned(dataset_path, partitions);

    std::vector<DetectionAccumulator> partials(partitioned_records.size());

    oneapi::tbb::parallel_for(
            oneapi::tbb::blocked_range<std::size_t>(0, partitioned_records.size(), 1),
            [&](const oneapi::tbb::blocked_range<std::size_t>& range) {
                for (std::size_t i = range.begin(); i != range.end(); ++i) {
                    partials[i] = run_partition(partitioned_records[i], model, opt);
                }
            }
    );

    DetectionAccumulator accumulated;
    for (auto& partial : partials) {
        accumulated.merge_from(std::move(partial));
    }

    fd::detection::DetectionResult result;
    result.total_events = accumulated.total_events;
    result.outliers = accumulated.outliers;
    result.latency_ms =
            fd::util::compute_latency_stats_ms(std::move(accumulated.latencies_ns));
    result.predictor_stats = accumulated.predictor_stats;
    return result;
}

}