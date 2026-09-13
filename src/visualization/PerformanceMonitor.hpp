#pragma once

#include <windows.h>
#include <psapi.h>
#include <cstdint>
#include <string>

namespace sim::vis {

enum class QualityLevel {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
    RESEARCH = 3
};

inline const char* qualityLevelToString(QualityLevel q) noexcept {
    switch (q) {
        case QualityLevel::LOW:      return "LOW";
        case QualityLevel::MEDIUM:   return "MEDIUM";
        case QualityLevel::HIGH:     return "HIGH";
        case QualityLevel::RESEARCH: return "RESEARCH";
        default:                     return "UNKNOWN";
    }
}

inline QualityLevel qualityLevelFromString(const std::string& s) noexcept {
    if (s == "LOW")      return QualityLevel::LOW;
    if (s == "HIGH")     return QualityLevel::HIGH;
    if (s == "RESEARCH") return QualityLevel::RESEARCH;
    return QualityLevel::MEDIUM; // default
}

// Quality-change event tracking
struct QualityChangeInfo {
    QualityLevel previousLevel{QualityLevel::MEDIUM};
    QualityLevel currentLevel{QualityLevel::MEDIUM};
    std::string reason{"Startup default"};
    double timeSinceChange{0.0};   // seconds since last quality change
    bool hasChanged{false};        // true if quality has ever changed
};

// Configurable hysteresis thresholds
struct HysteresisConfig {
    double downgradeThreshold{27.0};       // sustained FPS below this → consider downgrade
    double strongDowngradeThreshold{24.0}; // sustained FPS below this → strong downgrade
    double upgradeThreshold{35.0};         // sustained FPS above this → consider upgrade
    double sustainedSeconds{3.0};          // seconds the trend must sustain before acting
};

// Configurable particle caps
struct ParticleCapsConfig {
    size_t smoke{300};
    size_t exhaust{200};
    size_t beam{100};
    size_t general{150};
};

// Full performance configuration loaded from config/performance.json
struct PerformanceConfig {
    QualityLevel defaultQuality{QualityLevel::MEDIUM};
    HysteresisConfig hysteresis;
    ParticleCapsConfig particleCaps;
    int frameDelayTestMs{0};        // Artificial delay (ms) for downgrade testing. 0 = disabled.
    double lodDistanceNear{5000.0}; // below this distance = HIGH lod
    double lodDistanceFar{15000.0}; // above this distance = LOW lod
    bool gpuTelemetryEnabled{false};
};

struct PerformanceStats {
    double current_fps{60.0};
    double avg_frame_time_ms{16.6};
    double simulation_time_sec{0.0};
    double sim_steps_per_sec{0.0};
    double cpu_usage_pct{0.0};
    double ram_usage_mb{0.0};
    std::string gpu_usage_str{"N/A"};
    size_t active_3d_objects{0};
    size_t culled_3d_objects{0};
    size_t active_particles{0};
    size_t max_particles_budget{750};
    size_t draw_calls{0};
    QualityLevel quality_level{QualityLevel::MEDIUM};
    bool auto_quality_scaling{true};
    int target_fps{60};
    QualityChangeInfo qualityChange;
};

class PerformanceMonitor {
public:
    PerformanceMonitor();

    // Load configuration from config/performance.json
    bool loadConfig(const std::string& configPath);
    [[nodiscard]] const PerformanceConfig& getConfig() const noexcept;

    void update(double wall_dt, double sim_time, uint64_t sim_steps);

    void recordRenderStats(size_t active_objects, size_t culled_objects,
                           size_t active_particles, size_t particle_budget,
                           size_t draw_calls) noexcept;

    void setQualityLevel(QualityLevel level) noexcept;
    void cycleQualityLevel() noexcept;
    [[nodiscard]] QualityLevel getQualityLevel() const noexcept;

    void setAutoQuality(bool enabled) noexcept;
    [[nodiscard]] bool isAutoQuality() const noexcept;

    void setTargetFps(int fps) noexcept;
    void cycleTargetFps() noexcept;
    [[nodiscard]] int getTargetFps() const noexcept;

    [[nodiscard]] const PerformanceStats& getStats() const noexcept;
    [[nodiscard]] const QualityChangeInfo& getQualityChangeInfo() const noexcept;

private:
    PerformanceStats m_stats;
    PerformanceConfig m_config;

    // CPU calculation state
    ULARGE_INTEGER m_lastCpuTime{0};
    ULARGE_INTEGER m_lastSysKernel{0};
    ULARGE_INTEGER m_lastSysUser{0};
    double m_cpuMeasurementAccumulator{0.0};

    // FPS smoothing
    double m_fpsAccumulator{0.0};
    int m_frameCount{0};

    // Auto-scaling counters
    double m_lowFpsDuration{0.0};
    double m_highFpsDuration{0.0};

    // Step rate
    uint64_t m_lastSimSteps{0};
    double m_stepRateAccumulator{0.0};

    void sampleCpuAndRam();
    void evaluateDynamicScaling(double wall_dt);
    void recordQualityChange(QualityLevel newLevel, const std::string& reason);
};

} // namespace sim::vis
