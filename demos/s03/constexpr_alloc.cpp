// Demo: compile-time std::vector and std::string (C++20)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/nn5xMeab1
#include <algorithm>
#include <array>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// [snippet: alloc]
// Build a sorted list of names at compile time. The vector lives and dies inside
// the constant expression ("transient allocation"); only the result escapes.
constexpr std::array<std::string_view, 4> sorted_names() {
    std::vector<std::string_view> v{"rpm", "pressure", "temp_core", "current_bus"};
    std::sort(v.begin(), v.end());                      // constexpr algorithms, C++20
    std::array<std::string_view, 4> out{};
    std::copy(v.begin(), v.end(), out.begin());
    return out;                                          // v is freed here, before the expression ends
}
inline constexpr auto kSortedNames = sorted_names();
static_assert(kSortedNames[0] == "current_bus");

// constexpr std::vector<int> kNotAllowed = {1, 2, 3};   // error: the allocation would outlive the expression
// [/snippet]

// [snippet: string]
constexpr std::size_t joined_length(std::span<const std::string_view> parts) {
    std::string s;
    for (auto p : parts) { s += p; s += ','; }
    return s.size();                                     // the string is transient; its size is not
}
static_assert(joined_length(kSortedNames) == 35);
// [/snippet]

int main() { std::println("{} {}", kSortedNames[3], joined_length(kSortedNames)); }
