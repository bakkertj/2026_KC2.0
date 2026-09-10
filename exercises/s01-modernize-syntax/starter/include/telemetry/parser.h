#pragma once
#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "telemetry/record.h"

namespace telemetry {

enum class ParseError {
    None,
    EmptyLine,
    LineTooLong,
    MissingField,
    BadTimestamp,
    UnknownSensor,
    BadValue,
    OutOfRange,
    UnknownStatus,
};

const char* to_string(ParseError e);

// Splits text on a delimiter. Adjacent delimiters produce empty fields.
std::vector<std::string> split(const std::string& text, char delimiter);

// Parses one line into *out. On failure returns false and sets *err.
// A missing status field defaults to Status::Ok.
bool parse_record(const std::string& line, Record* out, ParseError* err);

struct LoadResult {
    std::vector<Record> records;             // parsed successfully, in input order
    std::map<ParseError, std::size_t> rejected;  // count of rejected lines by reason
    std::size_t lines_read;
};

// Parses every line of the stream. Blank lines are counted as EmptyLine
// rejections, not errors.
LoadResult load_stream(std::istream& in);

}  // namespace telemetry
