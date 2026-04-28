//
// Created by Carlos Villacañas.
//

#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

#include "DatasetReader.hpp"

namespace fd::io {



    // Reads the dataset and sends each record to a partition chosen from its key.
    std::vector<std::vector<RawRecord>> read_credit_card_dataset_partitioned(
            std::string_view path,
            std::size_t partitions);

}
