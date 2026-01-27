//
// Created by Carlos Villacañas.
//

#pragma once

#include <string>
#include "../util/String.hpp"

// Extracts the state token from the record string.
// state_position = 0 -> token before first comma in record
// state_position = 1 -> token after first comma in record
namespace fd::training {

inline std::string extract_state_from_record(const std::string& record, int state_position) {
    const std::size_t pos = record.find(',');
    if (pos == std::string::npos) {
        if (state_position == 0) return fd::util::trim_copy(record);
        return {};
    }

    if (state_position == 0) {
        return fd::util::trim_copy(record.substr(0, pos));
    }

    const std::size_t start = pos + 1;
    const std::size_t pos2 = record.find(',', start);
    if (pos2 == std::string::npos) {
        return fd::util::trim_copy(record.substr(start));
    }
    return fd::util::trim_copy(record.substr(start, pos2 - start));
}

}
