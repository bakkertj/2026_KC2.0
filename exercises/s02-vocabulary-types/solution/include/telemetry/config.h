#pragma once
#include <cstddef>
#include <optional>
#include <span>
#include <string_view>

namespace telemetry {

// Static description of one sensor: its name, units, and the range outside
// which a reading is rejected as out of range. string_view members: the table
// is constexpr and the strings are literals, so nothing is copied or owned.
struct SensorConfig {
    std::string_view name;
    std::string_view units;
    double min_valid;
    double max_valid;

    [[nodiscard]] constexpr bool in_range(double v) const { return v >= min_valid && v <= max_valid; }
};

// The whole configuration table, as a view. Replaces sensor_count()/sensor_at(i).
[[nodiscard]] std::span<const SensorConfig> sensors();

// The configuration for a sensor name, or nothing if the name is unknown.
// Replaces the nullptr-returning pointer.
[[nodiscard]] std::optional<SensorConfig> find_sensor(std::string_view name);

// Longest input line the parser will accept.
inline constexpr std::size_t kMaxLineLength = 256;

}  // namespace telemetry
