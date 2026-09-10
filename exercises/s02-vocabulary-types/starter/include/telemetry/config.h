#pragma once
#include <cstddef>
#include <string>

namespace telemetry {

// Static description of one sensor: its name, units, and the range outside
// which a reading is rejected as out of range.
struct SensorConfig {
    const char* name;
    const char* units;
    double min_valid;
    double max_valid;
};

// Number of configured sensors.
[[nodiscard]] std::size_t sensor_count();

// The i-th configured sensor; i must be less than sensor_count().
[[nodiscard]] const SensorConfig& sensor_at(std::size_t i);

// Looks a sensor up by name. Returns nullptr when the name is unknown.
[[nodiscard]] const SensorConfig* find_sensor(const std::string& name);

// Longest input line the parser will accept. An inline variable: one
// definition, in the header, no extern/.cpp pair.
inline constexpr std::size_t kMaxLineLength = 256;

}  // namespace telemetry
