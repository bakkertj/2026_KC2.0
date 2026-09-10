// Demo: inline variables (C++17)
// Session: s01
// Compiler Explorer: <add short link>
#include <cstdio>
#include <string>

// [snippet: before]
// header:   extern const std::size_t kMaxLineLength;
// one .cpp: const std::size_t kMaxLineLength = 256;
// (or a static per TU, or a function returning a static)
// [/snippet]

// [snippet: after]
// header, and nowhere else:
inline constexpr std::size_t kMaxLineLength = 256;

struct Limits {
    static inline const std::string kUnits = "raw";  // members too
};
// [/snippet]

int main() { std::printf("%zu %s\n", kMaxLineLength, Limits::kUnits.c_str()); }
