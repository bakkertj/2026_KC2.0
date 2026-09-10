#include "telemetry/config.h"

#include <cstring>
#include <iterator>

namespace telemetry {

namespace {

constexpr SensorConfig kSensors[] = {
    {"temp_core", "degC", -40.0, 125.0},
    {"temp_ambient", "degC", -55.0, 85.0},
    {"pressure", "kPa", 0.0, 400.0},
    {"voltage_bus", "V", 0.0, 32.0},
    {"current_bus", "A", -20.0, 20.0},
    {"rpm", "rpm", 0.0, 12'000.0},
};

}  // namespace

std::size_t sensor_count() { return std::size(kSensors); }

const SensorConfig& sensor_at(std::size_t i) { return kSensors[i]; }

const SensorConfig* find_sensor(const std::string& name) {
    for (const auto& cfg : kSensors) {
        if (std::strcmp(cfg.name, name.c_str()) == 0) {
            return &cfg;
        }
    }
    return nullptr;
}

}  // namespace telemetry
