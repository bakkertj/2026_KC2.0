// Demo: views::chunk_by, the group-by (C++23)
// Session: s04
// Compiler Explorer: https://godbolt.org/z/7v8cza533
#include <algorithm>
#include <print>
#include <ranges>
#include <string>
#include <vector>

struct Record { long long ts; std::string sensor; double value; };

// [snippet: before]
// C++11: group by sensor with a map of vectors, then iterate the map
#include <map>
void top_per_sensor_cpp11(std::vector<Record> v) {
    std::map<std::string, std::vector<Record>> groups;
    for (const auto& r : v) groups[r.sensor].push_back(r);
    for (auto& g : groups) {                      // no structured bindings yet
        std::vector<Record>& rs = g.second;
        std::sort(rs.begin(), rs.end(), [](const Record& a, const Record& b) { return a.value > b.value; });
        std::println("{}: {}", g.first, rs.front().value);   // println for output only; the rest is C++11
    }
}
// [/snippet]

// [snippet: after]
// C++23: sort by key, then chunk_by splits into one subrange per run of equal keys. No map.
void top_per_sensor(std::vector<Record> v) {
    std::ranges::sort(v, {}, &Record::sensor);
    for (auto group : v | std::views::chunk_by([](const Record& a, const Record& b) { return a.sensor == b.sensor; })) {
        auto best = std::ranges::max(group, {}, &Record::value);
        std::println("{}: {}", group.front().sensor, best.value);
    }
}
// [/snippet]

int main() {
    std::vector<Record> v{{1, "rpm", 4800}, {2, "temp", 41}, {3, "rpm", 4830}, {4, "temp", 42.5}};
    top_per_sensor_cpp11(v);
    top_per_sensor(v);
}
