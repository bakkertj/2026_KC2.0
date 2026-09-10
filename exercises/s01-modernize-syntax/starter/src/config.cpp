#include "telemetry/config.h"
#include <cstring>

namespace telemetry {

const std::size_t kMaxLineLength = 256;

namespace {

const SensorConfig kSensors[] = {
    {"temp_core", "degC", -40.0, 125.0},
    {"temp_ambient", "degC", -55.0, 85.0},
    {"pressure", "kPa", 0.0, 400.0},
    {"voltage_bus", "V", 0.0, 32.0},
    {"current_bus", "A", -20.0, 20.0},
    {"rpm", "rpm", 0.0, 12000.0},
};

const std::size_t kSensorCount = sizeof(kSensors) / sizeof(kSensors[0]);

}  // namespace

std::size_t sensor_count() { return kSensorCount; }

const SensorConfig& sensor_at(std::size_t i) { return kSensors[i]; }

const SensorConfig* find_sensor(const std::string& name) {
    for (std::size_t i = 0; i < kSensorCount; ++i) {
        if (std::strcmp(kSensors[i].name, name.c_str()) == 0) {
            return &kSensors[i];
        }
    }
    return nullptr;
}

}  // namespace telemetry
