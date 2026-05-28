//
// Created by Carlos Villacañas.
//

#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace fd::io {

constexpr std::size_t kTargetBlockBytes = 8ULL * 1024ULL * 1024ULL; // 8 MB
constexpr std::size_t kOverflowReadBytes = 64ULL * 1024ULL;         // 64 KB

// Stores one input line with the numeric key assigned to its entity.
struct RawRecord {
    std::size_t key;
    std::string record;
};

// Selects how records are distributed between partitions.
enum class DatasetPartitionMode {
    by_order,
    by_entity
};

// Reads a complete block and moves an unfinished final line to carry.
inline bool read_next_block(
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
    if (const std::streamsize first_read = in.gcount(); first_read > 0) {
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
inline std::string_view trim_view(std::string_view sv) {
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

// Splits a block into records and reports whether each line has a comma.
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
            fn(false, std::string_view{}, std::string_view{});
            continue;
        }

        const auto comma_pos = static_cast<std::size_t>(comma - data);
        const std::string_view entity_id{data + line_begin, comma_pos - line_begin};
        const std::string_view record{data + comma_pos + 1, line_end - (comma_pos + 1)};
        fn(true, entity_id, record);
    }
}

// Reads the dataset and gives the same key to records from the same entity.
std::vector<RawRecord> read_credit_card_dataset(std::string_view path);

// Reads the dataset and sends each record to one partition.
std::vector<std::vector<RawRecord>> read_credit_card_dataset_partitioned(
        std::string_view path,
        std::size_t partitions,
        DatasetPartitionMode mode = DatasetPartitionMode::by_entity);

// Reads the dataset and keeps records from the same entity in the same partition.
std::vector<std::vector<RawRecord>> read_credit_card_dataset_partitioned_by_id(
        std::string_view path,
        std::size_t partitions);

}
