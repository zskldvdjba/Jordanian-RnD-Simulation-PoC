#include "ParticleSystem.hpp"
#include <windows.h>
#include <GL/gl.h>
#include <cmath>
#include <algorithm>

namespace sim::vis {

ParticleSystem::ParticleSystem() {
    initialize();
}

void ParticleSystem::initialize() {
    m_smokePool.resize(MAX_SMOKE);
    m_exhaustPool.resize(MAX_EXHAUST);
    m_beamPool.resize(MAX_BEAM);
    m_eventPool.resize(MAX_EVENT);
    reset();
}

void ParticleSystem::reset() noexcept {
    for (auto& p : m_smokePool) p.active = false;
    for (auto& p : m_exhaustPool) p.active = false;
    for (auto& p : m_beamPool) p.active = false;
    for (auto& p : m_eventPool) p.active = false;
}

size_t ParticleSystem::getBudget(QualityLevel quality) const noexcept {
    switch (quality) {
        case QualityLevel::LOW:      return 185; // ~25%
        case QualityLevel::MEDIUM:   return 375; // ~50%
        case QualityLevel::HIGH:
        case QualityLevel::RESEARCH: return 750; // 100%
        default:                     return 375;
    }
}

size_t ParticleSystem::getActiveCount() const noexcept {
    size_t count = 0;
    for (const auto& p : m_smokePool) if (p.active) ++count;
    for (const auto& p : m_exhaustPool) if (p.active) ++count;
    for (const auto& p : m_beamPool) if (p.active) ++count;
    for (const auto& p : m_eventPool) if (p.active) ++count;
    return count;
}

void ParticleSystem::spawnInPool(std::vector<Particle>& pool, size_t activeBudget,
                                 const Vector3D& pos, const Vector3D& vel,
                                 float size, float lifetime,
                                 float r, float g, float b, float a) {
    size_t limit = std::min(pool.size(), activeBudget);

    // Find first inactive slot
    for (size_t i = 0; i < limit; ++i) {
        if (!pool[i].active) {
            pool[i].position = pos;
            pool[i].velocity = vel;
            pool[i].size = size;
            pool[i].life = 0.0f;
            pool[i].maxLife = (lifetime > 0.05f) ? lifetime : 1.0f;
            pool[i].r = r;
            pool[i].g = g;
            pool[i].b = b;
            pool[i].a = a;
            pool[i].active = true;
            return;
        }
    }
}

void ParticleSystem::spawnSmoke(const Vector3D& pos, const Vector3D& vel, float size, float lifetime) {
    spawnInPool(m_smokePool, MAX_SMOKE, pos, vel, size, lifetime, 0.7f, 0.75f, 0.8f, 0.45f);
}

void ParticleSystem::spawnExhaust(const Vector3D& pos, const Vector3D& vel, float size, float lifetime) {
    spawnInPool(m_exhaustPool, MAX_EXHAUST, pos, vel, size, lifetime, 1.0f, 0.65f, 0.2f, 0.7f);
}

void ParticleSystem::spawnBeam(const Vector3D& start, const Vector3D& end, float lifetime) {
    Vector3D mid = (start + end) * 0.5;
    Vector3D delta = end - start;
    spawnInPool(m_beamPool, MAX_BEAM, mid, delta * 0.1, 15.0f, lifetime, 0.2f, 0.8f, 1.0f, 0.9f);
}

void ParticleSystem::spawnEventBurst(const Vector3D& center, float radius, int count) {
    int actualCount = std::min(count, 35);
    for (int i = 0; i < actualCount; ++i) {
        double angle = (2.0 * 3.1415926535 * i) / actualCount;
        double speed = 120.0 + (i % 5) * 20.0;
        Vector3D vel{speed * std::cos(angle), speed * std::sin(angle), (i % 3 - 1) * 30.0};
        Vector3D pos = center + Vector3D{radius * 0.2 * std::cos(angle), radius * 0.2 * std::sin(angle), 0.0};
        spawnInPool(m_eventPool, MAX_EVENT, pos, vel, 25.0f, 0.9f, 1.0f, 0.4f, 0.15f, 0.85f);
    }
}

void ParticleSystem::update(double dt, QualityLevel quality) {
    float lifeMultiplier = 1.0f;
    if (quality == QualityLevel::LOW) lifeMultiplier = 1.6f; // Expires 60% faster on low

    auto updatePool = [&](std::vector<Particle>& pool) {
        for (auto& p : pool) {
            if (!p.active) continue;
            p.life += static_cast<float>(dt) * lifeMultiplier;
            if (p.life >= p.maxLife) {
                p.active = false;
            } else {
                p.position += p.velocity * dt;
                // Mild drag on particles
                p.velocity = p.velocity * 0.98;
            }
        }
    };

    updatePool(m_smokePool);
    updatePool(m_exhaustPool);
    updatePool(m_beamPool);
    updatePool(m_eventPool);
}

void ParticleSystem::renderPool(const std::vector<Particle>& pool, size_t activeBudget) {
    size_t limit = std::min(pool.size(), activeBudget);

    glBegin(GL_QUADS);
    for (size_t i = 0; i < limit; ++i) {
        const auto& p = pool[i];
        if (!p.active) continue;

        float progress = p.life / p.maxLife;
        float alpha = p.a * (1.0f - progress);
        float currentSize = p.size * (1.0f + progress * 0.5f);
        float halfS = currentSize * 0.5f;

        glColor4f(p.r, p.g, p.b, alpha);
        glVertex3d(p.position.x - halfS, p.position.y - halfS, p.position.z);
        glVertex3d(p.position.x + halfS, p.position.y - halfS, p.position.z);
        glVertex3d(p.position.x + halfS, p.position.y + halfS, p.position.z);
        glVertex3d(p.position.x - halfS, p.position.y + halfS, p.position.z);
    }
    glEnd();
}

void ParticleSystem::render(QualityLevel quality) {
    glDepthMask(GL_FALSE); // Don't write to depth buffer for transparent particles
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for energetic appearance

    size_t smokeBudget   = (quality == QualityLevel::LOW) ? 75  : ((quality == QualityLevel::MEDIUM) ? 150 : MAX_SMOKE);
    size_t exhaustBudget = (quality == QualityLevel::LOW) ? 50  : ((quality == QualityLevel::MEDIUM) ? 100 : MAX_EXHAUST);
    size_t beamBudget    = (quality == QualityLevel::LOW) ? 25  : ((quality == QualityLevel::MEDIUM) ? 50  : MAX_BEAM);
    size_t eventBudget   = (quality == QualityLevel::LOW) ? 35  : ((quality == QualityLevel::MEDIUM) ? 75  : MAX_EVENT);

    renderPool(m_smokePool, smokeBudget);
    renderPool(m_exhaustPool, exhaustBudget);
    renderPool(m_beamPool, beamBudget);
    renderPool(m_eventPool, eventBudget);

    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

} // namespace sim::vis
