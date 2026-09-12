#include "MetricsCollector.hpp"
#include <sstream>
#include <iomanip>
#include <cstring>
#include <algorithm>

namespace sim {

namespace {

constexpr uint64_t FNV_PRIME = 0x100000001b3ULL;

void fnv1a_update(uint64_t& hash, const void* data, size_t num_bytes) {
    const auto* bytes = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < num_bytes; ++i) {
        hash ^= bytes[i];
        hash *= FNV_PRIME;
    }
}

} // anonymous namespace

void MetricsCollector::reset() noexcept {
    m_metrics = SoftwareMetrics{};
}

void MetricsCollector::updateChecksum(const VirtualTarget& target) {
    const auto state = target.getState();
    fnv1a_update(m_metrics.state_checksum, state.target_id.data(), state.target_id.size());
    fnv1a_update(m_metrics.state_checksum, &state.timestamp, sizeof(state.timestamp));
    fnv1a_update(m_metrics.state_checksum, &state.position.x, sizeof(state.position.x));
    fnv1a_update(m_metrics.state_checksum, &state.position.y, sizeof(state.position.y));
    fnv1a_update(m_metrics.state_checksum, &state.position.z, sizeof(state.position.z));
    fnv1a_update(m_metrics.state_checksum, &state.velocity.x, sizeof(state.velocity.x));
    fnv1a_update(m_metrics.state_checksum, &state.velocity.y, sizeof(state.velocity.y));
    fnv1a_update(m_metrics.state_checksum, &state.velocity.z, sizeof(state.velocity.z));
}

void MetricsCollector::recordStep(double current_time, const std::vector<VirtualTarget>& targets) {
    ++m_metrics.total_simulation_steps;
    m_metrics.simulated_duration_seconds = current_time;
    m_metrics.active_targets = targets.size();
    m_metrics.total_target_evaluations += targets.size();

    double total_speed = 0.0;
    for (const auto& target : targets) {
        const auto& pos = target.getPosition();
        const auto& vel = target.getVelocity();
        double speed = vel.norm();

        total_speed += speed;
        m_metrics.min_speed_m_s = std::min(m_metrics.min_speed_m_s, speed);
        m_metrics.max_speed_m_s = std::max(m_metrics.max_speed_m_s, speed);

        m_metrics.bbox_min.x = std::min(m_metrics.bbox_min.x, pos.x);
        m_metrics.bbox_min.y = std::min(m_metrics.bbox_min.y, pos.y);
        m_metrics.bbox_min.z = std::min(m_metrics.bbox_min.z, pos.z);

        m_metrics.bbox_max.x = std::max(m_metrics.bbox_max.x, pos.x);
        m_metrics.bbox_max.y = std::max(m_metrics.bbox_max.y, pos.y);
        m_metrics.bbox_max.z = std::max(m_metrics.bbox_max.z, pos.z);

        updateChecksum(target);
    }

    if (!targets.empty()) {
        m_metrics.avg_speed_m_s = total_speed / static_cast<double>(targets.size());
    }
}

const SoftwareMetrics& MetricsCollector::getMetrics() const noexcept {
    return m_metrics;
}

std::string MetricsCollector::formatSummary() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    oss << "=== SIMULATION SOFTWARE METRICS ===\n"
        << "Total Simulation Steps   : " << m_metrics.total_simulation_steps << "\n"
        << "Total Target Evaluations : " << m_metrics.total_target_evaluations << "\n"
        << "Simulated Duration (s)   : " << m_metrics.simulated_duration_seconds << "\n"
        << "Active Target Count      : " << m_metrics.active_targets << "\n"
        << "Min Speed (m/s)          : " << m_metrics.min_speed_m_s << "\n"
        << "Max Speed (m/s)          : " << m_metrics.max_speed_m_s << "\n"
        << "Mean Target Speed (m/s)  : " << m_metrics.avg_speed_m_s << "\n"
        << "Bounding Box Min [X,Y,Z] : [" << m_metrics.bbox_min.x << ", " << m_metrics.bbox_min.y << ", " << m_metrics.bbox_min.z << "]\n"
        << "Bounding Box Max [X,Y,Z] : [" << m_metrics.bbox_max.x << ", " << m_metrics.bbox_max.y << ", " << m_metrics.bbox_max.z << "]\n"
        << "Final State FNV Checksum : 0x" << std::hex << std::uppercase << m_metrics.state_checksum << std::dec << "\n"
        << "===================================";
    return oss.str();
}

} // namespace sim
