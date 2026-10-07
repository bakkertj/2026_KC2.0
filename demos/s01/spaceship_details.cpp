// Demo: three-way comparison details (C++20)
// Session: s01
// Compiler Explorer: https://godbolt.org/z/K4ezjY9ME
#include <compare>
#include <cstdio>
#include <string>

// [snippet: categories]
struct Version {
    int major, minor;
    std::strong_ordering operator<=>(const Version&) const = default;
};

struct Reading {
    std::string sensor;
    double value;                                    // NaN exists, so...
    std::partial_ordering operator<=>(const Reading&) const = default;   // (auto deduces this)
};
// [/snippet]

// [snippet: rewriting]
// What the compiler does with a < b when only <=> is declared:
//     a < b      becomes   (a <=> b) < 0
//     a >= b     becomes   (a <=> b) >= 0
//     a > b      becomes   (a <=> b) > 0
//     42 > v     becomes   0 > (v <=> 42)    (reversed: only when the types differ)
// What it does NOT do: derive == from <=>. A defaulted <=> also defaults ==,
// but a hand-written <=> leaves == undeclared. Reason: == can be much faster
// (std::string compares lengths first), so the two are kept separate.
// [/snippet]

// [snippet: order]
struct Bad {
    double value;      // members compare in declaration order,
    long long ts;      // so this orders by value first. Oops.
    auto operator<=>(const Bad&) const = default;
};
// [/snippet]

int main() {
    Version a{1, 2}, b{1, 3};
    Reading r{"t", 1.0}, s{"t", 2.0};
    std::printf("%d %d %d\n", a < b, r < s, (r <=> s) == std::partial_ordering::less);
}
