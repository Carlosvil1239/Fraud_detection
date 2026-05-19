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

// Selects how records are distributed between partitions.
enum class DatasetPartitionMode {
    by_order,
    by_entity
};

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
