//
// Created by Carlos Villacañas.
//

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace fd::io {

// Stores one input line with the numeric key assigned to its entity.
struct RawRecord {
    std::size_t key;
    std::string record;
};

// Reads the dataset and gives the same key to records from the same entity.
std::vector<RawRecord> read_credit_card_dataset(std::string_view path);

}
