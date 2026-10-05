// Demo: the same task in C++11, C++20, and C++23
// Session: s04
// Compiler Explorer: https://godbolt.org/z/acK9aq7d6
// Task: for each sensor, print the top 2 readings by value, highest first.
#include <algorithm>
#include <map>
#include <print>
#include <ranges>
#include <string>
#include <vector>

struct Record { long long ts; std::string sensor; double value; };

// [snippet: cpp11]
void cpp11(const std::vector<Record>& records) {
    std::map<std::string, std::vector<Record>> groups;
    for (std::vector<Record>::const_iterator it = records.begin(); it != records.end(); ++it)
        groups[it->sensor].push_back(*it);
    for (std::map<std::string, std::vector<Record>>::iterator g = groups.begin(); g != groups.end(); ++g) {
        std::vector<Record>& rs = g->second;
        std::sort(rs.begin(), rs.end(), [](const Record& a, const Record& b) { return a.value > b.value; });
        for (std::size_t i = 0; i < rs.size() && i < 2; ++i)
            std::println("{} #{} {}", g->first, i + 1, rs[i].value);
    }
}
// [/snippet]

// [snippet: cpp20]
void cpp20(std::vector<Record> records) {
    std::ranges::sort(records, {}, &Record::sensor);
    auto it = records.begin();
    while (it != records.end()) {                        // no chunk_by yet: find each run by hand
        auto end = std::ranges::find_if(it, records.end(), [&](const Record& r) { return r.sensor != it->sensor; });
        std::ranges::sort(it, end, std::ranges::greater{}, &Record::value);
        std::size_t i = 0;
        for (const Record& r : std::ranges::subrange(it, end) | std::views::take(2))
            std::println("{} #{} {}", r.sensor, ++i, r.value);
        it = end;
    }
}
// [/snippet]

// [snippet: cpp23]
void cpp23(std::vector<Record> records) {
    std::ranges::sort(records, {}, &Record::sensor);
    for (auto group : records | std::views::chunk_by([](const Record& a, const Record& b) { return a.sensor == b.sensor; })) {
        std::ranges::sort(group, std::ranges::greater{}, &Record::value);
        for (auto [i, r] : std::views::zip(std::views::iota(1), group | std::views::take(2)))
            std::println("{} #{} {}", r.sensor, i, r.value);
    }
}
// [/snippet]

int main() {
    std::vector<Record> v{{1, "rpm", 4800}, {2, "temp", 41}, {3, "rpm", 4830}, {4, "temp", 42.5}, {5, "rpm", 4700}};
    cpp11(v); cpp20(v); cpp23(v);
}
