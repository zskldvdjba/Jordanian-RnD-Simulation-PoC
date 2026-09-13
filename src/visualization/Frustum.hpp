#pragma once

#include "core/Types.hpp"

namespace sim::vis {

struct Plane {
    double a{0.0};
    double b{0.0};
    double c{0.0};
    double d{0.0};

    void normalize() noexcept;
    [[nodiscard]] double distanceToPoint(const Vector3D& pt) const noexcept;
};

enum class LodLevel {
    HIGH = 0,
    MEDIUM = 1,
    LOW = 2
};

class Frustum {
public:
    Frustum() = default;

    void updateFromMatrices(const double* modelview, const double* projection) noexcept;

    [[nodiscard]] bool isSphereInside(const Vector3D& center, double radius) const noexcept;

    [[nodiscard]] LodLevel getLodForDistance(double distance) const noexcept;

    void setLodDistances(double nearDist, double farDist) noexcept;

private:
    Plane m_planes[6]; // Left, Right, Bottom, Top, Near, Far
    double m_lodNear{5000.0};
    double m_lodFar{15000.0};
};

} // namespace sim::vis
