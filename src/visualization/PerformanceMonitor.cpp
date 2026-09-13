#include "PerformanceMonitor.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace sim::vis {

namespace {

ULARGE_INTEGER fileTimeToUlarge(const FILETIME& ft) {
    ULARGE_INTEGER u;
    u.LowPart = ft.dwLowDateTime;
    u.HighPart = ft.dwHighDateTime;
    return u;
}

// Minimal JSON value extraction helpers (no third-party JSON library needed)
std::string extractStringValue(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    auto pos = json.find(searchKey);
    if (pos == std::string::npos) return "";
    pos = json.find(':', pos + searchKey.size());
    if (pos == std::string::npos) return "";
    auto startQuote = json.find('"', pos + 1);
    if (startQuote == std::string::npos) return "";
    auto endQuote = json.find('"', startQuote + 1);
    if (endQuote == std::string::npos) return "";
    return json.substr(startQuote + 1, endQuote - startQuote - 1);
}

double extractDoubleValue(const std::string& json, const std::string& key, double defaultVal) {
    std::string searchKey = "\"" + key + "\"";
    auto pos = json.find(searchKey);
    if (pos == std::string::npos) return defaultVal;
    pos = json.find(':', pos + searchKey.size());
    if (pos == std::string::npos) return defaultVal;
    ++pos;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) ++pos;
    try {
        return std::stod(json.substr(pos));
    } catch (...) {
        return defaultVal;
    }
}

int extractIntValue(const std::string& json, const std::string& key, int defaultVal) {
    return static_cast<int>(extractDoubleValue(json, key, static_cast<double>(defaultVal)));
}

bool extractBoolValue(const std::string& json, const std::string& key, bool defaultVal) {
    std::string searchKey = "\"" + key + "\"";
    auto pos = json.find(searchKey);
    if (pos == std::string::npos) return defaultVal;
    pos = json.find(':', pos + searchKey.size());
    if (pos == std::string::npos) return defaultVal;
    auto rest = json.substr(pos + 1);
    return rest.find("true") < rest.find("false");
}

// Extract a JSON sub-object as a string (simple nesting)
std::string extractObject(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    auto pos = json.find(searchKey);
    if (pos == std::string::npos) return "";
    auto braceStart = json.find('{', pos + searchKey.size());
    if (braceStart == std::string::npos) return "";
    int depth = 1;
    size_t i = braceStart + 1;
    while (i < json.size() && depth > 0) {
        if (json[i] == '{') ++depth;
        else if (json[i] == '}') --depth;
        ++i;
    }
    return json.substr(braceStart, i - braceStart);
}

} // anonymous namespace

PerformanceMonitor::PerformanceMonitor() {
    FILETIME ftCreation, ftExit, ftKernel, ftUser;
    FILETIME ftSysIdle, ftSysKernel, ftSysUser;

    if (GetProcessTimes(GetCurrentProcess(), &ftCreation, &ftExit, &ftKernel, &ftUser)) {
        ULARGE_INTEGER k = fileTimeToUlarge(ftKernel);
        ULARGE_INTEGER u = fileTimeToUlarge(ftUser);
        m_lastCpuTime.QuadPart = k.QuadPart + u.QuadPart;
    }

    if (GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser)) {
        m_lastSysKernel = fileTimeToUlarge(ftSysKernel);
        m_lastSysUser = fileTimeToUlarge(ftSysUser);
    }

    sampleCpuAndRam();
}

