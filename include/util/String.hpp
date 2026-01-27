//
// Created by Carlos Villacañas.
//

#pragma once
#include <string>
#include <cctype>

namespace fd::util {

inline std::string trim_copy(std::string s) {
    auto is_space = [](unsigned char c) {
        return std::isspace(c) || c == '\r' || c == '\n';
    };

    std::size_t start = 0;
    while (start < s.size() && is_space(static_cast<unsigned char>(s[start]))) start++;

    std::size_t end = s.size();
    while (end > start && is_space(static_cast<unsigned char>(s[end - 1]))) end--;

    return s.substr(start, end - start);
}

}
