#pragma once

#include "VisualizationAdapter.hpp"
#include <string>

namespace sim::vis {

class Renderer {
public:
    Renderer();

    void initialize(int width, int height);
    void resize(int width, int height);

    void render(const VisualizationAdapter& adapter);

    void orbit(float delta_azimuth, float delta_elevation) noexcept;
    void pan(float delta_x, float delta_y) noexcept;
    void zoom(float factor) noexcept;
    void resetCamera() noexcept;

    [[nodiscard]] std::string pickTarget(int mouse_x, int mouse_y, const std::vector<TargetVisualState>& targets) const;

    [[nodiscard]] int getWidth() const noexcept { return m_width; }
    [[nodiscard]] int getHeight() const noexcept { return m_height; }

private:
    int m_width{1280};
    int m_height{720};

    // Camera parameters
    float m_azimuth{45.0f};
    float m_elevation{30.0f};
    float m_distance{22000.0f};
    Vector3D m_targetCenter{0.0, 0.0, 1500.0};

    void setup3DProjection();
    void renderGrid(double size, double spacing);
    void renderAxes(double length);
    void renderTarget(const TargetVisualState& target, bool isSelected);
    void renderTrail(const TargetVisualState& target);
    void renderInteractionEntity(const VirtualInteractionEntity& entity);
    void renderVisualEffect(const VisualEffect& effect, double sim_time);
};

} // namespace sim::vis
