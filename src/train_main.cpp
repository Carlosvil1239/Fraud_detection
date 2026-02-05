//
// Created by Carlos Villacañas.
//
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <optional>
#include <stdexcept>

#include "training/MarkovTrainer.hpp"
#include "io/ModelWriter.hpp"

static void print_usage(const char* prog) {
    std::cerr
            << "Usage:\n"
            << "  " << prog << " --input <credit-card.dat> --output <model.txt>\n"
            << "       [--alpha <double>]\n"
            << "      [--no-uniform-dead-end]\n";
}

int main(int argc, char** argv) {
    try {
        std::string input_path;
        std::string output_path;

        constexpr int kStatePos = 1;
        double alpha = 0.0;
        bool uniform_dead_end = true;

        for (int i = 1; i < argc; ++i) {
            const std::string a = argv[i];

            auto need_value = [&](const std::string& flag) -> std::string {
                if (i + 1 >= argc) {
                    throw std::runtime_error("Missing value for " + flag);
                }
                return argv[++i];
            };

            if (a == "--help" || a == "-h") {
                print_usage(argv[0]);
                return 0;
            } else if (a == "--input") {
                input_path = need_value(a);
            } else if (a == "--output") {
                output_path = need_value(a);
            } else if (a == "--alpha") {
                alpha = std::stod(need_value(a));
                if (alpha < 0.0) {
                    throw std::runtime_error("--alpha must be >= 0");
                }
            } else if (a == "--no-uniform-dead-end") {
                uniform_dead_end = false;
            } else {
                throw std::runtime_error("Unknown argument: " + a);
            }
        }

        if (input_path.empty() || output_path.empty()) {
            print_usage(argv[0]);
            return 1;
        }

        fd::training::TrainingOptions opt;
        opt.alpha = alpha;
        opt.uniform_if_dead_end = uniform_dead_end;

        fd::training::TrainingReport rep;

        const auto t0 = std::chrono::steady_clock::now();
        fd::model::MarkovModel model =
                fd::training::train_from_dataset(input_path, kStatePos, opt, &rep);
        const auto t1 = std::chrono::steady_clock::now();

        const double seconds = std::chrono::duration<double>(t1 - t0).count();

        std::ofstream out(output_path);
        if (!out) {
            throw std::runtime_error("Cannot open output file: " + output_path);
        }
        fd::io::write_model_txt(out, model);

        std::cout << "Training done.\n";
        std::cout << "States: " << model.size() << "\n";
        std::cout << "Lines read: " << rep.lines_read << "\n";
        std::cout << "Entities seen: " << rep.entities_seen << "\n";
        std::cout << "Transitions counted: " << rep.transitions_counted << "\n";
        std::cout << "Unknown states skipped: " << rep.unknown_states_skipped << "\n";
        std::cout << "Output: " << output_path << "\n";

        std::cout << "Training time (s): " << seconds << "\n";
        if (seconds > 0.0) {
            std::cout << "Training throughput (lines/s): "
                      << (static_cast<double>(rep.lines_read) / seconds) << "\n";
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