bool PerformanceMonitor::loadConfig(const std::string& configPath) {
    // Try multiple resolution paths
    std::string resolvedPath = configPath;
    if (!std::filesystem::exists(resolvedPath)) {
        if (std::filesystem::exists("../" + configPath)) resolvedPath = "../" + configPath;
        else if (std::filesystem::exists("../../" + configPath)) resolvedPath = "../../" + configPath;
        else {
            std::cout << "[PERF] No config found at '" << configPath << "', using defaults.\n";
            return false;
        }
    }

    std::ifstream file(resolvedPath);
    if (!file.is_open()) {
        std::cout << "[PERF] Could not open '" << resolvedPath << "', using defaults.\n";
        return false;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string json = ss.str();
    file.close();

    // Parse top-level values
    std::string defQuality = extractStringValue(json, "defaultQuality");
    if (!defQuality.empty()) {
        m_config.defaultQuality = qualityLevelFromString(defQuality);
    }

    m_config.frameDelayTestMs = extractIntValue(json, "frameDelayTestMs", 0);
    m_config.lodDistanceNear = extractDoubleValue(json, "lodDistanceNear", 5000.0);
    m_config.lodDistanceFar = extractDoubleValue(json, "lodDistanceFar", 15000.0);
    m_config.gpuTelemetryEnabled = extractBoolValue(json, "gpuTelemetryEnabled", false);

    // Parse hysteresis sub-object
    std::string hystJson = extractObject(json, "hysteresis");
    if (!hystJson.empty()) {
        m_config.hysteresis.downgradeThreshold = extractDoubleValue(hystJson, "downgradeThreshold", 27.0);
        m_config.hysteresis.strongDowngradeThreshold = extractDoubleValue(hystJson, "strongDowngradeThreshold", 24.0);
        m_config.hysteresis.upgradeThreshold = extractDoubleValue(hystJson, "upgradeThreshold", 35.0);
        m_config.hysteresis.sustainedSeconds = extractDoubleValue(hystJson, "sustainedSeconds", 3.0);
    }

    // Parse particleCaps sub-object
    std::string capsJson = extractObject(json, "particleCaps");
    if (!capsJson.empty()) {
        m_config.particleCaps.smoke   = static_cast<size_t>(extractIntValue(capsJson, "Smoke", 300));
        m_config.particleCaps.exhaust = static_cast<size_t>(extractIntValue(capsJson, "Exhaust", 200));
        m_config.particleCaps.beam    = static_cast<size_t>(extractIntValue(capsJson, "Beam", 100));
        m_config.particleCaps.general = static_cast<size_t>(extractIntValue(capsJson, "General", 150));
    }

    // Apply default quality level
    m_stats.quality_level = m_config.defaultQuality;
    m_stats.qualityChange.currentLevel = m_config.defaultQuality;
    m_stats.qualityChange.previousLevel = m_config.defaultQuality;
    m_stats.qualityChange.reason = "Config loaded (default: " + std::string(qualityLevelToString(m_config.defaultQuality)) + ")";

    std::cout << "[PERF] Loaded config from '" << resolvedPath << "'\n"
              << "       Default quality: " << qualityLevelToString(m_config.defaultQuality) << "\n"
              << "       Hysteresis: down<" << m_config.hysteresis.downgradeThreshold
              << " strong<" << m_config.hysteresis.strongDowngradeThreshold
              << " up>" << m_config.hysteresis.upgradeThreshold
              << " sustain=" << m_config.hysteresis.sustainedSeconds << "s\n"
              << "       Frame delay test: " << m_config.frameDelayTestMs << " ms\n";

    return true;
}

const PerformanceConfig& PerformanceMonitor::getConfig() const noexcept {
    return m_config;
}

void PerformanceMonitor::sampleCpuAndRam() {
    // 1. RAM Working Set
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        m_stats.ram_usage_mb = static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
    }

    // 2. CPU Usage
    FILETIME ftCreation, ftExit, ftKernel, ftUser;
    FILETIME ftSysIdle, ftSysKernel, ftSysUser;

    if (GetProcessTimes(GetCurrentProcess(), &ftCreation, &ftExit, &ftKernel, &ftUser) &&
        GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser)) {

        ULARGE_INTEGER procKernel = fileTimeToUlarge(ftKernel);
        ULARGE_INTEGER procUser = fileTimeToUlarge(ftUser);
        ULARGE_INTEGER sysKernel = fileTimeToUlarge(ftSysKernel);
        ULARGE_INTEGER sysUser = fileTimeToUlarge(ftSysUser);

        ULONGLONG procDiff = (procKernel.QuadPart + procUser.QuadPart) - m_lastCpuTime.QuadPart;
        ULONGLONG sysDiff = (sysKernel.QuadPart + sysUser.QuadPart) - (m_lastSysKernel.QuadPart + m_lastSysUser.QuadPart);

        if (sysDiff > 0) {
            double cpuPct = (static_cast<double>(procDiff) / static_cast<double>(sysDiff)) * 100.0;
            m_stats.cpu_usage_pct = std::clamp(cpuPct, 0.0, 100.0);
        }

        m_lastCpuTime.QuadPart = procKernel.QuadPart + procUser.QuadPart;
        m_lastSysKernel = sysKernel;
        m_lastSysUser = sysUser;
    }

    // 3. GPU usage – report N/A (no heavy dependencies for GPU telemetry)
    m_stats.gpu_usage_str = "N/A";
}

