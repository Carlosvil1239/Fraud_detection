//
// Created by Carlos Villacañas.
//

#pragma once

#include <cstdint>
#include <string_view>

#include "../model/MarkovModel.hpp"
#include "../model/OutlierScorer.hpp"
#include "../pipeline/MarkovPredictor.hpp"
#include "../util/Stats.hpp"

namespace fd::detection {

    // Selects the detection implementation used from the command line.
    enum class Backend {
        sequential,
        tbb
    };

    // Stores the parameters used during detection.
    struct DetectionOptions {
        std::size_t window = 5;
        double threshold = 0.96;
        fd::model::ScoreType score_type = fd::model::ScoreType::MISS_PROBABILITY;
        int state_position = 1;
        Backend backend = Backend::sequential;
    };

    // Stores the values printed at the end of a detection run.
    struct DetectionResult {
        std::uint64_t total_events = 0;
        std::size_t outliers = 0;
        fd::util::LatencyStats latency_ms;
        fd::pipeline::PredictorStats predictor_stats;
    };

    // Dispatches detection to the selected backend.
    DetectionResult detect_from_dataset(
            std::string_view dataset_path,
            const fd::model::MarkovModel& model,
            const DetectionOptions& opt);

}