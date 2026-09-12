#include "VisualizationAdapter.hpp"
#include "data/ScenarioLoader.hpp"
#include "core/Logger.hpp"
#include <algorithm>

namespace sim::vis {

VisualizationAdapter::VisualizationAdapter() = default;

bool VisualizationAdapter::initializeFromFile(const std::string& scenario_path) {
    m_scenarioPath = scenario_path;
    auto configOpt = ScenarioLoader::loadFromFile(scenario_path);
    if (!configOpt) {
        Logger::instance().error("VisualizationAdapter", 0.0, "Failed to load scenario: " + scenario_path);
        return false;
    }

    m_config = *configOpt;
    if (!m_engine.initialize(m_config)) {
        Logger::instance().error("VisualizationAdapter", 0.0, "Failed to initialize simulation engine");
        return false;
    }

    m_visualTargets.clear();
    const auto& simTargets = m_engine.getTargets();
    m_visualTargets.reserve(simTargets.size());

    for (const auto& st : simTargets) {
        TargetVisualState vState;
        vState.target_id = st.getId();
        vState.position = st.getPosition();
        vState.velocity = st.getVelocity();
        vState.speed = st.getVelocity().norm();
        vState.timestamp = st.getTimestamp();
        vState.status = TargetStatus::ACTIVE;
        vState.addTrailPoint(vState.position);
        m_visualTargets.push_back(std::move(vState));
    }

    m_interactionEntities.clear();
    m_interactionEntities.reserve(m_config.virtual_events.size());
    for (const auto& evCfg : m_config.virtual_events) {
        m_interactionEntities.emplace_back(evCfg);
    }

    m_eventManager.clear();
    m_visualEffects.clear();
    m_timeAccumulator = 0.0;
    m_options.status = PlaybackStatus::RESET;

    Logger::instance().info("VisualizationAdapter", 0.0, "VisualizationAdapter initialized successfully");
    return true;
}

void VisualizationAdapter::start() noexcept {
    if (!m_engine.isFinished()) {
        m_options.status = PlaybackStatus::RUNNING;
    }
}

void VisualizationAdapter::pause() noexcept {
    m_options.status = PlaybackStatus::PAUSED;
}

void VisualizationAdapter::togglePlayback() noexcept {
    if (m_options.status == PlaybackStatus::RUNNING) {
        pause();
    } else {
        start();
    }
}

void VisualizationAdapter::setPlaybackSpeed(double speed) noexcept {
    if (speed > 0.0) {
        m_options.speed_multiplier = speed;
    }
}

void VisualizationAdapter::toggleTrails() noexcept {
    m_options.show_trails = !m_options.show_trails;
}

void VisualizationAdapter::toggleGrid() noexcept {
    m_options.show_grid = !m_options.show_grid;
}

void VisualizationAdapter::toggleEvents() noexcept {
    m_options.show_events = !m_options.show_events;
}

void VisualizationAdapter::selectTarget(const std::string& target_id) {
    m_options.selected_target_id = target_id;
}

void VisualizationAdapter::clearSelection() {
    m_options.selected_target_id.clear();
}

void VisualizationAdapter::reset() {
    if (!m_engine.initialize(m_config)) {
        return;
    }

    m_eventManager.clear();
    m_visualEffects.clear();
    m_timeAccumulator = 0.0;
    m_options.status = PlaybackStatus::RESET;

    const auto& simTargets = m_engine.getTargets();
    for (size_t i = 0; i < simTargets.size() && i < m_visualTargets.size(); ++i) {
        m_visualTargets[i].position = simTargets[i].getPosition();
        m_visualTargets[i].velocity = simTargets[i].getVelocity();
        m_visualTargets[i].speed = simTargets[i].getVelocity().norm();
        m_visualTargets[i].timestamp = simTargets[i].getTimestamp();
        m_visualTargets[i].status = TargetStatus::ACTIVE;
        m_visualTargets[i].clearTrail();
        m_visualTargets[i].addTrailPoint(m_visualTargets[i].position);
    }

    for (auto& entity : m_interactionEntities) {
        entity.reset();
    }

    Logger::instance().info("VisualizationAdapter", 0.0, "Simulation & visualization state reset to initial conditions");
}

void VisualizationAdapter::syncFromEngineState() {
    const auto& simTargets = m_engine.getTargets();
    for (size_t i = 0; i < simTargets.size() && i < m_visualTargets.size(); ++i) {
        m_visualTargets[i].position = simTargets[i].getPosition();
        m_visualTargets[i].velocity = simTargets[i].getVelocity();
        m_visualTargets[i].speed = simTargets[i].getVelocity().norm();
        m_visualTargets[i].timestamp = simTargets[i].getTimestamp();
        m_visualTargets[i].addTrailPoint(m_visualTargets[i].position);
    }
}

void VisualizationAdapter::step() {
    if (m_engine.isFinished()) {
        m_options.status = PlaybackStatus::PAUSED;
        return;
    }

    m_engine.step();
    syncFromEngineState();

    double simTime = m_engine.getClock().getCurrentTime();
    double dt = m_engine.getClock().getTimestep();

    if (m_options.show_events) {
        for (auto& entity : m_interactionEntities) {
            entity.update(simTime, dt);

            if (entity.isActive()) {
                auto tgtIt = std::find_if(m_visualTargets.begin(), m_visualTargets.end(),
                    [&](const TargetVisualState& ts) {
                        return ts.target_id == entity.getTargetId();
                    });

                if (tgtIt != m_visualTargets.end()) {
                    if (entity.checkTrigger(tgtIt->position)) {
                        tgtIt->status = TargetStatus::INTERACTED;
                        std::string evId = entity.getEntityId() + "-EVT";
                        m_eventManager.addEvent(evId, simTime, entity.getEntityId(), tgtIt->target_id, entity.getEventType());

                        VisualEffect effect;
                        effect.position = tgtIt->position;
                        effect.start_time = simTime;
                        effect.duration = 1.2;
                        effect.max_radius = 280.0;
                        effect.active = true;
                        m_visualEffects.push_back(effect);
                    }
                }
            }
        }
    }

    // Update active visual effects
    for (auto& eff : m_visualEffects) {
        if (eff.active && (simTime - eff.start_time >= eff.duration)) {
            eff.active = false;
        }
    }
}

void VisualizationAdapter::update(double wall_dt) {
    if (m_options.status != PlaybackStatus::RUNNING) {
        return;
    }

    m_timeAccumulator += wall_dt * m_options.speed_multiplier;
    double simDt = m_engine.getClock().getTimestep();

    // Prevent spiral of death if frame rate drops
    constexpr double maxAccumulator = 0.5;
    if (m_timeAccumulator > maxAccumulator) {
        m_timeAccumulator = maxAccumulator;
    }

    while (m_timeAccumulator >= simDt && !m_engine.isFinished()) {
        step();
        m_timeAccumulator -= simDt;
    }

    if (m_engine.isFinished()) {
        m_options.status = PlaybackStatus::PAUSED;
    }
}

const std::vector<TargetVisualState>& VisualizationAdapter::getTargets() const noexcept {
    return m_visualTargets;
}

const std::vector<VirtualInteractionEntity>& VisualizationAdapter::getInteractionEntities() const noexcept {
    return m_interactionEntities;
}

const EventManager& VisualizationAdapter::getEventManager() const noexcept {
    return m_eventManager;
}

const std::vector<VisualEffect>& VisualizationAdapter::getVisualEffects() const noexcept {
    return m_visualEffects;
}

const PlaybackOptions& VisualizationAdapter::getPlaybackOptions() const noexcept {
    return m_options;
}

const sim::SimulationEngine& VisualizationAdapter::getEngine() const noexcept {
    return m_engine;
}

const TargetVisualState* VisualizationAdapter::getSelectedTarget() const noexcept {
    if (m_options.selected_target_id.empty()) {
        return nullptr;
    }
    for (const auto& t : m_visualTargets) {
        if (t.target_id == m_options.selected_target_id) {
            return &t;
        }
    }
    return nullptr;
}

size_t VisualizationAdapter::getActiveInteractionCount() const noexcept {
    size_t count = 0;
    for (const auto& e : m_interactionEntities) {
        if (e.isActive()) ++count;
    }
    return count;
}

double VisualizationAdapter::getCurrentSimulationTime() const noexcept {
    return m_engine.getClock().getCurrentTime();
}

uint64_t VisualizationAdapter::getCurrentSimulationStep() const noexcept {
    return m_engine.getClock().getStepCount();
}

} // namespace sim::vis
