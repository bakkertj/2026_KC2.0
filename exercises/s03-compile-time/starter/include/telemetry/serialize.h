#pragma once
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "telemetry/record.h"

namespace telemetry {

// Text serialization used by the report and the tests. Session 3 turns this
// overload set into a single constrained template.
[[nodiscard]] std::string serialize(int v);
[[nodiscard]] std::string serialize(long long v);
[[nodiscard]] std::string serialize(unsigned long v);
[[nodiscard]] std::string serialize(double v);
[[nodiscard]] std::string serialize(std::string_view v);
[[nodiscard]] std::string serialize(Status v);
[[nodiscard]] std::string serialize(const Record& v);

// A span overload serves vectors, arrays, and sub-ranges alike.
template <typename T>
[[nodiscard]] std::string serialize(std::span<const T> items) {
    std::string out = "[";
    std::string_view sep;
    for (const auto& item : items) {
        out += sep;
        out += serialize(item);
        sep = ", ";
    }
    out += "]";
    return out;
}

template <typename T>
[[nodiscard]] std::string serialize(const std::vector<T>& items) {
    return serialize(std::span<const T>{items});
}

}  // namespace telemetry
