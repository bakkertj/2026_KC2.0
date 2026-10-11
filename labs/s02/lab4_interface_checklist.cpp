// Lab 2.4: the interface design checklist, applied
// Session: s02
// Compiler Explorer: https://godbolt.org/z/zzMKsTnsv
// Ten minutes. Three C++11 signatures, each hiding something the type system could say.
// Rewrite each one with today's vocabulary, then fix the callers in main. Build and run
// after each TODO. Finished version: lab4_interface_checklist_solution.cpp
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

// TODO 1: returns 0 on success or an error number, fills *out on success only. The
//         caller has to know the numbering and remember the check. Make it
//             std::expected<Config, ConfigError> load_config(std::string_view path);
//         with  enum class ConfigError { NotFound, BadRate };
int load_config(const std::string& path, Config* out) {
    if (path != "sensors.cfg") return 2;          // 2 = not found
    if (path.size() > 100) return 3;              // 3 = bad rate (pretend)
    *out = Config{"telemetry", 50};
    return 0;
}

// TODO 2: a container plus two indices plus an out-parameter, and "false" for an empty
//         slice. Make it
//             std::optional<double> average(std::span<const double> xs);
//         and have the callers pass a subspan instead of begin/end.
bool average(const std::vector<double>& v, std::size_t begin, std::size_t end, double* out) {
    if (begin >= end) return false;
    double sum = 0;
    for (std::size_t i = begin; i < end; ++i) sum += v[i];
    *out = sum / static_cast<double>(end - begin);
    return true;
}

// TODO 3: two out-parameters that only mean something together. Return
//             std::optional<Limits> limits_for(std::string_view sensor);
bool limits_for(const std::string& sensor, double& lo, double& hi) {
    if (sensor == "rpm")  { lo = 0;   hi = 12000; return true; }
    if (sensor == "temp") { lo = -40; hi = 125;   return true; }
    return false;
}

int main() {
    Config cfg;
    int rc = load_config("sensors.cfg", &cfg);
    if (rc == 0) std::println("config {} at {} Hz", cfg.name, cfg.rate_hz);
    else         std::println("config error {}", rc);
    rc = load_config("missing.cfg", &cfg);
    if (rc == 0) std::println("config {} at {} Hz", cfg.name, cfg.rate_hz);
    else         std::println("config error {}", rc);

    std::vector<double> samples{10, 20, 30, 40, 50, 60};
    double avg = 0;
    if (average(samples, 0, samples.size(), &avg)) std::println("avg all   {:.1f}", avg);
    if (average(samples, 2, 5, &avg))              std::println("avg 2..5  {:.1f}", avg);
    if (!average(samples, 3, 3, &avg))             std::println("avg empty: none");

    double lo = 0, hi = 0;
    if (limits_for("rpm", lo, hi)) std::println("rpm limits {} to {}", lo, hi);
    if (!limits_for("psi", lo, hi)) std::println("psi limits: unknown sensor");
}
