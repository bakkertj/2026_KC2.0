#include "telemetry/record.h"

namespace telemetry {

const char* to_string(Status s) {
    using enum Status;
    switch (s) {
    case Ok:
        return "ok";
    case Suspect:
        return "suspect";
    case Fault:
        return "fault";
    }
    return "?";
}

bool status_from_string(const std::string& text, Status* out) {
    using enum Status;
    if (text == "ok") {
        *out = Ok;
        return true;
    }
    if (text == "suspect") {
        *out = Suspect;
        return true;
    }
    if (text == "fault") {
        *out = Fault;
        return true;
    }
    return false;
}

}  // namespace telemetry
