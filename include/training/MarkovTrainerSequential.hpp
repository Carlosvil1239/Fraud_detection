//
// Created by Carlos Villacañas.
//

#pragma once

#include <string_view>

#include "MarkovModel.hpp"
#include "MarkovTrainer.hpp"

namespace fd::training::sequential {

    // Trains the Markov model using the sequential backend.
    fd::model::MarkovModel train_from_dataset(
            std::string_view dataset_path,
            int state_position,
            const fd::training::TrainingOptions& opt,
            fd::training::TrainingReport* out_report = nullptr);

}
