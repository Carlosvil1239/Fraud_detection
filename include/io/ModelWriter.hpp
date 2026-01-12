//
// Created by Carlos Villacañas Iglesias.
//

#pragma once

#include <ostream>
#include <iomanip>
#include "../model/MarkovModel.hpp"

// Writes a Markov model in the same CSV format used by model.txt:
// first line: state names separated by commas
// next lines: rows of probabilities separated by commas
inline void write_model_txt(std::ostream& out, const MarkovModel& model) {
    const auto& states = model.states();
    const std::size_t n = model.size();

    for (std::size_t i = 0; i < n; ++i) {
        out << states[i];
        if (i + 1 < n) out << ",";
    }
    out << "\n";

    out << std::setprecision(17);
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            out << model.prob(i, j);
            if (j + 1 < n) out << ",";
        }
        out << "\n";
    }
}
