//
// Created by Carlos Villacañas.
//

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace fd::io {

struct RawRecord {
    std::size_t key;
    std::string record;
};

std::vector<RawRecord> read_credit_card_dataset(std::string_view path);

}
