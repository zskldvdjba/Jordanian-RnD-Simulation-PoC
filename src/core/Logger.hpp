#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <mutex>

namespace sim {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static Logger& instance();

    void setLogLevel(LogLevel level) noexcept;
    [[nodiscard]] LogLevel getLogLevel() const noexcept;

    void log(LogLevel level, const std::string& component, double sim_time, const std::string& message);

    void info(const std::string& component, double sim_time, const std::string& message) {
        log(LogLevel::INFO, component, sim_time, message);
    }

    void debug(const std::string& component, double sim_time, const std::string& message) {
        log(LogLevel::DEBUG, component, sim_time, message);
    }

    void warn(const std::string& component, double sim_time, const std::string& message) {
        log(LogLevel::WARNING, component, sim_time, message);
    }

    void error(const std::string& component, double sim_time, const std::string& message) {
        log(LogLevel::ERROR, component, sim_time, message);
    }

private:
    Logger() = default;
    LogLevel m_minLevel{LogLevel::INFO};
    std::mutex m_mutex;

    static const char* levelToString(LogLevel level) noexcept;
};

} // namespace sim
