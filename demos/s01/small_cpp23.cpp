// Demo: the small C++23 features (C++23)
// Session: s01
// Compiler Explorer: <add short link>
#include <cstddef>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

enum class Level : unsigned char { Low = 1, High = 2 };

// [snippet: small]
void small(std::vector<std::string>& v) {
    for (auto i = 0uz; i < v.size(); ++i) {}              // uz: a size_t literal

    auto copy = auto(v.front());                            // auto(x): explicit decay copy
    v.erase(v.begin());                                     // ...which survives this erase
    std::printf("%s\n", copy.c_str());

    std::printf("%u\n", std::to_underlying(Level::High));   // no static_cast needed
}

int classify(int x) {
    if (x < 0) return -1;
    if (x == 0) return 0;
    if (x > 0) return 1;
    std::unreachable();                                     // a promise; UB if reached
}
// [/snippet]

// [snippet: preprocessor]
#ifdef NDEBUG
#elifdef TRACE          // C++23: #elifdef / #elifndef
#warning "tracing build"   // C++23: standard at last
#endif
// [/snippet]

int main() {
    std::vector<std::string> v{"first", "second"};
    small(v);
    std::printf("%d\n", classify(5));
}
