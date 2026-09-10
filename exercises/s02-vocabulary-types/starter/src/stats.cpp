#include "telemetry/stats.h"

#include <algorithm>

namespace telemetry {

void SensorStats::add(double value, Status status) {
    if (count == 0) {
        min = value;
        max = value;
    } else {
        min = std::min(min, value);
        max = std::max(max, value);
    }
    ++count;
    const double delta = value - mean;
    mean += delta / static_cast<double>(count);
    m2 += delta * (value - mean);
    if (status == Status::Fault) ++faults;
}

double SensorStats::variance() const {
    if (count < 2) return 0.0;
    return m2 / static_cast<double>(count);
}

StatsBySensor compute_stats(const std::vector<Record>& records) {
    StatsBySensor stats;
    for (const auto& r : records) {
        // try_emplace: constructs a SensorStats only when the key is new,
        // and the structured binding names both halves of the result.
        auto [it, inserted] = stats.try_emplace(r.sensor);
        it->second.add(r.value, r.status);
    }
    return stats;
}

std::pair<double, double> value_range(const std::vector<Record>& records) {
    auto [lo, hi] = std::pair{records[0].value, records[0].value};
    for (const auto& r : records) {
        lo = std::min(lo, r.value);
        hi = std::max(hi, r.value);
    }
    return {lo, hi};
}

std::vector<Record> top_n_by_value(const std::vector<Record>& records, const std::string& sensor,
                                   std::size_t n) {
    std::vector<Record> matching;
    std::copy_if(records.begin(), records.end(), std::back_inserter(matching),
                 [&sensor](const Record& r) { return r.sensor == sensor; });
    // A generic lambda replaces the ValueDescending function object.
    std::stable_sort(matching.begin(), matching.end(),
                     [](const auto& a, const auto& b) { return a.value > b.value; });
    if (matching.size() > n) matching.resize(n);
    return matching;
}

std::size_t first_at_or_after(const std::vector<Record>& records, Timestamp ts) {
    const auto it = std::lower_bound(records.begin(), records.end(), ts,
                                     [](const Record& r, Timestamp t) { return r.ts < t; });
    return static_cast<std::size_t>(it - records.begin());
}

}  // namespace telemetry
