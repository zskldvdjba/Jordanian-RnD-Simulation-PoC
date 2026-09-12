#include "ScenarioLoader.hpp"
#include "core/Logger.hpp"
#include <fstream>
#include <sstream>
#include <cctype>
#include <stdexcept>

namespace sim {

namespace {

class SimpleJsonScanner {
public:
    explicit SimpleJsonScanner(std::string_view src) : m_src(src) {}

    void skipWhitespace() {
        while (m_idx < m_src.size() && std::isspace(static_cast<unsigned char>(m_src[m_idx]))) {
            ++m_idx;
        }
    }

    [[nodiscard]] bool hasMore() const {
        return m_idx < m_src.size();
    }

    [[nodiscard]] char peek() {
        skipWhitespace();
        return hasMore() ? m_src[m_idx] : '\0';
    }

    char get() {
        skipWhitespace();
        return hasMore() ? m_src[m_idx++] : '\0';
    }

    bool match(char expected) {
        skipWhitespace();
        if (hasMore() && m_src[m_idx] == expected) {
            ++m_idx;
            return true;
        }
        return false;
    }

    std::string parseString() {
        skipWhitespace();
        if (!match('"')) {
            throw std::runtime_error("Expected '\"' for string");
        }
        std::string result;
        while (hasMore()) {
            char c = m_src[m_idx++];
            if (c == '"') {
                return result;
            }
            if (c == '\\' && hasMore()) {
                char esc = m_src[m_idx++];
                switch (esc) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default: result += esc; break;
                }
            } else {
                result += c;
            }
        }
        throw std::runtime_error("Unterminated string");
    }

    double parseNumber() {
        skipWhitespace();
        size_t start = m_idx;
        if (hasMore() && (m_src[m_idx] == '-' || m_src[m_idx] == '+')) {
            ++m_idx;
        }
        while (hasMore() && (std::isdigit(static_cast<unsigned char>(m_src[m_idx])) ||
                             m_src[m_idx] == '.' ||
                             m_src[m_idx] == 'e' ||
                             m_src[m_idx] == 'E' ||
                             m_src[m_idx] == '+' ||
                             m_src[m_idx] == '-')) {
            ++m_idx;
        }
        std::string numStr(m_src.substr(start, m_idx - start));
        return std::stod(numStr);
    }

private:
    std::string_view m_src;
    size_t m_idx{0};
};

Vector3D parseVec3(SimpleJsonScanner& scanner) {
    if (!scanner.match('{')) {
        throw std::runtime_error("Expected '{' for Vector3D");
    }
    Vector3D v;
    while (scanner.peek() != '}' && scanner.hasMore()) {
        std::string key = scanner.parseString();
        if (!scanner.match(':')) {
            throw std::runtime_error("Expected ':' after key in Vector3D");
        }
        double val = scanner.parseNumber();
        if (key == "x" || key == "vx") v.x = val;
        else if (key == "y" || key == "vy") v.y = val;
        else if (key == "z" || key == "vz") v.z = val;

        if (scanner.peek() == ',') {
            scanner.get();
        }
    }
    scanner.match('}');
    return v;
}

TargetState parseTarget(SimpleJsonScanner& scanner) {
    if (!scanner.match('{')) {
        throw std::runtime_error("Expected '{' for Target");
    }
    TargetState target;
    while (scanner.peek() != '}' && scanner.hasMore()) {
        std::string key = scanner.parseString();
        if (!scanner.match(':')) {
            throw std::runtime_error("Expected ':' after key in Target");
        }
        if (key == "target_id") {
            target.target_id = scanner.parseString();
        } else if (key == "timestamp") {
            target.timestamp = scanner.parseNumber();
        } else if (key == "position") {
            target.position = parseVec3(scanner);
        } else if (key == "velocity") {
            target.velocity = parseVec3(scanner);
        } else {
            if (scanner.peek() == '"') scanner.parseString();
            else scanner.parseNumber();
        }

        if (scanner.peek() == ',') {
            scanner.get();
        }
    }
    scanner.match('}');
    return target;
}

std::vector<TargetState> parseTargetList(SimpleJsonScanner& scanner) {
    if (!scanner.match('[')) {
        throw std::runtime_error("Expected '[' for target list");
    }
    std::vector<TargetState> targets;
    while (scanner.peek() != ']' && scanner.hasMore()) {
        targets.push_back(parseTarget(scanner));
        if (scanner.peek() == ',') {
            scanner.get();
        }
    }
    scanner.match(']');
    return targets;
}

VirtualEventConfig parseVirtualEvent(SimpleJsonScanner& scanner) {
    if (!scanner.match('{')) {
        throw std::runtime_error("Expected '{' for VirtualEvent");
    }
    VirtualEventConfig ev;
    while (scanner.peek() != '}' && scanner.hasMore()) {
        std::string key = scanner.parseString();
        if (!scanner.match(':')) {
            throw std::runtime_error("Expected ':' after key in VirtualEvent");
        }
        if (key == "event_entity_id") {
            ev.event_entity_id = scanner.parseString();
        } else if (key == "target_id" || key == "event_target_id") {
            ev.target_id = scanner.parseString();
        } else if (key == "spawn_time" || key == "event_spawn_time") {
            ev.spawn_time = scanner.parseNumber();
        } else if (key == "lifetime" || key == "event_lifetime") {
            ev.lifetime = scanner.parseNumber();
        } else if (key == "event_trigger_threshold") {
            ev.event_trigger_threshold = scanner.parseNumber();
        } else if (key == "event_type") {
            ev.event_type = scanner.parseString();
        } else if (key == "position" || key == "initial_position") {
            ev.position = parseVec3(scanner);
        } else if (key == "velocity") {
            ev.velocity = parseVec3(scanner);
        } else {
            if (scanner.peek() == '"') scanner.parseString();
            else scanner.parseNumber();
        }

        if (scanner.peek() == ',') {
            scanner.get();
        }
    }
    scanner.match('}');
    return ev;
}

std::vector<VirtualEventConfig> parseVirtualEventList(SimpleJsonScanner& scanner) {
    if (!scanner.match('[')) {
        throw std::runtime_error("Expected '[' for virtual events list");
    }
    std::vector<VirtualEventConfig> events;
    while (scanner.peek() != ']' && scanner.hasMore()) {
        events.push_back(parseVirtualEvent(scanner));
        if (scanner.peek() == ',') {
            scanner.get();
        }
    }
    scanner.match(']');
    return events;
}

} // anonymous namespace

