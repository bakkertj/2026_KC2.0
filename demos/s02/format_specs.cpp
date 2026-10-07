// Demo: std::format and the spec mini-language (C++20)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/T61MYrzd7
#include <cstdio>
#include <format>
#include <string>
#include <string_view>

// [snippet: before]
// C++11: snprintf. A buffer, a size, and a format string checked by nobody.
std::string fmt_cpp11(double v) {
    char buf[64];
    std::snprintf(buf, sizeof buf, "%.3f", v);
    return buf;
}
// [/snippet]

// [snippet: after]
// C++20: checked at compile time, no buffer, returns a std::string
std::string fmt(double v) { return std::format("{:.3f}", v); }
// [/snippet]

int main() {
    // [snippet: specs]
    auto show = [](std::string_view r) { std::printf("[%.*s]\n", static_cast<int>(r.size()), r.data()); };
    show(std::format("{:.3f}", 3.14159));        // [3.142]         precision
    show(std::format("{:<16}|", "left"));        // [left            |]   width, left-align
    show(std::format("{:>8}", 42));              // [      42]      right-align
    show(std::format("{:^9}", "mid"));           // [   mid   ]     center
    show(std::format("{:04X}", 0xBEEF));         // [BEEF]          zero-pad, hex upper
    show(std::format("{:#010x}", 255));          // [0x000000ff]    alternate form
    show(std::format("{:+d}", 5));               // [+5]            always sign
    show(std::format("{:e}", 12345.678));        // [1.234568e+04]
    show(std::format("{1} {0}", "a", "b"));      // [b a]           positional
    show(std::format("{:*^11}", "x"));           // [*****x*****]   fill character
    show(std::format("{{}}"));                   // [{}]            escaping
    // [/snippet]
    std::printf("%s %s\n", fmt_cpp11(3.14159).c_str(), fmt(3.14159).c_str());
}
