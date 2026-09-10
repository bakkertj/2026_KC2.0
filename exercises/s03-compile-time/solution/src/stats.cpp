#include "telemetry/stats.h"

#include <algorithm>

namespace telemetry {

StatsBySensor compute_stats(std::span<const Record> records) {
    StatsBySensor stats;
    for (const auto& r : records) {
        auto [it, inserted] = stats.try_emplace(r.sensor);
        it->second.add(r.value, r.status);
    }
    return stats;
}

std::optional<std::pair<double, double>> value_range(std::span<const Record> records) {
    if (records.empty()) return std::nullopt;
    auto [lo, hi] = std::pair{records.front().value, records.front().value};
    for (const auto& r : records) {
        lo = std::min(lo, r.value);
        hi = std::max(hi, r.value);
    }
    return std::pair{lo, hi};
}

std::vector<Record> top_n_by_value(std::span<const Record> records, std::string_view sensor,
                                   std::size_t n) {
    std::vector<Record> matching;
    std::copy_if(records.begin(), records.end(), std::back_inserter(matching),
                 [sensor](const Record& r) { return r.sensor == sensor; });
    std::stable_sort(matching.begin(), matching.end(),
                     [](const auto& a, const auto& b) { return a.value > b.value; });
    if (matching.size() > n) matching.resize(n);
    return matching;
}

std::size_t first_at_or_after(std::span<const Record> records, Timestamp ts) {
    const auto it = std::lower_bound(records.begin(), records.end(), ts,
                                     [](const Record& r, Timestamp t) { return r.ts < t; });
    return static_cast<std::size_t>(it - records.begin());
}

}  // namespace telemetry
