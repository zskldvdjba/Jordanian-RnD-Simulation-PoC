#pragma once

#include "core/Types.hpp"
#include "VisualizationState.hpp"
#include <string>

namespace sim::vis {

class VirtualInteractionEntity {
public:
    explicit VirtualInteractionEntity(VirtualEventConfig config) noexcept;

    void update(double current_sim_time, double dt) noexcept;
    void reset() noexcept;

    [[nodiscard]] bool checkTrigger(const Vector3D& target_pos) noexcept;

    [[nodiscard]] const std::string& getEntityId() const noexcept { return m_config.event_entity_id; }
    [[nodiscard]] const std::string& getTargetId() const noexcept { return m_config.target_id; }
    [[nodiscard]] const Vector3D& getPosition() const noexcept { return m_position; }
    [[nodiscard]] const Vector3D& getVelocity() const noexcept { return m_velocity; }
    [[nodiscard]] const std::string& getEventType() const noexcept { return m_config.event_type; }
    [[nodiscard]] double getSpawnTime() const noexcept { return m_config.spawn_time; }
    [[nodiscard]] double getLifetime() const noexcept { return m_config.lifetime; }
    [[nodiscard]] double getTriggerThreshold() const noexcept { return m_config.event_trigger_threshold; }

    [[nodiscard]] bool isActive() const noexcept { return m_active; }
    [[nodiscard]] bool isTriggered() const noexcept { return m_triggered; }

private:
    VirtualEventConfig m_config;
    Vector3D m_position;
    Vector3D m_velocity;
    bool m_active{false};
    bool m_triggered{false};
    double m_timeActive{0.0};
};

} // namespace sim::vis
