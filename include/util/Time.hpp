//
// Created by Carlos Villacañas Iglesias.
//

#pragma once

#include <cstdint>
#include <chrono>

inline std::uint64_t now_ns() {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}
