//
// Created by Carlos Villacañas Iglesias.
//
#include "../../include/model/OutlierScorer.hpp"

#include <algorithm>

double OutlierScorer::score_sequence(const MarkovModel& model,
                                     const std::vector<std::size_t>& s) {
    if (s.size() < 2) return 0.0;

    const std::size_t n = model.size();
    double acc = 0.0;
    std::size_t steps = 0;

    for (std::size_t i = 1; i < s.size(); ++i) {
        const std::size_t a = s[i - 1];
        const std::size_t b = s[i];

        if (a >= n || b >= n) continue;

        const double p = model.prob(a, b);
        acc += (1.0 - p);
        steps++;
    }

    if (steps == 0) return 0.0;
    return acc / static_cast<double>(steps);
}
