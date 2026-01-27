//
// Created by Carlos Villacañas.
//

#pragma once

#include <string_view>

#include "../model/MarkovModel.hpp"

namespace fd::io {

fd::model::MarkovModel read_model_txt(std::string_view path);

}
