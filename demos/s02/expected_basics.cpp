// Demo: std::expected (C++23)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/MEn6ojqdz
#include <charconv>
#include <cstdio>
#include <expected>
#include <string_view>

enum class ParseError { Empty, NotANumber, Trailing };

// [snippet: before]
// C++11: a flag and two out-parameters; the caller can ignore all three
bool parse_cpp11(std::string_view s, int* out, ParseError* err) {
    if (s.empty()) { *err = ParseError::Empty; return false; }
    auto r = std::from_chars(s.data(), s.data() + s.size(), *out);
    if (r.ec != std::errc{}) { *err = ParseError::NotANumber; return false; }
    if (r.ptr != s.data() + s.size()) { *err = ParseError::Trailing; return false; }
    return true;
}
// [/snippet]

// [snippet: after]
// C++23: the value, or the reason. One return; add [[nodiscard]] so the caller must look at it.
std::expected<int, ParseError> parse(std::string_view s) {
    if (s.empty()) return std::unexpected(ParseError::Empty);
    int v = 0;
    auto r = std::from_chars(s.data(), s.data() + s.size(), v);
    if (r.ec != std::errc{}) return std::unexpected(ParseError::NotANumber);
    if (r.ptr != s.data() + s.size()) return std::unexpected(ParseError::Trailing);
    return v;
}
// [/snippet]

int main() {
    // [snippet: use]
    if (auto n = parse("42")) std::printf("%d\n", *n);               // bool, *, -> like optional
    auto e = parse("4x");
    if (!e) std::printf("error %d\n", static_cast<int>(e.error()));   // the reason
    int v = parse("").value_or(-1);                                    // default
    ParseError why = parse("7").error_or(ParseError::Empty);           // C++23: the error, or a default
    // parse("").value();   // throws std::bad_expected_access<ParseError>, carrying the error
    // [/snippet]
    std::printf("%d %d\n", v, static_cast<int>(why));
}
