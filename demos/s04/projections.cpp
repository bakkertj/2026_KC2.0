// Demo: projections (C++20)
// Session: s04
// Compiler Explorer: https://godbolt.org/z/r95e9TbcM
#include <algorithm>
#include <array>
#include <print>
#include <string>
#include <string_view>
#include <vector>

struct Record { long long ts; std::string sensor; double value; };
struct SensorConfig { std::string_view name; double max; };
constexpr std::array<SensorConfig, 2> kSensors{{{"rpm", 12000.0}, {"temp", 125.0}}};

// [snippet: projections]
// Applied to each element BEFORE the algorithm compares it. Any invocable works.
void examples(std::vector<Record>& v, std::string_view name, long long ts) {
    auto [lo, hi] = std::ranges::minmax(v, {}, &Record::value);          // min and max RECORDS, by value
    auto it = std::ranges::lower_bound(v, ts, {}, &Record::ts);          // search timestamps
    auto cfg = std::ranges::find(kSensors, name, &SensorConfig::name);   // search names, get the config
    auto n = std::ranges::count(v, "rpm", &Record::sensor);              // count by sensor
    std::ranges::sort(v, std::ranges::greater{}, &Record::value);       // descending by value
    auto longest = std::ranges::max(v, {}, [](const Record& r) { return r.sensor.size(); });
    std::println("{} {} {} {} {} {}", lo.value, hi.value, it - v.begin(), cfg->max, n, longest.sensor);
}
// [/snippet]

int main() {
    std::vector<Record> v{{1, "rpm", 3.0}, {2, "temperature", 1.0}, {3, "rpm", 2.0}};
    examples(v, "temp", 2);
}
