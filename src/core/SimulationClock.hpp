#pragma once

#include <cstdint>

namespace sim {

class SimulationClock {
public:
    explicit SimulationClock(double timestep = 0.05, double initial_time = 0.0) noexcept;

    void setTimestep(double dt) noexcept;
    [[nodiscard]] double getTimestep() const noexcept;

    void advance() noexcept;
    void reset(double initial_time = 0.0) noexcept;

    [[nodiscard]] double getCurrentTime() const noexcept;
    [[nodiscard]] uint64_t getStepCount() const noexcept;

private:
    double m_currentTime{0.0};
    double m_timestep{0.05};
    uint64_t m_stepCount{0};
};

} // namespace sim
