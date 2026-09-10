// Demo: [[nodiscard]] (C++17), [[maybe_unused]] and [[fallthrough]] (C++17)
// Session: s01
// Compiler Explorer: <add short link>
#include <cstdio>
#include <string>

// [snippet: nodiscard]
[[nodiscard]] bool parse(const std::string& s, int* out);   // dropping it is a bug
[[nodiscard("released immediately if dropped")]] int acquire();   // C++20: reason

struct [[nodiscard]] Error { int code; };   // applies to every function returning Error
Error try_write();
// [/snippet]

// [snippet: others]
enum class Level { Trace, Debug, Info };

int verbosity(Level lvl, [[maybe_unused]] bool color) {   // used only in some builds
    switch (lvl) {
    case Level::Trace:
        std::puts("trace on");
        [[fallthrough]];                                  // intentional, and checked
    case Level::Debug:
        return 2;
    case Level::Info:
        return 1;
    }
    return 0;
}
// [/snippet]

bool parse(const std::string& s, int* out) { *out = static_cast<int>(s.size()); return true; }
int acquire() { return 1; }
Error try_write() { return {0}; }

int main() {
    int n = 0;
#ifdef SHOW_ERRORS
    parse("abc", &n);     // warning: ignoring return value declared with [[nodiscard]]
    try_write();          // warning: same, via the type
#endif
    if (parse("abc", &n)) std::printf("%d %d %d\n", n, acquire(), verbosity(Level::Trace, true));
}
