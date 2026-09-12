#include "core/Types.hpp"
#include "core/SimulationClock.hpp"
#include "simulation/VirtualTarget.hpp"
#include "simulation/GroundTruthGenerator.hpp"
#include "simulation/SimulationEngine.hpp"
#include "data/ScenarioLoader.hpp"
#include "visualization/VisualizationAdapter.hpp"
#include <iostream>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <vector>
#include <algorithm>

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

// ==========================================
// Existing Milestone S-001 Tests
// ==========================================

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

// ==========================================
// Milestone S-001-VIS Visualization Tests
// ==========================================

std::string resolveScenarioPath(const std::string& relPath) {
    if (fs::exists(relPath)) return relPath;
    if (fs::exists("../" + relPath)) return "../" + relPath;
    if (fs::exists("../../" + relPath)) return "../../" + relPath;
    return relPath;
}

bool test_vis_state_50_targets() {
    std::cout << "[TEST] Running test_vis_state_50_targets...\n";
    std::string path = resolveScenarioPath("scenarios/S-001_visual_demo.json");

    sim::vis::VisualizationAdapter adapter;
    ASSERT_TRUE(adapter.initializeFromFile(path), "Failed to load visual demo scenario");

    const auto& targets = adapter.getTargets();
    ASSERT_TRUE(targets.size() == 50, "Expected exactly 50 targets, got " + std::to_string(targets.size()));

    for (size_t i = 0; i < 50; ++i) {
        ASSERT_TRUE(!targets[i].target_id.empty(), "Target ID empty at " + std::to_string(i));
        ASSERT_APPROX_EQ(targets[i].timestamp, 0.0, 1e-9, "Initial target timestamp mismatch");
        ASSERT_TRUE(targets[i].status == sim::vis::TargetStatus::ACTIVE, "Target not active initially");
        ASSERT_TRUE(!targets[i].trail.empty(), "Initial trail point missing");
    }

    ASSERT_APPROX_EQ(adapter.getCurrentSimulationTime(), 0.0, 1e-9, "Initial sim time non-zero");
    ASSERT_TRUE(adapter.getCurrentSimulationStep() == 0, "Initial sim step non-zero");

    std::cout << "[+] PASS: test_vis_state_50_targets\n";
    return true;
}

bool test_vis_deterministic_replay() {
    std::cout << "[TEST] Running test_vis_deterministic_replay...\n";
    std::string path = resolveScenarioPath("scenarios/S-001_visual_demo.json");

    sim::vis::VisualizationAdapter adapter1;
    ASSERT_TRUE(adapter1.initializeFromFile(path), "Failed adapter1 init");
    adapter1.start();
    for (int i = 0; i < 200; ++i) {
        adapter1.step();
    }

    sim::vis::VisualizationAdapter adapter2;
    ASSERT_TRUE(adapter2.initializeFromFile(path), "Failed adapter2 init");
    adapter2.start();
    for (int i = 0; i < 200; ++i) {
        adapter2.step();
    }

    const auto& tgts1 = adapter1.getTargets();
    const auto& tgts2 = adapter2.getTargets();
    ASSERT_TRUE(tgts1.size() == 50 && tgts2.size() == 50, "Target count mismatch between runs");

    for (size_t i = 0; i < 50; ++i) {
        ASSERT_TRUE(tgts1[i].target_id == tgts2[i].target_id, "Target ID mismatch between runs");
        ASSERT_APPROX_EQ(tgts1[i].position.x, tgts2[i].position.x, 1e-12, "Replay pos.x mismatch");
        ASSERT_APPROX_EQ(tgts1[i].position.y, tgts2[i].position.y, 1e-12, "Replay pos.y mismatch");
        ASSERT_APPROX_EQ(tgts1[i].position.z, tgts2[i].position.z, 1e-12, "Replay pos.z mismatch");
        ASSERT_TRUE(tgts1[i].status == tgts2[i].status, "Replay target status mismatch");
    }

    const auto& evs1 = adapter1.getEventManager().getEvents();
    const auto& evs2 = adapter2.getEventManager().getEvents();
    ASSERT_TRUE(evs1.size() == evs2.size(), "Event count mismatch between deterministic runs");

    for (size_t i = 0; i < evs1.size(); ++i) {
        ASSERT_TRUE(evs1[i].event_id == evs2[i].event_id, "Event ID mismatch");
        ASSERT_APPROX_EQ(evs1[i].simulation_time, evs2[i].simulation_time, 1e-9, "Event time mismatch");
        ASSERT_TRUE(evs1[i].source_entity == evs2[i].source_entity, "Event source mismatch");
        ASSERT_TRUE(evs1[i].target_entity == evs2[i].target_entity, "Event target mismatch");
    }

    std::cout << "[+] PASS: test_vis_deterministic_replay\n";
    return true;
}

