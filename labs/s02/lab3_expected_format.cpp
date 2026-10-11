// Lab 2.3: or this error (expected), and formatting
// Session: s02
// Compiler Explorer: https://godbolt.org/z/v79xqbPaz
// Ten minutes, three TODOs in order. Build and run after each one.
// Finished version: lab3_expected_format_solution.cpp
#include <charconv>
#include <cstdio>
#include <expected>
#include <format>
#include <print>
#include <string>
#include <string_view>

enum class ParseError { Empty, BadNumber, OutOfRange };
struct Reading { std::string sensor; int value; };   // int: libc++ 18 has no from_chars for double

// TODO 1: a bool plus an out-parameter plus an error out-parameter. Return
//         std::expected<Reading, ParseError>: the value or the reason, in one object.
//         (Each `return false` becomes `return std::unexpected(ParseError::...)`.)
bool parse_reading(std::string_view line, Reading* out, ParseError* err) {
    if (line.empty()) { *err = ParseError::Empty; return false; }
    auto comma = line.find(',');
    std::string_view num = line.substr(comma + 1);
    int v = 0;
    auto [p, ec] = std::from_chars(num.data(), num.data() + num.size(), v);
    if (ec != std::errc{} || p != num.data() + num.size()) { *err = ParseError::BadNumber; return false; }
    *out = Reading{std::string(line.substr(0, comma)), v};
    return true;
}

// TODO 2: check_range already returns expected. Write validated() as ONE expression:
//         parse_reading(line).and_then(check_range)
//         and delete the hand-written unwrapping.
std::expected<Reading, ParseError> check_range(Reading r) {
    if (r.value < -50 || r.value > 150) return std::unexpected(ParseError::OutOfRange);
    return r;
}
std::expected<Reading, ParseError> validated(std::string_view line) {
    Reading r; ParseError e;
    if (!parse_reading(line, &r, &e)) return std::unexpected(e);
    return check_range(r);
}

// TODO 3: give Reading a std::formatter so the println below can say "{}" instead of
//         spelling the fields. Format as `<sensor>=<value>`, e.g. rpm=42.
//         Shape (parse is trivial: return ctx.begin()):
//           template <> struct std::formatter<Reading> {
//               constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
//               template <class Ctx> auto format(const Reading& r, Ctx& ctx) const {
//                   return std::format_to(ctx.out(), "...", ...);
//               }
//           };

std::string_view name(ParseError e) {
    switch (e) {
    case ParseError::Empty:      return "empty line";
    case ParseError::BadNumber:  return "bad number";
    case ParseError::OutOfRange: return "out of range";
    }
    return "?";
}

int main() {
    for (std::string_view line : {"rpm,42", "temp,x", "", "temp,900"}) {
        if (auto r = validated(line)) {
            std::println("ok   {}={}", r->sensor, r->value);        // TODO 3: std::println("ok   {}", *r);
        } else {
            std::println("fail {:<10} ({})", line, name(r.error()));
        }
    }
}
