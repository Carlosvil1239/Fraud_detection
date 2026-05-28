//
// Created by Carlos Villacañas.
//

#include "inference/MarkovPredictor.hpp"
#include "training/StateExtractor.hpp" // reuse the training parser

namespace fd::pipeline {

// Stores the model and parameters needed for online scoring.
MarkovPredictor::MarkovPredictor(const fd::model::MarkovModel& model,
                                 std::size_t window_size,
                                 int state_position,
                                 fd::model::ScoreType score_type,
                                 double threshold)
        : model_(model),
          W_(window_size),
          statepos_(state_position),
          type_(score_type),
          threshold_(threshold)
{
    windows_.reserve(1 << 16);
}

// Converts the records in a window into state indexes from the model.
bool MarkovPredictor::build_state_sequence(const std::deque<std::string>& recs,
                                           std::vector<std::size_t>& out_idx) const {
    out_idx.clear();
    out_idx.reserve(recs.size());

    for (const auto& r : recs) {
        const std::string st = fd::training::extract_state_from_record(r, statepos_);
        if (st.empty()) return false;

        const auto idx = model_.state_index(st);
        if (!idx) return false;

        out_idx.push_back(*idx);
    }
    return out_idx.size() >= 2;
}

// Updates the entity window and returns true when the score passes the threshold.
bool MarkovPredictor::process(Event& e) {
    stats_.events++;

    auto& dq = windows_[e.key];
    dq.push_back(e.record);
    if (dq.size() > W_) dq.pop_front();

    if (dq.size() < W_) return false;

    stats_.windows_full++;

    std::vector<std::size_t> seq;
    if (!build_state_sequence(dq, seq)) {
        stats_.state_fail++;
        return false;
    }

    e.score = fd::model::score_sequence(model_, seq, type_);
    stats_.scored++;
    stats_.sum_score += e.score;
    if (e.score > stats_.max_score) stats_.max_score = e.score;

    return e.score > threshold_;
}

}
