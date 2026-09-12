#include "SimulationEngine.hpp"
#include "data/ScenarioLoader.hpp"
#include "core/Logger.hpp"
#include <sstream>

namespace sim {

bool SimulationEngine::initialize(const ScenarioConfig& config) {
    m_config = config;
    m_clock = SimulationClock(m_config.timestep_seconds, 0.0);
    m_gtGenerator.setSeed(m_config.seed);
    m_metricsCollector.reset();
    m_targets.clear();

    if (!m_config.targets.empty()) {
        m_targets.reserve(m_config.targets.size());
        for (const auto& state : m_config.targets) {
            m_targets.emplace_back(state);
        }
    } else if (m_config.target_count > 0) {
        m_targets = m_gtGenerator.generateSyntheticTargets(m_config.target_count, 0.0);
    }

    m_initialized = true;

    std::ostringstream msg;
    msg << "SimulationEngine initialized with scenario '" << m_config.scenario_id
        << "', targets=" << m_targets.size()
        << ", dt=" << m_config.timestep_seconds
        << "s, duration=" << m_config.duration_seconds
        << "s, seed=" << m_config.seed;
    Logger::instance().info("SimulationEngine", m_clock.getCurrentTime(), msg.str());

    // Record initial step state
    m_metricsCollector.recordStep(m_clock.getCurrentTime(), m_targets);

    return true;
}

bool SimulationEngine::initializeFromFile(const std::string& scenario_path) {
    auto configOpt = ScenarioLoader::loadFromFile(scenario_path);
    if (!configOpt) {
        Logger::instance().error("SimulationEngine", 0.0, "Failed to load scenario configuration from " + scenario_path);
        return false;
    }
    return initialize(*configOpt);
}

void SimulationEngine::step() {
    if (!m_initialized || isFinished()) {
        return;
    }

    double dt = m_clock.getTimestep();
    m_clock.advance();
    double current_time = m_clock.getCurrentTime();

    for (auto& target : m_targets) {
        target.update(dt);
    }

    m_metricsCollector.recordStep(current_time, m_targets);

    // Periodic structured log every 1 second of simulation time
    uint64_t stepCount = m_clock.getStepCount();
    uint64_t stepsPerSec = static_cast<uint64_t>(std::round(1.0 / dt));
    if (stepsPerSec > 0 && (stepCount % stepsPerSec == 0)) {
        std::ostringstream msg;
        msg << "Step " << stepCount << " completed. Active targets: " << m_targets.size();
        Logger::instance().debug("SimulationEngine", current_time, msg.str());
    }
}

void SimulationEngine::run() {
    if (!m_initialized) {
        return;
    }

    Logger::instance().info("SimulationEngine", m_clock.getCurrentTime(), "Beginning deterministic simulation run...");
    while (!isFinished()) {
        step();
    }
    Logger::instance().info("SimulationEngine", m_clock.getCurrentTime(), "Deterministic simulation run finished.");
}

bool SimulationEngine::isFinished() const noexcept {
    return m_clock.getCurrentTime() >= (m_config.duration_seconds - 1e-9);
}

const SimulationClock& SimulationEngine::getClock() const noexcept {
    return m_clock;
}

const std::vector<VirtualTarget>& SimulationEngine::getTargets() const noexcept {
    return m_targets;
}

const MetricsCollector& SimulationEngine::getMetricsCollector() const noexcept {
    return m_metricsCollector;
}

const ScenarioConfig& SimulationEngine::getConfig() const noexcept {
    return m_config;
}

} // namespace sim
