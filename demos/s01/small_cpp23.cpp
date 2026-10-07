// Demo: the small C++23 features (C++23)
// Session: s01
// Compiler Explorer: https://godbolt.org/z/4ofefxc1h
#include <cstddef>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

enum class Level : unsigned char { Low = 1, High = 2 };

// [snippet: small]
void small(std::vector<std::string>& v) {
    for (auto i = 0uz; i < v.size(); ++i) {}              // uz: a size_t literal

    std::erase(v, auto(v.front()));                         // auto(x): explicit decay copy.
    // Without it, erase takes v.front() by reference, and that reference is to an
    // element the erase is moving. The copy cannot alias.

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
    std::vector<std::string> v{"first", "second", "first", "third"};
    small(v);
    std::printf("%zu\n", v.size());
    std::printf("%d\n", classify(5));
}
