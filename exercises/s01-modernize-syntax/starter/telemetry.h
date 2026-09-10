// Session 1 starter: careful, idiomatic C++11. The full cumulative program
// replaces this placeholder in working session 1; the shape is the same.
#pragma once
#include <map>
#include <string>
#include <utility>
#include <vector>

typedef long long timestamp_t;

struct Record {
    timestamp_t ts;
    std::string sensor;
    double value;
};

bool operator==(const Record& a, const Record& b);
bool operator<(const Record& a, const Record& b);

// Parses "ts,sensor,value". Returns false on malformed input.
bool parse_record(const std::string& line, Record* out);

// Counts records per sensor.
typedef std::map<std::string, int> SensorCounts;
SensorCounts count_by_sensor(const std::vector<Record>& records);

// Returns (min, max) over all values; caller must check records is non-empty.
std::pair<double, double> value_range(const std::vector<Record>& records);
