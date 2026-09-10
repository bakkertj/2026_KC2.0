#pragma once
#include <expected>
#include <format>
#include <iosfwd>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "telemetry/record.h"

namespace telemetry {

enum class ParseError {
    EmptyLine,
    LineTooLong,
    MissingField,
    BadTimestamp,
    UnknownSensor,
    BadValue,
    OutOfRange,
    UnknownStatus,
};
// Note: no ParseError::None. An error type no longer needs a "not an error" value,
// because std::expected carries either a Record or a ParseError, never both.

[[nodiscard]] std::string_view to_string(ParseError e);

// Splits text on a delimiter into views of the same buffer. No copies.
// Adjacent delimiters produce empty fields.
[[nodiscard]] std::vector<std::string_view> split(std::string_view text, char delimiter);

// Parses one line. Returns the Record, or the reason it was rejected.
// A missing status field defaults to Status::Ok.
[[nodiscard]] std::expected<Record, ParseError> parse_record(std::string_view line);

struct LoadResult {
    std::vector<Record> records;                 // parsed successfully, in input order
    std::map<ParseError, std::size_t> rejected;  // count of rejected lines by reason
    std::size_t lines_read = 0;
};

// Parses every line of the stream. Blank lines are counted as EmptyLine
// rejections, not errors.
[[nodiscard]] LoadResult load_stream(std::istream& in);

}  // namespace telemetry

template <>
struct std::formatter<telemetry::ParseError> : std::formatter<std::string_view> {
    template <typename Ctx>
    auto format(telemetry::ParseError e, Ctx& ctx) const {
        return std::formatter<std::string_view>::format(telemetry::to_string(e), ctx);
    }
};
