// Lab 2.3 solution: or this error (expected), and formatting
// Session: s02
// Compiler Explorer: https://godbolt.org/z/nx8fseMc7
#include <charconv>
#include <cstdio>
#include <expected>
#include <format>
#include <print>
#include <string>
#include <string_view>

enum class ParseError { Empty, BadNumber, OutOfRange };
struct Reading { std::string sensor; int value; };   // int: libc++ 18 has no from_chars for double

// TODO 1 done: the return type carries the value or the reason; no out-parameters.
std::expected<Reading, ParseError> parse_reading(std::string_view line) {
    if (line.empty()) return std::unexpected(ParseError::Empty);
    auto comma = line.find(',');
    std::string_view num = line.substr(comma + 1);
    int v = 0;
    auto [p, ec] = std::from_chars(num.data(), num.data() + num.size(), v);
    if (ec != std::errc{} || p != num.data() + num.size()) return std::unexpected(ParseError::BadNumber);
    return Reading{std::string(line.substr(0, comma)), v};
}

std::expected<Reading, ParseError> check_range(Reading r) {
    if (r.value < -50 || r.value > 150) return std::unexpected(ParseError::OutOfRange);
    return r;
}
// TODO 2 done: and_then runs the next stage only on success; a failure passes straight through.
std::expected<Reading, ParseError> validated(std::string_view line) {
    return parse_reading(line).and_then(check_range);
}

// TODO 3 done: Reading knows how to print itself; every call site just says "{}".
template <> struct std::formatter<Reading> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    template <class Ctx>                      // a template on the context: required by libc++, allowed everywhere
    auto format(const Reading& r, Ctx& ctx) const {
        return std::format_to(ctx.out(), "{}={}", r.sensor, r.value);
    }
};

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
            std::println("ok   {}", *r);
        } else {
            std::println("fail {:<10} ({})", line, name(r.error()));
        }
    }
}
