//
// Created by Carlos Villacañas.
//

#pragma once

#include <cstddef>
#include <cctype>
#include <string>
#include <string_view>

// Extracts the state token from the record string.
// state_position 0 reads the first token, and 1 reads the second token.
namespace fd::training {

    // Removes spaces from both ends of a string view.
    inline std::string trim_copy_view(const std::string_view sv) {
        std::size_t begin = 0;
        while (begin < sv.size() &&
               std::isspace(static_cast<unsigned char>(sv[begin])) != 0) {
            ++begin;
               }

        std::size_t end = sv.size();
        while (end > begin &&
               std::isspace(static_cast<unsigned char>(sv[end - 1])) != 0) {
            --end;
               }

        return std::string(sv.substr(begin, end - begin));
    }

    // Gets the selected comma-separated token from a record.
    inline std::string extract_state_from_record(std::string_view record, int state_position) {
        const std::size_t pos = record.find(',');
        if (pos == std::string_view::npos) {
            if (state_position == 0) {
                return trim_copy_view(record);
            }
            return {};
        }

        if (state_position == 0) {
            return trim_copy_view(record.substr(0, pos));
        }

        const std::size_t start = pos + 1;
        const std::size_t pos2 = record.find(',', start);

        if (pos2 == std::string_view::npos) {
            return trim_copy_view(record.substr(start));
        }

        return trim_copy_view(record.substr(start, pos2 - start));
    }

    // Overload used when the caller already has a string.
    inline std::string extract_state_from_record(const std::string& record, int state_position) {
        return extract_state_from_record(std::string_view(record), state_position);
    }

}
