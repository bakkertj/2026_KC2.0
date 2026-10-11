// Lab 4.2 solution: views (lazy pipelines, ranges::to, and the const trap)
// Session: s04
// Compiler Explorer: https://godbolt.org/z/je5qz167a
#include <cstdio>
#include <ranges>
#include <string>
#include <vector>

struct Reading { std::string sensor; double value; bool fault; };

std::vector<Reading> load() {
    return {{"rpm", 3200, false}, {"temp", 88.5, false}, {"psi", 31, true},
            {"rpm", 3350, false}, {"temp", 91.0, false}, {"psi", 29, false}, {"rpm", 3100, false}};
}

int main() {
    std::vector<Reading> rs = load();
    auto not_fault = [](const Reading& r) { return !r.fault; };

    // TODO 1 done: one lazy pipeline; no scratch vectors; stops after three results.
    for (double v : rs | std::views::filter(not_fault)
                       | std::views::transform(&Reading::value)
                       | std::views::take(3)) {
        std::printf("%.1f ", v);
    }
    std::printf("\n");

    // TODO 2 done: the same pipeline, collected into a container when you need one.
    auto all_ok = rs | std::views::filter(not_fault)
                     | std::views::transform(&Reading::value)
                     | std::ranges::to<std::vector>();
    std::printf("non-fault count: %zu\n", all_ok.size());

    // TODO 3 done: no const on the view. (The elements are still const through const Reading&.)
    auto hot = rs | std::views::filter([](const Reading& r) { return r.sensor == "temp" && r.value > 90; });
    for (const auto& r : hot) std::printf("hot: %s=%.1f\n", r.sensor.c_str(), r.value);
}