bool test_vis_virtual_interaction() {
    std::cout << "[TEST] Running test_vis_virtual_interaction...\n";
    std::string path = resolveScenarioPath("scenarios/S-001_visual_demo.json");

    sim::vis::VisualizationAdapter adapter;
    ASSERT_TRUE(adapter.initializeFromFile(path), "Failed adapter init");

    const auto& entities = adapter.getInteractionEntities();
    ASSERT_TRUE(entities.size() == 3, "Expected 3 virtual interaction entities, got " + std::to_string(entities.size()));

    ASSERT_TRUE(entities[0].getEntityId() == "VE-001", "VE-001 ID mismatch");
    ASSERT_TRUE(entities[0].getTargetId() == "VT-017", "VE-001 target ID mismatch");
    ASSERT_TRUE(entities[1].getEntityId() == "VE-002", "VE-002 ID mismatch");
    ASSERT_TRUE(entities[1].getTargetId() == "VT-031", "VE-002 target ID mismatch");
    ASSERT_TRUE(entities[2].getEntityId() == "VE-003", "VE-003 ID mismatch");
    ASSERT_TRUE(entities[2].getTargetId() == "VT-045", "VE-003 target ID mismatch");

    // Advance to 4.0s (80 steps at 0.05s). VE-001 is configured to trigger around 3.8s
    adapter.start();
    for (int i = 0; i < 80; ++i) {
        adapter.step();
    }

    const auto& events = adapter.getEventManager().getEvents();
    ASSERT_TRUE(!events.empty(), "Expected at least 1 interaction event to trigger by 4.0s");
    ASSERT_TRUE(events[0].source_entity == "VE-001", "First event source was not VE-001");
    ASSERT_TRUE(events[0].target_entity == "VT-017", "First event target was not VT-017");

    // Check target state changed to INTERACTED
    const auto& targets = adapter.getTargets();
    auto it = std::find_if(targets.begin(), targets.end(), [](const sim::vis::TargetVisualState& ts) {
        return ts.target_id == "VT-017";
    });
    ASSERT_TRUE(it != targets.end(), "VT-017 not found");
    ASSERT_TRUE(it->status == sim::vis::TargetStatus::INTERACTED, "Target status did not update to INTERACTED");

    std::cout << "[+] PASS: test_vis_virtual_interaction\n";
    return true;
}

bool test_vis_reset() {
    std::cout << "[TEST] Running test_vis_reset...\n";
    std::string path = resolveScenarioPath("scenarios/S-001_visual_demo.json");

    sim::vis::VisualizationAdapter adapter;
    ASSERT_TRUE(adapter.initializeFromFile(path), "Failed adapter init");

    // Advance 50 steps (2.5s)
    adapter.start();
    for (int i = 0; i < 50; ++i) {
        adapter.step();
    }

    ASSERT_APPROX_EQ(adapter.getCurrentSimulationTime(), 2.5, 1e-6, "Sim time mismatch before reset");
    ASSERT_TRUE(adapter.getCurrentSimulationStep() == 50, "Sim step mismatch before reset");

    // Call RESET
    adapter.reset();

    ASSERT_APPROX_EQ(adapter.getCurrentSimulationTime(), 0.0, 1e-9, "Sim time not reset to 0.0");
    ASSERT_TRUE(adapter.getCurrentSimulationStep() == 0, "Sim step not reset to 0");
    ASSERT_TRUE(adapter.getEventManager().getEventCount() == 0, "Event log not cleared after reset");
    ASSERT_TRUE(adapter.getVisualEffects().empty(), "Visual effects not cleared after reset");

    for (const auto& tgt : adapter.getTargets()) {
        ASSERT_TRUE(tgt.status == sim::vis::TargetStatus::ACTIVE, "Target status not reset to ACTIVE");
        ASSERT_APPROX_EQ(tgt.timestamp, 0.0, 1e-9, "Target timestamp not reset to 0.0");
    }

    for (const auto& entity : adapter.getInteractionEntities()) {
        ASSERT_TRUE(!entity.isActive(), "Interaction entity active flag not reset");
        ASSERT_TRUE(!entity.isTriggered(), "Interaction entity triggered flag not reset");
    }

    std::cout << "[+] PASS: test_vis_reset\n";
    return true;
}

int main() {
    std::cout << "==================================================\n";
    std::cout << " RUNNING UNIT & VISUALIZATION TESTS (MILESTONE S-001-VIS)\n";
    std::cout << "==================================================\n";

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

    // Original S-001 Tests
    runTest(test_target_creation, "target_creation");
    runTest(test_deterministic_seed_behavior, "deterministic_seed_behavior");
    runTest(test_simulation_clock_progression, "simulation_clock_progression");
    runTest(test_target_state_update, "target_state_update");
    runTest(test_50_target_scenario_loading, "50_target_scenario_loading");

    // New S-001-VIS Tests
    runTest(test_vis_state_50_targets, "vis_state_50_targets");
    runTest(test_vis_deterministic_replay, "vis_deterministic_replay");
    runTest(test_vis_virtual_interaction, "vis_virtual_interaction");
    runTest(test_vis_reset, "vis_reset");

    std::cout << "==================================================\n";
    std::cout << "TOTAL SUMMARY: " << passed << " PASSED, " << failed << " FAILED\n";
    std::cout << "==================================================\n";

    return failed == 0 ? 0 : 1;
}
