#pragma once

#include "VisualizationAdapter.hpp"
#include "Frustum.hpp"
#include "ParticleSystem.hpp"
#include "PerformanceMonitor.hpp"
#include <string>

namespace sim::vis {

class Renderer {
public:
    Renderer();

    void initialize(int width, int height);
    void resize(int width, int height);

    void render(const VisualizationAdapter& adapter, PerformanceMonitor& perf_monitor);

    void orbit(float delta_azimuth, float delta_elevation) noexcept;
    void pan(float delta_x, float delta_y) noexcept;
    void zoom(float factor) noexcept;
    void resetCamera() noexcept;

    [[nodiscard]] std::string pickTarget(int mouse_x, int mouse_y, const std::vector<TargetVisualState>& targets) const;

    [[nodiscard]] int getWidth() const noexcept { return m_width; }
    [[nodiscard]] int getHeight() const noexcept { return m_height; }

    [[nodiscard]] ParticleSystem& getParticleSystem() noexcept { return m_particleSystem; }

private:
    int m_width{1280};
    int m_height{720};

    // Camera parameters
    float m_azimuth{45.0f};
    float m_elevation{30.0f};
    float m_distance{22000.0f};
    Vector3D m_targetCenter{0.0, 0.0, 1500.0};
    Vector3D m_cameraPos{0.0, 0.0, 0.0};

    // Performance & Optimization
    Frustum m_frustum;
    ParticleSystem m_particleSystem;
    size_t m_drawCalls{0};

    void setup3DProjection();
    void setupLighting();
    void renderGrid(double size, double spacing, QualityLevel quality);
    void renderAxes(double length);
    void renderTarget(const TargetVisualState& target, bool isSelected, LodLevel lod, QualityLevel quality);
    void renderTrail(const TargetVisualState& target, QualityLevel quality);
    void renderInteractionEntity(const VirtualInteractionEntity& entity);
    void renderVisualEffect(const VisualEffect& effect, double sim_time);
};

} // namespace sim::vis
