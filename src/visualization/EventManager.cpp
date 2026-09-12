#include "EventManager.hpp"
#include "core/Logger.hpp"
#include <sstream>
#include <iomanip>

namespace sim::vis {

void EventManager::addEvent(const std::string& event_id,
                            double simulation_time,
                            const std::string& source_entity,
                            const std::string& target_entity,
                            const std::string& event_type) {
    InteractionEvent ev{
        .event_id = event_id,
        .simulation_time = simulation_time,
        .source_entity = source_entity,
        .target_entity = target_entity,
        .event_type = event_type
    };
    m_events.push_back(ev);

    std::ostringstream msg;
    msg << "[" << std::fixed << std::setprecision(2) << simulation_time << "s] "
        << source_entity << " -> " << target_entity
        << " Event: " << event_type << " (id: " << event_id << ")";
    Logger::instance().info("EventManager", simulation_time, msg.str());
}

void EventManager::clear() noexcept {
    m_events.clear();
}

const std::vector<InteractionEvent>& EventManager::getEvents() const noexcept {
    return m_events;
}

size_t EventManager::getEventCount() const noexcept {
    return m_events.size();
}

} // namespace sim::vis
