#include "Dashboard.hpp"
#include "VisualizationAdapter.hpp"
#include <GL/gl.h>
#include <sstream>
#include <iomanip>

namespace sim::vis {

Dashboard::Dashboard() = default;

Dashboard::~Dashboard() {
    if (m_fontInitialized && m_fontListBase != 0) {
        glDeleteLists(m_fontListBase, 256);
    }
}

void Dashboard::initialize(HDC hdc) {
    if (m_fontInitialized) return;

    m_fontListBase = glGenLists(256);
    HFONT hFont = CreateFontA(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                              ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                              ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");

    HFONT hOldFont = static_cast<HFONT>(SelectObject(hdc, hFont));
    wglUseFontBitmapsA(hdc, 0, 256, m_fontListBase);
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);

    m_fontInitialized = true;
}

void Dashboard::drawPanel(float x, float y, float w, float h, float bgA) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Background
    glColor4f(0.07f, 0.09f, 0.13f, bgA);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();

    // Border
    glLineWidth(1.5f);
    glColor4f(0.22f, 0.35f, 0.48f, 0.85f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void Dashboard::drawText(float x, float y, const std::string& text, float r, float g, float b) {
    if (!m_fontInitialized || text.empty()) return;

    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    glListBase(m_fontListBase);
    glCallLists(static_cast<GLsizei>(text.size()), GL_UNSIGNED_BYTE, text.data());
}

void Dashboard::drawButton(const Button& btn) {
    float bgR = btn.active ? 0.16f : 0.11f;
    float bgG = btn.active ? 0.32f : 0.16f;
    float bgB = btn.active ? 0.52f : 0.25f;

    // Fill
    glColor4f(bgR, bgG, bgB, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(btn.rect.x, btn.rect.y);
    glVertex2f(btn.rect.x + btn.rect.w, btn.rect.y);
    glVertex2f(btn.rect.x + btn.rect.w, btn.rect.y + btn.rect.h);
    glVertex2f(btn.rect.x, btn.rect.y + btn.rect.h);
    glEnd();

    // Border
    glLineWidth(1.0f);
    if (btn.active) {
        glColor4f(0.4f, 0.8f, 1.0f, 1.0f);
    } else {
        glColor4f(0.28f, 0.38f, 0.52f, 0.8f);
    }
    glBegin(GL_LINE_LOOP);
    glVertex2f(btn.rect.x, btn.rect.y);
    glVertex2f(btn.rect.x + btn.rect.w, btn.rect.y);
    glVertex2f(btn.rect.x + btn.rect.w, btn.rect.y + btn.rect.h);
    glVertex2f(btn.rect.x, btn.rect.y + btn.rect.h);
    glEnd();

    // Text
    float textX = btn.rect.x + 8.0f;
    float textY = btn.rect.y + btn.rect.h - 8.0f;
    if (btn.active) {
        drawText(textX, textY, btn.label, 1.0f, 1.0f, 1.0f);
    } else {
        drawText(textX, textY, btn.label, 0.78f, 0.86f, 0.94f);
    }
}

void Dashboard::layoutButtons(int screen_width, int screen_height, const PlaybackOptions& options, const PerformanceStats& stats) {
    (void)screen_width;
    m_buttons.clear();

    float startY = static_cast<float>(screen_height) - 52.0f;
    float curX = 20.0f;

    // Playback control buttons
    bool isRunning = (options.status == PlaybackStatus::RUNNING);
    m_buttons.push_back(Button{"btn_start", "[ START ]", Rect{curX, startY, 78.0f, 32.0f}, isRunning});
    curX += 84.0f;

    bool isPaused = (options.status == PlaybackStatus::PAUSED);
    m_buttons.push_back(Button{"btn_pause", "[ PAUSE ]", Rect{curX, startY, 78.0f, 32.0f}, isPaused});
    curX += 84.0f;

    bool isReset = (options.status == PlaybackStatus::RESET);
    m_buttons.push_back(Button{"btn_reset", "[ RESET ]", Rect{curX, startY, 78.0f, 32.0f}, isReset});
    curX += 95.0f;

    // Playback Speed buttons
    const std::vector<std::pair<std::string, double>> speeds = {
        {"0.25x", 0.25}, {"0.5x", 0.5}, {"1x", 1.0}, {"2x", 2.0}, {"5x", 5.0}
    };
    for (const auto& [label, val] : speeds) {
        bool active = std::abs(options.speed_multiplier - val) < 1e-4;
        m_buttons.push_back(Button{"speed_" + label, label, Rect{curX, startY, 48.0f, 32.0f}, active});
        curX += 52.0f;
    }
    curX += 14.0f;

    // Quality Level Toggle
    std::string qLabel = std::string("Quality: ") + qualityLevelToString(stats.quality_level);
    m_buttons.push_back(Button{"cycle_quality", qLabel, Rect{curX, startY, 125.0f, 32.0f}, false});
    curX += 131.0f;

    // Auto Quality Scaling Toggle
    std::string autoLabel = std::string("AutoQ: ") + (stats.auto_quality_scaling ? "ON" : "OFF");
    m_buttons.push_back(Button{"toggle_auto_q", autoLabel, Rect{curX, startY, 88.0f, 32.0f}, stats.auto_quality_scaling});
    curX += 94.0f;

    // FPS Target Limiter Toggle
    std::string fpsLabel;
    if (stats.target_fps == 0) fpsLabel = "FPS: UNCAPPED";
    else fpsLabel = "FPS: " + std::to_string(stats.target_fps);
    m_buttons.push_back(Button{"cycle_fps", fpsLabel, Rect{curX, startY, 95.0f, 32.0f}, stats.target_fps > 0});
    curX += 101.0f;

    // Visual Toggles
    std::string trailLabel = std::string("Trails: ") + (options.show_trails ? "ON" : "OFF");
    m_buttons.push_back(Button{"toggle_trails", trailLabel, Rect{curX, startY, 95.0f, 32.0f}, options.show_trails});
    curX += 101.0f;

    std::string gridLabel = std::string("Grid: ") + (options.show_grid ? "ON" : "OFF");
    m_buttons.push_back(Button{"toggle_grid", gridLabel, Rect{curX, startY, 85.0f, 32.0f}, options.show_grid});
    curX += 91.0f;

    std::string evLabel = std::string("Events: ") + (options.show_events ? "ON" : "OFF");
    m_buttons.push_back(Button{"toggle_events", evLabel, Rect{curX, startY, 95.0f, 32.0f}, options.show_events});
}

void Dashboard::render(const VisualizationAdapter& adapter, const PerformanceMonitor& perf_monitor, int screen_width, int screen_height) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, screen_width, screen_height, 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    const auto& options = adapter.getPlaybackOptions();
    const auto& stats = perf_monitor.getStats();

    double simTime = adapter.getCurrentSimulationTime();
    uint64_t simStep = adapter.getCurrentSimulationStep();
    size_t targetCount = adapter.getTargets().size();
    size_t activeEvents = adapter.getActiveInteractionCount();
    size_t totalEvents = adapter.getEventManager().getEventCount();

    // 1. TOP-LEFT: Main Status Panel
    drawPanel(16.0f, 16.0f, 360.0f, 210.0f);
    drawText(28.0f, 36.0f, "PROJECT: Jordanian Advanced R&D Simulation", 0.4f, 0.85f, 1.0f);
    drawText(28.0f, 54.0f, "SCENARIO: S-001 (Baseline 3D Visualization)", 0.8f, 0.9f, 0.95f);

    std::string statusStr;
    switch (options.status) {
        case PlaybackStatus::RUNNING: statusStr = "RUNNING"; break;
        case PlaybackStatus::PAUSED:  statusStr = "PAUSED"; break;
        case PlaybackStatus::RESET:   statusStr = "RESET"; break;
    }
    drawText(28.0f, 78.0f, "STATUS                 : " + statusStr,
             options.status == PlaybackStatus::RUNNING ? 0.3f : 1.0f,
             options.status == PlaybackStatus::RUNNING ? 1.0f : 0.8f,
             options.status == PlaybackStatus::RUNNING ? 0.4f : 0.3f);

    std::ostringstream timeOss;
    timeOss << std::fixed << std::setprecision(2) << simTime << " / 10.00 s";
    drawText(28.0f, 98.0f, "SIMULATION TIME        : " + timeOss.str());

    std::ostringstream stepOss;
    stepOss << simStep << " / 200";
    drawText(28.0f, 118.0f, "SIMULATION STEP        : " + stepOss.str());

    drawText(28.0f, 138.0f, "VIRTUAL TARGETS        : " + std::to_string(targetCount));
    drawText(28.0f, 158.0f, "ACTIVE VIRTUAL EVENTS  : " + std::to_string(activeEvents), 0.95f, 0.4f, 0.9f);
    drawText(28.0f, 178.0f, "INTERACTION EVENTS     : " + std::to_string(totalEvents), 1.0f, 0.65f, 0.2f);
    drawText(28.0f, 202.0f, "[Controls: Orbit=L-Drag, Pan=R-Drag, Zoom=Wheel]", 0.5f, 0.62f, 0.72f);

    // 2. MID-LEFT: System & Performance Telemetry Panel (PERFORMANCE-FIRST)
    drawPanel(16.0f, 234.0f, 360.0f, 305.0f);
    drawText(28.0f, 254.0f, "HARDWARE & PERFORMANCE MONITOR", 0.3f, 1.0f, 0.7f);

    std::ostringstream fpsOss;
    fpsOss << std::fixed << std::setprecision(1) << stats.current_fps << " FPS ("
           << std::fixed << std::setprecision(2) << stats.avg_frame_time_ms << " ms)";
    drawText(28.0f, 274.0f, "Frame Rate / Time      : " + fpsOss.str(),
             stats.current_fps >= 30.0 ? 0.3f : 1.0f,
             stats.current_fps >= 30.0 ? 0.95f : 0.4f,
             0.3f);

    std::ostringstream limitOss;
    if (stats.target_fps > 0) {
        limitOss << stats.target_fps << " FPS (Pacing: Active)";
    } else {
        limitOss << "Uncapped (Maximum Load)";
    }
    drawText(28.0f, 294.0f, "Target Frame Limit     : " + limitOss.str());

    std::ostringstream simRateOss;
    simRateOss << std::fixed << std::setprecision(1) << stats.sim_steps_per_sec << " steps/s";
    drawText(28.0f, 314.0f, "Simulation Step Rate   : " + simRateOss.str());

    std::ostringstream cpuOss;
    cpuOss << std::fixed << std::setprecision(1) << stats.cpu_usage_pct << " %";
    drawText(28.0f, 334.0f, "Process CPU Usage      : " + cpuOss.str(),
             stats.cpu_usage_pct < 20.0 ? 0.4f : 1.0f,
             stats.cpu_usage_pct < 20.0 ? 1.0f : 0.6f,
             0.4f);

    std::ostringstream ramOss;
    ramOss << std::fixed << std::setprecision(1) << stats.ram_usage_mb << " MB (~8 GB Total)";
    drawText(28.0f, 354.0f, "Working Set (RAM)      : " + ramOss.str());

    std::ostringstream objOss;
    objOss << stats.active_3d_objects << " visible (" << stats.culled_3d_objects << " culled)";
    drawText(28.0f, 374.0f, "Frustum Culling        : " + objOss.str());

    std::ostringstream partOss;
    partOss << stats.active_particles << " active / " << stats.max_particles_budget << " budget";
    drawText(28.0f, 394.0f, "Particle Pool Usage    : " + partOss.str());

    drawText(28.0f, 414.0f, "Draw Calls / Frame     : " + std::to_string(stats.draw_calls));

    std::string qStr = std::string(qualityLevelToString(stats.quality_level)) + (stats.auto_quality_scaling ? " [AUTO]" : " [MANUAL]");
    drawText(28.0f, 434.0f, "Dynamic Visual Quality : " + qStr, 1.0f, 0.85f, 0.3f);

    const auto& qci = stats.qualityChange;
    drawText(28.0f, 454.0f, "Prev Quality / Changed : " + std::string(qualityLevelToString(qci.previousLevel))
             + " -> " + std::string(qualityLevelToString(qci.currentLevel)), 0.8f, 0.9f, 0.95f);

    std::ostringstream sinceOss;
    sinceOss << std::fixed << std::setprecision(1) << qci.timeSinceChange << "s";
    drawText(28.0f, 474.0f, "Time Since Change      : " + sinceOss.str(), 0.6f, 0.75f, 0.85f);

    drawText(28.0f, 494.0f, "Quality Reason         : " + qci.reason, 0.75f, 0.78f, 0.82f);

    drawText(28.0f, 514.0f, "GPU Telemetry          : " + stats.gpu_usage_str, 0.5f, 0.7f, 0.9f);

    // 3. TOP-RIGHT: Selected Target Panel
    const auto* selected = adapter.getSelectedTarget();
    if (selected) {
        float panelW = 340.0f;
        float panelX = static_cast<float>(screen_width) - panelW - 16.0f;
        drawPanel(panelX, 16.0f, panelW, 190.0f);
        drawText(panelX + 14.0f, 38.0f, "SELECTED TARGET: " + selected->target_id, 1.0f, 0.95f, 0.2f);

        std::string tState = (selected->status == TargetStatus::INTERACTED) ? "INTERACTED" : "ACTIVE";
        drawText(panelX + 14.0f, 62.0f, "State     : " + tState,
                 selected->status == TargetStatus::INTERACTED ? 1.0f : 0.3f,
                 selected->status == TargetStatus::INTERACTED ? 0.5f : 1.0f,
                 0.2f);

        std::ostringstream speedOss;
        speedOss << std::fixed << std::setprecision(2) << selected->speed << " m/s";
        drawText(panelX + 14.0f, 84.0f, "Speed     : " + speedOss.str());

        std::ostringstream posOss;
        posOss << std::fixed << std::setprecision(1) << "[" << selected->position.x << ", "
               << selected->position.y << ", " << selected->position.z << "]";
        drawText(panelX + 14.0f, 106.0f, "Position  : " + posOss.str());

        std::ostringstream velOss;
        velOss << std::fixed << std::setprecision(1) << "[" << selected->velocity.x << ", "
               << selected->velocity.y << ", " << selected->velocity.z << "]";
        drawText(panelX + 14.0f, 128.0f, "Velocity  : " + velOss.str());

        std::ostringstream timeTgtOss;
        timeTgtOss << std::fixed << std::setprecision(2) << selected->timestamp << " s";
        drawText(panelX + 14.0f, 150.0f, "Timestamp : " + timeTgtOss.str());
        drawText(panelX + 14.0f, 180.0f, "[Click background or entity to select]", 0.5f, 0.6f, 0.7f);
    }

    // 4. BOTTOM-RIGHT: Live Event Log Panel
    float logW = 390.0f;
    float logH = 175.0f;
    float logX = static_cast<float>(screen_width) - logW - 16.0f;
    float logY = static_cast<float>(screen_height) - logH - 64.0f;
    drawPanel(logX, logY, logW, logH);
    drawText(logX + 14.0f, logY + 22.0f, "SYNTHETIC EVENT LOG", 1.0f, 0.75f, 0.3f);

    const auto& eventList = adapter.getEventManager().getEvents();
    if (eventList.empty()) {
        drawText(logX + 14.0f, logY + 50.0f, "(No interaction events triggered yet)", 0.55f, 0.65f, 0.75f);
    } else {
        float entryY = logY + 48.0f;
        size_t startIdx = (eventList.size() > 4) ? (eventList.size() - 4) : 0;
        for (size_t i = startIdx; i < eventList.size(); ++i) {
            const auto& ev = eventList[i];
            std::ostringstream logLine;
            logLine << "[" << std::fixed << std::setprecision(2) << ev.simulation_time << "s] "
                    << ev.source_entity << " -> " << ev.target_entity;
            drawText(logX + 14.0f, entryY, logLine.str(), 0.95f, 0.9f, 0.95f);
            entryY += 16.0f;
            drawText(logX + 24.0f, entryY, "Event: " + ev.event_type, 0.9f, 0.6f, 0.2f);
            entryY += 16.0f;
        }
    }

    // 5. BOTTOM: Interactive Control Bar
    float barH = 50.0f;
    float barY = static_cast<float>(screen_height) - barH - 8.0f;
    float barW = static_cast<float>(screen_width) - 32.0f;
    drawPanel(16.0f, barY, barW, barH, 0.92f);

    layoutButtons(screen_width, screen_height, options, stats);
    for (const auto& btn : m_buttons) {
        drawButton(btn);
    }

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

bool Dashboard::handleClick(int mouse_x, int mouse_y, VisualizationAdapter& adapter, PerformanceMonitor& perf_monitor) {
    float mx = static_cast<float>(mouse_x);
    float my = static_cast<float>(mouse_y);

    for (const auto& btn : m_buttons) {
        if (btn.rect.contains(mx, my)) {
            if (btn.id == "btn_start") {
                adapter.start();
                return true;
            } else if (btn.id == "btn_pause") {
                adapter.pause();
                return true;
            } else if (btn.id == "btn_reset") {
                adapter.reset();
                return true;
            } else if (btn.id == "speed_0.25x") {
                adapter.setPlaybackSpeed(0.25);
                return true;
            } else if (btn.id == "speed_0.5x") {
                adapter.setPlaybackSpeed(0.5);
                return true;
            } else if (btn.id == "speed_1x") {
                adapter.setPlaybackSpeed(1.0);
                return true;
            } else if (btn.id == "speed_2x") {
                adapter.setPlaybackSpeed(2.0);
                return true;
            } else if (btn.id == "speed_5x") {
                adapter.setPlaybackSpeed(5.0);
                return true;
            } else if (btn.id == "cycle_quality") {
                perf_monitor.cycleQualityLevel();
                return true;
            } else if (btn.id == "toggle_auto_q") {
                perf_monitor.setAutoQuality(!perf_monitor.isAutoQuality());
                return true;
            } else if (btn.id == "cycle_fps") {
                perf_monitor.cycleTargetFps();
                return true;
            } else if (btn.id == "toggle_trails") {
                adapter.toggleTrails();
                return true;
            } else if (btn.id == "toggle_grid") {
                adapter.toggleGrid();
                return true;
            } else if (btn.id == "toggle_events") {
                adapter.toggleEvents();
                return true;
            }
        }
    }
    return false;
}

} // namespace sim::vis
