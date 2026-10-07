// Demo: std::ranges::to (C++23)
// Session: s04
// Compiler Explorer: https://godbolt.org/z/h9jKjzK9b
#include <map>
#include <print>
#include <ranges>
#include <set>
#include <string>
#include <utility>
#include <vector>

int main() {
    std::vector<int> v{3, 1, 4, 1, 5};
    auto evens = v | std::views::filter([](int x) { return x % 2 == 0; });
    // [snippet: before]
    // C++20: a view is not a container. Materializing one was awkward:
    std::vector<int> a(evens.begin(), evens.end());          // only if begin/end have the same type ("common")
    auto common = evens | std::views::common;                // ...otherwise adapt first
    std::vector<int> b(common.begin(), common.end());
    // [/snippet]

    // [snippet: after]
    // C++23: one adaptor, any container, nested if needed
    auto c = v | std::views::filter([](int x) { return x > 1; }) | std::ranges::to<std::vector>();
    auto s = v | std::ranges::to<std::set>();                                            // dedupe + sort
    auto m = v | std::views::transform([](int x) { return std::pair{x, x * x}; })
               | std::ranges::to<std::map>();                                          // pairs -> map
    auto words = std::vector<std::vector<char>>{{'a', 'b'}, {'c'}}
               | std::ranges::to<std::vector<std::string>>();                          // element-wise conversion
    // [/snippet]
    std::println("{} {} {} {} {} {}", a.size(), b.size(), c.size(), s.size(), m[4], words[0]);
}
