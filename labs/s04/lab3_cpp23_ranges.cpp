// Lab 4.3: C++23 ranges (chunk_by, fold_left, enumerate)
// Session: s04
// Compiler Explorer: https://godbolt.org/z/8jGxP9Yhj
// Ten minutes, three TODOs in order. Build and run after each one; the output must not
// change. Finished version: lab3_cpp23_ranges_solution.cpp
#include <algorithm>
#include <cstdio>
#include <numeric>
#include <ranges>
#include <string>
#include <vector>

struct Reading { std::string sensor; double value; };

std::vector<Reading> load() {   // already sorted by sensor, as a log usually is after a sort
    return {{"psi", 31}, {"psi", 29}, {"rpm", 3200}, {"rpm", 3350}, {"rpm", 3100}, {"temp", 88.5}, {"temp", 91.0}};
}

int main() {
    std::vector<Reading> rs = load();

    // TODO 1: the hand-written group-by: a running "previous sensor" and an index. Replace
    //         it with views::chunk_by, which yields one subrange per run of equal keys:
    //             for (auto group : rs | std::views::chunk_by(same_sensor)) { ... }
    //         where same_sensor compares two Readings' sensor fields. Inside, group.size()
    //         (or std::ranges::distance) gives the count and group.front() the first one.
    std::size_t i = 0;
    while (i < rs.size()) {
        std::size_t j = i;
        while (j < rs.size() && rs[j].sensor == rs[i].sensor) ++j;
        std::printf("%s: %zu readings\n", rs[i].sensor.c_str(), j - i);
        i = j;
    }

    // TODO 2: std::accumulate needs begin/end, an initial value whose TYPE decides the
    //         result type (0 here would truncate!), and a lambda that unpacks the element.
    //         Use std::ranges::fold_left over a transform view of the values:
    //             std::ranges::fold_left(rs | std::views::transform(&Reading::value), 0.0, std::plus<>{})
    double total = std::accumulate(rs.begin(), rs.end(), 0.0,
                                   [](double acc, const Reading& r) { return acc + r.value; });
    std::printf("total: %.1f\n", total);

    // TODO 3: an index loop whose index is only used for printing. Use views::enumerate:
    //             for (auto [idx, r] : rs | std::views::enumerate) ...
    //         (idx is a signed difference_type; cast or use %td. libstdc++ 14 has it; libc++ 18 does not.)
    for (std::size_t k = 0; k < rs.size(); ++k) {
        if (rs[k].value > 3000) std::printf("#%zu %s=%.0f\n", k, rs[k].sensor.c_str(), rs[k].value);
    }
}
