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

    void add(double value, Status status);
    [[nodiscard]] double variance() const;  // population variance; 0 when count < 2
};

using StatsBySensor = std::map<std::string, SensorStats, std::less<>>;
// std::less<> (transparent comparator, C++14): find() accepts a string_view
// without constructing a std::string.

// Every function below takes std::span<const Record>: a vector, an array, a
// sub-range, anything contiguous, without a copy and without the function
// caring which.

// Statistics for every sensor that appears in records.
[[nodiscard]] StatsBySensor compute_stats(std::span<const Record> records);

// (min, max) over all values, or nothing for an empty span. The precondition
// "records must not be empty" became part of the return type.
[[nodiscard]] std::optional<std::pair<double, double>> value_range(std::span<const Record> records);

// The n largest readings for one sensor, highest first. Ties keep input order.
[[nodiscard]] std::vector<Record> top_n_by_value(std::span<const Record> records,
                                                 std::string_view sensor, std::size_t n);

// Index of the first record at or after ts, or records.size(). records must be
// sorted by timestamp.
[[nodiscard]] std::size_t first_at_or_after(std::span<const Record> records, Timestamp ts);

}  // namespace telemetry
