#pragma once

#include "core/Types.hpp"
#include "simulation/SimulationEngine.hpp"
#include "VisualizationState.hpp"
#include "VirtualInteraction.hpp"
#include "EventManager.hpp"
#include <vector>
#include <string>
#include <memory>

namespace sim::vis {

class VisualizationAdapter {
public:
    VisualizationAdapter();

    bool initializeFromFile(const std::string& scenario_path);

    void start() noexcept;
    void pause() noexcept;
    void reset();
    void togglePlayback() noexcept;
    void setPlaybackSpeed(double speed) noexcept;

    void toggleTrails() noexcept;
    void toggleGrid() noexcept;
    void toggleEvents() noexcept;

    void selectTarget(const std::string& target_id);
    void clearSelection();

    void step();
    void update(double wall_dt);

    [[nodiscard]] const std::vector<TargetVisualState>& getTargets() const noexcept;
    [[nodiscard]] const std::vector<VirtualInteractionEntity>& getInteractionEntities() const noexcept;
    [[nodiscard]] const EventManager& getEventManager() const noexcept;
    [[nodiscard]] const std::vector<VisualEffect>& getVisualEffects() const noexcept;
    [[nodiscard]] const PlaybackOptions& getPlaybackOptions() const noexcept;
    [[nodiscard]] const sim::SimulationEngine& getEngine() const noexcept;
    [[nodiscard]] const TargetVisualState* getSelectedTarget() const noexcept;

    [[nodiscard]] size_t getActiveInteractionCount() const noexcept;
    [[nodiscard]] double getCurrentSimulationTime() const noexcept;
    [[nodiscard]] uint64_t getCurrentSimulationStep() const noexcept;

private:
    sim::SimulationEngine m_engine;
    sim::ScenarioConfig m_config;
    std::vector<TargetVisualState> m_visualTargets;
    std::vector<VirtualInteractionEntity> m_interactionEntities;
    EventManager m_eventManager;
    std::vector<VisualEffect> m_visualEffects;
    PlaybackOptions m_options;

    double m_timeAccumulator{0.0};
    std::string m_scenarioPath;

    void syncFromEngineState();
};

} // namespace sim::vis
