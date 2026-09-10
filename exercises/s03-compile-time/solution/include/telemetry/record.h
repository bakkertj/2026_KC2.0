// Telemetry record processor after Session 3: compile-time and generic programming.
//
// Work that used to happen at startup or per call now happens at compile time:
// the CRC table, the configuration table's validation, sensor lookups of
// literal names. The serialize overload set is a set of constrained templates.
// Behavior is unchanged: the report is byte-identical to Session 1.
#pragma once
#include <compare>
#include <format>
#include <optional>
#include <string>
#include <string_view>

namespace telemetry {

// Milliseconds since the epoch.
using Timestamp = long long;

enum class Status { Ok, Suspect, Fault };

[[nodiscard]] constexpr std::string_view to_string(Status s) {
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

// The status named by text, or nothing if the name is unknown.
[[nodiscard]] constexpr std::optional<Status> parse_status(std::string_view text) {
    using enum Status;
    if (text == "ok") return Ok;
    if (text == "suspect") return Suspect;
    if (text == "fault") return Fault;
    return std::nullopt;
}

struct Record {
    Timestamp ts;
    std::string sensor;
    double value;
    Status status;

    auto operator<=>(const Record&) const = default;
};

}  // namespace telemetry

// std::formatter specializations make Status and Record usable in std::format,
// std::print, and std::println with "{}". Record formats as {ts,"sensor",value,status}.
// format() is a template on the context type: the standard requires it to work
// with any basic_format_context, and libc++ checks that at compile time.
template <>
struct std::formatter<telemetry::Status> : std::formatter<std::string_view> {
    template <typename Ctx>
    auto format(telemetry::Status s, Ctx& ctx) const {
        return std::formatter<std::string_view>::format(telemetry::to_string(s), ctx);
    }
};

template <>
struct std::formatter<telemetry::Record> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    template <typename Ctx>
    auto format(const telemetry::Record& r, Ctx& ctx) const {
        return std::format_to(ctx.out(), "{{{},\"{}\",{:.3f},{}}}", r.ts, r.sensor, r.value, r.status);
    }
};
