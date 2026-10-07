// Demo: generic lambdas and init-capture (C++14)
// Session: s01
// Compiler Explorer: https://godbolt.org/z/ov3Me8163
// Slide: slides/01-everyday-language.md
#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

// [snippet: generic]
// auto parameters: one lambda, any type with .value
auto value_descending = [](const auto& a, const auto& b) { return a.value > b.value; };
// [/snippet]

// [snippet: init_capture]
// init-capture moves the unique_ptr into the closure
auto make_printer(std::unique_ptr<std::string> owned) {
    return [s = std::move(owned)] { std::printf("%s\n", s->c_str()); };
}
// [/snippet]

int main() {
    struct Reading { const char* sensor; double value; };
    std::vector<Reading> readings{{"rpm", 4800.0}, {"rpm", 4830.0}, {"rpm", 4795.0}};
    std::sort(readings.begin(), readings.end(), value_descending);
    for (const auto& r : readings) std::printf("%s=%.0f ", r.sensor, r.value);
    std::printf("\n");

    auto print = make_printer(std::make_unique<std::string>("captured by move"));
    print();
}
