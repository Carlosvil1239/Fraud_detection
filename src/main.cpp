//
// Created by Carlos Villacañas Iglesias.
//
#include <iostream>
#include <string>
#include <stdexcept>

#include "io/ModelReader.hpp"
#include "io/DatasetReader.hpp"
#include "pipeline/Event.hpp"
#include "pipeline/MarkovPredictor.hpp"
#include "pipeline/Sink.hpp"
#include "util/Time.hpp"

struct AppConfig {
    std::string input_path = "credit-card.dat";
    std::string model_path = "model.txt";

    std::size_t window = 5;
    double threshold = 0.96;
};

static void usage(const char* prog) {
    std::cerr
            << "Usage:\n"
            << "  " << prog << " --input <credit-card.dat> --model <model.txt>\n"
            << "           [--window 5] [--statepos 1] [--threshold 0.96]\n"
            << "           [--score miss_prob|miss_rate]\n";
}

static AppConfig parse_args(int argc, char** argv) {
    AppConfig c;

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];

        auto need = [&](const std::string& flag) -> std::string {
            if (i + 1 >= argc) throw std::runtime_error("Missing value for " + flag);
            return argv[++i];
        };

        if (a == "--help" || a == "-h") {
            usage(argv[0]);
            std::exit(0);
        } else if (a == "--input") {
            c.input_path = need(a);
        } else if (a == "--model") {
            c.model_path = need(a);
        } else if (a == "--window") {
            c.window = static_cast<std::size_t>(std::stoull(need(a)));

        } else if (a == "--threshold") {
            c.threshold = std::stod(need(a));
        } else {
            throw std::runtime_error("Unknown argument: " + a);
        }
    }

    return c;
}

int main(int argc, char** argv) {
    try {
        const AppConfig cfg = parse_args(argc, argv);

        MarkovModel model = ModelReader::read_model_txt(cfg.model_path);
        auto data = DatasetReader::read_credit_card_dataset(cfg.input_path);


        MarkovPredictor predictor(model, cfg.window, cfg.threshold);

        Sink sink;

        std::size_t total = 0;

        for (const auto& r : data) {
            Event e;
            e.key = r.key;
            e.record = r.record;
            e.ts_ns = now_ns();

            total++;
            if (predictor.process(e)) {
                sink.consume(e);
            }
        }

        const auto st = sink.final_stats_ms();

        std::cout << "Done.\n";
        std::cout << "Total events: " << total << "\n";
        std::cout << "Outliers: " << sink.outliers() << "\n";
        std::cout << "Latency mean (ms): " << st.mean_ms << "\n";
        std::cout << "Latency p50  (ms): " << st.p50_ms << "\n";
        std::cout << "Latency p95  (ms): " << st.p95_ms << "\n";
        std::cout << "Latency p99  (ms): " << st.p99_ms << "\n";
        const auto& ps = predictor.stats();
        std::cout << "Predictor stats:\n";
        std::cout << "  events: " << ps.events << "\n";
        std::cout << "  windows_full: " << ps.windows_full << "\n";
        std::cout << "  scored: " << ps.scored << "\n";
        std::cout << "  state_fail: " << ps.state_fail << "\n";
        if (ps.scored > 0) {
            std::cout << "  mean_score: " << (ps.sum_score / ps.scored) << "\n";
            std::cout << "  max_score: " << ps.max_score << "\n";
        }


        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        usage(argv[0]);
        return 1;
    }
}
