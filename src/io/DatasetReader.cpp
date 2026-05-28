//
// Created by Carlos Villacañas.
//

#include "io/DatasetReader.hpp"

#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {

// Allows string and string_view keys to be searched in the same map.
struct TransparentStringHash {
    using is_transparent = void;

    std::size_t operator()(std::string_view sv) const noexcept {
        return std::hash<std::string_view>{}(sv);
    }

    std::size_t operator()(const std::string& s) const noexcept {
        return std::hash<std::string_view>{}(s);
    }
};

// Compares strings without forcing extra temporary strings.
struct TransparentStringEq {
    using is_transparent = void;

    bool operator()(const std::string_view a, const std::string_view b) const noexcept {
        return a == b;
    }
};

std::size_t get_or_create_key(
        std::unordered_map<std::string, std::size_t, TransparentStringHash, TransparentStringEq>& entity_to_key,
        const std::string_view entity_id
) {
    if (const auto it = entity_to_key.find(entity_id); it != entity_to_key.end()) {
        return it->second;
    }

    const std::size_t key = entity_to_key.size();
    entity_to_key.emplace(std::string(entity_id), key);
    return key;
}

bool has_any_record(const std::vector<std::vector<fd::io::RawRecord>>& partitions) {
    for (const auto& part : partitions) {
        if (!part.empty()) {
            return true;
        }
    }
    return false;
}

}

namespace fd::io {

// Reads the dataset and assigns a compact numeric key to each entity id.
std::vector<RawRecord> read_credit_card_dataset(const std::string_view path) {
    std::ifstream in{std::string(path), std::ios::binary};
    if (!in) {
        throw std::runtime_error(std::string("Cannot open dataset file: ") + std::string(path));
    }

    std::vector<RawRecord> out;
    out.reserve(200000);

    std::unordered_map<std::string, std::size_t, TransparentStringHash, TransparentStringEq> entity_to_key;
    entity_to_key.reserve(1 << 16);

    std::string carry;

    while (true) {
        std::string block;
        if (!read_next_block(in, kTargetBlockBytes, carry, block)) {
            break;
        }

        for_each_record_in_buffer(block, [&](const bool has_comma, std::string_view entity_id, const std::string_view record) {
            if (!has_comma) {
                return;
            }

            entity_id = trim_view(entity_id);
            const std::size_t key = get_or_create_key(entity_to_key, entity_id);
            out.push_back(RawRecord{key, std::string(record)});
        });
    }

    if (out.empty()) {
        throw std::runtime_error(std::string("Dataset is empty or malformed: ") + std::string(path));
    }

    return out;
}

// Reads the dataset and sends each record to one partition.
std::vector<std::vector<RawRecord>> read_credit_card_dataset_partitioned(
        std::string_view path,
        std::size_t partitions,
        DatasetPartitionMode mode
) {
    if (partitions == 0) {
        throw std::runtime_error("Partition count must be greater than zero.");
    }

    std::ifstream in{std::string(path), std::ios::binary};
    if (!in) {
        throw std::runtime_error(std::string("Cannot open dataset file: ") + std::string(path));
    }

    std::vector<std::vector<RawRecord>> out(partitions);
    std::unordered_map<std::string, std::size_t, TransparentStringHash, TransparentStringEq> entity_to_key;
    entity_to_key.reserve(1 << 16);

    std::size_t record_index = 0;
    std::string carry;

    while (true) {
        std::string block;
        if (!read_next_block(in, kTargetBlockBytes, carry, block)) {
            break;
        }

        for_each_record_in_buffer(block, [&](const bool has_comma, std::string_view entity_id, const std::string_view record) {
            if (!has_comma) {
                return;
            }

            entity_id = trim_view(entity_id);
            const std::size_t key = get_or_create_key(entity_to_key, entity_id);
            const std::size_t partition =
                    mode == DatasetPartitionMode::by_entity ? key % partitions : record_index % partitions;

            out[partition].push_back(RawRecord{key, std::string(record)});
            ++record_index;
        });
    }

    if (!has_any_record(out)) {
        throw std::runtime_error(std::string("Dataset is empty or malformed: ") + std::string(path));
    }

    return out;
}

// Reads the dataset and keeps records from the same entity in the same partition.
std::vector<std::vector<RawRecord>> read_credit_card_dataset_partitioned_by_id(
        std::string_view path,
        std::size_t partitions
) {
    return read_credit_card_dataset_partitioned(path, partitions, DatasetPartitionMode::by_entity);
}

}
