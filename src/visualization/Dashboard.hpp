#pragma once

#include "VisualizationState.hpp"
#include <windows.h>
#include <GL/gl.h>
#include <string>
#include <vector>

namespace sim::vis {

class VisualizationAdapter;

struct Rect {
    float x{0.0f};
    float y{0.0f};
    float w{0.0f};
    float h{0.0f};

    [[nodiscard]] bool contains(float px, float py) const noexcept {
        return px >= x && px <= (x + w) && py >= y && py <= (y + h);
    }
};

struct Button {
    std::string id;
    std::string label;
    Rect rect;
    bool active{false};
};

class Dashboard {
public:
    Dashboard();
    ~Dashboard();

    void initialize(HDC hdc);
    void render(const VisualizationAdapter& adapter, int screen_width, int screen_height);
    bool handleClick(int mouse_x, int mouse_y, VisualizationAdapter& adapter);

private:
    GLuint m_fontListBase{0};
    bool m_fontInitialized{false};
    std::vector<Button> m_buttons;

    void drawPanel(float x, float y, float w, float h, float bgA = 0.82f);
    void drawText(float x, float y, const std::string& text, float r = 0.9f, float g = 0.92f, float b = 0.95f);
    void drawButton(const Button& btn);
    void layoutButtons(int screen_width, int screen_height, const PlaybackOptions& options);
};

} // namespace sim::vis
