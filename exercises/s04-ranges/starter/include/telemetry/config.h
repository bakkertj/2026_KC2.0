#pragma once
#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <string_view>

namespace telemetry {

struct SensorConfig {
    std::string_view name;
    std::string_view units;
    double min_valid;
    double max_valid;

    [[nodiscard]] constexpr bool in_range(double v) const { return v >= min_valid && v <= max_valid; }
};

// The table lives in the header as inline constexpr so that lookups and the
// validation below can run at compile time.
inline constexpr std::array<SensorConfig, 6> kSensors{{
    {.name = "temp_core", .units = "degC", .min_valid = -40.0, .max_valid = 125.0},
    {.name = "temp_ambient", .units = "degC", .min_valid = -55.0, .max_valid = 85.0},
    {.name = "pressure", .units = "kPa", .min_valid = 0.0, .max_valid = 400.0},
    {.name = "voltage_bus", .units = "V", .min_valid = 0.0, .max_valid = 32.0},
    {.name = "current_bus", .units = "A", .min_valid = -20.0, .max_valid = 20.0},
    {.name = "rpm", .units = "rpm", .min_valid = 0.0, .max_valid = 12'000.0},
}};

// consteval: this function can ONLY run at compile time. A configuration table
// that fails validation is a build error, not a startup crash or a silent bug.
// Returns a message so the static_assert can print what is wrong.
[[nodiscard]] consteval std::string_view validate(std::span<const SensorConfig> table) {
    if (table.empty()) return "table is empty";
    for (std::size_t i = 0; i < table.size(); ++i) {
        const auto& s = table[i];
        if (s.name.empty()) return "a sensor has an empty name";
        if (s.units.empty()) return "a sensor has empty units";
        if (!(s.min_valid < s.max_valid)) return "a sensor has min_valid >= max_valid";
        for (std::size_t j = 0; j < i; ++j) {
            if (table[j].name == s.name) return "duplicate sensor name";
        }
    }
    return {};
}

static_assert(validate(kSensors).empty(), "sensor configuration table is invalid");

[[nodiscard]] constexpr std::span<const SensorConfig> sensors() { return kSensors; }

// constexpr: a lookup of a literal name is folded to a constant.
[[nodiscard]] constexpr std::optional<SensorConfig> find_sensor(std::string_view name) {
    for (const auto& cfg : kSensors) {
        if (cfg.name == name) return cfg;
    }
    return std::nullopt;
}

inline constexpr std::size_t kMaxLineLength = 256;

}  // namespace telemetry
