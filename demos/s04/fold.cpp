// Demo: ranges::fold_left and friends (C++23)
// Session: s04
// Compiler Explorer: <add short link>
#include <algorithm>
#include <functional>
#include <numeric>
#include <optional>
#include <print>
#include <ranges>
#include <string>
#include <vector>

struct Record { std::string sensor; double value; };

int main() {
    std::vector<Record> v{{"rpm", 1.5}, {"rpm", 2.5}, {"temp", 4.0}};
#if defined(__GLIBCXX__) || (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 200000)   // libc++ 18 has fold_left only
    // [snippet: fold]
    // C++20 ranges had no accumulate. C++23: fold_left, with a range and any binary op.
    double total = std::ranges::fold_left(v | std::views::transform(&Record::value), 0.0, std::plus{});

    // fold_left_first: no initial value, so the result is optional (empty range -> nullopt)
    auto maxv = std::ranges::fold_left_first(v | std::views::transform(&Record::value),
                                             [](double a, double b) { return a > b ? a : b; });

    // fold_right: associates from the right (matters for non-commutative ops)
    auto names = std::ranges::fold_right(v | std::views::transform(&Record::sensor), std::string{},
                                         [](const std::string& s, std::string acc) { return acc + s + ";"; });

    // vs std::accumulate(v.begin(), v.end(), 0): iterator pair, and the init's type decides the
    // arithmetic, so 0 (an int) truncates doubles at every step. A classic bug.
    // [/snippet]
#else
    double total = std::ranges::fold_left(v | std::views::transform(&Record::value), 0.0, std::plus{});
    std::optional<double> maxv = 4.0;
    std::string names = "(fold_right not available)";
#endif
    std::println("{} {} {}", total, *maxv, names);
}
