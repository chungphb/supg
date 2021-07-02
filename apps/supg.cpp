//
// Created by chungphb on 21/5/21.
//

#include <supg/core/simulator.h>
#include <supg/util/helper.h>
#include <argagg/argagg.hpp>
#include <csignal>
#include <csetjmp>
#include <atomic>
#include <iostream>

constexpr char version[] = "0.1.0.";
constexpr char default_config_file[] = "supg.toml";

std::atomic<bool> quit{false};
jmp_buf buf;

void signal_handler(int signal) {
    if (quit.load()) {
        exit(EXIT_SUCCESS);
    } else {
        quit.store(true);
        longjmp(buf, 1);
    }
}

int main(int argc, char** argv) {
    supg::simulator sim;

    // Handle SIGINT signal
    signal(SIGINT, signal_handler);
    if (setjmp(buf) == 1) {
        sim.stop();
        return EXIT_SUCCESS;
    }

    // Handle command-line options
    argagg::parser parser{{
        {"version", {"-v", "--version"}, "Show version info and exit.", 0},
        {"help", {"-h", "--help"}, "Show this text and exit.", 0},
        {"generate-config-file", {"-g", "--generate-config-file"}, "Generate a new configuration file and exit.", 1},
        {"config", {"-c", "--config"}, "Specify configuration file.", 1},
        {"base64-encode", {"--base64-encode"}, "Encode string to Base64 format and exit.", 1},
        {"base64-decode", {"--base64-decode"}, "Decode string from Base64 format and exit.", 1}
    }};
    argagg::parser_results options;
    try {
        options = parser.parse(argc, argv);
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';
        return EXIT_FAILURE;
    }
    if (options["help"]) {
        std::cerr << "Usage: supg [options] ARGS..." << '\n';
        std::cerr << parser << '\n';
        return EXIT_SUCCESS;
    }
    if (options["version"]) {
        std::cerr << "Semtech UDP Packet Generator " << version << '\n';
        return EXIT_SUCCESS;
    }
    if (options["base64-encode"]) {
        std::cout << supg::base64_encode(options["base64-encode"].as<std::string>()) << '\n';
        return EXIT_SUCCESS;
    }
    if (options["base64-decode"]) {
        std::cout << supg::base64_decode(options["base64-decode"].as<std::string>()) << '\n';
        return EXIT_SUCCESS;
    }

    // Start simulator
    try {
        if (options["generate-config-file"]) {
            sim.generate_config_file(options["generate-config-file"].as<std::string>());
            return EXIT_SUCCESS;
        }
        auto config_file = options["config"].as<std::string>(default_config_file);
        sim.set_config_file(config_file);
        sim.init();
        sim.run();
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';
        sim.stop();
        return EXIT_FAILURE;
    } catch (...) {
        sim.stop();
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}