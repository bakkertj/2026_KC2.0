// Lab 4.2: views (lazy pipelines, ranges::to, and the const trap)
// Session: s04
// Compiler Explorer: https://godbolt.org/z/8oe1E7aMb
// Ten minutes, three TODOs in order. Build and run after each one.
// Finished version: lab2_views_solution.cpp
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

    // TODO 1: three eager loops with two scratch vectors. Rewrite as ONE lazy pipeline:
    //         rs | views::filter(not fault) | views::transform(value) | views::take(3)
    //         and range-for over it. Nothing is computed until the loop pulls, and only
    //         four elements are ever looked at.
    std::vector<Reading> ok;
    for (const auto& r : rs) if (!r.fault) ok.push_back(r);
    std::vector<double> values;
    for (const auto& r : ok) values.push_back(r.value);
    for (std::size_t i = 0; i < values.size() && i < 3; ++i) std::printf("%.1f ", values[i]);
    std::printf("\n");

    // TODO 2: the same pipeline, materialized: build a std::vector<double> of ALL non-fault
    //         values with `| std::ranges::to<std::vector>()` (C++23) in place of the loop.
    std::vector<double> all_ok;
    for (const auto& r : rs) if (!r.fault) all_ok.push_back(r.value);
    std::printf("non-fault count: %zu\n", all_ok.size());

    // TODO 3: the const view trap. Uncomment the two lines. They do not compile: a const
    //         filter_view cannot be iterated, because begin() caches its result and is
    //         therefore non-const. Fix it (drop the const; a view is a cheap handle, and
    //         const on the view was never const on the elements), then explain to your
    //         neighbor why `const std::vector&` is fine but `const filter_view` is not.
    // const auto hot = rs | std::views::filter([](const Reading& r) { return r.sensor == "temp" && r.value > 90; });
    // for (const auto& r : hot) std::printf("hot: %s=%.1f\n", r.sensor.c_str(), r.value);
}
