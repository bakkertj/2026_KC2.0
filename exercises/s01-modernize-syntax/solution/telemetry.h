// Session 1 solution: the same program after the Session 1 tasks.
#pragma once
#include <compare>
#include <map>
#include <string>
#include <utility>
#include <vector>

using timestamp_t = long long;

struct Record {
    timestamp_t ts;
    std::string sensor;
    double value;
    auto operator<=>(const Record&) const = default;
};

[[nodiscard]] bool parse_record(const std::string& line, Record* out);

using SensorCounts = std::map<std::string, int>;
[[nodiscard]] SensorCounts count_by_sensor(const std::vector<Record>& records);

[[nodiscard]] std::pair<double, double> value_range(const std::vector<Record>& records);
