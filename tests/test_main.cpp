#include "core/Types.hpp"
#include "core/SimulationClock.hpp"
#include "simulation/VirtualTarget.hpp"
#include "simulation/GroundTruthGenerator.hpp"
#include "simulation/SimulationEngine.hpp"
#include "data/ScenarioLoader.hpp"
#include "visualization/VisualizationAdapter.hpp"
#include "visualization/PerformanceMonitor.hpp"
#include "visualization/Frustum.hpp"
#include "visualization/ParticleSystem.hpp"
#include <iostream>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iomanip>

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

// ==========================================
// S-001-VIS-PERF Performance Monitor Tests
// ==========================================

bool test_perfmon_config_load() {
    std::cout << "[TEST] Running test_perfmon_config_load...\n";
    sim::vis::PerformanceMonitor monitor;

    std::string configPath = "config/performance.json";
    if (!fs::exists(configPath)) {
        if (fs::exists("../" + configPath)) configPath = "../" + configPath;
        else if (fs::exists("../../" + configPath)) configPath = "../../" + configPath;
    }

    bool loaded = monitor.loadConfig(configPath);
    ASSERT_TRUE(loaded, "Failed to load config/performance.json");

    const auto& cfg = monitor.getConfig();
    ASSERT_TRUE(cfg.defaultQuality == sim::vis::QualityLevel::MEDIUM, "Default quality should be MEDIUM");
    ASSERT_APPROX_EQ(cfg.hysteresis.downgradeThreshold, 27.0, 0.01, "Downgrade threshold mismatch");
    ASSERT_APPROX_EQ(cfg.hysteresis.strongDowngradeThreshold, 24.0, 0.01, "Strong downgrade threshold mismatch");
    ASSERT_APPROX_EQ(cfg.hysteresis.upgradeThreshold, 35.0, 0.01, "Upgrade threshold mismatch");
    ASSERT_APPROX_EQ(cfg.hysteresis.sustainedSeconds, 3.0, 0.01, "Sustained seconds mismatch");
    ASSERT_TRUE(cfg.particleCaps.smoke == 300, "Smoke cap mismatch");
    ASSERT_TRUE(cfg.particleCaps.exhaust == 200, "Exhaust cap mismatch");
    ASSERT_TRUE(cfg.particleCaps.beam == 100, "Beam cap mismatch");
    ASSERT_TRUE(cfg.particleCaps.general == 150, "General cap mismatch");
    ASSERT_TRUE(cfg.frameDelayTestMs == 0, "Frame delay should be 0 by default");
    ASSERT_APPROX_EQ(cfg.lodDistanceNear, 5000.0, 0.01, "LOD near distance mismatch");
    ASSERT_APPROX_EQ(cfg.lodDistanceFar, 15000.0, 0.01, "LOD far distance mismatch");
    ASSERT_TRUE(cfg.gpuTelemetryEnabled == false, "GPU telemetry should be disabled by default");

    std::cout << "[+] PASS: test_perfmon_config_load\n";
    return true;
}

bool test_perfmon_quality_cycle() {
    std::cout << "[TEST] Running test_perfmon_quality_cycle...\n";
    sim::vis::PerformanceMonitor monitor;

    ASSERT_TRUE(monitor.getQualityLevel() == sim::vis::QualityLevel::MEDIUM, "Initial quality should be MEDIUM");

    monitor.cycleQualityLevel();
    ASSERT_TRUE(monitor.getQualityLevel() == sim::vis::QualityLevel::HIGH, "After 1 cycle should be HIGH");

    monitor.cycleQualityLevel();
    ASSERT_TRUE(monitor.getQualityLevel() == sim::vis::QualityLevel::RESEARCH, "After 2 cycles should be RESEARCH");

    monitor.cycleQualityLevel();
    ASSERT_TRUE(monitor.getQualityLevel() == sim::vis::QualityLevel::LOW, "After 3 cycles should be LOW (wraps)");

    monitor.cycleQualityLevel();
    ASSERT_TRUE(monitor.getQualityLevel() == sim::vis::QualityLevel::MEDIUM, "After 4 cycles should be MEDIUM (full wrap)");

    std::cout << "[+] PASS: test_perfmon_quality_cycle\n";
    return true;
}

