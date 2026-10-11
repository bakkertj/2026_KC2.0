// Lab 4.4 solution: the same task, three ways (C++11 loops, C++17 algorithms, C++20/23 ranges)
// Session: s04
// Compiler Explorer: https://godbolt.org/z/ofz3r1Ed3
#include <algorithm>
#include <cstdio>
#include <functional>
#include <iterator>
#include <ranges>
#include <string>
#include <vector>

struct Reading { std::string sensor; double value; bool fault; };

std::vector<Reading> load() {
    return {{"rpm", 3200, false}, {"temp", 88.5, false}, {"psi", 31, true}, {"rpm", 3350, false},
            {"temp", 91.0, false}, {"psi", 29, false}, {"rpm", 3100, false}, {"temp", 95.5, true}};
}

std::string label(const Reading& r) { return r.sensor + "=" + std::to_string(static_cast<int>(r.value)); }

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

// TODO 1 done: the loops become named algorithms; the criteria become lambdas at the call.
std::vector<std::string> top3_cpp17(const std::vector<Reading>& rs) {
    std::vector<Reading> ok;
    std::copy_if(rs.begin(), rs.end(), std::back_inserter(ok), [](const Reading& r) { return !r.fault; });
    std::sort(ok.begin(), ok.end(), [](const Reading& a, const Reading& b) { return a.value > b.value; });
    std::vector<std::string> out;
    std::transform(ok.begin(), ok.begin() + static_cast<std::ptrdiff_t>(std::min<std::size_t>(3, ok.size())),
                   std::back_inserter(out), label);
    return out;
}

// TODO 2 done: the whole task reads top to bottom as the sentence that describes it.
std::vector<std::string> top3_ranges(const std::vector<Reading>& rs) {
    auto ok = rs | std::views::filter([](const Reading& r) { return !r.fault; }) | std::ranges::to<std::vector>();
    std::ranges::sort(ok, std::ranges::greater{}, &Reading::value);
    return ok | std::views::take(3) | std::views::transform(label) | std::ranges::to<std::vector>();
}

int main() {
    std::vector<Reading> rs = load();
    auto a = top3_cpp11(rs), b = top3_cpp17(rs), c = top3_ranges(rs);
    for (const auto& s : a) std::printf("%s ", s.c_str());
    std::printf("\n%s\n", (a == b && b == c) ? "all three agree" : "MISMATCH");
}
