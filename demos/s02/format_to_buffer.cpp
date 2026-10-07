// Demo: format_to, format_to_n, formatted_size (C++20)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/Te6Y5PqWP
#include <array>
#include <cstdio>
#include <format>
#include <iterator>
#include <string>

int main() {
    // [snippet: buffer]
    std::array<char, 32> buf;                                 // no heap: embedded-friendly
    auto r = std::format_to_n(buf.data(), buf.size(), "{}:{:.2f}", "rpm", 4811.0);
    std::size_t written = static_cast<std::size_t>(r.out - buf.data());   // chars actually in buf
    std::size_t would_need = static_cast<std::size_t>(r.size);            // what it WOULD have needed
    std::size_t needed = std::formatted_size("{}:{:.2f}", "rpm", 4811.0);

    std::string s;
    std::format_to(std::back_inserter(s), "{} ", 1);          // append to any output iterator
    std::format_to(std::back_inserter(s), "{}", 2);

    std::string user_fmt = "{:>6}";                            // not a literal: needs vformat
    int answer = 42;                                           // make_format_args takes lvalues (C++23 DR)
    std::string t = std::vformat(user_fmt, std::make_format_args(answer));
    // [/snippet]
    std::printf("%.*s %zu %zu %zu %s %s\n", static_cast<int>(written), buf.data(), written, would_need, needed, s.c_str(), t.c_str());
}
