//
// Created by Carlos Villacañas.
//
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

        constexpr int kStatePos = 1;              // default to match the common setup
        double alpha = 0.0;             // Laplace smoothing (0 = none)
        bool uniform_dead_end = true;   // if a row has no transitions, use uniform probabilities
        std::optional<std::string> fixed_order_from;

        // Very small CLI parser.
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
        fd::model::MarkovModel model = fd::training::train_from_dataset(input_path, kStatePos, opt, &rep);

        std::ofstream out(output_path);
        if (!out) {
            throw std::runtime_error("Cannot open output file: " + output_path);
        }
        fd::io::write_model_txt(out, model);

        // Simple training summary
        std::cout << "Training done.\n";
        std::cout << "States: " << model.size() << "\n";
        std::cout << "Lines read: " << rep.lines_read << "\n";
        std::cout << "Entities seen: " << rep.entities_seen << "\n";
        std::cout << "Transitions counted: " << rep.transitions_counted << "\n";
        std::cout << "Unknown states skipped: " << rep.unknown_states_skipped << "\n";
        std::cout << "Output: " << output_path << "\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
