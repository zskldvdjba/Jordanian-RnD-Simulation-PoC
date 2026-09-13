#pragma once

#include "VisualizationAdapter.hpp"
#include "Renderer.hpp"
#include "Dashboard.hpp"
#include "PerformanceMonitor.hpp"
#include <windows.h>
#include <string>

namespace sim::vis {

struct BenchmarkMetrics {
    double startup_time_ms{0.0};
    double avg_frame_rate_fps{0.0};
    double avg_frame_time_ms{0.0};
    double avg_sim_update_time_us{0.0};
    double cpu_usage_pct{0.0};
    double memory_usage_mb{0.0};
    size_t rendered_targets{0};
    size_t active_virtual_events{0};
    size_t triggered_interaction_events{0};
    size_t culled_objects{0};
    size_t draw_calls{0};
    std::string quality_level{"MEDIUM"};
};

class VisualizationApp {
public:
    VisualizationApp();
    ~VisualizationApp();

    bool initialize(const std::string& scenario_path, int width = 1280, int height = 720);
    int run();
    BenchmarkMetrics runBenchmark(int num_frames = 300);

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    HWND m_hwnd{NULL};
    HDC m_hdc{NULL};
    HGLRC m_hglrc{NULL};
    int m_width{1280};
    int m_height{720};
    bool m_running{false};

    VisualizationAdapter m_adapter;
    Renderer m_renderer;
    Dashboard m_dashboard;
    PerformanceMonitor m_perfMonitor;

    // Mouse state
    bool m_isLeftDragging{false};
    bool m_isRightDragging{false};
    POINT m_lastMousePos{0, 0};

    void processInput(UINT msg, WPARAM wParam, LPARAM lParam);
};

} // namespace sim::vis