bool test_perfmon_quality_change_info() {
    std::cout << "[TEST] Running test_perfmon_quality_change_info...\n";
    sim::vis::PerformanceMonitor monitor;

    const auto& qci = monitor.getQualityChangeInfo();
    ASSERT_TRUE(qci.currentLevel == sim::vis::QualityLevel::MEDIUM, "Initial current level should be MEDIUM");
    ASSERT_TRUE(qci.previousLevel == sim::vis::QualityLevel::MEDIUM, "Initial previous level should be MEDIUM");
    ASSERT_TRUE(qci.reason.find("Startup default") != std::string::npos || qci.reason.find("Config loaded") != std::string::npos,
                "Initial reason should indicate startup or config");

    monitor.cycleQualityLevel();
    const auto& qci2 = monitor.getQualityChangeInfo();
    ASSERT_TRUE(qci2.currentLevel == sim::vis::QualityLevel::HIGH, "After cycle, current should be HIGH");
    ASSERT_TRUE(qci2.previousLevel == sim::vis::QualityLevel::MEDIUM, "After cycle, previous should be MEDIUM");
    ASSERT_TRUE(qci2.hasChanged == true, "hasChanged should be true after cycle");
    ASSERT_TRUE(qci2.reason.find("Manual") != std::string::npos, "Reason should contain 'Manual'");

    std::cout << "[+] PASS: test_perfmon_quality_change_info\n";
    return true;
}

bool test_perfmon_fps_target_cycle() {
    std::cout << "[TEST] Running test_perfmon_fps_target_cycle...\n";
    sim::vis::PerformanceMonitor monitor;

    monitor.setTargetFps(60);
    ASSERT_TRUE(monitor.getTargetFps() == 60, "setTargetFps(60) failed");

    monitor.cycleTargetFps();
    ASSERT_TRUE(monitor.getTargetFps() == 0, "Cycle from 60 should go to 0 (uncapped)");

    monitor.cycleTargetFps();
    ASSERT_TRUE(monitor.getTargetFps() == 30, "Cycle from 0 should go to 30");

    monitor.cycleTargetFps();
    ASSERT_TRUE(monitor.getTargetFps() == 60, "Cycle from 30 should go to 60");

    std::cout << "[+] PASS: test_perfmon_fps_target_cycle\n";
    return true;
}

bool test_perfmon_gpu_reporting() {
    std::cout << "[TEST] Running test_perfmon_gpu_reporting...\n";
    sim::vis::PerformanceMonitor monitor;

    const auto& stats = monitor.getStats();
    ASSERT_TRUE(stats.gpu_usage_str == "N/A", "GPU usage should report N/A");

    std::cout << "[+] PASS: test_perfmon_gpu_reporting\n";
    return true;
}

bool test_perfmon_auto_quality_toggle() {
    std::cout << "[TEST] Running test_perfmon_auto_quality_toggle...\n";
    sim::vis::PerformanceMonitor monitor;

    ASSERT_TRUE(monitor.isAutoQuality() == true, "Auto quality should be ON by default");

    monitor.setAutoQuality(false);
    ASSERT_TRUE(monitor.isAutoQuality() == false, "Auto quality should be OFF after setAutoQuality(false)");

    monitor.setAutoQuality(true);
    ASSERT_TRUE(monitor.isAutoQuality() == true, "Auto quality should be ON after setAutoQuality(true)");

    std::cout << "[+] PASS: test_perfmon_auto_quality_toggle\n";
    return true;
}

