#include "telemetry.h"
#include <cstdlib>
#include <sstream>

bool operator==(const Record& a, const Record& b) {
    return a.ts == b.ts && a.sensor == b.sensor && a.value == b.value;
}
bool operator<(const Record& a, const Record& b) {
    if (a.ts != b.ts) return a.ts < b.ts;
    if (a.sensor != b.sensor) return a.sensor < b.sensor;
    return a.value < b.value;
}

bool parse_record(const std::string& line, Record* out) {
    std::istringstream in(line);
    std::string ts, sensor, value;
    if (!std::getline(in, ts, ',') || !std::getline(in, sensor, ',') || !std::getline(in, value)) {
        return false;
    }
    char* end = 0;
    out->ts = std::strtoll(ts.c_str(), &end, 10);
    if (*end != '\0') return false;
    out->sensor = sensor;
    out->value = std::strtod(value.c_str(), &end);
    if (*end != '\0') return false;
    return true;
}

SensorCounts count_by_sensor(const std::vector<Record>& records) {
    SensorCounts counts;
    for (std::vector<Record>::const_iterator it = records.begin(); it != records.end(); ++it) {
        std::pair<SensorCounts::iterator, bool> r = counts.insert(std::make_pair(it->sensor, 1));
        if (!r.second) {
            ++r.first->second;
        }
    }
    return counts;
}

std::pair<double, double> value_range(const std::vector<Record>& records) {
    double lo = records[0].value, hi = records[0].value;
    for (size_t i = 1; i < records.size(); ++i) {
        if (records[i].value < lo) lo = records[i].value;
        if (records[i].value > hi) hi = records[i].value;
    }
    return std::make_pair(lo, hi);
}
