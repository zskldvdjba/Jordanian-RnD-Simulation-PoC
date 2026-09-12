#pragma once

#include <string>
#include <vector>
#include <cmath>
#include <cstdint>

namespace sim {

struct Vector3D {
    double x{0.0};
    double y{0.0};
    double z{0.0};

    constexpr Vector3D() noexcept = default;
    constexpr Vector3D(double px, double py, double pz) noexcept : x(px), y(py), z(pz) {}

    constexpr Vector3D operator+(const Vector3D& other) const noexcept {
        return {x + other.x, y + other.y, z + other.z};
    }

    constexpr Vector3D operator-(const Vector3D& other) const noexcept {
        return {x - other.x, y - other.y, z - other.z};
    }

    constexpr Vector3D operator*(double scalar) const noexcept {
        return {x * scalar, y * scalar, z * scalar};
    }

    Vector3D& operator+=(const Vector3D& other) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    [[nodiscard]] double normSquared() const noexcept {
        return x * x + y * y + z * z;
    }

    [[nodiscard]] double norm() const noexcept {
        return std::sqrt(normSquared());
    }

    [[nodiscard]] bool isApprox(const Vector3D& other, double eps = 1e-6) const noexcept {
        return (std::abs(x - other.x) <= eps) &&
               (std::abs(y - other.y) <= eps) &&
               (std::abs(z - other.z) <= eps);
    }
};

struct TargetState {
    std::string target_id;
    double timestamp{0.0};
    Vector3D position;
    Vector3D velocity;
};

struct ScenarioConfig {
    std::string scenario_id;
    std::string scenario_name;
    std::string description;
    uint64_t seed{42};
    double timestep_seconds{0.05};
    double duration_seconds{10.0};
    size_t target_count{0};
    std::vector<TargetState> targets;
};

} // namespace sim
