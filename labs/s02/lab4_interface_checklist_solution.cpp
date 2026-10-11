// Lab 2.4 solution: the interface design checklist, applied
// Session: s02
// Compiler Explorer: https://godbolt.org/z/44WPd9nac
#include <cstdio>
#include <expected>
#include <optional>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <vector>

struct Config { std::string name; int rate_hz; };
struct Limits { double lo, hi; };
enum class ConfigError { NotFound, BadRate };

// TODO 1 done: the reason travels with the result, and the result cannot be used unchecked.
std::expected<Config, ConfigError> load_config(std::string_view path) {
    if (path != "sensors.cfg") return std::unexpected(ConfigError::NotFound);
    if (path.size() > 100) return std::unexpected(ConfigError::BadRate);
    return Config{"telemetry", 50};
}

// TODO 2 done: the slice is one argument; "no answer" is nullopt, not false.
std::optional<double> average(std::span<const double> xs) {
    if (xs.empty()) return std::nullopt;
    double sum = 0;
    for (double x : xs) sum += x;
    return sum / static_cast<double>(xs.size());
}

// TODO 3 done: the two numbers that belong together come back together, or not at all.
std::optional<Limits> limits_for(std::string_view sensor) {
    if (sensor == "rpm")  return Limits{0, 12000};
    if (sensor == "temp") return Limits{-40, 125};
    return std::nullopt;
}

std::string_view name(ConfigError e) {
    return e == ConfigError::NotFound ? "not found" : "bad rate";
}

int main() {
    for (std::string_view path : {"sensors.cfg", "missing.cfg"}) {
        if (auto cfg = load_config(path)) std::println("config {} at {} Hz", cfg->name, cfg->rate_hz);
        else                              std::println("config error: {}", name(cfg.error()));
    }

    std::vector<double> samples{10, 20, 30, 40, 50, 60};
    std::span all{samples};
    if (auto a = average(all))                std::println("avg all   {:.1f}", *a);
    if (auto a = average(all.subspan(2, 3)))  std::println("avg 2..5  {:.1f}", *a);
    if (!average(all.subspan(3, 0)))          std::println("avg empty: none");

    if (auto l = limits_for("rpm")) std::println("rpm limits {} to {}", l->lo, l->hi);
    if (!limits_for("psi"))         std::println("psi limits: unknown sensor");
}
