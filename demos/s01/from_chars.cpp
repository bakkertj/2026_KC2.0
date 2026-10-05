// Demo: std::from_chars replacing strtod (C++17)
// Session: s01
// Compiler Explorer: <add short link>
#include <cerrno>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <system_error>
#include <version>

// [snippet: before]
bool parse_value_cpp11(const std::string& text,
                       double* out) {
    if (text.empty()) return false;
    errno = 0;                    // global state
    char* end = nullptr;          // NUL-terminated; locale
    const double v = std::strtod(text.c_str(), &end);
    if (errno == ERANGE || *end != '\0') return false;
    *out = v;
    return true;
}
// [/snippet]

#ifdef __cpp_lib_to_chars   // libc++ < 20 lacks floating-point from_chars
// [snippet: after]
bool parse_value(const std::string& text,
                 double* out) {
    const char* last = text.data() + text.size();
    double v = 0.0;               // parse into a local: *out untouched on failure
    auto [ptr, ec] = std::from_chars(text.data(), last, v);
    if (ec != std::errc{} || ptr != last) return false;
    *out = v;
    return true;
}
// [/snippet]
#else
bool parse_value(const std::string& text, double* out) { return parse_value_cpp11(text, out); }
#endif

int main() {
    double a = 0, b = 0;
    const bool ok_a = parse_value_cpp11("41.25", &a);   // separate statements: printf's
    const bool ok_b = parse_value("41.25x", &b);        // argument order is unspecified
    std::printf("%d %d %.2f %.2f\n", ok_a, ok_b, a, b);
}
