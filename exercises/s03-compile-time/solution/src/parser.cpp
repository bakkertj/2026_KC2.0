#include "telemetry/parser.h"

#include <charconv>
#include <cstdlib>
#include <istream>
#include <optional>
#include <system_error>
#include <version>

#include "telemetry/config.h"

namespace telemetry {

std::string_view to_string(ParseError e) {
    using enum ParseError;
    switch (e) {
    case EmptyLine:
        return "empty line";
    case LineTooLong:
        return "line too long";
    case MissingField:
        return "missing field";
    case BadTimestamp:
        return "bad timestamp";
    case UnknownSensor:
        return "unknown sensor";
    case BadValue:
        return "bad value";
    case OutOfRange:
        return "out of range";
    case UnknownStatus:
        return "unknown status";
    }
    return "?";
}

std::vector<std::string_view> split(std::string_view text, char delimiter) {
    std::vector<std::string_view> fields;
    while (true) {
        const auto pos = text.find(delimiter);
        fields.push_back(text.substr(0, pos));
        if (pos == std::string_view::npos) break;
        text.remove_prefix(pos + 1);
    }
    return fields;
}

namespace {

// The whole of text as a number of type T, or nothing. Works on a view: no
// NUL terminator needed, so a field sliced out of a line parses in place.
template <typename T>
[[nodiscard]] std::optional<T> parse_number(std::string_view text) {
    T value{};
    const char* const last = text.data() + text.size();
    if (auto [ptr, ec] = std::from_chars(text.data(), last, value); ec != std::errc{} || ptr != last) {
        return std::nullopt;
    }
    return value;
}

// libc++ before 20 has no floating-point from_chars (feature-test macro
// __cpp_lib_to_chars is undefined there). strtod needs a NUL, so copy the field.
#ifndef __cpp_lib_to_chars
template <>
[[nodiscard]] std::optional<double> parse_number<double>(std::string_view text) {
    if (text.empty()) return std::nullopt;
    const std::string copy(text);
    char* end = nullptr;
    const double v = std::strtod(copy.c_str(), &end);
    if (*end != '\0') return std::nullopt;
    return v;
}
#endif

}  // namespace

std::expected<Record, ParseError> parse_record(std::string_view line) {
    using enum ParseError;
    if (line.empty()) return std::unexpected(EmptyLine);
    if (line.size() > kMaxLineLength) return std::unexpected(LineTooLong);

    const auto fields = split(line, ',');
    if (fields.size() < 3) return std::unexpected(MissingField);

    const auto ts = parse_number<Timestamp>(fields[0]);
    if (!ts) return std::unexpected(BadTimestamp);

    const auto cfg = find_sensor(fields[1]);
    if (!cfg) return std::unexpected(UnknownSensor);

    const auto value = parse_number<double>(fields[2]);
    if (!value) return std::unexpected(BadValue);
    if (!cfg->in_range(*value)) return std::unexpected(OutOfRange);

    // optional's monadic interface (C++23): a missing field is Ok, a present but
    // unknown one is an error. value_or handles the first case in one expression.
    const auto status = fields.size() > 3 ? parse_status(fields[3]) : std::optional{Status::Ok};
    if (!status) return std::unexpected(UnknownStatus);

    return Record{.ts = *ts, .sensor = std::string(fields[1]), .value = *value, .status = *status};
}

LoadResult load_stream(std::istream& in) {
    LoadResult result;
    std::string line;
    while (std::getline(in, line)) {
        ++result.lines_read;
        if (line.ends_with('\r')) line.pop_back();
        if (auto parsed = parse_record(line)) {
            result.records.push_back(std::move(*parsed));
        } else if (auto [it, inserted] = result.rejected.try_emplace(parsed.error(), 1); !inserted) {
            ++it->second;
        }
    }
    return result;
}

}  // namespace telemetry