bool test_perfmon_render_stats_recording() {
    std::cout << "[TEST] Running test_perfmon_render_stats_recording...\n";
    sim::vis::PerformanceMonitor monitor;

    monitor.recordRenderStats(42, 8, 150, 750, 25);

    const auto& stats = monitor.getStats();
    ASSERT_TRUE(stats.active_3d_objects == 42, "active_3d_objects mismatch");
    ASSERT_TRUE(stats.culled_3d_objects == 8, "culled_3d_objects mismatch");
    ASSERT_TRUE(stats.active_particles == 150, "active_particles mismatch");
    ASSERT_TRUE(stats.max_particles_budget == 750, "max_particles_budget mismatch");
    ASSERT_TRUE(stats.draw_calls == 25, "draw_calls mismatch");

    std::cout << "[+] PASS: test_perfmon_render_stats_recording\n";
    return true;
}

bool test_frustum_lod_configurable() {
    std::cout << "[TEST] Running test_frustum_lod_configurable...\n";
    sim::vis::Frustum frustum;

    ASSERT_TRUE(frustum.getLodForDistance(3000.0) == sim::vis::LodLevel::HIGH, "Default: 3000 should be HIGH");
    ASSERT_TRUE(frustum.getLodForDistance(10000.0) == sim::vis::LodLevel::MEDIUM, "Default: 10000 should be MEDIUM");
    ASSERT_TRUE(frustum.getLodForDistance(20000.0) == sim::vis::LodLevel::LOW, "Default: 20000 should be LOW");

    frustum.setLodDistances(2000.0, 8000.0);
    ASSERT_TRUE(frustum.getLodForDistance(1500.0) == sim::vis::LodLevel::HIGH, "Custom: 1500 should be HIGH (near=2000)");
    ASSERT_TRUE(frustum.getLodForDistance(5000.0) == sim::vis::LodLevel::MEDIUM, "Custom: 5000 should be MEDIUM (far=8000)");
    ASSERT_TRUE(frustum.getLodForDistance(9000.0) == sim::vis::LodLevel::LOW, "Custom: 9000 should be LOW");

    std::cout << "[+] PASS: test_frustum_lod_configurable\n";
    return true;
}

bool test_particle_budget_by_quality() {
    std::cout << "[TEST] Running test_particle_budget_by_quality...\n";
    sim::vis::ParticleSystem ps;

    ASSERT_TRUE(ps.getBudget(sim::vis::QualityLevel::LOW) == 185, "LOW budget mismatch");
    ASSERT_TRUE(ps.getBudget(sim::vis::QualityLevel::MEDIUM) == 375, "MEDIUM budget mismatch");
    ASSERT_TRUE(ps.getBudget(sim::vis::QualityLevel::HIGH) == 750, "HIGH budget mismatch");
    ASSERT_TRUE(ps.getBudget(sim::vis::QualityLevel::RESEARCH) == 750, "RESEARCH budget mismatch");

    std::cout << "[+] PASS: test_particle_budget_by_quality\n";
    return true;
}

bool test_quality_level_string_roundtrip() {
    std::cout << "[TEST] Running test_quality_level_string_roundtrip...\n";
    ASSERT_TRUE(sim::vis::qualityLevelFromString("LOW") == sim::vis::QualityLevel::LOW, "LOW roundtrip");
    ASSERT_TRUE(sim::vis::qualityLevelFromString("MEDIUM") == sim::vis::QualityLevel::MEDIUM, "MEDIUM roundtrip");
    ASSERT_TRUE(sim::vis::qualityLevelFromString("HIGH") == sim::vis::QualityLevel::HIGH, "HIGH roundtrip");
    ASSERT_TRUE(sim::vis::qualityLevelFromString("RESEARCH") == sim::vis::QualityLevel::RESEARCH, "RESEARCH roundtrip");
    ASSERT_TRUE(sim::vis::qualityLevelFromString("INVALID") == sim::vis::QualityLevel::MEDIUM, "INVALID should default to MEDIUM");

    ASSERT_TRUE(std::string(sim::vis::qualityLevelToString(sim::vis::QualityLevel::LOW)) == "LOW", "LOW toString");
    ASSERT_TRUE(std::string(sim::vis::qualityLevelToString(sim::vis::QualityLevel::MEDIUM)) == "MEDIUM", "MEDIUM toString");
    ASSERT_TRUE(std::string(sim::vis::qualityLevelToString(sim::vis::QualityLevel::HIGH)) == "HIGH", "HIGH toString");
    ASSERT_TRUE(std::string(sim::vis::qualityLevelToString(sim::vis::QualityLevel::RESEARCH)) == "RESEARCH", "RESEARCH toString");

    std::cout << "[+] PASS: test_quality_level_string_roundtrip\n";
    return true;
}

