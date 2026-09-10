// Telemetry record processor after Session 1: the same program with modern
// syntax. Behavior is unchanged; the shared test suite proves it.
#pragma once
#include <compare>
#include <string>

namespace telemetry {

// Milliseconds since the epoch.
using Timestamp = long long;

enum class Status { Ok, Suspect, Fault };

[[nodiscard]] const char* to_string(Status s);

// Returns false if the text is not a known status name.
[[nodiscard]] bool status_from_string(const std::string& text, Status* out);

struct Record {
    Timestamp ts;
    std::string sensor;
    double value;
    Status status;

    // Records order by timestamp, then sensor, then value, then status:
    // exactly the member order, so the defaulted operator does it.
    auto operator<=>(const Record&) const = default;
};

}  // namespace telemetry
