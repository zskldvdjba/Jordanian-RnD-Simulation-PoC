#include "core/Types.hpp"
#include "core/SimulationClock.hpp"
#include "simulation/VirtualTarget.hpp"
#include "simulation/GroundTruthGenerator.hpp"
#include "simulation/SimulationEngine.hpp"
#include "data/ScenarioLoader.hpp"
#include <iostream>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

#define ASSERT_TRUE(expr, msg) \
    do { \
        if (!(expr)) { \
            std::cerr << "[-] FAIL: " << msg << " (" << #expr << ") at line " << __LINE__ << "\n"; \
            return false; \
        } \
    } while (0)

#define ASSERT_APPROX_EQ(a, b, eps, msg) \
    do { \
        if (std::abs((a) - (b)) > (eps)) { \
            std::cerr << "[-] FAIL: " << msg << " (" << (a) << " != " << (b) << ") at line " << __LINE__ << "\n"; \
            return false; \
        } \
    } while (0)

bool test_target_creation() {
    std::cout << "[TEST] Running test_target_creation...\n";
    sim::Vector3D pos{100.0, -250.0, 1500.0};
    sim::Vector3D vel{50.0, 0.0, -10.0};
    sim::VirtualTarget target("VT-TEST-01", 0.0, pos, vel);

    ASSERT_TRUE(target.getId() == "VT-TEST-01", "Target ID mismatch");
    ASSERT_APPROX_EQ(target.getTimestamp(), 0.0, 1e-9, "Initial timestamp mismatch");
    ASSERT_APPROX_EQ(target.getPosition().x, 100.0, 1e-9, "Position X mismatch");
    ASSERT_APPROX_EQ(target.getPosition().y, -250.0, 1e-9, "Position Y mismatch");
    ASSERT_APPROX_EQ(target.getPosition().z, 1500.0, 1e-9, "Position Z mismatch");
    ASSERT_APPROX_EQ(target.getVelocity().x, 50.0, 1e-9, "Velocity X mismatch");
    ASSERT_APPROX_EQ(target.getVelocity().y, 0.0, 1e-9, "Velocity Y mismatch");
    ASSERT_APPROX_EQ(target.getVelocity().z, -10.0, 1e-9, "Velocity Z mismatch");

    std::cout << "[+] PASS: test_target_creation\n";
    return true;
}

bool test_deterministic_seed_behavior() {
    std::cout << "[TEST] Running test_deterministic_seed_behavior...\n";
    constexpr uint64_t seed = 987654321ULL;

    sim::GroundTruthGenerator gen1(seed);
    sim::GroundTruthGenerator gen2(seed);

    auto targets1 = gen1.generateSyntheticTargets(50, 0.0);
    auto targets2 = gen2.generateSyntheticTargets(50, 0.0);

    ASSERT_TRUE(targets1.size() == 50, "Gen1 size mismatch");
    ASSERT_TRUE(targets2.size() == 50, "Gen2 size mismatch");

    for (size_t i = 0; i < 50; ++i) {
        ASSERT_TRUE(targets1[i].getId() == targets2[i].getId(), "ID mismatch at index " + std::to_string(i));
        ASSERT_APPROX_EQ(targets1[i].getPosition().x, targets2[i].getPosition().x, 1e-12, "Deterministic pos.x mismatch");
        ASSERT_APPROX_EQ(targets1[i].getPosition().y, targets2[i].getPosition().y, 1e-12, "Deterministic pos.y mismatch");
        ASSERT_APPROX_EQ(targets1[i].getPosition().z, targets2[i].getPosition().z, 1e-12, "Deterministic pos.z mismatch");
        ASSERT_APPROX_EQ(targets1[i].getVelocity().x, targets2[i].getVelocity().x, 1e-12, "Deterministic vel.x mismatch");
        ASSERT_APPROX_EQ(targets1[i].getVelocity().y, targets2[i].getVelocity().y, 1e-12, "Deterministic vel.y mismatch");
        ASSERT_APPROX_EQ(targets1[i].getVelocity().z, targets2[i].getVelocity().z, 1e-12, "Deterministic vel.z mismatch");
    }

    // Different seed must produce different trajectories
    sim::GroundTruthGenerator gen3(112233ULL);
    auto targets3 = gen3.generateSyntheticTargets(50, 0.0);
    bool anyDifferent = false;
    for (size_t i = 0; i < 50; ++i) {
        if (!targets1[i].getPosition().isApprox(targets3[i].getPosition(), 1e-3)) {
            anyDifferent = true;
            break;
        }
    }
    ASSERT_TRUE(anyDifferent, "Different seeds unexpectedly produced identical states");

    std::cout << "[+] PASS: test_deterministic_seed_behavior\n";
    return true;
}

