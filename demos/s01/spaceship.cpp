// Demo: three-way comparison and defaulted comparisons (C++20)
// Session: s01
// Compiler Explorer: <add short link>
// Slide: slides/01-everyday-language.md
#include <compare>
#include <cstdio>
#include <cstdint>

// [snippet: before]
// C++11: six operators, all hand-written
struct Version11 {
    int major, minor, patch;
};
using V = Version11;
bool operator==(const V& a, const V& b) {
    return a.major == b.major && a.minor == b.minor
        && a.patch == b.patch;
}
bool operator<(const V& a, const V& b) {
    if (a.major != b.major) return a.major < b.major;
    if (a.minor != b.minor) return a.minor < b.minor;
    return a.patch < b.patch;
}
bool operator!=(const V& a, const V& b) {return !(a == b);}
bool operator>(const V& a, const V& b)  {return b < a;}
bool operator<=(const V& a, const V& b) {return !(b < a);}
bool operator>=(const V& a, const V& b) {return !(a < b);}
// [/snippet]

// [snippet: after]
// C++20: one line; == and <=> generated memberwise
struct Version20 {
    int major, minor, patch;
    auto operator<=>(const Version20&) const = default;
};
// [/snippet]

int main() {
    Version20 a{1, 2, 3}, b{1, 3, 0};
    std::printf("a<b %d  a==b %d  a>=b %d\n", a < b, a == b, a >= b);
    Version11 c{1, 2, 3}, d{1, 3, 0};
    std::printf("c<d %d  c==d %d  c>=d %d\n", c < d, c == d, c >= d);
}
