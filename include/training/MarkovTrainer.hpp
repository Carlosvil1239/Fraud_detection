//
// // Created by Carlos Villacañas..
//

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "../model/MarkovModel.hpp"

namespace fd::training {

struct TrainingReport {
    std::uint64_t lines_read = 0;
    std::uint64_t transitions_counted = 0;
    std::uint64_t unknown_states_skipped = 0;
    std::uint64_t entities_seen = 0;
};

struct TrainingOptions {
    double alpha = 0.0;
    bool uniform_if_dead_end = true;
};

fd::model::MarkovModel train_from_dataset(
        std::string_view dataset_path,
        int state_position,
        const TrainingOptions& opt,
        TrainingReport* out_report = nullptr);

}
