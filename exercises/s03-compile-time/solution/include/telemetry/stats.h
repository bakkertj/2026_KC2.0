#pragma once
#include <cstddef>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "telemetry/record.h"

namespace telemetry {

// Running statistics for one sensor (Welford's algorithm for the variance).
struct SensorStats {
    std::size_t count = 0;
    double min = 0.0;
    double max = 0.0;
    double mean = 0.0;
    double m2 = 0.0;         // sum of squared deviations from the mean
    std::size_t faults = 0;  // records with Status::Fault

    // Deducing this (C++23): the object parameter is explicit and its type is
    // deduced, so one definition serves lvalues (returns SensorStats&) and
    // rvalues (returns SensorStats&&). Calls can chain: stats.add(1).add(2),
    // and SensorStats{}.add(1).add(2) moves through without a copy.
    template <typename Self>
    constexpr Self&& add(this Self&& self, double value, Status status) {
        if (self.count == 0) {
            self.min = value;
            self.max = value;
        } else {
            self.min = value < self.min ? value : self.min;
            self.max = value > self.max ? value : self.max;
        }
        ++self.count;
        const double delta = value - self.mean;
        self.mean += delta / static_cast<double>(self.count);
        self.m2 += delta * (value - self.mean);
        if (status == Status::Fault) ++self.faults;
        return std::forward<Self>(self);
    }

    [[nodiscard]] constexpr double variance() const {
        return count < 2 ? 0.0 : m2 / static_cast<double>(count);
    }
};

using StatsBySensor = std::map<std::string, SensorStats, std::less<>>;

[[nodiscard]] StatsBySensor compute_stats(std::span<const Record> records);

[[nodiscard]] std::optional<std::pair<double, double>> value_range(std::span<const Record> records);

[[nodiscard]] std::vector<Record> top_n_by_value(std::span<const Record> records,
                                                 std::string_view sensor, std::size_t n);

[[nodiscard]] std::size_t first_at_or_after(std::span<const Record> records, Timestamp ts);

}  // namespace telemetry
