// Lab 2.2: maybe, either (optional and variant)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/r3s9Gx8x8
// Ten minutes, three TODOs in order. Build and run after each one.
// Finished version: lab2_optional_variant_solution.cpp
#include <cstdint>
#include <cstdio>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// ---- Part A: "maybe" -------------------------------------------------------------

// TODO 1: a bool plus an out-parameter. Return std::optional<std::string_view> instead
//         (the names are string literals, so a view is safe), and update the caller in
//         main to use value_or("?").
bool find_unit(std::string_view sensor, std::string* out) {
    if (sensor == "rpm")  { *out = "1/min"; return true; }
    if (sensor == "temp") { *out = "degC";  return true; }
    return false;
}

// TODO 2: parse_reading returns an optional. scaled() unwraps it by hand with an if.
//         Rewrite scaled() as one expression using .transform() (and nothing else):
//         scale the value by 0.5. The type stays std::optional<double>.
std::optional<double> parse_reading(std::string_view s) {
    if (s.empty() || s[0] < '0' || s[0] > '9') return std::nullopt;
    return std::stod(std::string(s));
}
std::optional<double> scaled(std::string_view s) {
    auto v = parse_reading(s);
    if (!v) return std::nullopt;
    return *v * 0.5;
}

// ---- Part B: "either" ------------------------------------------------------------

// TODO 3: Message is a hand-rolled tagged union: a kind enum plus every field of every
//         kind. Replace it with three structs and
//             using Message = std::variant<Reading, Heartbeat, Fault>;
//         then rewrite handle() with std::visit and the `overloaded` helper below.
//         (Delete the enum; if you forget to handle a kind, it no longer compiles.)
struct Message {
    enum Kind { Reading, Heartbeat, Fault } kind;
    std::uint16_t sensor = 0; double value = 0;     // Reading
    std::uint32_t uptime_s = 0;                     // Heartbeat
    std::uint16_t code = 0; std::string detail;     // Fault
};

template <class... Fs> struct overloaded : Fs... { using Fs::operator()...; };
template <class... Fs> overloaded(Fs...) -> overloaded<Fs...>;

void handle(const Message& m) {
    switch (m.kind) {
    case Message::Reading:   std::printf("reading %u = %.2f\n", m.sensor, m.value); break;
    case Message::Heartbeat: std::printf("alive %us\n", m.uptime_s); break;
    case Message::Fault:     std::printf("FAULT %u: %s\n", m.code, m.detail.c_str()); break;
    }
}

int main() {
    std::string unit;
    std::printf("rpm unit: %s\n", find_unit("rpm", &unit) ? unit.c_str() : "?");
    std::printf("psi unit: %s\n", find_unit("psi", &unit) ? unit.c_str() : "?");

    std::printf("scaled: %.2f %.2f\n", scaled("42").value_or(-1), scaled("x").value_or(-1));

    std::vector<Message> log;
    log.push_back({Message::Reading, 3, 11.5, 0, 0, {}});
    log.push_back({Message::Heartbeat, 0, 0, 120, 0, {}});
    log.push_back({Message::Fault, 0, 0, 0, 7, "over temperature"});
    for (const auto& m : log) handle(m);
}
