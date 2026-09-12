#include "VirtualTarget.hpp"
#include <utility>

namespace sim {

VirtualTarget::VirtualTarget(std::string target_id,
                             double timestamp,
                             const Vector3D& position,
                             const Vector3D& velocity) noexcept
    : m_targetId(std::move(target_id)),
      m_timestamp(timestamp),
      m_position(position),
      m_velocity(velocity) {}

VirtualTarget::VirtualTarget(const TargetState& state) noexcept
    : m_targetId(state.target_id),
      m_timestamp(state.timestamp),
      m_position(state.position),
      m_velocity(state.velocity) {}

void VirtualTarget::update(double dt) noexcept {
    m_position += m_velocity * dt;
    m_timestamp += dt;
}

const std::string& VirtualTarget::getId() const noexcept {
    return m_targetId;
}

double VirtualTarget::getTimestamp() const noexcept {
    return m_timestamp;
}

const Vector3D& VirtualTarget::getPosition() const noexcept {
    return m_position;
}

const Vector3D& VirtualTarget::getVelocity() const noexcept {
    return m_velocity;
}

TargetState VirtualTarget::getState() const noexcept {
    return TargetState{
        .target_id = m_targetId,
        .timestamp = m_timestamp,
        .position = m_position,
        .velocity = m_velocity
    };
}

void VirtualTarget::setVelocity(const Vector3D& v) noexcept {
    m_velocity = v;
}

void VirtualTarget::setPosition(const Vector3D& p) noexcept {
    m_position = p;
}

} // namespace sim
