#include "telemetry.h"
#include <charconv>
#include <sstream>

bool parse_record(const std::string& line, Record* out) {
    std::istringstream in(line);
    std::string ts, sensor, value;
    if (!std::getline(in, ts, ',') || !std::getline(in, sensor, ',') || !std::getline(in, value)) {
        return false;
    }
    if (auto [p, ec] = std::from_chars(ts.data(), ts.data() + ts.size(), out->ts);
        ec != std::errc{} || p != ts.data() + ts.size()) {
        return false;
    }
    out->sensor = sensor;
    if (auto [p, ec] = std::from_chars(value.data(), value.data() + value.size(), out->value);
        ec != std::errc{} || p != value.data() + value.size()) {
        return false;
    }
    return true;
}

SensorCounts count_by_sensor(const std::vector<Record>& records) {
    SensorCounts counts;
    for (const auto& r : records) {
        if (auto [it, inserted] = counts.insert({r.sensor, 1}); !inserted) {
            ++it->second;
        }
    }
    return counts;
}

std::pair<double, double> value_range(const std::vector<Record>& records) {
    auto [lo, hi] = std::pair{records[0].value, records[0].value};
    for (const auto& r : records) {
        if (r.value < lo) lo = r.value;
        if (r.value > hi) hi = r.value;
    }
    return {lo, hi};
}
