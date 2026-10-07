// Demo: cartesian_product, repeat, join_with, as_rvalue (C++23)
// Session: s04
// Compiler Explorer: https://godbolt.org/z/q6es6Yjad
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
#if defined(__cpp_lib_ranges_cartesian_product) && defined(__cpp_lib_ranges_join_with)   // libc++ (Apple Clang 17 included) lacks these
    // [snippet: family]
    for (auto [s, c] : v::cartesian_product(sensors, channels)) std::print("{}/{} ", s, c);   // every pair
    std::println("");

    for (int x : v::repeat(7, 3)) std::print("{} ", x);                                      // 7 7 7
    std::println("");

    for (char ch : sensors | v::join_with(std::string_view{", "})) std::print("{}", ch);    // rpm, temp
    std::println("");                                                                        // (pattern: a range or a single element, e.g. ',')

    std::vector<std::string> src{"a", "b"};
    auto moved = src | v::as_rvalue | std::ranges::to<std::vector>();   // moves the strings out of src
    std::println("{} {}", moved.size(), moved[0]);                      // src's strings are now moved-from: valid but unspecified
    // [/snippet]
#else
    std::println("C++23 view family not available in this standard library");
#endif
}