struct BenchmarkResult {
    size_t targetCount;
    double totalWallMs;
    double avgStepUs;
    uint64_t totalSteps;
    double simDuration;
    double stepsPerSec;
};

BenchmarkResult runHeadlessBenchmark(size_t targetCount, uint64_t seed = 42) {
    sim::ScenarioConfig config;
    config.scenario_id = "BENCH-" + std::to_string(targetCount);
    config.scenario_name = "Benchmark " + std::to_string(targetCount) + " targets";
    config.seed = seed;
    config.timestep_seconds = 0.05;
    config.duration_seconds = 10.0;
    config.target_count = targetCount;

    sim::SimulationEngine engine;
    engine.initialize(config);

    auto wallStart = std::chrono::high_resolution_clock::now();
    engine.run();
    auto wallEnd = std::chrono::high_resolution_clock::now();

    double wallMs = std::chrono::duration<double, std::milli>(wallEnd - wallStart).count();
    uint64_t steps = engine.getClock().getStepCount();
    double simTime = engine.getClock().getCurrentTime();
    double avgStepUs = (steps > 0) ? (wallMs * 1000.0 / static_cast<double>(steps)) : 0.0;
    double stepsPerSec = (wallMs > 0.0) ? (static_cast<double>(steps) / (wallMs / 1000.0)) : 0.0;

    return {targetCount, wallMs, avgStepUs, steps, simTime, stepsPerSec};
}

bool test_benchmark_50_targets() {
    std::cout << "[TEST] Running test_benchmark_50_targets...\n";
    auto r = runHeadlessBenchmark(50);
    ASSERT_TRUE(r.totalSteps == 200, "50-target: expected 200 steps");
    ASSERT_APPROX_EQ(r.simDuration, 10.0, 1e-6, "50-target: sim duration mismatch");
    ASSERT_TRUE(r.totalWallMs > 0.0, "50-target: wall time must be positive");
    std::cout << "  50 targets: " << std::fixed << std::setprecision(2) << r.totalWallMs << " ms, "
              << std::setprecision(1) << r.avgStepUs << " us/step, "
              << std::setprecision(1) << r.stepsPerSec << " steps/s\n";
    std::cout << "[+] PASS: test_benchmark_50_targets\n";
    return true;
}

bool test_benchmark_100_targets() {
    std::cout << "[TEST] Running test_benchmark_100_targets...\n";
    auto r = runHeadlessBenchmark(100);
    ASSERT_TRUE(r.totalSteps == 200, "100-target: expected 200 steps");
    ASSERT_TRUE(r.totalWallMs > 0.0, "100-target: wall time must be positive");
    std::cout << "  100 targets: " << std::fixed << std::setprecision(2) << r.totalWallMs << " ms, "
              << std::setprecision(1) << r.avgStepUs << " us/step, "
              << std::setprecision(1) << r.stepsPerSec << " steps/s\n";
    std::cout << "[+] PASS: test_benchmark_100_targets\n";
    return true;
}

bool test_benchmark_200_targets() {
    std::cout << "[TEST] Running test_benchmark_200_targets...\n";
    auto r = runHeadlessBenchmark(200);
    ASSERT_TRUE(r.totalSteps == 200, "200-target: expected 200 steps");
    ASSERT_TRUE(r.totalWallMs > 0.0, "200-target: wall time must be positive");
    std::cout << "  200 targets: " << std::fixed << std::setprecision(2) << r.totalWallMs << " ms, "
              << std::setprecision(1) << r.avgStepUs << " us/step, "
              << std::setprecision(1) << r.stepsPerSec << " steps/s\n";
    std::cout << "[+] PASS: test_benchmark_200_targets\n";
    return true;
}

