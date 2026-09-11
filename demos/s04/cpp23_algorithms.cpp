// Demo: C++23 constrained algorithm additions
// Session: s04
// Compiler Explorer: <add short link>
#include <algorithm>
#include <array>
#include <numeric>
#include <print>
#include <ranges>
#include <string_view>
#include <version>
#include <vector>

struct SensorConfig { std::string_view name; };

int main() {
    std::vector<int> v{1, 2, 3, 2, 5};
    constexpr std::array<SensorConfig, 3> table{{{"rpm"}, {"temp"}, {"pressure"}}};
    // [snippet: algos]
    bool has = std::ranges::contains(v, 3);                                         // no more find() != end()
    bool dup = std::ranges::contains(table, "temp", &SensorConfig::name);            // with a projection: the exercise's validate()
#ifdef __cpp_lib_ranges_starts_ends_with                                          // GCC 15 / libc++ 17
    bool pre = std::ranges::starts_with(v, std::array{1, 2});
    bool suf = std::ranges::ends_with(v, std::array{2, 5});
#else
    bool pre = false, suf = false;
#endif
    std::vector<int> seq(5);
#if defined(__GLIBCXX__) || (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 190000)   // libc++ 18: no find_last, ranges::iota
    auto last2 = std::ranges::find_last(v, 2);                                       // a subrange from the last match to the end
    std::ranges::iota(seq, 10);                                                      // 10 11 12 13 14 (ranges version)
#else
    auto last2 = std::ranges::subrange(v.begin() + 3, v.end());
    std::iota(seq.begin(), seq.end(), 10);
#endif
    // std::ranges::shift_left / shift_right: GCC 15 / libc++ 20 (std::shift_left is C++20 and everywhere)
    // [/snippet]
    std::println("{} {} {} {} {} {}", has, dup, pre, suf, last2.begin() - v.begin(), seq[4]);
}
