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

    setupLighting();
    m_particleSystem.initialize();
    m_assetManager.initialize();
}

void Renderer::setupLighting() {
    GLfloat lightPos[] = {0.5f, 0.7f, 1.0f, 0.0f}; // Directional sun/sky light
    GLfloat lightAmbient[] = {0.35f, 0.38f, 0.45f, 1.0f};
    GLfloat lightDiffuse[] = {0.8f, 0.85f, 0.9f, 1.0f};
    GLfloat lightSpecular[] = {0.4f, 0.45f, 0.5f, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
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

    m_cameraPos = Vector3D{eyeX, eyeY, eyeZ};

    gluLookAt(eyeX, eyeY, eyeZ,
              m_targetCenter.x, m_targetCenter.y, m_targetCenter.z,
              0.0, 0.0, 1.0);
}

void Renderer::renderGrid(double size, double spacing, QualityLevel quality) {
    glDisable(GL_LIGHTING);
    glLineWidth(1.0f);
    glColor4f(0.18f, 0.22f, 0.28f, 0.6f);

    double actualSpacing = (quality == QualityLevel::LOW) ? (spacing * 2.0) : spacing;

    glBegin(GL_LINES);
    for (double x = -size; x <= size; x += actualSpacing) {
        glVertex3d(x, -size, 0.0);
        glVertex3d(x, size, 0.0);
    }
    for (double y = -size; y <= size; y += actualSpacing) {
        glVertex3d(-size, y, 0.0);
        glVertex3d(size, y, 0.0);
    }
    glEnd();
    ++m_drawCalls;

    // Optional contour ring for high/research quality
    if (quality >= QualityLevel::HIGH) {
        glColor4f(0.15f, 0.28f, 0.38f, 0.4f);
        glBegin(GL_LINE_LOOP);
        constexpr int segs = 32;
        for (int i = 0; i < segs; ++i) {
            double th = 2.0 * M_PI * i / segs;
            glVertex3d(size * 0.7 * std::cos(th), size * 0.7 * std::sin(th), 0.0);
        }
        glEnd();
        ++m_drawCalls;
    }
}

void Renderer::renderAxes(double length) {
    glDisable(GL_LIGHTING);
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
    ++m_drawCalls;
}

void Renderer::renderTrail(const TargetVisualState& target, QualityLevel quality) {
    if (target.trail.size() < 2) return;

    glDisable(GL_LIGHTING);
    glLineWidth(1.5f);
    glBegin(GL_LINE_STRIP);

    size_t count = target.trail.size();
    size_t step = (quality == QualityLevel::LOW) ? 2 : 1;

    for (size_t i = 0; i < count; i += step) {
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
    ++m_drawCalls;
}

void Renderer::renderTarget(const TargetVisualState& target, bool isSelected, LodLevel lod, QualityLevel quality) {
    const auto& pos = target.position;
    const auto& vel = target.velocity;

    if (lod != LodLevel::LOW) {
        glDisable(GL_LIGHTING);
        double velScale = 4.0;
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
        ++m_drawCalls;
    }

    float assetScale = isSelected ? 1.4f : 1.0f;

    if (lod == LodLevel::LOW) {
        double r = isSelected ? 108.0 : 72.0;
        glPushMatrix();
        glTranslated(pos.x, pos.y, pos.z);

        if (quality >= QualityLevel::HIGH) {
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
        } else {
            glDisable(GL_LIGHTING);
        }

        if (isSelected) {
            glColor4f(1.0f, 0.95f, 0.2f, 1.0f);
        } else if (target.status == TargetStatus::INTERACTED) {
            glColor4f(1.0f, 0.5f, 0.1f, 0.95f);
        } else {
            glColor4f(0.2f, 0.8f, 1.0f, 0.9f);
        }

        double s = r * 0.6;
        glBegin(GL_TRIANGLES);
        glVertex3d(0, 0, s);
        glVertex3d(s, 0, -s * 0.5);
        glVertex3d(-s, 0, -s * 0.5);
        glVertex3d(0, 0, s);
        glVertex3d(0, s, -s * 0.5);
        glVertex3d(s, 0, -s * 0.5);
        glVertex3d(0, 0, s);
        glVertex3d(-s, 0, -s * 0.5);
        glVertex3d(0, s, -s * 0.5);
        glVertex3d(0, s, -s * 0.5);
        glVertex3d(0, 0, s);
        glVertex3d(0, -s, -s * 0.5);
        glEnd();
        ++m_drawCalls;

        if (isSelected && lod != LodLevel::LOW) {
            glDisable(GL_LIGHTING);
            glLineWidth(2.0f);
            glColor4f(1.0f, 1.0f, 0.3f, 0.9f);
            double ringRadius = r * 1.8;
            glBegin(GL_LINE_LOOP);
            constexpr int segments = 24;
            for (int i = 0; i < segments; ++i) {
                double theta = 2.0 * M_PI * i / segments;
                glVertex3d(ringRadius * std::cos(theta), ringRadius * std::sin(theta), 0);
            }
            glEnd();
            ++m_drawCalls;
        }

        glPopMatrix();
    } else {
        const auto& lpMesh = m_assetManager.getMesh(AssetType::LAUNCHER_PLATFORM, assetScale);

        if (isSelected) {
            glPushMatrix();
            glTranslated(pos.x, pos.y, pos.z);
            glDisable(GL_LIGHTING);
            glLineWidth(2.0f);
            glColor4f(1.0f, 1.0f, 0.3f, 0.9f);
            double ringRadius = lpMesh.boundingRadius * assetScale * 1.3;
            glBegin(GL_LINE_LOOP);
            constexpr int segments = 24;
            for (int i = 0; i < segments; ++i) {
                double theta = 2.0 * M_PI * i / segments;
                glVertex3d(ringRadius * std::cos(theta), ringRadius * std::sin(theta), 0);
            }
            glEnd();
            ++m_drawCalls;
            glPopMatrix();
        }

        renderAsset(lpMesh, pos, quality, assetScale);
    }
}

void Renderer::renderInteractionEntity(const VirtualInteractionEntity& entity) {
    if (!entity.isActive()) return;

    const auto& pos = entity.getPosition();
    const auto& vel = entity.getVelocity();

    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);
    glColor4f(0.95f, 0.2f, 0.85f, 0.8f);
    glBegin(GL_LINES);
    glVertex3d(pos.x, pos.y, pos.z);
    glVertex3d(pos.x - vel.x * 0.8, pos.y - vel.y * 0.8, pos.z - vel.z * 0.8);
    glEnd();
    ++m_drawCalls;

    const auto& eventType = entity.getEventType();
    const AssetMesh* meshPtr = nullptr;
    const auto& detectorMesh = m_assetManager.getMesh(AssetType::RADAR_NODE);
    const auto& emitterMesh = m_assetManager.getMesh(AssetType::EMITTER_POINT);

    if (eventType == "VE-001") {
        meshPtr = &detectorMesh;
    } else {
        meshPtr = &emitterMesh;
    }

    if (meshPtr && !meshPtr->empty()) {
        renderAsset(*meshPtr, pos, QualityLevel::HIGH);
    } else {
        glPushMatrix();
        glTranslated(pos.x, pos.y, pos.z);
        glColor4f(1.0f, 0.25f, 0.9f, 1.0f);

        double r = 160.0;
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
        ++m_drawCalls;
    }
}

void Renderer::renderVisualEffect(const VisualEffect& effect, double sim_time) {
    if (!effect.active) return;

    double p = effect.getProgress(sim_time);
    if (p >= 1.0) return;

    double currentR = effect.max_radius * std::sin(p * M_PI * 0.5);
    float alpha = static_cast<float>(1.0 - p) * 0.85f;

    glDisable(GL_LIGHTING);
    glLineWidth(2.5f);
    glColor4f(1.0f, 0.6f, 0.2f, alpha);

    glPushMatrix();
    glTranslated(effect.position.x, effect.position.y, effect.position.z);

    glBegin(GL_LINE_LOOP);
    constexpr int segments = 24;
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
    m_drawCalls += 2;
}

void Renderer::renderAsset(const AssetMesh& mesh, const Vector3D& position, QualityLevel quality, float scale) {
    if (mesh.empty()) return;

    glPushMatrix();
    glTranslated(position.x, position.y, position.z);
    if (scale != 1.0f) {
        glScaled(scale, scale, scale);
    }

    if (quality >= QualityLevel::HIGH) {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
    } else {
        glDisable(GL_LIGHTING);
    }

    glBegin(GL_TRIANGLES);
    for (const auto& v : mesh.vertices) {
        glNormal3f(v.nx, v.ny, v.nz);
        glColor4ub(v.r, v.g, v.b, v.a);
        glVertex3f(v.px, v.py, v.pz);
    }
    glEnd();
    ++m_drawCalls;

    glPopMatrix();
}

void Renderer::renderEnvironment(QualityLevel quality) {
    glDisable(GL_LIGHTING);

    float gR = 0.12f, gG = 0.15f, gB = 0.18f;
    float gSize = 30000.0f;

    glBegin(GL_QUADS);
    glColor4f(gR, gG, gB, 0.4f);
    glVertex3f(-gSize, -gSize, -2.0f);
    glVertex3f(gSize, -gSize, -2.0f);
    glVertex3f(gSize, gSize, -2.0f);
    glVertex3f(-gSize, gSize, -2.0f);
    glEnd();
    ++m_drawCalls;

    if (quality >= QualityLevel::HIGH) {
        float horizonR = 15000.0f;
        int segs = (quality >= QualityLevel::RESEARCH) ? 48 : 32;
        glColor4f(0.15f, 0.22f, 0.30f, 0.3f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segs; ++i) {
            float theta = 2.0f * 3.14159265f * i / segs;
            glVertex3f(horizonR * std::cos(theta), horizonR * std::sin(theta), -1.0f);
        }
        glEnd();
        ++m_drawCalls;

        glColor4f(0.12f, 0.18f, 0.25f, 0.2f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segs; ++i) {
            float theta = 2.0f * 3.14159265f * i / segs;
            glVertex3f(horizonR * 0.6f * std::cos(theta), horizonR * 0.6f * std::sin(theta), -1.0f);
        }
        glEnd();
        ++m_drawCalls;
    }

    const auto& ccMesh = m_assetManager.getMesh(AssetType::CONTROL_CENTER);
    renderAsset(ccMesh, Vector3D{0.0, 0.0, 0.0}, quality);
}

void Renderer::render(const VisualizationAdapter& adapter, PerformanceMonitor& perf_monitor) {
    m_drawCalls = 0;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    setup3DProjection();

    // Extract frustum matrices
    double modelview[16];
    double projection[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    m_frustum.updateFromMatrices(modelview, projection);

    const auto& options = adapter.getPlaybackOptions();
    double simTime = adapter.getCurrentSimulationTime();
    QualityLevel quality = perf_monitor.getQualityLevel();
    const auto& config = perf_monitor.getConfig();
    m_frustum.setLodDistances(config.lodDistanceNear, config.lodDistanceFar);

    if (options.show_grid) {
        renderEnvironment(quality);
        renderGrid(20000.0, 2000.0, quality);
        renderAxes(5000.0);
    }

    size_t activeObjects = 0;
    size_t culledObjects = 0;

    const auto& targets = adapter.getTargets();
    for (const auto& tgt : targets) {
        // Frustum Culling
        if (!m_frustum.isSphereInside(tgt.position, 250.0)) {
            ++culledObjects;
            continue; // Culled! Skip drawing entirely
        }

        ++activeObjects;

        double distToCamera = (tgt.position - m_cameraPos).norm();
        LodLevel lod = m_frustum.getLodForDistance(distToCamera);

        if (options.show_trails) {
            renderTrail(tgt, quality);
        }

        bool isSelected = (tgt.target_id == options.selected_target_id);
        renderTarget(tgt, isSelected, lod, quality);

        if (quality >= QualityLevel::MEDIUM && tgt.speed > 50.0) {
            m_particleSystem.spawnExhaust(tgt.position, tgt.velocity);
        }
    }

    if (options.show_events) {
        for (const auto& entity : adapter.getInteractionEntities()) {
            if (m_frustum.isSphereInside(entity.getPosition(), 200.0)) {
                renderInteractionEntity(entity);
            }
        }
    }

    for (const auto& eff : adapter.getVisualEffects()) {
        renderVisualEffect(eff, simTime);

        if (eff.active) {
            double progress = eff.getProgress(simTime);
            if (progress < 0.15) {
                m_particleSystem.spawnEventBurst(eff.position, 8);
            }
        }
    }

    // Render pooled particles
    m_particleSystem.render(quality);
    ++m_drawCalls;

    // Record stats to monitor
    perf_monitor.recordRenderStats(
        activeObjects,
        culledObjects,
        m_particleSystem.getActiveCount(),
        m_particleSystem.getBudget(quality),
        m_drawCalls
    );
}

std::string Renderer::pickTarget(int mouse_x, int mouse_y, const std::vector<TargetVisualState>& targets) const {
    GLdouble modelview[16];
    GLdouble projection[16];
    GLint viewport[4];

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    std::string closestId;
    double minDistanceSq = 25.0 * 25.0;

    for (const auto& tgt : targets) {
        GLdouble winX, winY, winZ;
        if (gluProject(tgt.position.x, tgt.position.y, tgt.position.z,
                       modelview, projection, viewport,
                       &winX, &winY, &winZ) == GL_TRUE) {
            if (winZ < 0.0 || winZ > 1.0) continue;

            double screenY = viewport[3] - winY;
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
