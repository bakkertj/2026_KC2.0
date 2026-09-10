#include "telemetry/config.h"

namespace telemetry {

namespace {

constexpr SensorConfig kSensors[] = {
    {.name = "temp_core", .units = "degC", .min_valid = -40.0, .max_valid = 125.0},
    {.name = "temp_ambient", .units = "degC", .min_valid = -55.0, .max_valid = 85.0},
    {.name = "pressure", .units = "kPa", .min_valid = 0.0, .max_valid = 400.0},
    {.name = "voltage_bus", .units = "V", .min_valid = 0.0, .max_valid = 32.0},
    {.name = "current_bus", .units = "A", .min_valid = -20.0, .max_valid = 20.0},
    {.name = "rpm", .units = "rpm", .min_valid = 0.0, .max_valid = 12'000.0},
};

}  // namespace

std::span<const SensorConfig> sensors() { return kSensors; }

std::optional<SensorConfig> find_sensor(std::string_view name) {
    for (const auto& cfg : kSensors) {
        if (cfg.name == name) return cfg;   // string_view == string_view: no allocation
    }
    return std::nullopt;
}

}  // namespace telemetry
