#include "telemetry/record.h"

namespace telemetry {

const char* to_string(Status s) {
    switch (s) {
    case Status::Ok:
        return "ok";
    case Status::Suspect:
        return "suspect";
    case Status::Fault:
        return "fault";
    }
    return "?";
}

bool status_from_string(const std::string& text, Status* out) {
    if (text == "ok") {
        *out = Status::Ok;
        return true;
    }
    if (text == "suspect") {
        *out = Status::Suspect;
        return true;
    }
    if (text == "fault") {
        *out = Status::Fault;
        return true;
    }
    return false;
}

bool operator==(const Record& a, const Record& b) {
    return a.ts == b.ts && a.sensor == b.sensor && a.value == b.value && a.status == b.status;
}

bool operator<(const Record& a, const Record& b) {
    if (a.ts != b.ts) return a.ts < b.ts;
    if (a.sensor != b.sensor) return a.sensor < b.sensor;
    if (a.value != b.value) return a.value < b.value;
    return static_cast<int>(a.status) < static_cast<int>(b.status);
}

bool operator!=(const Record& a, const Record& b) { return !(a == b); }
bool operator>(const Record& a, const Record& b) { return b < a; }
bool operator<=(const Record& a, const Record& b) { return !(b < a); }
bool operator>=(const Record& a, const Record& b) { return !(a < b); }

}  // namespace telemetry
