// Lab 4.4: the same task, three ways (C++11 loops, C++17 algorithms, C++20/23 ranges)
// Session: s04
// Compiler Explorer: <add short link>
// Ten minutes. The task: the three highest non-fault readings, highest first, as
// "sensor=value" strings. top3_cpp11 is done. Write top3_cpp17 with <algorithm> and
// lambdas, then top3_ranges with constrained algorithms and views. main() checks that all
// three agree. Finished version: lab4_three_ways_solution.cpp
#include <algorithm>
#include <cstdio>
#include <ranges>
#include <string>
#include <vector>

struct Reading { std::string sensor; double value; bool fault; };

std::vector<Reading> load() {
    return {{"rpm", 3200, false}, {"temp", 88.5, false}, {"psi", 31, true}, {"rpm", 3350, false},
            {"temp", 91.0, false}, {"psi", 29, false}, {"rpm", 3100, false}, {"temp", 95.5, true}};
}

std::string label(const Reading& r) { return r.sensor + "=" + std::to_string(static_cast<int>(r.value)); }

// Done: careful C++11. Three loops, a hand-written sort criterion, an index.
std::vector<std::string> top3_cpp11(const std::vector<Reading>& rs) {
    std::vector<Reading> ok;
    for (std::size_t i = 0; i < rs.size(); ++i)
        if (!rs[i].fault) ok.push_back(rs[i]);
    struct ByValueDesc { bool operator()(const Reading& a, const Reading& b) const { return a.value > b.value; } };
    std::sort(ok.begin(), ok.end(), ByValueDesc());
    std::vector<std::string> out;
    for (std::size_t i = 0; i < ok.size() && i < 3; ++i) out.push_back(label(ok[i]));
    return out;
}

// TODO 1: C++17. Same shape, but: copy_if with a lambda, sort with a lambda, transform
//         with std::back_inserter for the labels (bound the count with std::min).
std::vector<std::string> top3_cpp17(const std::vector<Reading>& rs) {
    return top3_cpp11(rs);     // replace
}

// TODO 2: C++20/23. filter | ranges::to<vector>, ranges::sort with std::ranges::greater
//         and the projection &Reading::value, then take(3) | transform(label) | to<vector>.
//         No index, no lambda that only picks a member, no scratch loop.
std::vector<std::string> top3_ranges(const std::vector<Reading>& rs) {
    return top3_cpp11(rs);     // replace
}

int main() {
    std::vector<Reading> rs = load();
    auto a = top3_cpp11(rs), b = top3_cpp17(rs), c = top3_ranges(rs);
    for (const auto& s : a) std::printf("%s ", s.c_str());
    std::printf("\n%s\n", (a == b && b == c) ? "all three agree" : "MISMATCH");
}