bool test_simulation_clock_progression() {
    std::cout << "[TEST] Running test_simulation_clock_progression...\n";
    double dt = 0.05;
    sim::SimulationClock clock(dt, 0.0);

    ASSERT_APPROX_EQ(clock.getCurrentTime(), 0.0, 1e-9, "Initial time mismatch");
    ASSERT_TRUE(clock.getStepCount() == 0, "Initial step count mismatch");

    for (int i = 1; i <= 20; ++i) {
        clock.advance();
        ASSERT_TRUE(clock.getStepCount() == static_cast<uint64_t>(i), "Step count mismatch");
        ASSERT_APPROX_EQ(clock.getCurrentTime(), i * dt, 1e-9, "Clock time progression mismatch");
    }

    ASSERT_APPROX_EQ(clock.getCurrentTime(), 1.0, 1e-9, "Total time mismatch after 20 steps");

    clock.reset(5.0);
    ASSERT_APPROX_EQ(clock.getCurrentTime(), 5.0, 1e-9, "Reset time mismatch");
    ASSERT_TRUE(clock.getStepCount() == 0, "Reset step count mismatch");

    std::cout << "[+] PASS: test_simulation_clock_progression\n";
    return true;
}

bool test_target_state_update() {
    std::cout << "[TEST] Running test_target_state_update...\n";
    sim::Vector3D p0{100.0, 200.0, 300.0};
    sim::Vector3D v{10.0, -20.0, 5.0};
    sim::VirtualTarget target("VT-UPDATE", 0.0, p0, v);

    double dt = 0.1;
    target.update(dt);

    ASSERT_APPROX_EQ(target.getTimestamp(), 0.1, 1e-9, "Updated timestamp mismatch");
    ASSERT_APPROX_EQ(target.getPosition().x, 101.0, 1e-9, "Updated X mismatch");
    ASSERT_APPROX_EQ(target.getPosition().y, 198.0, 1e-9, "Updated Y mismatch");
    ASSERT_APPROX_EQ(target.getPosition().z, 300.5, 1e-9, "Updated Z mismatch");

    // Velocity should remain constant in linear kinematic baseline
    ASSERT_APPROX_EQ(target.getVelocity().x, 10.0, 1e-9, "Velocity X altered");
    ASSERT_APPROX_EQ(target.getVelocity().y, -20.0, 1e-9, "Velocity Y altered");
    ASSERT_APPROX_EQ(target.getVelocity().z, 5.0, 1e-9, "Velocity Z altered");

    std::cout << "[+] PASS: test_target_state_update\n";
    return true;
}

bool test_50_target_scenario_loading() {
    std::cout << "[TEST] Running test_50_target_scenario_loading...\n";
    std::string scenarioPath = "scenarios/S-001_baseline.json";
    if (!fs::exists(scenarioPath)) {
        if (fs::exists("../" + scenarioPath)) {
            scenarioPath = "../" + scenarioPath;
        } else if (fs::exists("../../" + scenarioPath)) {
            scenarioPath = "../../" + scenarioPath;
        }
    }

    auto configOpt = sim::ScenarioLoader::loadFromFile(scenarioPath);
    ASSERT_TRUE(configOpt.has_value(), "Failed to load scenario file: " + scenarioPath);

    const auto& config = *configOpt;
    ASSERT_TRUE(config.scenario_id == "S-001", "Scenario ID mismatch: " + config.scenario_id);
    ASSERT_TRUE(config.target_count == 50, "Expected target_count == 50, got " + std::to_string(config.target_count));
    ASSERT_TRUE(config.targets.size() == 50, "Expected 50 targets vector size, got " + std::to_string(config.targets.size()));
    ASSERT_APPROX_EQ(config.timestep_seconds, 0.05, 1e-6, "Timestep mismatch");
    ASSERT_APPROX_EQ(config.duration_seconds, 10.0, 1e-6, "Duration mismatch");

    for (size_t i = 0; i < 50; ++i) {
        ASSERT_TRUE(!config.targets[i].target_id.empty(), "Empty target ID at index " + std::to_string(i));
    }

    std::cout << "[+] PASS: test_50_target_scenario_loading\n";
    return true;
}

int main() {
    std::cout << "========================================\n";
    std::cout << " RUNNING UNIT TESTS FOR MILESTONE S-001\n";
    std::cout << "========================================\n";

    int passed = 0;
    int failed = 0;

    auto runTest = [&](bool (*func)(), const char* name) {
        if (func()) {
            ++passed;
        } else {
            ++failed;
            std::cerr << "[-] FAILED TEST: " << name << "\n";
        }
    };

    runTest(test_target_creation, "target_creation");
    runTest(test_deterministic_seed_behavior, "deterministic_seed_behavior");
    runTest(test_simulation_clock_progression, "simulation_clock_progression");
    runTest(test_target_state_update, "target_state_update");
    runTest(test_50_target_scenario_loading, "50_target_scenario_loading");

    std::cout << "========================================\n";
    std::cout << "TEST SUMMARY: " << passed << " PASSED, " << failed << " FAILED\n";
    std::cout << "========================================\n";

    return failed == 0 ? 0 : 1;
}
