//
// Created by Carlos Villacañas.
//
#include "../../include/io/ModelReader.hpp"

#include <fstream>
#include <stdexcept>
#include <vector>
#include <string>

static std::vector<std::string> split_csv_simple(const std::string& line) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : line) {
        if (c == ',') {
            out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    out.push_back(cur);
    return out;
}

namespace fd::io {

fd::model::MarkovModel read_model_txt(std::string_view path) {
    std::ifstream in{std::string(path)};
    if (!in) throw std::runtime_error(std::string("Cannot open model file: ") + std::string(path));

    std::string first;
    if (!std::getline(in, first) || first.empty())
        throw std::runtime_error(std::string("Model file has empty first line: ") + std::string(path));

    std::vector<std::string> states = split_csv_simple(first);
    const std::size_t n = states.size();
    if (n == 0) throw std::runtime_error(std::string("No states found in model file: ") + std::string(path));

    std::vector<double> probs;
    probs.reserve(n * n);

    std::string line;
    std::size_t rows = 0;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto parts = split_csv_simple(line);
        if (parts.size() != n) {
            throw std::runtime_error("Model row has wrong number of columns.");
        }
        for (const auto& s : parts) {
            probs.push_back(std::stod(s));
        }
        rows++;
    }

    if (rows != n) {
        throw std::runtime_error("Model file has wrong number of rows (expected n).");
    }

    return {std::move(states), std::move(probs)};
}

}