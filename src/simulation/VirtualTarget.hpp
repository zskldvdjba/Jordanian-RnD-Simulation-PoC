#pragma once

#include "core/Types.hpp"

namespace sim {

class VirtualTarget {
public:
    VirtualTarget(std::string target_id,
                  double timestamp,
                  const Vector3D& position,
                  const Vector3D& velocity) noexcept;

    explicit VirtualTarget(const TargetState& state) noexcept;

    void update(double dt) noexcept;

    [[nodiscard]] const std::string& getId() const noexcept;
    [[nodiscard]] double getTimestamp() const noexcept;
    [[nodiscard]] const Vector3D& getPosition() const noexcept;
    [[nodiscard]] const Vector3D& getVelocity() const noexcept;

    [[nodiscard]] TargetState getState() const noexcept;

    void setVelocity(const Vector3D& v) noexcept;
    void setPosition(const Vector3D& p) noexcept;

private:
    std::string m_targetId;
    double m_timestamp{0.0};
    Vector3D m_position;
    Vector3D m_velocity;
};

} // namespace sim
