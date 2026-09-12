#include "VisualizationApp.hpp"
#include "core/Logger.hpp"
#include <windows.h>
#include <psapi.h>
#include <chrono>
#include <iostream>
#include <numeric>

namespace sim::vis {

namespace {
VisualizationApp* g_appInstance = nullptr;

double getProcessMemoryMB() {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
    }
    return 0.0;
}
} // anonymous namespace

VisualizationApp::VisualizationApp() {
    g_appInstance = this;
}

VisualizationApp::~VisualizationApp() {
    if (m_hglrc) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(m_hglrc);
        m_hglrc = NULL;
    }
    if (m_hdc && m_hwnd) {
        ReleaseDC(m_hwnd, m_hdc);
        m_hdc = NULL;
    }
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
    g_appInstance = nullptr;
}

LRESULT CALLBACK VisualizationApp::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_appInstance) {
        g_appInstance->processInput(msg, wParam, lParam);
    }
    switch (msg) {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

bool VisualizationApp::initialize(const std::string& scenario_path, int width, int height) {
    auto startInit = std::chrono::high_resolution_clock::now();
    m_width = width;
    m_height = height;

    if (!m_adapter.initializeFromFile(scenario_path)) {
        std::cerr << "[ERROR] Failed to initialize VisualizationAdapter with " << scenario_path << "\n";
        return false;
    }

    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = VisualizationApp::WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "JordanianRnDSimVisClass";

    RegisterClassExA(&wc);

    RECT wr = {0, 0, m_width, m_height};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    m_hwnd = CreateWindowExA(
        WS_EX_APPWINDOW,
        wc.lpszClassName,
        "Jordanian Advanced Industrial R&D Simulation PoC - Milestone S-001-VIS",
        WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left, wr.bottom - wr.top,
        NULL, NULL, hInstance, NULL
    );

    if (!m_hwnd) {
        std::cerr << "[ERROR] Failed to create Win32 window\n";
        return false;
    }

    m_hdc = GetDC(m_hwnd);

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;

    int pixelFormat = ChoosePixelFormat(m_hdc, &pfd);
    SetPixelFormat(m_hdc, pixelFormat, &pfd);

    m_hglrc = wglCreateContext(m_hdc);
    if (!m_hglrc) {
        std::cerr << "[ERROR] Failed to create OpenGL WGL context\n";
        return false;
    }

    wglMakeCurrent(m_hdc, m_hglrc);

    m_renderer.initialize(m_width, m_height);
    m_dashboard.initialize(m_hdc);

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);

    auto endInit = std::chrono::high_resolution_clock::now();
    double initMs = std::chrono::duration<double, std::milli>(endInit - startInit).count();
    std::cout << "[INFO] Visualization initialized in " << initMs << " ms\n";

    return true;
}

void VisualizationApp::processInput(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_SIZE: {
            m_width = LOWORD(lParam);
            m_height = HIWORD(lParam);
            m_renderer.resize(m_width, m_height);
            break;
        }
        case WM_LBUTTONDOWN: {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            if (!m_dashboard.handleClick(mx, my, m_adapter)) {
                // If dashboard didn't consume click, test target picking
                std::string picked = m_renderer.pickTarget(mx, my, m_adapter.getTargets());
                if (!picked.empty()) {
                    m_adapter.selectTarget(picked);
                } else {
                    m_adapter.clearSelection();
                }
            }
            m_isLeftDragging = true;
            m_lastMousePos.x = mx;
            m_lastMousePos.y = my;
            break;
        }
        case WM_LBUTTONUP: {
            m_isLeftDragging = false;
            break;
        }
        case WM_RBUTTONDOWN: {
            m_isRightDragging = true;
            m_lastMousePos.x = LOWORD(lParam);
            m_lastMousePos.y = HIWORD(lParam);
            break;
        }
        case WM_RBUTTONUP: {
            m_isRightDragging = false;
            break;
        }
        case WM_MOUSEMOVE: {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            int dx = mx - m_lastMousePos.x;
            int dy = my - m_lastMousePos.y;

            if (m_isLeftDragging) {
                m_renderer.orbit(dx * 0.35f, -dy * 0.35f);
            } else if (m_isRightDragging) {
                m_renderer.pan(static_cast<float>(dx), static_cast<float>(dy));
            }
            m_lastMousePos.x = mx;
            m_lastMousePos.y = my;
            break;
        }
        case WM_MOUSEWHEEL: {
            short delta = GET_WHEEL_DELTA_WPARAM(wParam);
            float factor = (delta > 0) ? 0.90f : 1.11f;
            m_renderer.zoom(factor);
            break;
        }
        case WM_KEYDOWN: {
            switch (wParam) {
                case VK_SPACE:
                    m_adapter.togglePlayback();
                    break;
                case 'R':
                    m_adapter.reset();
                    break;
                case 'T':
                    m_adapter.toggleTrails();
                    break;
                case 'G':
                    m_adapter.toggleGrid();
                    break;
                case 'E':
                    m_adapter.toggleEvents();
                    break;
                case '1':
                    m_adapter.setPlaybackSpeed(0.25);
                    break;
                case '2':
                    m_adapter.setPlaybackSpeed(0.5);
                    break;
                case '3':
                    m_adapter.setPlaybackSpeed(1.0);
                    break;
                case '4':
                    m_adapter.setPlaybackSpeed(2.0);
                    break;
                case '5':
                    m_adapter.setPlaybackSpeed(5.0);
                    break;
                case VK_ESCAPE:
                    m_running = false;
                    PostQuitMessage(0);
                    break;
            }
            break;
        }
    }
}

