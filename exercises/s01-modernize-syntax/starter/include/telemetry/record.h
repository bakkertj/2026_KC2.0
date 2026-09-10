// Telemetry record processor: the C++11 baseline.
//
// This program reads lines of the form
//     <timestamp>,<sensor>,<value>[,<status>]
// validates them against a sensor configuration table, computes per-sensor
// statistics, and writes a report. It is written in careful, idiomatic C++11
// on purpose: every session of the course modernizes one aspect of it.
#pragma once
#include <string>

namespace telemetry {

// Milliseconds since the epoch.
typedef long long Timestamp;

enum class Status { Ok, Suspect, Fault };

const char* to_string(Status s);

// Returns false if the text is not a known status name.
bool status_from_string(const std::string& text, Status* out);

struct Record {
    Timestamp ts;
    std::string sensor;
    double value;
    Status status;
};

// Records order by timestamp, then sensor, then value, then status.
bool operator==(const Record& a, const Record& b);
bool operator!=(const Record& a, const Record& b);
bool operator<(const Record& a, const Record& b);
bool operator>(const Record& a, const Record& b);
bool operator<=(const Record& a, const Record& b);
bool operator>=(const Record& a, const Record& b);

}  // namespace telemetry
