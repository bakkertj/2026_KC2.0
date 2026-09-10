#pragma once
#include <string>
#include <vector>

#include "telemetry/record.h"

namespace telemetry {

// Text serialization used by the report and the tests. Session 3 turns this
// overload set into a single constrained template.
[[nodiscard]] std::string serialize(int v);
[[nodiscard]] std::string serialize(long long v);
[[nodiscard]] std::string serialize(unsigned long v);
[[nodiscard]] std::string serialize(double v);
[[nodiscard]] std::string serialize(const std::string& v);
[[nodiscard]] std::string serialize(Status v);
[[nodiscard]] std::string serialize(const Record& v);

template <typename T>
[[nodiscard]] std::string serialize(const std::vector<T>& items) {
    std::string out = "[";
    const char* sep = "";
    for (const auto& item : items) {
        out += sep;
        out += serialize(item);
        sep = ", ";
    }
    out += "]";
    return out;
}

}  // namespace telemetry
