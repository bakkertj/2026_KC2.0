// Demo: range-for with initializer (C++20)
// Session: s01
// Compiler Explorer: <add short link>
#include <cstdio>
#include <string>
#include <vector>

struct Batch {
    std::vector<std::string> names{"a", "b"};
    const std::vector<std::string>& items() const { return names; }
};
Batch load() { return {}; }

int main() {
    // [snippet: bug]
    // for (const auto& n : load().items()) {}   // BUG before C++23: the Batch dies first
    // [/snippet]

    // [snippet: fix]
    for (auto batch = load(); const auto& n : batch.items()) {   // C++20: lives all loop
        std::printf("%s\n", n.c_str());
    }
    // [/snippet]
}
