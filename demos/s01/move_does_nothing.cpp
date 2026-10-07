// Demo: when std::move does nothing (C++11 calibration)
// Session: s01
// Compiler Explorer: https://godbolt.org/z/64dKeG8jj
#include <cstdio>
#include <string>
#include <utility>

// [snippet: cases]
std::string make() {
    std::string s(100, 'x');
#ifdef SHOW_ERRORS
    return std::move(s);   // WRONG: "prevents copy elision", say both compilers
#else
    return s;              // RIGHT: locals are moved anyway, usually elided
#endif
}

void consume(std::string s) { std::printf("%zu\n", s.size()); }

void cases() {
    const std::string c(100, 'c');
    consume(std::move(c));   // COPIES: cannot move from const. No warning.

    std::string s(100, 's');
    consume(std::move(s));   // moves; s is now valid-but-unspecified
    std::printf("%zu\n", s.size());   // legal; never rely on the value
}
// [/snippet]

int main() {
    consume(make());
    cases();
}
