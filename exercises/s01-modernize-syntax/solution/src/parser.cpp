#include "telemetry/parser.h"

#include <charconv>
#include <cstdlib>
#include <istream>
#include <system_error>
#include <version>

#include "telemetry/config.h"

namespace telemetry {

const char* to_string(ParseError e) {
    using enum ParseError;
    switch (e) {
    case None:
        return "none";
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

std::vector<std::string> split(const std::string& text, char delimiter) {
    std::vector<std::string> fields;
    std::string current;
    for (const char c : text) {
        if (c == delimiter) {
            fields.push_back(current);
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    fields.push_back(current);
    return fields;
}

namespace {

// Parses the whole of text as a number of type T. std::from_chars never
// touches errno, never allocates, and reports trailing garbage through the
// returned pointer. One function now covers both integers and doubles.
template <typename T>
[[nodiscard]] bool parse_number(const std::string& text, T* out) {
    const char* const first = text.data();
    const char* const last = first + text.size();
    if (auto [ptr, ec] = std::from_chars(first, last, *out); ec != std::errc{} || ptr != last) {
        return false;
    }
    return true;
}

// Feature-test macro (C++20 <version>): libc++ before 20 has no floating-point
// from_chars, and does not define __cpp_lib_to_chars. Fall back to strtod there.
#ifndef __cpp_lib_to_chars
template <>
[[nodiscard]] bool parse_number<double>(const std::string& text, double* out) {
    if (text.empty()) return false;
    char* end = nullptr;
    *out = std::strtod(text.c_str(), &end);
    return *end == '\0';
}
#endif

}  // namespace

bool parse_record(const std::string& line, Record* out, ParseError* err) {
    using enum ParseError;
    if (line.empty()) {
        *err = EmptyLine;
        return false;
    }
    if (line.size() > kMaxLineLength) {
        *err = LineTooLong;
        return false;
    }

    const auto fields = split(line, ',');
    if (fields.size() < 3) {
        *err = MissingField;
        return false;
    }

    Record r{};
    if (!parse_number(fields[0], &r.ts)) {
        *err = BadTimestamp;
        return false;
    }

    const SensorConfig* cfg = find_sensor(fields[1]);
    if (cfg == nullptr) {
        *err = UnknownSensor;
        return false;
    }
    r.sensor = fields[1];

    if (!parse_number(fields[2], &r.value)) {
        *err = BadValue;
        return false;
    }
    if (r.value < cfg->min_valid || r.value > cfg->max_valid) {
        *err = OutOfRange;
        return false;
    }

    r.status = Status::Ok;
    if (fields.size() > 3 && !status_from_string(fields[3], &r.status)) {
        *err = UnknownStatus;
        return false;
    }

    *out = r;
    *err = None;
    return true;
}

LoadResult load_stream(std::istream& in) {
    LoadResult result;
    std::string line;
    while (std::getline(in, line)) {
        ++result.lines_read;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        Record r;
        ParseError err;
        if (parse_record(line, &r, &err)) {
            result.records.push_back(r);
        } else if (auto [it, inserted] = result.rejected.try_emplace(err, 1); !inserted) {
            ++it->second;
        }
    }
    return result;
}

}  // namespace telemetry
