// Lab 2.2 solution: maybe, either (optional and variant)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/hbvE4K9fE
#include <cstdint>
#include <cstdio>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// ---- Part A: "maybe" -------------------------------------------------------------

// TODO 1 done: the signature says "maybe a unit"; no out-parameter, no sentinel.
std::optional<std::string_view> find_unit(std::string_view sensor) {
    if (sensor == "rpm")  return "1/min";
    if (sensor == "temp") return "degC";
    return std::nullopt;
}

std::optional<double> parse_reading(std::string_view s) {
    if (s.empty() || s[0] < '0' || s[0] > '9') return std::nullopt;
    return std::stod(std::string(s));
}
// TODO 2 done: transform applies the function when there is a value and passes nullopt through.
std::optional<double> scaled(std::string_view s) {
    return parse_reading(s).transform([](double v) { return v * 0.5; });
}

// ---- Part B: "either" ------------------------------------------------------------

// TODO 3 done: a closed set of alternatives; each one carries only its own fields.
struct Reading   { std::uint16_t sensor; double value; };
struct Heartbeat { std::uint32_t uptime_s; };
struct Fault     { std::uint16_t code; std::string detail; };
using Message = std::variant<Reading, Heartbeat, Fault>;

template <class... Fs> struct overloaded : Fs... { using Fs::operator()...; };
template <class... Fs> overloaded(Fs...) -> overloaded<Fs...>;

void handle(const Message& m) {
    std::visit(overloaded{
        [](const Reading& r)   { std::printf("reading %u = %.2f\n", r.sensor, r.value); },
        [](const Heartbeat& h) { std::printf("alive %us\n", h.uptime_s); },
        [](const Fault& f)     { std::printf("FAULT %u: %s\n", f.code, f.detail.c_str()); },
    }, m);
}

int main() {
    std::printf("rpm unit: %s\n", std::string(find_unit("rpm").value_or("?")).c_str());
    std::printf("psi unit: %s\n", std::string(find_unit("psi").value_or("?")).c_str());

    std::printf("scaled: %.2f %.2f\n", scaled("42").value_or(-1), scaled("x").value_or(-1));

    std::vector<Message> log;
    log.push_back(Reading{3, 11.5});
    log.push_back(Heartbeat{120});
    log.push_back(Fault{7, "over temperature"});
    for (const auto& m : log) handle(m);
}