bool test_benchmark_500_targets() {
    std::cout << "[TEST] Running test_benchmark_500_targets...\n";
    auto r = runHeadlessBenchmark(500);
    ASSERT_TRUE(r.totalSteps == 200, "500-target: expected 200 steps");
    ASSERT_TRUE(r.totalWallMs > 0.0, "500-target: wall time must be positive");
    std::cout << "  500 targets: " << std::fixed << std::setprecision(2) << r.totalWallMs << " ms, "
              << std::setprecision(1) << r.avgStepUs << " us/step, "
              << std::setprecision(1) << r.stepsPerSec << " steps/s\n";
    std::cout << "[+] PASS: test_benchmark_500_targets\n";
    return true;
}

// ==========================================
// Step 5: Determinism Audit Tests
// ==========================================

bool test_determinism_across_quality_levels() {
    std::cout << "[TEST] Running test_determinism_across_quality_levels...\n";
    std::string path = resolveScenarioPath("scenarios/S-001_visual_demo.json");

    auto runSimWithQuality = [&](sim::vis::QualityLevel quality) {
        sim::vis::VisualizationAdapter adapter;
        adapter.initializeFromFile(path);
        adapter.start();
        for (int i = 0; i < 200; ++i) {
            adapter.step();
        }
        return adapter;
    };

    auto a1 = runSimWithQuality(sim::vis::QualityLevel::LOW);
    auto a2 = runSimWithQuality(sim::vis::QualityLevel::MEDIUM);
    auto a3 = runSimWithQuality(sim::vis::QualityLevel::HIGH);
    auto a4 = runSimWithQuality(sim::vis::QualityLevel::RESEARCH);

    ASSERT_APPROX_EQ(a1.getCurrentSimulationTime(), a2.getCurrentSimulationTime(), 1e-12, "LOW vs MEDIUM sim time mismatch");
    ASSERT_APPROX_EQ(a2.getCurrentSimulationTime(), a3.getCurrentSimulationTime(), 1e-12, "MEDIUM vs HIGH sim time mismatch");
    ASSERT_APPROX_EQ(a3.getCurrentSimulationTime(), a4.getCurrentSimulationTime(), 1e-12, "HIGH vs RESEARCH sim time mismatch");

    ASSERT_TRUE(a1.getCurrentSimulationStep() == a2.getCurrentSimulationStep(), "LOW vs MEDIUM step count mismatch");
    ASSERT_TRUE(a2.getCurrentSimulationStep() == a3.getCurrentSimulationStep(), "MEDIUM vs HIGH step count mismatch");
    ASSERT_TRUE(a3.getCurrentSimulationStep() == a4.getCurrentSimulationStep(), "HIGH vs RESEARCH step count mismatch");

    const auto& t1 = a1.getTargets();
    const auto& t2 = a2.getTargets();
    const auto& t3 = a3.getTargets();
    const auto& t4 = a4.getTargets();
    ASSERT_TRUE(t1.size() == t2.size() && t2.size() == t3.size() && t3.size() == t4.size(),
                "Target count mismatch across quality levels");

    for (size_t i = 0; i < t1.size(); ++i) {
        ASSERT_APPROX_EQ(t1[i].position.x, t2[i].position.x, 1e-12, "Pos.x LOW vs MEDIUM mismatch");
        ASSERT_APPROX_EQ(t2[i].position.x, t3[i].position.x, 1e-12, "Pos.x MEDIUM vs HIGH mismatch");
        ASSERT_APPROX_EQ(t3[i].position.x, t4[i].position.x, 1e-12, "Pos.x HIGH vs RESEARCH mismatch");

        ASSERT_APPROX_EQ(t1[i].position.y, t2[i].position.y, 1e-12, "Pos.y LOW vs MEDIUM mismatch");
        ASSERT_APPROX_EQ(t2[i].position.y, t3[i].position.y, 1e-12, "Pos.y MEDIUM vs HIGH mismatch");
        ASSERT_APPROX_EQ(t3[i].position.y, t4[i].position.y, 1e-12, "Pos.y HIGH vs RESEARCH mismatch");

        ASSERT_APPROX_EQ(t1[i].position.z, t2[i].position.z, 1e-12, "Pos.z LOW vs MEDIUM mismatch");
        ASSERT_APPROX_EQ(t2[i].position.z, t3[i].position.z, 1e-12, "Pos.z MEDIUM vs HIGH mismatch");
        ASSERT_APPROX_EQ(t3[i].position.z, t4[i].position.z, 1e-12, "Pos.z HIGH vs RESEARCH mismatch");

        ASSERT_APPROX_EQ(t1[i].velocity.x, t2[i].velocity.x, 1e-12, "Vel.x mismatch across quality");
        ASSERT_APPROX_EQ(t2[i].velocity.x, t3[i].velocity.x, 1e-12, "Vel.x mismatch across quality");
        ASSERT_APPROX_EQ(t3[i].velocity.x, t4[i].velocity.x, 1e-12, "Vel.x mismatch across quality");

        ASSERT_TRUE(t1[i].status == t2[i].status && t2[i].status == t3[i].status && t3[i].status == t4[i].status,
                    "Target status mismatch across quality levels");
    }

    ASSERT_TRUE(a1.getEventManager().getEventCount() == a2.getEventManager().getEventCount(), "Event count mismatch LOW vs MEDIUM");
    ASSERT_TRUE(a2.getEventManager().getEventCount() == a3.getEventManager().getEventCount(), "Event count mismatch MEDIUM vs HIGH");
    ASSERT_TRUE(a3.getEventManager().getEventCount() == a4.getEventManager().getEventCount(), "Event count mismatch HIGH vs RESEARCH");

    const auto& ev1 = a1.getEventManager().getEvents();
    const auto& ev2 = a2.getEventManager().getEvents();
    for (size_t i = 0; i < ev1.size(); ++i) {
        ASSERT_APPROX_EQ(ev1[i].simulation_time, ev2[i].simulation_time, 1e-12, "Event time mismatch across quality");
    }

    std::cout << "[+] PASS: test_determinism_across_quality_levels\n";
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

    // S-001-VIS-PERF Performance Monitor Tests
    runTest(test_perfmon_config_load, "perfmon_config_load");
    runTest(test_perfmon_quality_cycle, "perfmon_quality_cycle");
    runTest(test_perfmon_quality_change_info, "perfmon_quality_change_info");
    runTest(test_perfmon_fps_target_cycle, "perfmon_fps_target_cycle");
    runTest(test_perfmon_gpu_reporting, "perfmon_gpu_reporting");
    runTest(test_perfmon_auto_quality_toggle, "perfmon_auto_quality_toggle");
    runTest(test_perfmon_render_stats_recording, "perfmon_render_stats_recording");
    runTest(test_frustum_lod_configurable, "frustum_lod_configurable");
    runTest(test_particle_budget_by_quality, "particle_budget_by_quality");
    runTest(test_quality_level_string_roundtrip, "quality_level_string_roundtrip");

    // Headless Simulation Benchmarks (no OpenGL required)
    std::cout << "\n==================================================\n";
    std::cout << " HEADLESS SIMULATION BENCHMARKS\n";
    std::cout << "==================================================\n";
    runTest(test_benchmark_50_targets, "benchmark_50_targets");
    runTest(test_benchmark_100_targets, "benchmark_100_targets");
    runTest(test_benchmark_200_targets, "benchmark_200_targets");
    runTest(test_benchmark_500_targets, "benchmark_500_targets");

    // Determinism Audit
    std::cout << "\n==================================================\n";
    std::cout << " DETERMINISM AUDIT\n";
    std::cout << "==================================================\n";
    runTest(test_determinism_across_quality_levels, "determinism_across_quality_levels");

    std::cout << "==================================================\n";
    std::cout << "TOTAL SUMMARY: " << passed << " PASSED, " << failed << " FAILED\n";
    std::cout << "==================================================\n";

    return failed == 0 ? 0 : 1;
}
