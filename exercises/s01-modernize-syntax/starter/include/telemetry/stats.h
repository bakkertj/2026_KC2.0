#pragma once
#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "telemetry/record.h"

namespace telemetry {

// Running statistics for one sensor (Welford's algorithm for the variance).
struct SensorStats {
    std::size_t count;
    double min;
    double max;
    double mean;
    double m2;  // sum of squared deviations from the mean
    std::size_t faults;  // records with Status::Fault

    SensorStats();
    void add(double value, Status status);
    double variance() const;  // population variance; 0 when count < 2
};

typedef std::map<std::string, SensorStats> StatsBySensor;

// Statistics for every sensor that appears in records.
StatsBySensor compute_stats(const std::vector<Record>& records);

// (min, max) over all values. records must not be empty.
std::pair<double, double> value_range(const std::vector<Record>& records);

// The n largest readings for one sensor, highest first. Ties keep input order.
std::vector<Record> top_n_by_value(const std::vector<Record>& records, const std::string& sensor,
                                   std::size_t n);

// Index of the first record at or after ts, or records.size(). records must be
// sorted by timestamp.
std::size_t first_at_or_after(const std::vector<Record>& records, Timestamp ts);

}  // namespace telemetry