std::optional<ScenarioConfig> ScenarioLoader::parseJson(const std::string& json_content) {
    try {
        SimpleJsonScanner scanner(json_content);
        if (!scanner.match('{')) {
            return std::nullopt;
        }

        ScenarioConfig config;
        while (scanner.peek() != '}' && scanner.hasMore()) {
            std::string key = scanner.parseString();
            if (!scanner.match(':')) {
                return std::nullopt;
            }

            if (key == "scenario_id") {
                config.scenario_id = scanner.parseString();
            } else if (key == "scenario_name") {
                config.scenario_name = scanner.parseString();
            } else if (key == "description") {
                config.description = scanner.parseString();
            } else if (key == "seed") {
                config.seed = static_cast<uint64_t>(scanner.parseNumber());
            } else if (key == "timestep_seconds") {
                config.timestep_seconds = scanner.parseNumber();
            } else if (key == "duration_seconds") {
                config.duration_seconds = scanner.parseNumber();
            } else if (key == "target_count") {
                config.target_count = static_cast<size_t>(scanner.parseNumber());
            } else if (key == "targets") {
                config.targets = parseTargetList(scanner);
            } else if (key == "virtual_event_count") {
                config.virtual_event_count = static_cast<size_t>(scanner.parseNumber());
            } else if (key == "virtual_events") {
                config.virtual_events = parseVirtualEventList(scanner);
            } else {
                if (scanner.peek() == '"') scanner.parseString();
                else scanner.parseNumber();
            }

            if (scanner.peek() == ',') {
                scanner.get();
            }
        }
        scanner.match('}');

        if (config.target_count == 0 && !config.targets.empty()) {
            config.target_count = config.targets.size();
        }
        if (config.virtual_event_count == 0 && !config.virtual_events.empty()) {
            config.virtual_event_count = config.virtual_events.size();
        }

        return config;
    } catch (const std::exception& e) {
        Logger::instance().error("ScenarioLoader", 0.0, std::string("JSON parsing error: ") + e.what());
        return std::nullopt;
    }
}

std::optional<ScenarioConfig> ScenarioLoader::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        Logger::instance().error("ScenarioLoader", 0.0, "Failed to open scenario file: " + filepath);
        return std::nullopt;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return parseJson(buffer.str());
}

} // namespace sim
