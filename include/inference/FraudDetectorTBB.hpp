//
// Created by Carlos Villacañas.
//

#pragma once

#include <string_view>

#include "../training/MarkovModel.hpp"
#include "FraudDetector.hpp"

namespace fd::detection::parallel {

    // Runs detection with the TBB backend.
    fd::detection::DetectionResult detect_from_dataset_tbb(
            std::string_view dataset_path,
            const fd::model::MarkovModel& model,
            const fd::detection::DetectionOptions& opt);

}