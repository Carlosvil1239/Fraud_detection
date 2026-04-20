//
// Created by Carlos Villacañas.
//

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <stdexcept>

#include "detection/FraudDetector.hpp"
#include "io/ModelReader.hpp"

// Holds the options used by the detection executable.
struct AppConfig {
    std::string input_path = "credit-card.dat";
    std::string model_path = "model.txt";

    std::size_t window = 5;
    double threshold = 0.96;
    fd::model::ScoreType score_type = fd::model::ScoreType::MISS_PROBABILITY;
    fd::detection::Backend backend = fd::detection::Backend::sequential;
};

// Prints the accepted command line arguments.
static void usage(const char* prog) {
    std::cerr
            << "Usage:\n"
            << "  " << prog << " --input <credit-card.dat> --model <model.txt>\n"
            << "           [--window 5] [--threshold 0.96]\n"
            << "           [--score miss_prob|miss_rate]\n"
            << "           [--backend <seq|tbb>]\n";
}

// Reads command line arguments and fills the detection configuration.
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
        } else if (a == "--score") {
            const std::string v = need(a);
            if (v == "miss_prob") {
                c.score_type = fd::model::ScoreType::MISS_PROBABILITY;
            } else if (v == "miss_rate") {
                c.score_type = fd::model::ScoreType::MISS_RATE;
            } else {
                throw std::runtime_error("Unknown --score value: " + v);
            }
        } else if (a == "--backend") {
            const std::string v = need(a);
            if (v == "seq") {
                c.backend = fd::detection::Backend::sequential;
            } else if (v == "tbb") {
                c.backend = fd::detection::Backend::tbb;
            } else {
                throw std::runtime_error("--backend must be seq or tbb");
            }
        } else {
            throw std::runtime_error("Unknown argument: " + a);
        }
    }

    return c;
}

int main(int argc, char** argv) {
    try {
        const AppConfig cfg = parse_args(argc, argv);

        // Load the model before starting the detection loop.
        fd::model::MarkovModel model = fd::io::read_model_txt(cfg.model_path);

        fd::detection::DetectionOptions opt;
        opt.window = cfg.window;
        opt.threshold = cfg.threshold;
        opt.score_type = cfg.score_type;
        opt.state_position = 1;
        opt.backend = cfg.backend;

        const auto t0 = std::chrono::steady_clock::now();
        const fd::detection::DetectionResult result =
                fd::detection::detect_from_dataset(cfg.input_path, model, opt);
        const auto t1 = std::chrono::steady_clock::now();

        const double seconds = std::chrono::duration<double>(t1 - t0).count();

        // Print throughput, latency and predictor counters for the experiment.
        std::cout << "Done.\n";
        std::cout << "Backend: "
                  << (cfg.backend == fd::detection::Backend::sequential ? "seq" : "tbb")
                  << "\n";
        std::cout << "Total events: " << result.total_events << "\n";
        std::cout << "Total time (s): " << seconds << "\n";
        if (seconds > 0.0) {
            std::cout << "Throughput (events/s): "
                      << (static_cast<double>(result.total_events) / seconds) << "\n";
        }
        std::cout << "Outliers: " << result.outliers << "\n";
        std::cout << "Latency mean (ms): " << result.latency_ms.mean_ms << "\n";
        std::cout << "Latency p50  (ms): " << result.latency_ms.p50_ms << "\n";
        std::cout << "Latency p95  (ms): " << result.latency_ms.p95_ms << "\n";
        std::cout << "Latency p99  (ms): " << result.latency_ms.p99_ms << "\n";
        std::cout << "Predictor stats:\n";
        std::cout << "  events: " << result.predictor_stats.events << "\n";
        std::cout << "  windows_full: " << result.predictor_stats.windows_full << "\n";
        std::cout << "  scored: " << result.predictor_stats.scored << "\n";
        std::cout << "  state_fail: " << result.predictor_stats.state_fail << "\n";
        if (result.predictor_stats.scored > 0) {
            std::cout << "  mean_score: "
                      << (result.predictor_stats.sum_score / result.predictor_stats.scored)
                      << "\n";
            std::cout << "  max_score: " << result.predictor_stats.max_score << "\n";
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        usage(argv[0]);
        return 1;
    }
}