// Lab 4.1: constrained algorithms and projections
// Session: s04
// Compiler Explorer: https://godbolt.org/z/h7dM73Kc8
// Ten minutes, three TODOs in order. Build and run after each one; the output must not
// change. Finished version: lab1_constrained_algorithms_solution.cpp
#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

struct Reading { std::string sensor; double value; bool fault; };

std::vector<Reading> load() {
    return {{"rpm", 3200, false}, {"temp", 88.5, false}, {"psi", 31, true},
            {"rpm", 3350, false}, {"temp", 91.0, false}, {"psi", 29, false}};
}

int main() {
    std::vector<Reading> rs = load();

    // TODO 1: a lambda that exists only to pick a member. Use std::ranges::sort with a
    //         projection instead:  std::ranges::sort(rs, {}, &Reading::value);
    //         ({} is the default comparator, std::ranges::less.)
    std::sort(rs.begin(), rs.end(), [](const Reading& a, const Reading& b) { return a.value < b.value; });
    for (const auto& r : rs) std::printf("%s=%.1f ", r.sensor.c_str(), r.value);
    std::printf("\n");

    // TODO 2: find the first "temp" reading. Replace find_if + lambda with
    //         std::ranges::find(rs, "temp", &Reading::sensor): the projection is applied
    //         to each element before comparing with the value.
    auto it = std::find_if(rs.begin(), rs.end(), [](const Reading& r) { return r.sensor == "temp"; });
    if (it != rs.end()) std::printf("first temp: %.1f\n", it->value);

    // TODO 3: the highest non-fault reading. Switch to std::ranges::max_element with the
    //         projection &Reading::value (no lambda). Then uncomment the `dangling` line:
    //         it passes a TEMPORARY vector, which the C++11 algorithm would have accepted
    //         and silently left you with an iterator into freed memory. The ranges version
    //         returns std::ranges::dangling instead, so the -> does not compile. Read the
    //         error, then comment it out again.
    auto non_fault = [](const std::vector<Reading>& v) {
        std::vector<Reading> out;
        std::copy_if(v.begin(), v.end(), std::back_inserter(out), [](const Reading& r) { return !r.fault; });
        return out;
    };
    std::vector<Reading> ok = non_fault(rs);
    auto best = std::max_element(ok.begin(), ok.end(),
                                 [](const Reading& a, const Reading& b) { return a.value < b.value; });
    std::printf("best non-fault: %s=%.1f\n", best->sensor.c_str(), best->value);
    // auto dangling = std::ranges::max_element(non_fault(rs), {}, &Reading::value); std::printf("%.1f\n", dangling->value);
}
