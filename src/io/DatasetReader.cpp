//
// Created by Carlos Villacañas Iglesias.
//
#include "../../include/io/DatasetReader.hpp"
#include "../../include/util/String.hpp"

#include <fstream>
#include <unordered_map>
#include <stdexcept>

std::vector<RawRecord> DatasetReader::read_credit_card_dataset(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open dataset file: " + path);

    std::vector<RawRecord> out;
    out.reserve(200000);

    std::unordered_map<std::string, std::size_t> entity_to_key;
    entity_to_key.reserve(1 << 16);

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;

        const std::size_t first_comma = line.find(',');
        if (first_comma == std::string::npos) continue;

        const std::string entity_id = trim_copy(line.substr(0, first_comma));
        const std::string record    = line.substr(first_comma + 1);

        std::size_t key;
        auto it = entity_to_key.find(entity_id);
        if (it == entity_to_key.end()) {
            key = entity_to_key.size();
            entity_to_key.emplace(entity_id, key);
        } else {
            key = it->second;
        }

        out.push_back(RawRecord{key, record});
    }

    if (out.empty()) throw std::runtime_error("Dataset is empty or malformed: " + path);
    return out;
}
