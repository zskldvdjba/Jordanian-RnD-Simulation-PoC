#pragma once

#include "core/Types.hpp"
#include <string>
#include <vector>
#include <deque>

namespace sim::vis {

enum class TargetStatus {
    ACTIVE,
    INTERACTED
};

struct TargetVisualState {
    std::string target_id;
    Vector3D position;
    Vector3D velocity;
    double speed{0.0};
    double timestamp{0.0};
    TargetStatus status{TargetStatus::ACTIVE};
    std::deque<Vector3D> trail;
    size_t max_trail_points{40};

    void addTrailPoint(const Vector3D& pt) {
        trail.push_back(pt);
        while (trail.size() > max_trail_points) {
            trail.pop_front();
        }
    }

    void clearTrail() noexcept {
        trail.clear();
    }
};

struct InteractionEvent {
    std::string event_id;
    double simulation_time{0.0};
    std::string source_entity;
    std::string target_entity;
    std::string event_type;
};

struct VisualEffect {
    Vector3D position;
    double start_time{0.0};
    double duration{1.0};
    double max_radius{250.0};
    bool active{true};

    [[nodiscard]] double getProgress(double current_sim_time) const noexcept {
        if (!active) return 1.0;
        double elapsed = current_sim_time - start_time;
        if (elapsed < 0.0) return 0.0;
        if (elapsed >= duration) return 1.0;
        return elapsed / duration;
    }
};

enum class PlaybackStatus {
    RESET,
    RUNNING,
    PAUSED
};

struct PlaybackOptions {
    PlaybackStatus status{PlaybackStatus::PAUSED};
    double speed_multiplier{1.0};
    bool show_trails{true};
    bool show_grid{true};
    bool show_events{true};
    std::string selected_target_id;
};

} // namespace sim::vis
