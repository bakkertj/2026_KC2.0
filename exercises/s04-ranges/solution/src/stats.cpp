#include "telemetry/stats.h"

#include <algorithm>
#include <functional>
#include <ranges>

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
    // A constrained algorithm with a projection: compare records BY value, and
    // get back the min and max records in one pass. No comparator lambda.
    const auto [lo, hi] = std::ranges::minmax(records, {}, &Record::value);
    return std::pair{lo.value, hi.value};
}

std::vector<Record> top_n_by_value(std::span<const Record> records, std::string_view sensor,
                                   std::size_t n) {
    // A pipeline: keep this sensor's records, materialize, sort by value descending
    // (projection again), keep the first n. Each step says one thing.
    auto matching = records
        | std::views::filter([sensor](const Record& r) { return r.sensor == sensor; })
        | std::ranges::to<std::vector>();                              // C++23
    std::ranges::stable_sort(matching, std::ranges::greater{}, &Record::value);
    if (matching.size() > n) matching.resize(n);
    return matching;
}

std::size_t first_at_or_after(std::span<const Record> records, Timestamp ts) {
    // lower_bound with a projection: search the timestamps without a comparator.
    const auto it = std::ranges::lower_bound(records, ts, {}, &Record::ts);
    return static_cast<std::size_t>(it - records.begin());
}

}  // namespace telemetry
