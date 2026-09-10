#include "telemetry/parser.h"

#include <cerrno>
#include <cstdlib>
#include <istream>

#include "telemetry/config.h"

namespace telemetry {

const char* to_string(ParseError e) {
    switch (e) {
    case ParseError::None:
        return "none";
    case ParseError::EmptyLine:
        return "empty line";
    case ParseError::LineTooLong:
        return "line too long";
    case ParseError::MissingField:
        return "missing field";
    case ParseError::BadTimestamp:
        return "bad timestamp";
    case ParseError::UnknownSensor:
        return "unknown sensor";
    case ParseError::BadValue:
        return "bad value";
    case ParseError::OutOfRange:
        return "out of range";
    case ParseError::UnknownStatus:
        return "unknown status";
    }
    return "?";
}

std::vector<std::string> split(const std::string& text, char delimiter) {
    std::vector<std::string> fields;
    std::string current;
    for (std::string::const_iterator it = text.begin(); it != text.end(); ++it) {
        if (*it == delimiter) {
            fields.push_back(current);
            current.clear();
        } else {
            current.push_back(*it);
        }
    }
    fields.push_back(current);
    return fields;
}

namespace {

// Parses the whole of text as a decimal integer. Returns false on any
// trailing garbage, empty input, or overflow.
bool parse_timestamp(const std::string& text, Timestamp* out) {
    if (text.empty()) return false;
    errno = 0;
    char* end = nullptr;
    const long long v = std::strtoll(text.c_str(), &end, 10);
    if (errno == ERANGE || *end != '\0') return false;
    *out = v;
    return true;
}

bool parse_value(const std::string& text, double* out) {
    if (text.empty()) return false;
    errno = 0;
    char* end = nullptr;
    const double v = std::strtod(text.c_str(), &end);
    if (errno == ERANGE || *end != '\0') return false;
    *out = v;
    return true;
}

}  // namespace

bool parse_record(const std::string& line, Record* out, ParseError* err) {
    if (line.empty()) {
        *err = ParseError::EmptyLine;
        return false;
    }
    if (line.size() > kMaxLineLength) {
        *err = ParseError::LineTooLong;
        return false;
    }

    const std::vector<std::string> fields = split(line, ',');
    if (fields.size() < 3) {
        *err = ParseError::MissingField;
        return false;
    }

    Record r;
    if (!parse_timestamp(fields[0], &r.ts)) {
        *err = ParseError::BadTimestamp;
        return false;
    }

    const SensorConfig* cfg = find_sensor(fields[1]);
    if (cfg == nullptr) {
        *err = ParseError::UnknownSensor;
        return false;
    }
    r.sensor = fields[1];

    if (!parse_value(fields[2], &r.value)) {
        *err = ParseError::BadValue;
        return false;
    }
    if (r.value < cfg->min_valid || r.value > cfg->max_valid) {
        *err = ParseError::OutOfRange;
        return false;
    }

    r.status = Status::Ok;
    if (fields.size() > 3 && !status_from_string(fields[3], &r.status)) {
        *err = ParseError::UnknownStatus;
        return false;
    }

    *out = r;
    *err = ParseError::None;
    return true;
}

LoadResult load_stream(std::istream& in) {
    LoadResult result;
    result.lines_read = 0;
    std::string line;
    while (std::getline(in, line)) {
        ++result.lines_read;
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }
        Record r;
        ParseError err;
        if (parse_record(line, &r, &err)) {
            result.records.push_back(r);
        } else {
            std::pair<std::map<ParseError, std::size_t>::iterator, bool> ins =
                result.rejected.insert(std::make_pair(err, static_cast<std::size_t>(1)));
            if (!ins.second) {
                ++ins.first->second;
            }
        }
    }
    return result;
}

}  // namespace telemetry
