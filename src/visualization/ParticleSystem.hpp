#pragma once

#include "core/Types.hpp"
#include "PerformanceMonitor.hpp"
#include <vector>
#include <cstddef>

namespace sim::vis {

enum class ParticleType {
    SMOKE,
    EXHAUST,
    BEAM,
    EVENT
};

struct Particle {
    Vector3D position;
    Vector3D velocity;
    float life{0.0f};
    float maxLife{1.0f};
    float size{10.0f};
    float r{1.0f};
    float g{1.0f};
    float b{1.0f};
    float a{1.0f};
    bool active{false};
};

class ParticleSystem {
public:
    ParticleSystem();

    void initialize();
    void reset() noexcept;

    void update(double dt, QualityLevel quality);

    void spawnSmoke(const Vector3D& pos, const Vector3D& vel, float size = 40.0f, float lifetime = 1.2f);
    void spawnExhaust(const Vector3D& pos, const Vector3D& vel, float size = 25.0f, float lifetime = 0.8f);
    void spawnBeam(const Vector3D& start, const Vector3D& end, float lifetime = 0.4f);
    void spawnEventBurst(const Vector3D& center, float radius, int count = 25);

    void render(QualityLevel quality);

    [[nodiscard]] size_t getActiveCount() const noexcept;
    [[nodiscard]] size_t getBudget(QualityLevel quality) const noexcept;

private:
    // Fixed pre-allocated particle pools
    static constexpr size_t MAX_SMOKE = 300;
    static constexpr size_t MAX_EXHAUST = 200;
    static constexpr size_t MAX_BEAM = 100;
    static constexpr size_t MAX_EVENT = 150;

    std::vector<Particle> m_smokePool;
    std::vector<Particle> m_exhaustPool;
    std::vector<Particle> m_beamPool;
    std::vector<Particle> m_eventPool;

    void spawnInPool(std::vector<Particle>& pool, size_t activeBudget,
                     const Vector3D& pos, const Vector3D& vel,
                     float size, float lifetime,
                     float r, float g, float b, float a);

    void renderPool(const std::vector<Particle>& pool, size_t activeBudget);
};

} // namespace sim::vis
