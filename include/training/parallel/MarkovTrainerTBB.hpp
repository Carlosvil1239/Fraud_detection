//
// Created by Carlos Villacañas.
//

#pragma once

#include <string_view>

#include "../../model/MarkovModel.hpp"
#include "../MarkovTrainer.hpp"

namespace fd::training::parallel {

    // Trains the Markov model using the parallel TBB backend.
    fd::model::MarkovModel train_from_dataset_tbb(
            std::string_view dataset_path,
            int state_position,
            const fd::training::TrainingOptions& opt,
            fd::training::TrainingReport* out_report = nullptr);

}
