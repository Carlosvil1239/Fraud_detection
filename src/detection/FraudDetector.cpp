//
// Created by Carlos Villacañas.
//

#include "../../include/detection/FraudDetector.hpp"
#include "../../include/detection/parallel/FraudDetectorTBB.hpp"
#include "../../include/detection/sequential/FraudDetectorSequential.hpp"

#include <stdexcept>

namespace fd::detection {

    // Calls the concrete detector selected in the detection options.
    DetectionResult detect_from_dataset(
            std::string_view dataset_path,
            const fd::model::MarkovModel& model,
            const DetectionOptions& opt
    ) {
        switch (opt.backend) {
            case Backend::sequential:
                return fd::detection::sequential::detect_from_dataset(
                        dataset_path, model, opt
                );

            case Backend::tbb:
                return fd::detection::parallel::detect_from_dataset_tbb(
                        dataset_path, model, opt
                );

            default:
                throw std::runtime_error("Unknown detection backend.");
        }
    }

}