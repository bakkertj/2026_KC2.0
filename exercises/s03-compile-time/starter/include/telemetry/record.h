// Telemetry record processor after Session 2: vocabulary types.
//
// Interfaces now say what they mean. optional means "maybe", expected means
// "or this error", string_view means "I will only look", span means "a
// contiguous run I do not own". printf is gone; std::print and std::formatter
// replace it. Behavior is unchanged: the report is byte-identical to Session 1.
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

[[nodiscard]] std::string_view to_string(Status s);

// The status named by text, or nothing if the name is unknown.
[[nodiscard]] std::optional<Status> parse_status(std::string_view text);

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