void PerformanceMonitor::update(double wall_dt, double sim_time, uint64_t sim_steps) {
    m_stats.simulation_time_sec = sim_time;

    // Track time since last quality change
    m_stats.qualityChange.timeSinceChange += wall_dt;

    // Instantaneous Frame time and smoothing
    if (wall_dt > 0.0) {
        double instantFps = 1.0 / wall_dt;
        double alpha = 0.1; // Exponential moving average
        m_stats.current_fps = (1.0 - alpha) * m_stats.current_fps + alpha * instantFps;
        m_stats.avg_frame_time_ms = wall_dt * 1000.0;
    }

    // Sample CPU & RAM every 300 ms
    m_cpuMeasurementAccumulator += wall_dt;
    if (m_cpuMeasurementAccumulator >= 0.3) {
        sampleCpuAndRam();
        m_cpuMeasurementAccumulator = 0.0;
    }

    // Simulation steps per second
    m_stepRateAccumulator += wall_dt;
    if (m_stepRateAccumulator >= 0.5) {
        uint64_t stepDelta = (sim_steps >= m_lastSimSteps) ? (sim_steps - m_lastSimSteps) : 0;
        m_stats.sim_steps_per_sec = static_cast<double>(stepDelta) / m_stepRateAccumulator;
        m_lastSimSteps = sim_steps;
        m_stepRateAccumulator = 0.0;
    }

    // Dynamic Quality auto-scaling
    if (m_stats.auto_quality_scaling) {
        evaluateDynamicScaling(wall_dt);
    }
}

void PerformanceMonitor::evaluateDynamicScaling(double wall_dt) {
    const auto& h = m_config.hysteresis;

    if (m_stats.current_fps < h.strongDowngradeThreshold) {
        // Strong downgrade path – treat as accelerated
        m_lowFpsDuration += wall_dt * 2.0;
        m_highFpsDuration = 0.0;
    } else if (m_stats.current_fps < h.downgradeThreshold) {
        m_lowFpsDuration += wall_dt;
        m_highFpsDuration = 0.0;
    } else if (m_stats.current_fps >= h.upgradeThreshold) {
        m_highFpsDuration += wall_dt;
        m_lowFpsDuration = 0.0;
    } else {
        // In the acceptable range – reset both accumulators
        m_lowFpsDuration = 0.0;
        m_highFpsDuration = 0.0;
    }

    // Check downgrade
    if (m_lowFpsDuration >= h.sustainedSeconds) {
        if (m_stats.quality_level > QualityLevel::LOW) {
            QualityLevel newLevel = static_cast<QualityLevel>(static_cast<int>(m_stats.quality_level) - 1);
            std::string reason;
            if (m_stats.current_fps < h.strongDowngradeThreshold) {
                reason = "FPS strongly below target (" + std::to_string(static_cast<int>(m_stats.current_fps))
                         + " < " + std::to_string(static_cast<int>(h.strongDowngradeThreshold)) + ")";
            } else {
                reason = "FPS below target (" + std::to_string(static_cast<int>(m_stats.current_fps))
                         + " < " + std::to_string(static_cast<int>(h.downgradeThreshold)) + ")";
            }
            recordQualityChange(newLevel, reason);
            m_lowFpsDuration = 0.0;
        }
    }

    // Check upgrade
    if (m_highFpsDuration >= h.sustainedSeconds * 1.5) {
        if (m_stats.quality_level < QualityLevel::HIGH) {
            QualityLevel newLevel = static_cast<QualityLevel>(static_cast<int>(m_stats.quality_level) + 1);
            std::string reason = "FPS above threshold (" + std::to_string(static_cast<int>(m_stats.current_fps))
                                 + " > " + std::to_string(static_cast<int>(h.upgradeThreshold)) + ")";
            recordQualityChange(newLevel, reason);
            m_highFpsDuration = 0.0;
        }
    }
}

