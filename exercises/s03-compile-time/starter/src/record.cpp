#include "telemetry/record.h"

namespace telemetry {

std::string_view to_string(Status s) {
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

std::optional<Status> parse_status(std::string_view text) {
    using enum Status;
    if (text == "ok") return Ok;
    if (text == "suspect") return Suspect;
    if (text == "fault") return Fault;
    return std::nullopt;
}

}  // namespace telemetry
