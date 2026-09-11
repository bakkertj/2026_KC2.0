// Demo: cartesian_product, repeat, join_with, as_rvalue, as_const (C++23)
// Session: s04
// Compiler Explorer: <add short link>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
#include <version>

int main() {
    namespace v = std::views;
    std::vector<std::string> sensors{"rpm", "temp"};
    std::vector<int> channels{1, 2};
#if defined(__GLIBCXX__) || (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 200000)   // libc++ 18 lacks cartesian_product, join_with
    // [snippet: family]
    for (auto [s, c] : v::cartesian_product(sensors, channels)) std::print("{}/{} ", s, c);   // every pair
    std::println("");

    for (int x : v::repeat(7, 3)) std::print("{} ", x);                                      // 7 7 7
    std::println("");

    for (char ch : sensors | v::join_with(std::string_view{", "})) std::print("{}", ch);    // rpm, temp
    std::println("");                                                                        // (pattern must be a range)

    std::vector<std::string> src{"a", "b"};
    auto moved = src | v::as_rvalue | std::ranges::to<std::vector>();   // moves the strings out of src
    std::println("{} {}", moved.size(), src[0].empty());
    // [/snippet]
#else
    std::println("C++23 view family not available in this standard library");
#endif
}
