#pragma once
#include <string>
#include <vector>

#include "telemetry/record.h"

namespace telemetry {

// Text serialization used by the report and the tests. Session 3 turns this
// overload set into a single constrained template.
std::string serialize(int v);
std::string serialize(long long v);
std::string serialize(unsigned long v);
std::string serialize(double v);
std::string serialize(const std::string& v);
std::string serialize(Status v);
std::string serialize(const Record& v);

template <typename T>
std::string serialize(const std::vector<T>& items) {
    std::string out = "[";
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i != 0) out += ", ";
        out += serialize(items[i]);
    }
    out += "]";
    return out;
}

}  // namespace telemetry
