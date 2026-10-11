// Lab 4.1 solution: constrained algorithms and projections
// Session: s04
// Compiler Explorer: https://godbolt.org/z/67zbenxKq
#include <algorithm>
#include <cstdio>
#include <iterator>
#include <string>
#include <vector>

struct Reading { std::string sensor; double value; bool fault; };

std::vector<Reading> load() {
    return {{"rpm", 3200, false}, {"temp", 88.5, false}, {"psi", 31, true},
            {"rpm", 3350, false}, {"temp", 91.0, false}, {"psi", 29, false}};
}

int main() {
    std::vector<Reading> rs = load();

    // TODO 1 done: the range, the default comparator, and the member to compare by.
    std::ranges::sort(rs, {}, &Reading::value);
    for (const auto& r : rs) std::printf("%s=%.1f ", r.sensor.c_str(), r.value);
    std::printf("\n");

    // TODO 2 done: find by a projected member; no lambda, no begin/end.
    auto it = std::ranges::find(rs, "temp", &Reading::sensor);
    if (it != rs.end()) std::printf("first temp: %.1f\n", it->value);

    // TODO 3 done: max_element with a projection, on a named vector so the iterator is valid.
    auto non_fault = [](const std::vector<Reading>& v) {
        std::vector<Reading> out;
        std::ranges::copy_if(v, std::back_inserter(out), [](const Reading& r) { return !r.fault; });
        return out;
    };
    std::vector<Reading> ok = non_fault(rs);
    auto best = std::ranges::max_element(ok, {}, &Reading::value);
    std::printf("best non-fault: %s=%.1f\n", best->sensor.c_str(), best->value);
    // std::ranges::max_element(non_fault(rs), {}, &Reading::value) returns std::ranges::dangling:
    // a temporary is not a borrowed range, so there is no iterator to give back.
}
