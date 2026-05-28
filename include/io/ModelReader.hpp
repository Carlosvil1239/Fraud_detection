//
// Created by Carlos Villacañas.
//

#pragma once

#include <string_view>

#include "../training/MarkovModel.hpp"

namespace fd::io {

// Loads a Markov model from the text format used by this project.
fd::model::MarkovModel read_model_txt(std::string_view path);

}
