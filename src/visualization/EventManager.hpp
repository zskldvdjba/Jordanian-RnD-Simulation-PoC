#pragma once

#include "VisualizationState.hpp"
#include <vector>
#include <string>

namespace sim::vis {

class EventManager {
public:
    EventManager() = default;

    void addEvent(const std::string& event_id,
                  double simulation_time,
                  const std::string& source_entity,
                  const std::string& target_entity,
                  const std::string& event_type);

    void clear() noexcept;

    [[nodiscard]] const std::vector<InteractionEvent>& getEvents() const noexcept;
    [[nodiscard]] size_t getEventCount() const noexcept;

private:
    std::vector<InteractionEvent> m_events;
};

} // namespace sim::vis
