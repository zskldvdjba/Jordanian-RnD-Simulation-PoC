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
    glColor4f(0.08f, 0.11f, 0.16f, bgA);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();

    // Border
    glLineWidth(1.5f);
    glColor4f(0.25f, 0.38f, 0.52f, 0.85f);
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
    float bgR = btn.active ? 0.18f : 0.12f;
    float bgG = btn.active ? 0.35f : 0.18f;
    float bgB = btn.active ? 0.55f : 0.28f;

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
        glColor4f(0.3f, 0.42f, 0.58f, 0.8f);
    }
    glBegin(GL_LINE_LOOP);
    glVertex2f(btn.rect.x, btn.rect.y);
    glVertex2f(btn.rect.x + btn.rect.w, btn.rect.y);
    glVertex2f(btn.rect.x + btn.rect.w, btn.rect.y + btn.rect.h);
    glVertex2f(btn.rect.x, btn.rect.y + btn.rect.h);
    glEnd();

    // Text centered vertically
    float textX = btn.rect.x + 8.0f;
    float textY = btn.rect.y + btn.rect.h - 8.0f;
    if (btn.active) {
        drawText(textX, textY, btn.label, 1.0f, 1.0f, 1.0f);
    } else {
        drawText(textX, textY, btn.label, 0.8f, 0.88f, 0.95f);
    }
}

void Dashboard::layoutButtons(int screen_width, int screen_height, const PlaybackOptions& options) {
    (void)screen_width;
    m_buttons.clear();

    float startY = static_cast<float>(screen_height) - 52.0f;
    float curX = 20.0f;

    // Playback control buttons
    bool isRunning = (options.status == PlaybackStatus::RUNNING);
    m_buttons.push_back(Button{"btn_start", "[ START ]", Rect{curX, startY, 82.0f, 32.0f}, isRunning});
    curX += 88.0f;

    bool isPaused = (options.status == PlaybackStatus::PAUSED);
    m_buttons.push_back(Button{"btn_pause", "[ PAUSE ]", Rect{curX, startY, 82.0f, 32.0f}, isPaused});
    curX += 88.0f;

    bool isReset = (options.status == PlaybackStatus::RESET);
    m_buttons.push_back(Button{"btn_reset", "[ RESET ]", Rect{curX, startY, 82.0f, 32.0f}, isReset});
    curX += 105.0f;

    // Playback Speed buttons
    const std::vector<std::pair<std::string, double>> speeds = {
        {"0.25x", 0.25}, {"0.5x", 0.5}, {"1x", 1.0}, {"2x", 2.0}, {"5x", 5.0}
    };
    for (const auto& [label, val] : speeds) {
        bool active = std::abs(options.speed_multiplier - val) < 1e-4;
        m_buttons.push_back(Button{"speed_" + label, label, Rect{curX, startY, 52.0f, 32.0f}, active});
        curX += 56.0f;
    }
    curX += 20.0f;

    // Toggles
    std::string trailLabel = std::string("Trails: ") + (options.show_trails ? "ON" : "OFF");
    m_buttons.push_back(Button{"toggle_trails", trailLabel, Rect{curX, startY, 100.0f, 32.0f}, options.show_trails});
    curX += 106.0f;

    std::string gridLabel = std::string("Grid: ") + (options.show_grid ? "ON" : "OFF");
    m_buttons.push_back(Button{"toggle_grid", gridLabel, Rect{curX, startY, 90.0f, 32.0f}, options.show_grid});
    curX += 96.0f;

    std::string evLabel = std::string("Events: ") + (options.show_events ? "ON" : "OFF");
    m_buttons.push_back(Button{"toggle_events", evLabel, Rect{curX, startY, 100.0f, 32.0f}, options.show_events});
}

void Dashboard::render(const VisualizationAdapter& adapter, int screen_width, int screen_height) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, screen_width, screen_height, 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    const auto& options = adapter.getPlaybackOptions();
    double simTime = adapter.getCurrentSimulationTime();
    uint64_t simStep = adapter.getCurrentSimulationStep();
    size_t targetCount = adapter.getTargets().size();
    size_t activeEvents = adapter.getActiveInteractionCount();
    size_t totalEvents = adapter.getEventManager().getEventCount();

    // 1. TOP-LEFT: Main Status Panel
    drawPanel(16.0f, 16.0f, 360.0f, 220.0f);
    drawText(28.0f, 38.0f, "PROJECT: Jordanian Advanced R&D Simulation", 0.4f, 0.85f, 1.0f);
    drawText(28.0f, 56.0f, "SCENARIO: S-001 (Baseline 3D Visualization)", 0.8f, 0.9f, 0.95f);

    std::string statusStr;
    switch (options.status) {
        case PlaybackStatus::RUNNING: statusStr = "RUNNING"; break;
        case PlaybackStatus::PAUSED:  statusStr = "PAUSED"; break;
        case PlaybackStatus::RESET:   statusStr = "RESET"; break;
    }
    drawText(28.0f, 82.0f, "STATUS                 : " + statusStr,
             options.status == PlaybackStatus::RUNNING ? 0.3f : 1.0f,
             options.status == PlaybackStatus::RUNNING ? 1.0f : 0.8f,
             options.status == PlaybackStatus::RUNNING ? 0.4f : 0.3f);

    std::ostringstream timeOss;
    timeOss << std::fixed << std::setprecision(2) << simTime << " / 10.00 s";
    drawText(28.0f, 104.0f, "SIMULATION TIME        : " + timeOss.str());

    std::ostringstream stepOss;
    stepOss << simStep << " / 200";
    drawText(28.0f, 126.0f, "SIMULATION STEP        : " + stepOss.str());

    drawText(28.0f, 148.0f, "VIRTUAL TARGETS        : " + std::to_string(targetCount));
    drawText(28.0f, 170.0f, "ACTIVE VIRTUAL EVENTS  : " + std::to_string(activeEvents), 0.95f, 0.4f, 0.9f);
    drawText(28.0f, 192.0f, "INTERACTION EVENTS     : " + std::to_string(totalEvents), 1.0f, 0.65f, 0.2f);
    drawText(28.0f, 218.0f, "[Controls: Orbit=L-Drag, Pan=R-Drag, Zoom=Wheel]", 0.55f, 0.65f, 0.75f);

    // 2. TOP-RIGHT: Selected Target Panel
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

    // 3. BOTTOM-RIGHT: Live Event Log Panel
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

    // 4. BOTTOM: Control Bar & Buttons
    float barH = 50.0f;
    float barY = static_cast<float>(screen_height) - barH - 8.0f;
    float barW = static_cast<float>(screen_width) - 32.0f;
    drawPanel(16.0f, barY, barW, barH, 0.90f);

    layoutButtons(screen_width, screen_height, options);
    for (const auto& btn : m_buttons) {
        drawButton(btn);
    }

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

bool Dashboard::handleClick(int mouse_x, int mouse_y, VisualizationAdapter& adapter) {
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
