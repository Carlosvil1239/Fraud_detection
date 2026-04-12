//
// Created by Carlos Villacañas.
//

#include "../../include/model/OutlierScorer.hpp"

#include <algorithm>

namespace fd::model {

// Returns a higher value when the sequence fits the model worse.
double score_sequence(const MarkovModel& model, const std::vector<std::size_t>& s, ScoreType type) {
    if (s.size() < 2) return 0.0;

    const std::size_t n = model.size();
    double acc = 0.0;
    std::size_t steps = 0;

    for (std::size_t i = 1; i < s.size(); ++i) {
        const std::size_t a = s[i - 1];
        const std::size_t b = s[i];

        if (a >= n || b >= n) continue;

        if (type == ScoreType::MISS_PROBABILITY) {
            const double p = model.prob(a, b);
            acc += (1.0 - p);
        } else {
            // MISS_RATE adds 1 when the transition is not the best one in the row.
            std::size_t best = 0;
            double bestp = model.prob(a, 0);
            for (std::size_t j = 1; j < n; ++j) {
                const double pj = model.prob(a, j);
                if (pj > bestp) {
                    bestp = pj;
                    best = j;
                }
            }
            acc += (b == best) ? 0.0 : 1.0;
        }

        steps++;
    }

    if (steps == 0) return 0.0;
    return acc / static_cast<double>(steps);
}

}
