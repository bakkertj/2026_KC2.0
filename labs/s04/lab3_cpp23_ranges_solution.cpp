// Lab 4.3 solution: C++23 ranges (chunk_by, fold_left, enumerate)
// Session: s04
// Compiler Explorer: https://godbolt.org/z/7vPYaxrMq
#include <algorithm>
#include <cstdio>
#include <functional>
#include <ranges>
#include <string>
#include <vector>

struct Reading { std::string sensor; double value; };

std::vector<Reading> load() {
    return {{"psi", 31}, {"psi", 29}, {"rpm", 3200}, {"rpm", 3350}, {"rpm", 3100}, {"temp", 88.5}, {"temp", 91.0}};
}

int main() {
    std::vector<Reading> rs = load();

    // TODO 1 done: chunk_by turns "runs of equal keys" into a view of subranges.
    auto same_sensor = [](const Reading& a, const Reading& b) { return a.sensor == b.sensor; };
    for (auto group : rs | std::views::chunk_by(same_sensor)) {
        std::printf("%s: %zu readings\n", group.front().sensor.c_str(), static_cast<std::size_t>(group.size()));
    }

    // TODO 2 done: fold_left takes a range; the init value's type still sets the result type,
    // but there is no begin/end and no unpacking lambda.
    double total = std::ranges::fold_left(rs | std::views::transform(&Reading::value), 0.0, std::plus<>{});
    std::printf("total: %.1f\n", total);

    // TODO 3 done: the index comes with the element; no manual counter.
    // (libc++ 18 does not ship views::enumerate yet, hence the feature-test macro.)
#if defined(__cpp_lib_ranges_enumerate)
    for (auto [idx, r] : rs | std::views::enumerate) {
        if (r.value > 3000) std::printf("#%zu %s=%.0f\n", static_cast<std::size_t>(idx), r.sensor.c_str(), r.value);
    }
#else
    for (std::size_t k = 0; k < rs.size(); ++k) {
        if (rs[k].value > 3000) std::printf("#%zu %s=%.0f\n", k, rs[k].sensor.c_str(), rs[k].value);
    }
#endif
}
