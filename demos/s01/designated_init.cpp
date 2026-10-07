// Demo: designated initializers (C++20)
// Session: s01
// Compiler Explorer: https://godbolt.org/z/Yd9ejj8zo
#include <cstdio>

// [snippet: designated]
struct SensorConfig {
    const char* name;
    const char* units;
    double min_valid;
    double max_valid;
};

constexpr SensorConfig kRpm{
    .name = "rpm", .units = "rpm", .min_valid = 0.0, .max_valid = 12'000.0};
constexpr SensorConfig kTemp{
    .name = "temp_core", .units = "degC", .min_valid = -40.0, .max_valid = 125.0};
// Skipped members are value-initialized (but -Wextra warns).
// SensorConfig bad{.units = "V", .name = "x"};   // error: wrong order
// [/snippet]

int main() { std::printf("%s %.0f %s %.0f\n", kRpm.name, kRpm.max_valid, kTemp.units ? kTemp.units : "(null)", kTemp.min_valid); }
