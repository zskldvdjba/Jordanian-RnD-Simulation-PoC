#include "Renderer.hpp"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace sim::vis {

Renderer::Renderer() = default;

void Renderer::initialize(int width, int height) {
    m_width = (width > 0) ? width : 1280;
    m_height = (height > 0) ? height : 720;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glClearColor(0.06f, 0.08f, 0.11f, 1.0f);
}

void Renderer::resize(int width, int height) {
    m_width = (width > 0) ? width : 1;
    m_height = (height > 0) ? height : 1;
    glViewport(0, 0, m_width, m_height);
}

void Renderer::orbit(float delta_azimuth, float delta_elevation) noexcept {
    m_azimuth += delta_azimuth;
    m_elevation += delta_elevation;
    if (m_elevation > 89.0f) m_elevation = 89.0f;
    if (m_elevation < -89.0f) m_elevation = -89.0f;
}

void Renderer::pan(float delta_x, float delta_y) noexcept {
    float radAz = static_cast<float>(m_azimuth * M_PI / 180.0);
    float panSpeed = m_distance * 0.001f;

    // Camera coordinate frame
    float rightX = std::cos(radAz);
    float rightY = -std::sin(radAz);

    float forwardX = -std::sin(radAz);
    float forwardY = -std::cos(radAz);

    m_targetCenter.x += (-delta_x * rightX + delta_y * forwardX) * panSpeed;
    m_targetCenter.y += (-delta_x * rightY + delta_y * forwardY) * panSpeed;
}

void Renderer::zoom(float factor) noexcept {
    m_distance *= factor;
    if (m_distance < 1000.0f) m_distance = 1000.0f;
    if (m_distance > 100000.0f) m_distance = 100000.0f;
}

void Renderer::resetCamera() noexcept {
    m_azimuth = 45.0f;
    m_elevation = 30.0f;
    m_distance = 22000.0f;
    m_targetCenter = Vector3D{0.0, 0.0, 1500.0};
}

void Renderer::setup3DProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double aspect = static_cast<double>(m_width) / static_cast<double>(m_height);
    gluPerspective(45.0, aspect, 100.0, 300000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    double radAz = m_azimuth * M_PI / 180.0;
    double radEl = m_elevation * M_PI / 180.0;

    double eyeX = m_targetCenter.x + m_distance * std::cos(radEl) * std::sin(radAz);
    double eyeY = m_targetCenter.y - m_distance * std::cos(radEl) * std::cos(radAz);
    double eyeZ = m_targetCenter.z + m_distance * std::sin(radEl);

    gluLookAt(eyeX, eyeY, eyeZ,
              m_targetCenter.x, m_targetCenter.y, m_targetCenter.z,
              0.0, 0.0, 1.0);
}

void Renderer::renderGrid(double size, double spacing) {
    glLineWidth(1.0f);
    glColor4f(0.18f, 0.22f, 0.28f, 0.6f);

    glBegin(GL_LINES);
    for (double x = -size; x <= size; x += spacing) {
        glVertex3d(x, -size, 0.0);
        glVertex3d(x, size, 0.0);
    }
    for (double y = -size; y <= size; y += spacing) {
        glVertex3d(-size, y, 0.0);
        glVertex3d(size, y, 0.0);
    }
    glEnd();
}

void Renderer::renderAxes(double length) {
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    // X Axis - Red
    glColor4f(0.9f, 0.2f, 0.2f, 1.0f);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(length, 0.0, 0.0);

    // Y Axis - Green
    glColor4f(0.2f, 0.9f, 0.2f, 1.0f);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(0.0, length, 0.0);

    // Z Axis - Blue
    glColor4f(0.2f, 0.4f, 0.9f, 1.0f);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(0.0, 0.0, length);
    glEnd();
}

void Renderer::renderTrail(const TargetVisualState& target) {
    if (target.trail.size() < 2) return;

    glLineWidth(1.5f);
    glBegin(GL_LINE_STRIP);
    size_t count = target.trail.size();
    for (size_t i = 0; i < count; ++i) {
        float alpha = static_cast<float>(i + 1) / static_cast<float>(count) * 0.7f;
        if (target.status == TargetStatus::INTERACTED) {
            glColor4f(0.95f, 0.5f, 0.1f, alpha);
        } else {
            glColor4f(0.2f, 0.75f, 0.9f, alpha);
        }
        const auto& pt = target.trail[i];
        glVertex3d(pt.x, pt.y, pt.z);
    }
    glEnd();
}