int VisualizationApp::run() {
    m_running = true;
    MSG msg = {};
    auto lastFrameTime = std::chrono::high_resolution_clock::now();

    while (m_running) {
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                m_running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        if (!m_running) break;

        auto now = std::chrono::high_resolution_clock::now();
        double wallDt = std::chrono::duration<double>(now - lastFrameTime).count();
        lastFrameTime = now;

        m_adapter.update(wallDt);
        m_renderer.render(m_adapter);
        m_dashboard.render(m_adapter, m_width, m_height);

        SwapBuffers(m_hdc);

        // Frame rate limiter (~60 FPS) to keep CPU/GPU cool
        Sleep(1);
    }

    return 0;
}

BenchmarkMetrics VisualizationApp::runBenchmark(int num_frames) {
    BenchmarkMetrics bm;
    bm.memory_usage_mb = getProcessMemoryMB();

    // Start simulation playback
    m_adapter.start();

    std::vector<double> frameTimes;
    std::vector<double> simUpdateTimes;
    frameTimes.reserve(num_frames);
    simUpdateTimes.reserve(num_frames);

    auto benchStart = std::chrono::high_resolution_clock::now();
    auto lastFrame = benchStart;

    for (int i = 0; i < num_frames; ++i) {
        auto now = std::chrono::high_resolution_clock::now();
        lastFrame = now;

        auto simStart = std::chrono::high_resolution_clock::now();
        m_adapter.update(0.016667); // Simulate 60 FPS delta
        auto simEnd = std::chrono::high_resolution_clock::now();
        simUpdateTimes.push_back(std::chrono::duration<double, std::micro>(simEnd - simStart).count());

        m_renderer.render(m_adapter);
        m_dashboard.render(m_adapter, m_width, m_height);
        SwapBuffers(m_hdc);

        auto frameEnd = std::chrono::high_resolution_clock::now();
        frameTimes.push_back(std::chrono::duration<double, std::milli>(frameEnd - now).count());
    }

    auto benchEnd = std::chrono::high_resolution_clock::now();
    double totalWallSec = std::chrono::duration<double>(benchEnd - benchStart).count();

    double avgFrameMs = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0) / frameTimes.size();
    double avgSimUs = std::accumulate(simUpdateTimes.begin(), simUpdateTimes.end(), 0.0) / simUpdateTimes.size();

    bm.avg_frame_time_ms = avgFrameMs;
    bm.avg_frame_rate_fps = (totalWallSec > 0.0) ? (num_frames / totalWallSec) : 0.0;
    bm.avg_sim_update_time_us = avgSimUs;
    bm.rendered_targets = m_adapter.getTargets().size();
    bm.active_virtual_events = m_adapter.getActiveInteractionCount();
    bm.triggered_interaction_events = m_adapter.getEventManager().getEventCount();
    bm.memory_usage_mb = getProcessMemoryMB();

    return bm;
}

} // namespace sim::vis
