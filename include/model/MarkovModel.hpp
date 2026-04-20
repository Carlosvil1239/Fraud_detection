//
// Created by Carlos Villacañas.
//

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <stdexcept>

namespace fd::model {

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
    bool operator()(std::string_view a, std::string_view b) const noexcept {
        return a == b;
    }
};

// Stores the states and transition probability matrix of the Markov model.
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

    // Returns the index of a state, throws if not found.
    std::size_t state_index_or_throw(std::string_view s) const {
        auto it = index_.find(s);
        if (it == index_.end()) {
            throw std::runtime_error(std::string("Unknown state: ") + std::string(s));
        }
        return it->second;
    }

    std::optional<std::size_t> state_index(std::string_view s) const {
        auto it = index_.find(s);
        if (it == index_.end()) return std::nullopt;
        return it->second;
    }

    // Returns the probability of moving from state i to state j.
    double prob(std::size_t i, std::size_t j) const {
        const std::size_t n = size();
        return p_[i * n + j];
    }

private:
    std::vector<std::string> states_;
    std::vector<double> p_; // row-major matrix
    std::unordered_map<std::string, std::size_t, TransparentStringHash, TransparentStringEq> index_;
};

}
