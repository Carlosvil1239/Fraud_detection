//
// Created by Carlos Villacañas.
//

#include "../../include/training/MarkovTrainer.hpp"
#include "../../include/training/sequential/MarkovTrainerSequential.hpp"
#include "../../include/training/parallel/MarkovTrainerTBB.hpp"

#include <stdexcept>

namespace fd::training {

    // Calls the concrete trainer selected in the training options.
    fd::model::MarkovModel train_from_dataset(
            std::string_view dataset_path,
            int state_position,
            const TrainingOptions& opt,
            TrainingReport* out_report
    ) {
        switch (opt.backend) {
            case Backend::sequential:
                return fd::training::sequential::train_from_dataset(
                        dataset_path, state_position, opt, out_report
                );

            case Backend::tbb:
                return fd::training::parallel::train_from_dataset_tbb(
                        dataset_path, state_position, opt, out_report
                );

            default:
                throw std::runtime_error("Unknown training backend.");
        }
    }
}
