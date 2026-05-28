//
// Created by Carlos Villacañas.
//

#pragma once

#include <unordered_map>
#include <deque>
#include <vector>
#include <cstddef>

#include "../training/MarkovModel.hpp"
#include "OutlierScorer.hpp"
#include "Event.hpp"

namespace fd::pipeline {

// Keeps counters that help check how the predictor behaved.
struct PredictorStats {
    std::uint64_t events = 0;
    std::uint64_t windows_full = 0;
    std::uint64_t scored = 0;
    std::uint64_t state_fail = 0;
    double max_score = 0.0;
    double sum_score = 0.0;
};

// Builds sliding windows per entity and marks records as outliers.
class MarkovPredictor {
public:
    MarkovPredictor(const fd::model::MarkovModel& model,
                    std::size_t window_size,
                    int state_position,
                    fd::model::ScoreType score_type,
                    double threshold);

    // Returns true if event is an outlier (and sets event.score).
    bool process(Event& e);
    const PredictorStats& stats() const { return stats_; }

private:
    const fd::model::MarkovModel& model_;
    std::size_t W_;
    int statepos_;
    fd::model::ScoreType type_;
    double threshold_;

    // Each entity has its own recent records window.
    std::unordered_map<std::size_t, std::deque<std::string>> windows_;

    bool build_state_sequence(const std::deque<std::string>& recs,
                              std::vector<std::size_t>& out_idx) const;
    PredictorStats stats_;
};

}
