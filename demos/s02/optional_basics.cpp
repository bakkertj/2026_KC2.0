// Demo: std::optional (C++17)
// Session: s02
// Compiler Explorer: <add short link>
#include <cstdio>
#include <optional>
#include <string_view>

struct SensorConfig { std::string_view name; double max; };
constexpr SensorConfig table[] = {{"rpm", 12000.0}, {"temp", 125.0}};

// [snippet: before]
// C++11: a pointer that might be null, and "look elsewhere for the object"
const SensorConfig* find_cpp11(std::string_view name) {
    for (const auto& s : table) if (s.name == name) return &s;
    return nullptr;
}
// [/snippet]

// [snippet: after]
// C++17: "maybe a SensorConfig", held inline, no pointer, no allocation
std::optional<SensorConfig> find(std::string_view name) {
    for (const auto& s : table) if (s.name == name) return s;
    return std::nullopt;
}
// [/snippet]

int main() {
    // [snippet: use]
    if (auto cfg = find("rpm")) {                  // contextual bool
        std::printf("%f\n", cfg->max);              // -> and * : unchecked, like a pointer
    }
    double m = find("nope").value_or(SensorConfig{"none", 0.0}).max;   // default when empty
    auto c = find("temp");
    c->max = 1.0;                                   // mutable: optional holds the object itself
    c.reset();                                      // now empty
    // find("nope").value();                        // throws std::bad_optional_access
    // [/snippet]
    std::printf("%f %d\n", m, c.has_value());
}
