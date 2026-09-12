#pragma once

#include "core/Types.hpp"
#include "simulation/VirtualTarget.hpp"
#include <vector>
#include <cstdint>
#include <limits>
#include <string>

namespace sim {

struct SoftwareMetrics {
    uint64_t total_simulation_steps{0};
    uint64_t total_target_evaluations{0};
    double simulated_duration_seconds{0.0};
    size_t active_targets{0};

    // Kinematic metrics (CPU/RAM-independent)
    double min_speed_m_s{std::numeric_limits<double>::max()};
    double max_speed_m_s{0.0};
    double avg_speed_m_s{0.0};

    // Spatial bounding volume
    Vector3D bbox_min{std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
    Vector3D bbox_max{std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest()};

    // Deterministic state checksum (FNV-1a 64-bit of all target states)
    uint64_t state_checksum{0xcbf29ce484222325ULL};
};

class MetricsCollector {
public:
    MetricsCollector() = default;

    void reset() noexcept;

    void recordStep(double current_time, const std::vector<VirtualTarget>& targets);

    [[nodiscard]] const SoftwareMetrics& getMetrics() const noexcept;
    [[nodiscard]] std::string formatSummary() const;

private:
    SoftwareMetrics m_metrics;

    void updateChecksum(const VirtualTarget& target);
};

} // namespace sim