void Renderer::renderTarget(const TargetVisualState& target, bool isSelected) {
    const auto& pos = target.position;
    const auto& vel = target.velocity;

    // Draw velocity vector line
    double velScale = 4.0; // 4 seconds of projected motion
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    if (isSelected) {
        glColor4f(1.0f, 0.95f, 0.2f, 0.9f);
    } else if (target.status == TargetStatus::INTERACTED) {
        glColor4f(1.0f, 0.5f, 0.1f, 0.8f);
    } else {
        glColor4f(0.2f, 0.8f, 1.0f, 0.7f);
    }
    glVertex3d(pos.x, pos.y, pos.z);
    glVertex3d(pos.x + vel.x * velScale, pos.y + vel.y * velScale, pos.z + vel.z * velScale);
    glEnd();

    // Draw 3D diamond/octahedron marker
    double r = isSelected ? 180.0 : 120.0;
    glPushMatrix();
    glTranslated(pos.x, pos.y, pos.z);

    if (isSelected) {
        glColor4f(1.0f, 0.95f, 0.2f, 1.0f);
    } else if (target.status == TargetStatus::INTERACTED) {
        glColor4f(1.0f, 0.5f, 0.1f, 0.95f);
    } else {
        glColor4f(0.2f, 0.8f, 1.0f, 0.9f);
    }

    glBegin(GL_TRIANGLE_FAN);
    glVertex3d(0, 0, r);
    glVertex3d(r, 0, 0);
    glVertex3d(0, r, 0);
    glVertex3d(-r, 0, 0);
    glVertex3d(0, -r, 0);
    glVertex3d(r, 0, 0);
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glVertex3d(0, 0, -r);
    glVertex3d(r, 0, 0);
    glVertex3d(0, -r, 0);
    glVertex3d(-r, 0, 0);
    glVertex3d(0, r, 0);
    glVertex3d(r, 0, 0);
    glEnd();

    // Selection ring / halo
    if (isSelected) {
        glLineWidth(2.0f);
        glColor4f(1.0f, 1.0f, 0.3f, 0.9f);
        glBegin(GL_LINE_LOOP);
        constexpr int segments = 24;
        double ringRadius = r * 1.8;
        for (int i = 0; i < segments; ++i) {
            double theta = 2.0 * M_PI * i / segments;
            glVertex3d(ringRadius * std::cos(theta), ringRadius * std::sin(theta), 0);
        }
        glEnd();
    }

    glPopMatrix();
}

void Renderer::renderInteractionEntity(const VirtualInteractionEntity& entity) {
    if (!entity.isActive()) return;

    const auto& pos = entity.getPosition();
    const auto& vel = entity.getVelocity();

    // Draw magenta trail vector
    glLineWidth(2.0f);
    glColor4f(0.95f, 0.2f, 0.85f, 0.8f);
    glBegin(GL_LINES);
    glVertex3d(pos.x, pos.y, pos.z);
    glVertex3d(pos.x - vel.x * 0.8, pos.y - vel.y * 0.8, pos.z - vel.z * 0.8);
    glEnd();

    // Draw magenta diamond
    double r = 160.0;
    glPushMatrix();
    glTranslated(pos.x, pos.y, pos.z);
    glColor4f(1.0f, 0.25f, 0.9f, 1.0f);

    glBegin(GL_TRIANGLE_FAN);
    glVertex3d(0, 0, r);
    glVertex3d(r, 0, 0);
    glVertex3d(0, r, 0);
    glVertex3d(-r, 0, 0);
    glVertex3d(0, -r, 0);
    glVertex3d(r, 0, 0);
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glVertex3d(0, 0, -r);
    glVertex3d(r, 0, 0);
    glVertex3d(0, -r, 0);
    glVertex3d(-r, 0, 0);
    glVertex3d(0, r, 0);
    glVertex3d(r, 0, 0);
    glEnd();
    glPopMatrix();
}

void Renderer::renderVisualEffect(const VisualEffect& effect, double sim_time) {
    if (!effect.active) return;

    double p = effect.getProgress(sim_time);
    if (p >= 1.0) return;

    double currentR = effect.max_radius * std::sin(p * M_PI * 0.5);
    float alpha = static_cast<float>(1.0 - p) * 0.85f;

    glLineWidth(2.5f);
    glColor4f(1.0f, 0.6f, 0.2f, alpha);

    glPushMatrix();
    glTranslated(effect.position.x, effect.position.y, effect.position.z);

    glBegin(GL_LINE_LOOP);
    constexpr int segments = 32;
    for (int i = 0; i < segments; ++i) {
        double theta = 2.0 * M_PI * i / segments;
        glVertex3d(currentR * std::cos(theta), currentR * std::sin(theta), 0.0);
    }
    glEnd();

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; ++i) {
        double theta = 2.0 * M_PI * i / segments;
        glVertex3d(currentR * std::cos(theta), 0.0, currentR * std::sin(theta));
    }
    glEnd();

    glPopMatrix();
}

void Renderer::render(const VisualizationAdapter& adapter) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    setup3DProjection();

    const auto& options = adapter.getPlaybackOptions();
    double simTime = adapter.getCurrentSimulationTime();

    if (options.show_grid) {
        renderGrid(20000.0, 2000.0);
        renderAxes(5000.0);
    }

    const auto& targets = adapter.getTargets();
    for (const auto& tgt : targets) {
        if (options.show_trails) {
            renderTrail(tgt);
        }
        bool isSelected = (tgt.target_id == options.selected_target_id);
        renderTarget(tgt, isSelected);
    }

    if (options.show_events) {
        for (const auto& entity : adapter.getInteractionEntities()) {
            renderInteractionEntity(entity);
        }
    }

    for (const auto& eff : adapter.getVisualEffects()) {
        renderVisualEffect(eff, simTime);
    }
}

std::string Renderer::pickTarget(int mouse_x, int mouse_y, const std::vector<TargetVisualState>& targets) const {
    GLdouble modelview[16];
    GLdouble projection[16];
    GLint viewport[4];

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    std::string closestId;
    double minDistanceSq = 25.0 * 25.0; // 25 pixel radius threshold

    for (const auto& tgt : targets) {
        GLdouble winX, winY, winZ;
        if (gluProject(tgt.position.x, tgt.position.y, tgt.position.z,
                       modelview, projection, viewport,
                       &winX, &winY, &winZ) == GL_TRUE) {
            // Check if in front of near plane
            if (winZ < 0.0 || winZ > 1.0) continue;

            double screenY = viewport[3] - winY; // OpenGL y-inversion
            double dx = winX - mouse_x;
            double dy = screenY - mouse_y;
            double distSq = dx * dx + dy * dy;

            if (distSq < minDistanceSq) {
                minDistanceSq = distSq;
                closestId = tgt.target_id;
            }
        }
    }

    return closestId;
}

} // namespace sim::vis
