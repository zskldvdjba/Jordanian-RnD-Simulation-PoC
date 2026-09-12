#include "simulation/SimulationEngine.hpp"
#include "core/Logger.hpp"
#include <iostream>
#include <chrono>
#include <filesystem>
#include <iomanip>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    std::string scenarioPath = "scenarios/S-001_baseline.json";
    if (argc > 1) {
        scenarioPath = argv[1];
    }

    // Fallback path resolution if run from build directory
    if (!fs::exists(scenarioPath)) {
        if (fs::exists("../" + scenarioPath)) {
            scenarioPath = "../" + scenarioPath;
        } else if (fs::exists("../../" + scenarioPath)) {
            scenarioPath = "../../" + scenarioPath;
        }
    }

    std::cout << "=================================================================\n";
    std::cout << " Jordanian Advanced Industrial R&D Simulation & Digital Engineering\n";
    std::cout << " Proof-of-Concept (PoC) - Milestone S-001 Baseline Simulation\n";
    std::cout << "=================================================================\n";

    sim::SimulationEngine engine;
    if (!engine.initializeFromFile(scenarioPath)) {
        std::cerr << "[ERROR] Failed to load scenario from: " << scenarioPath << "\n";
        return 1;
    }

    const auto& config = engine.getConfig();

    auto wallClockStart = std::chrono::high_resolution_clock::now();
    engine.run();
    auto wallClockEnd = std::chrono::high_resolution_clock::now();

    auto elapsedWallClockUs = std::chrono::duration_cast<std::chrono::microseconds>(wallClockEnd - wallClockStart).count();
    double elapsedWallClockMs = static_cast<double>(elapsedWallClockUs) / 1000.0;

    const auto& clock = engine.getClock();

    std::cout << "\n================ EXECUTION RESULTS ================\n";
    std::cout << "Scenario ID              : " << config.scenario_id << "\n";
    std::cout << "Scenario Name            : " << config.scenario_name << "\n";
    std::cout << "Number of Targets        : " << engine.getTargets().size() << "\n";
    std::cout << "Simulation Timestep (dt) : " << std::fixed << std::setprecision(4) << clock.getTimestep() << " s\n";
    std::cout << "Simulation Duration      : " << std::fixed << std::setprecision(2) << clock.getCurrentTime() << " s\n";
    std::cout << "Simulation Steps         : " << clock.getStepCount() << "\n";
    std::cout << "Elapsed Wall-Clock Time  : " << std::fixed << std::setprecision(3) << elapsedWallClockMs << " ms (" << elapsedWallClockUs << " us)\n";
    std::cout << "---------------------------------------------------\n";
    std::cout << engine.getMetricsCollector().formatSummary() << "\n";
    std::cout << "===================================================\n";

    return 0;
}
