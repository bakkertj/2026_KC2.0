// Demo: monadic optional (C++23)
// Session: s02
// Compiler Explorer: <add short link>
#include <cstdio>
#include <optional>
#include <string>
#include <string_view>

struct SensorConfig { std::string_view name; std::string_view units; };
std::optional<SensorConfig> find_sensor(std::string_view n) {
    if (n == "rpm") return SensorConfig{"rpm", "rpm"};
    return std::nullopt;
}
std::optional<int> to_int(std::string_view s) {
    if (s.empty()) return std::nullopt;
    return static_cast<int>(s.size());
}

// [snippet: before]
std::string units_cpp17(std::string_view name) {
    auto cfg = find_sensor(name);
    if (!cfg) return "";
    return std::string(cfg->units);
}
// [/snippet]

// [snippet: after]
std::string units(std::string_view name) {
    return std::string(find_sensor(name)
        .transform([](const SensorConfig& c) { return c.units; })   // optional<T> -> optional<U>
        .value_or(""));
}

std::optional<int> doubled(std::string_view s) {
    return to_int(s)
        .and_then([](int n) -> std::optional<int> { return n > 100 ? std::nullopt : std::optional{n}; })  // may fail
        .transform([](int n) { return n * 2; })                                                          // cannot fail
        .or_else([] { return std::optional{0}; });                                                       // recover
}
// [/snippet]

// [snippet: trap]
// find_sensor(name).transform(&SensorConfig::units)   // does not compile: on a temporary optional
//                                                     // the member yields string_view&&, and
//                                                     // optional<T&&> is ill-formed. Use a lambda.
// [/snippet]

int main() { std::printf("%s %s %d\n", units_cpp17("rpm").c_str(), units("rpm").c_str(), *doubled("abc")); }
