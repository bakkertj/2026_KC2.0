#pragma once
#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "telemetry/record.h"

namespace telemetry {

// Running statistics for one sensor (Welford's algorithm for the variance).
// Default member initializers replace the hand-written constructor.
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

using StatsBySensor = std::map<std::string, SensorStats>;

// Statistics for every sensor that appears in records.
[[nodiscard]] StatsBySensor compute_stats(const std::vector<Record>& records);

// (min, max) over all values. records must not be empty.
[[nodiscard]] std::pair<double, double> value_range(const std::vector<Record>& records);

// The n largest readings for one sensor, highest first. Ties keep input order.
[[nodiscard]] std::vector<Record> top_n_by_value(const std::vector<Record>& records,
                                                 const std::string& sensor, std::size_t n);

// Index of the first record at or after ts, or records.size(). records must be
// sorted by timestamp.
[[nodiscard]] std::size_t first_at_or_after(const std::vector<Record>& records, Timestamp ts);

}  // namespace telemetry
