//
// Created by Carlos Villacañas.
//

#pragma once

#include <ostream>
#include <iomanip>
#include "../training/MarkovModel.hpp"

// Writes a Markov model in the CSV-like text format used by model.txt.
// The first line stores states and the next lines store probability rows.
namespace fd::io {

inline void write_model_txt(std::ostream& out, const fd::model::MarkovModel& model) {
    const auto& states = model.states();
    const std::size_t n = model.size();

    for (std::size_t i = 0; i < n; ++i) {
        out << states[i];
        if (i + 1 < n) out << ",";
    }
    out << "\n";

    // Keep enough precision so the model can be read again with little loss.
    out << std::setprecision(17);
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            out << model.prob(i, j);
            if (j + 1 < n) out << ",";
        }
        out << "\n";
    }
}

}
