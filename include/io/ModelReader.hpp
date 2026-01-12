//
// Created by Carlos Villacañas Iglesias.
//

#pragma once

#include <string>
#include "../model/MarkovModel.hpp"

class ModelReader {
public:
    // Reads the same format as model.txt:
    // first line: comma-separated state names
    // next lines: rows of probabilities (comma-separated)
    static MarkovModel read_model_txt(const std::string& path);
};
