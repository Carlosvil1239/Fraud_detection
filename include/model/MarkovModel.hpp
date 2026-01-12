//
// Created by Carlos Villacañas Iglesias.
//

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

class MarkovModel {
public:
    MarkovModel() = default;

    MarkovModel(std::vector<std::string> states, std::vector<double> probsRowMajor)
            : states_(std::move(states)), p_(std::move(probsRowMajor))
    {
        const std::size_t n = states_.size();
        if (p_.size() != n * n) {
            throw std::runtime_error("MarkovModel: probability matrix has wrong size.");
        }
        index_.reserve(n);
        for (std::size_t i = 0; i < n; ++i) {
            index_[states_[i]] = i;
        }
    }

    std::size_t size() const { return states_.size(); }

    const std::vector<std::string>& states() const { return states_; }



    // Returns pointer to index if exists, nullptr otherwise.
    const std::size_t* stateIndexPtr(const std::string& s) const {
        auto it = index_.find(s);
        if (it == index_.end()) return nullptr;
        return &it->second;
    }

    // Row-major access: P(i,j)
    double prob(std::size_t i, std::size_t j) const {
        const std::size_t n = size();
        return p_[i * n + j];
    }

private:
    std::vector<std::string> states_;
    std::vector<double> p_; // row-major n*n
    std::unordered_map<std::string, std::size_t> index_;
};
