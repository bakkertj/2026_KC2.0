#include "telemetry/serialize.h"

#include <format>

namespace telemetry {

// std::format replaces snprintf: no buffer, no size, no format/argument mismatch
// (the format string is checked at compile time).
std::string serialize(int v) { return std::format("{}", v); }
std::string serialize(long long v) { return std::format("{}", v); }
std::string serialize(unsigned long v) { return std::format("{}", v); }
std::string serialize(double v) { return std::format("{:.3f}", v); }
std::string serialize(std::string_view v) { return std::format("\"{}\"", v); }
std::string serialize(Status v) { return std::format("{}", v); }
std::string serialize(const Record& v) { return std::format("{}", v); }

}  // namespace telemetry
