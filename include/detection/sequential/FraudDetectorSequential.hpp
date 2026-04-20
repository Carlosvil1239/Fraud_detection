//
// Created by Carlos Villacañas.
//

#pragma once

#include <string_view>

#include "../../model/MarkovModel.hpp"
#include "../FraudDetector.hpp"

namespace fd::detection::sequential {

    // Runs detection with the sequential backend.
    fd::detection::DetectionResult detect_from_dataset(
            std::string_view dataset_path,
            const fd::model::MarkovModel& model,
            const fd::detection::DetectionOptions& opt);

}