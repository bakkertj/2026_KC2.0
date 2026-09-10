#include "telemetry/serialize.h"

#include <cstdio>

namespace telemetry {

std::string serialize(int v) {
    char buf[32];
    std::snprintf(buf, sizeof buf, "%d", v);
    return buf;
}

std::string serialize(long long v) {
    char buf[32];
    std::snprintf(buf, sizeof buf, "%lld", v);
    return buf;
}

std::string serialize(unsigned long v) {
    char buf[32];
    std::snprintf(buf, sizeof buf, "%lu", v);
    return buf;
}

std::string serialize(double v) {
    char buf[64];
    std::snprintf(buf, sizeof buf, "%.3f", v);
    return buf;
}

std::string serialize(const std::string& v) { return "\"" + v + "\""; }

std::string serialize(Status v) { return to_string(v); }

std::string serialize(const Record& v) {
    return "{" + serialize(v.ts) + "," + serialize(v.sensor) + "," + serialize(v.value) + "," +
           serialize(v.status) + "}";
}

}  // namespace telemetry
