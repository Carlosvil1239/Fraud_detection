//
// Created by Carlos Villacañas.
//

#pragma once

#include <cstdint>
#include <chrono>

namespace fd::util {

// Returns a monotonic timestamp in nanoseconds.
inline std::uint64_t now_ns() {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}

}
