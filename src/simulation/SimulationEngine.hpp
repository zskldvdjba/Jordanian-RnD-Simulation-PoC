#pragma once

#include "core/Types.hpp"
#include "core/SimulationClock.hpp"
#include "simulation/VirtualTarget.hpp"
#include "simulation/GroundTruthGenerator.hpp"
#include "metrics/MetricsCollector.hpp"
#include <vector>
#include <string>

namespace sim {

class SimulationEngine {
public:
    SimulationEngine() = default;

    bool initialize(const ScenarioConfig& config);
    bool initializeFromFile(const std::string& scenario_path);

    void step();
    void run();

    [[nodiscard]] bool isFinished() const noexcept;

    [[nodiscard]] const SimulationClock& getClock() const noexcept;
    [[nodiscard]] const std::vector<VirtualTarget>& getTargets() const noexcept;
    [[nodiscard]] const MetricsCollector& getMetricsCollector() const noexcept;
    [[nodiscard]] const ScenarioConfig& getConfig() const noexcept;

private:
    ScenarioConfig m_config;
    SimulationClock m_clock;
    GroundTruthGenerator m_gtGenerator;
    std::vector<VirtualTarget> m_targets;
    MetricsCollector m_metricsCollector;
    bool m_initialized{false};
};

} // namespace sim
