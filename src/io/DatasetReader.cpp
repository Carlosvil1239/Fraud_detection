//
// Created by Carlos Villacañas.
//

#include "../../include/io/DatasetReader.hpp"


#include <algorithm>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {

constexpr std::size_t kTargetBlockBytes = 8ULL * 1024ULL * 1024ULL; // 8 MB
constexpr std::size_t kOverflowReadBytes = 64ULL * 1024ULL;         // 64 KB

// Reads a block and keeps the last unfinished line for the next read.
bool read_next_block(
    std::ifstream& in,
    std::size_t target_bytes,
    std::string& carry,
    std::string& block
) {
    block = std::move(carry);
    carry.clear();

    if (!in && block.empty()) {
        return false;
    }

    std::string buffer(target_bytes, '\0');
    in.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    const std::streamsize first_read = in.gcount();
    if (first_read > 0) {
        block.append(buffer.data(), static_cast<std::size_t>(first_read));
    }

    if (block.empty()) {
        return false;
    }

    if (in.eof()) {
        return true;
    }

    std::size_t last_newline = block.find_last_of('\n');

    while (last_newline == std::string::npos && in) {
        std::string extra(kOverflowReadBytes, '\0');
        in.read(extra.data(), static_cast<std::streamsize>(extra.size()));
        const std::streamsize extra_read = in.gcount();
        if (extra_read <= 0) {
            break;
        }

        block.append(extra.data(), static_cast<std::size_t>(extra_read));

        if (in.eof()) {
            return true;
        }

        last_newline = block.find_last_of('\n');
    }

    if (in.eof()) {
        return true;
    }

    if (last_newline != std::string::npos && last_newline + 1 < block.size()) {
        carry.assign(block.data() + last_newline + 1, block.size() - (last_newline + 1));
        block.resize(last_newline + 1);
    }

    return !block.empty();
}

// Removes spaces from both sides without copying the record text.
std::string_view trim_view(std::string_view sv) {
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

    return sv.substr(begin, end - begin);
}

// Splits a memory block into dataset records and sends them to the callback.
template <typename Fn>
void for_each_record_in_buffer(const std::string& buffer, Fn&& fn) {
    const char* data = buffer.data();
    const std::size_t size = buffer.size();

    std::size_t pos = 0;
    while (pos < size) {
        const std::size_t line_begin = pos;

        while (pos < size && data[pos] != '\n') {
            ++pos;
        }

        std::size_t line_end = pos;
        if (pos < size && data[pos] == '\n') {
            ++pos;
        }

        if (line_begin == line_end) {
            continue;
        }

        if (data[line_end - 1] == '\r') {
            --line_end;
        }

        if (line_begin == line_end) {
            continue;
        }

        const char* comma = std::find(data + line_begin, data + line_end, ',');
        if (comma == data + line_end) {
            continue;
        }

        const std::size_t comma_pos = static_cast<std::size_t>(comma - data);
        const std::string_view entity_id =
            trim_view(std::string_view{data + line_begin, comma_pos - line_begin});
        const std::string_view record{
            data + comma_pos + 1,
            line_end - (comma_pos + 1)
        };

        fn(entity_id, record);
    }
}

}

namespace fd::io {

// Reads the dataset and assigns a compact numeric key to each entity id.
std::vector<RawRecord> read_credit_card_dataset(std::string_view path) {
    std::ifstream in{std::string(path), std::ios::binary};
    if (!in) {
        throw std::runtime_error(std::string("Cannot open dataset file: ") + std::string(path));
    }

    std::vector<RawRecord> out;
    out.reserve(200000);

    std::unordered_map<std::string, std::size_t> entity_to_key;
    entity_to_key.reserve(1 << 16);

    std::string carry;

    while (true) {
        std::string block;
        if (!read_next_block(in, kTargetBlockBytes, carry, block)) {
            break;
        }

        for_each_record_in_buffer(block, [&](std::string_view entity_id, std::string_view record) {
            // Reuse the same key when the entity was already seen.
            const std::string entity_key(entity_id);

            std::size_t key;
            const auto it = entity_to_key.find(entity_key);
            if (it == entity_to_key.end()) {
                key = entity_to_key.size();
                entity_to_key.emplace(entity_key, key);
            } else {
                key = it->second;
            }

            out.push_back(RawRecord{key, std::string(record)});
        });
    }

    if (out.empty()) {
        throw std::runtime_error(std::string("Dataset is empty or malformed: ") + std::string(path));
    }

    return out;
}

}
