//
// Created by Carlos Villacañas Iglesias.
//

#pragma once

#include <unordered_map>
#include <deque>
#include <vector>
#include <cstddef>

#include "../model/MarkovModel.hpp"
#include "../model/OutlierScorer.hpp"
#include "Event.hpp"

struct PredictorStats {
    std::uint64_t events = 0;
    std::uint64_t windows_full = 0;
    std::uint64_t scored = 0;
    std::uint64_t state_fail = 0;
    double max_score = 0.0;
    double sum_score = 0.0;
};

class MarkovPredictor {
public:
    MarkovPredictor(const MarkovModel& model,
                    std::size_t window_size,
                    double threshold);

    // Returns true if event is an outlier (and sets event.score).
    bool process(Event& e);
    const PredictorStats& stats() const { return stats_; }

private:
    const MarkovModel& model_;
    std::size_t W_;
    double threshold_;

    std::unordered_map<std::size_t, std::deque<std::string>> windows_;

    bool build_state_sequence(const std::deque<std::string>& recs,
                              std::vector<std::size_t>& out_idx) const;
    PredictorStats stats_;
};
