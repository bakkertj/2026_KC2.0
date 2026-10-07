// Demo: std::string_view (C++17)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/sMneTbzKT
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

// [snippet: before]
// C++11: three overloads, or one that forces a std::string
std::size_t count_commas_cpp11(const std::string& s) {
    std::size_t n = 0;
    for (char c : s) n += (c == ',');
    return n;
}
// count_commas_cpp11("a,b");         // constructs a std::string
// count_commas_cpp11(buf, len);      // no such overload
// [/snippet]

// [snippet: after]
// C++17: one function; literal, std::string, or slice, no copy
std::size_t count_commas(std::string_view s) {
    std::size_t n = 0;
    for (char c : s) n += (c == ',');
    return n;
}
// [/snippet]

// [snippet: ops]
void ops(std::string_view line) {
    auto head = line.substr(0, 5);          // O(1): a smaller view
    line.remove_prefix(2);                  // O(1): moves the start
    bool b = line.starts_with("25");        // C++20
    bool c = line.contains("temp");         // C++23
    auto pos = line.find(',');              // like std::string
    std::printf("%.*s %d %d %zu\n", static_cast<int>(head.size()), head.data(), b, c, pos);
}
// [/snippet]

int main() {
    std::string s = "1725,temp_core,41.25";
    std::printf("%zu %zu %zu\n", count_commas_cpp11(s), count_commas(s), count_commas("a,b,c"));
    ops(s);
}
