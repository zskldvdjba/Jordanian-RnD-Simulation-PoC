#include "VirtualInteraction.hpp"
#include <utility>

namespace sim::vis {

VirtualInteractionEntity::VirtualInteractionEntity(VirtualEventConfig config) noexcept
    : m_config(std::move(config)),
      m_position(m_config.position),
      m_velocity(m_config.velocity) {}

void VirtualInteractionEntity::update(double current_sim_time, double dt) noexcept {
    if (m_triggered) {
        m_active = false;
        return;
    }

    if (current_sim_time >= m_config.spawn_time &&
        current_sim_time < (m_config.spawn_time + m_config.lifetime)) {
        m_active = true;
        m_timeActive += dt;
        m_position += m_velocity * dt;
    } else {
        m_active = false;
    }
}

bool VirtualInteractionEntity::checkTrigger(const Vector3D& target_pos) noexcept {
    if (!m_active || m_triggered) {
        return false;
    }

    double dist = m_position.distanceTo(target_pos);
    if (dist <= m_config.event_trigger_threshold) {
        m_triggered = true;
        m_active = false;
        return true;
    }
    return false;
}

void VirtualInteractionEntity::reset() noexcept {
    m_position = m_config.position;
    m_velocity = m_config.velocity;
    m_active = false;
    m_triggered = false;
    m_timeActive = 0.0;
}

} // namespace sim::vis
