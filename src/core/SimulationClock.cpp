#include "SimulationClock.hpp"

namespace sim {

SimulationClock::SimulationClock(double timestep, double initial_time) noexcept
    : m_currentTime(initial_time), m_timestep(timestep), m_stepCount(0) {}

void SimulationClock::setTimestep(double dt) noexcept {
    if (dt > 0.0) {
        m_timestep = dt;
    }
}

double SimulationClock::getTimestep() const noexcept {
    return m_timestep;
}

void SimulationClock::advance() noexcept {
    m_currentTime += m_timestep;
    ++m_stepCount;
}

void SimulationClock::reset(double initial_time) noexcept {
    m_currentTime = initial_time;
    m_stepCount = 0;
}

double SimulationClock::getCurrentTime() const noexcept {
    return m_currentTime;
}

uint64_t SimulationClock::getStepCount() const noexcept {
    return m_stepCount;
}

} // namespace sim
