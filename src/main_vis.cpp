#include "visualization/VisualizationApp.hpp"
#include <iostream>
#include <filesystem>
#include <iomanip>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    std::string scenarioPath = "scenarios/S-001_visual_demo.json";
    bool benchmarkMode = false;
    int benchmarkFrames = 250;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--benchmark" || arg == "--verify") {
            benchmarkMode = true;
        } else if (arg.rfind("--frames=", 0) == 0) {
            benchmarkFrames = std::stoi(arg.substr(9));
        } else if (arg.rfind("-", 0) != 0) {
            scenarioPath = arg;
        }
    }

    if (!fs::exists(scenarioPath)) {
        if (fs::exists("../" + scenarioPath)) {
            scenarioPath = "../" + scenarioPath;
        } else if (fs::exists("../../" + scenarioPath)) {
            scenarioPath = "../../" + scenarioPath;
        }
    }

    std::cout << "=================================================================\n";
    std::cout << " Jordanian Advanced Industrial R&D Simulation & Digital Engineering\n";
    std::cout << " Proof-of-Concept (PoC) - Milestone S-001-VIS 3D Visualization\n";
    std::cout << "=================================================================\n";
    std::cout << "Loading scenario: " << scenarioPath << "\n";

    auto appStart = std::chrono::high_resolution_clock::now();
    sim::vis::VisualizationApp app;
    if (!app.initialize(scenarioPath, 1280, 720)) {
        std::cerr << "[ERROR] Failed to start 3D visualization application\n";
        return 1;
    }
    auto appReady = std::chrono::high_resolution_clock::now();
    double startupTimeMs = std::chrono::duration<double, std::milli>(appReady - appStart).count();

    if (benchmarkMode) {
        std::cout << "[INFO] Running automated visual benchmark for " << benchmarkFrames << " frames...\n";
        auto bm = app.runBenchmark(benchmarkFrames);
        bm.startup_time_ms = startupTimeMs;

        std::cout << "\n================ VISUALIZATION BENCHMARK RESULTS ================\n";
        std::cout << "Startup Time (ms)               : " << std::fixed << std::setprecision(2) << bm.startup_time_ms << " ms\n";
        std::cout << "Average Render Frame Rate       : " << std::fixed << std::setprecision(1) << bm.avg_frame_rate_fps << " FPS\n";
        std::cout << "Average Frame Time              : " << std::fixed << std::setprecision(3) << bm.avg_frame_time_ms << " ms\n";
        std::cout << "Simulation Adapter Update Time  : " << std::fixed << std::setprecision(2) << bm.avg_sim_update_time_us << " us\n";
        std::cout << "Process Working Set (Memory)    : " << std::fixed << std::setprecision(2) << bm.memory_usage_mb << " MB\n";
        std::cout << "Rendered Virtual Targets        : " << bm.rendered_targets << "\n";
        std::cout << "Active Virtual Event Entities   : " << bm.active_virtual_events << "\n";
        std::cout << "Triggered Interaction Events    : " << bm.triggered_interaction_events << "\n";
        std::cout << "=================================================================\n";
        return 0;
    }

    return app.run();
}
