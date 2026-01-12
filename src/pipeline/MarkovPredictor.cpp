//
// Created by Carlos Villacañas Iglesias.
//
#include "../../include/pipeline/MarkovPredictor.hpp"


MarkovPredictor::MarkovPredictor(const MarkovModel& model,
                                 std::size_t window_size,
                                 double threshold)
        : model_(model),
          W_(window_size),
          threshold_(threshold)
{
    windows_.reserve(1 << 16);
}

bool MarkovPredictor::build_state_sequence(const std::deque<std::string>& recs,
                                           std::vector<std::size_t>& out_idx) const {
    out_idx.clear();
    out_idx.reserve(recs.size());

    for (const auto& r : recs) {

        const std::size_t last_comma = r.rfind(',');
        if (last_comma == std::string::npos) return false;

        const std::string st = r.substr(last_comma + 1);
        if (st.empty()) return false;

        const std::size_t* p = model_.stateIndexPtr(st);
        if (!p) return false;

        out_idx.push_back(*p);
    }

    return out_idx.size() >= 2;
}


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

    e.score = OutlierScorer::score_sequence(model_, seq);
    stats_.scored++;
    stats_.sum_score += e.score;
    if (e.score > stats_.max_score) stats_.max_score = e.score;

    return e.score > threshold_;
}