void PerformanceMonitor::recordQualityChange(QualityLevel newLevel, const std::string& reason) {
    m_stats.qualityChange.previousLevel = m_stats.quality_level;
    m_stats.qualityChange.currentLevel = newLevel;
    m_stats.qualityChange.reason = reason;
    m_stats.qualityChange.timeSinceChange = 0.0;
    m_stats.qualityChange.hasChanged = true;
    m_stats.quality_level = newLevel;

    std::cout << "[PERF] Quality change: " << qualityLevelToString(m_stats.qualityChange.previousLevel)
              << " -> " << qualityLevelToString(newLevel) << " | Reason: " << reason << "\n";
}

void PerformanceMonitor::recordRenderStats(size_t active_objects, size_t culled_objects,
                                          size_t active_particles, size_t particle_budget,
                                          size_t draw_calls) noexcept {
    m_stats.active_3d_objects = active_objects;
    m_stats.culled_3d_objects = culled_objects;
    m_stats.active_particles = active_particles;
    m_stats.max_particles_budget = particle_budget;
    m_stats.draw_calls = draw_calls;
}

void PerformanceMonitor::setQualityLevel(QualityLevel level) noexcept {
    if (level != m_stats.quality_level) {
        recordQualityChange(level, "Manual override");
    }
    m_lowFpsDuration = 0.0;
    m_highFpsDuration = 0.0;
}

void PerformanceMonitor::cycleQualityLevel() noexcept {
    int next = (static_cast<int>(m_stats.quality_level) + 1) % 4;
    setQualityLevel(static_cast<QualityLevel>(next));
}

QualityLevel PerformanceMonitor::getQualityLevel() const noexcept {
    return m_stats.quality_level;
}

void PerformanceMonitor::setAutoQuality(bool enabled) noexcept {
    m_stats.auto_quality_scaling = enabled;
}

bool PerformanceMonitor::isAutoQuality() const noexcept {
    return m_stats.auto_quality_scaling;
}

void PerformanceMonitor::setTargetFps(int fps) noexcept {
    m_stats.target_fps = fps;
}

void PerformanceMonitor::cycleTargetFps() noexcept {
    if (m_stats.target_fps == 30) {
        m_stats.target_fps = 60;
    } else if (m_stats.target_fps == 60) {
        m_stats.target_fps = 0; // Uncapped
    } else {
        m_stats.target_fps = 30;
    }
}

int PerformanceMonitor::getTargetFps() const noexcept {
    return m_stats.target_fps;
}

const PerformanceStats& PerformanceMonitor::getStats() const noexcept {
    return m_stats;
}

const QualityChangeInfo& PerformanceMonitor::getQualityChangeInfo() const noexcept {
    return m_stats.qualityChange;
}

} // namespace sim::vis
