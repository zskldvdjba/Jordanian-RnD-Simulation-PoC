#include "Logger.hpp"
#include <iomanip>

namespace sim {

Logger& Logger::instance() {
    static Logger s_instance;
    return s_instance;
}

void Logger::setLogLevel(LogLevel level) noexcept {
    m_minLevel = level;
}

LogLevel Logger::getLogLevel() const noexcept {
    return m_minLevel;
}

const char* Logger::levelToString(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERR:     return "ERROR";
        default:                return "UNKNOWN";
    }
}

void Logger::log(LogLevel level, const std::string& component, double sim_time, const std::string& message) {
    if (static_cast<int>(level) < static_cast<int>(m_minLevel)) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "{\"sim_time_sec\":" << std::fixed << std::setprecision(4) << sim_time
              << ",\"level\":\"" << levelToString(level) << "\""
              << ",\"component\":\"" << component << "\""
              << ",\"message\":\"" << message << "\"}\n";
}

} // namespace sim
