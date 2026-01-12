//
// Created by Carlos Villacañas Iglesias.
//

#pragma once

#include <string>
#include <vector>
#include <cstddef>

struct RawRecord {
    std::size_t key;      // numeric key derived from entity_id
    std::string record;   // everything after the first comma
};

class DatasetReader {
public:
    // Reads credit-card.dat:
    // line = "<entity_id>,<record...>"
    // key is assigned by mapping entity_id -> 0...N-1
    static std::vector<RawRecord> read_credit_card_dataset(const std::string& path);
};
