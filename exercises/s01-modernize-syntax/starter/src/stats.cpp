#include "telemetry/stats.h"

#include <algorithm>

namespace telemetry {

SensorStats::SensorStats() : count(0), min(0.0), max(0.0), mean(0.0), m2(0.0), faults(0) {}

void SensorStats::add(double value, Status status) {
    if (count == 0) {
        min = value;
        max = value;
    } else {
        if (value < min) min = value;
        if (value > max) max = value;
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
    for (std::vector<Record>::const_iterator it = records.begin(); it != records.end(); ++it) {
        std::pair<StatsBySensor::iterator, bool> ins =
            stats.insert(std::make_pair(it->sensor, SensorStats()));
        ins.first->second.add(it->value, it->status);
    }
    return stats;
}

std::pair<double, double> value_range(const std::vector<Record>& records) {
    double lo = records[0].value;
    double hi = records[0].value;
    for (std::size_t i = 1; i < records.size(); ++i) {
        if (records[i].value < lo) lo = records[i].value;
        if (records[i].value > hi) hi = records[i].value;
    }
    return std::make_pair(lo, hi);
}

namespace {

struct ValueDescending {
    bool operator()(const Record& a, const Record& b) const { return a.value > b.value; }
};

}  // namespace

std::vector<Record> top_n_by_value(const std::vector<Record>& records, const std::string& sensor,
                                   std::size_t n) {
    std::vector<Record> matching;
    for (std::vector<Record>::const_iterator it = records.begin(); it != records.end(); ++it) {
        if (it->sensor == sensor) matching.push_back(*it);
    }
    std::stable_sort(matching.begin(), matching.end(), ValueDescending());
    if (matching.size() > n) matching.resize(n);
    return matching;
}

namespace {

struct TimestampLess {
    bool operator()(const Record& r, Timestamp ts) const { return r.ts < ts; }
};

}  // namespace

std::size_t first_at_or_after(const std::vector<Record>& records, Timestamp ts) {
    std::vector<Record>::const_iterator it =
        std::lower_bound(records.begin(), records.end(), ts, TimestampLess());
    return static_cast<std::size_t>(it - records.begin());
}

}  // namespace telemetry
