//
// Created by Carlos Villacañas Iglesias.
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

MarkovModel ModelReader::read_model_txt(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open model file: " + path);

    std::string first;
    if (!std::getline(in, first) || first.empty())
        throw std::runtime_error("Model file has empty first line: " + path);

    std::vector<std::string> states = split_csv_simple(first);
    const std::size_t n = states.size();
    if (n == 0) throw std::runtime_error("No states found in model file: " + path);

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

    return MarkovModel(std::move(states), std::move(probs));
}
