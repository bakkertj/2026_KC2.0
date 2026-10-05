// Demo: the same computation under each standard's constexpr rules
// Session: s03
// Compiler Explorer: https://godbolt.org/z/c7b5cW96v
#include <array>
#include <cstdint>
#include <print>
#include <string>
#include <vector>

// [snippet: cpp11]
// C++11: one return statement. Loops are spelled as recursion.
constexpr int factorial11(int n) { return n <= 1 ? 1 : n * factorial11(n - 1); }
// [/snippet]

// [snippet: cpp14]
// C++14: loops, locals, if, mutation. It looks like a normal function.
constexpr int factorial14(int n) {
    int r = 1;
    for (int i = 2; i <= n; ++i) r *= i;
    return r;
}
// [/snippet]

// [snippet: cpp17]
// C++17: std::array works in a constant expression; lambdas can be constexpr.
constexpr std::array<int, 6> factorials17() {
    std::array<int, 6> out{};
    auto fact = [](int n) constexpr { return factorial14(n); };
    for (int i = 0; i < 6; ++i) out[static_cast<std::size_t>(i)] = fact(i);
    return out;
}
inline constexpr auto kFactorials = factorials17();   // computed once, at compile time, in the header
// [/snippet]

// [snippet: cpp20]
// C++20: allocation is allowed, as long as it is freed before the expression ends.
constexpr int digits_in_factorial20(int n) {
    std::string s;                                    // a heap string, at compile time
    for (int v = factorial14(n); v > 0; v /= 10) s.push_back(static_cast<char>('0' + v % 10));
    return static_cast<int>(s.size());                // (std::to_string is constexpr only from C++26)
}
// [/snippet]

// [snippet: cpp23]
// C++23: a static local in a constexpr function (as long as it is not touched during constant evaluation).
constexpr int cached_or_computed(int n) {
    if consteval {
        return factorial14(n);                          // constant evaluation: just compute
    } else {
        static std::array<int, 13> cache{};             // runtime: memoize
        if (cache[static_cast<std::size_t>(n)] == 0) cache[static_cast<std::size_t>(n)] = factorial14(n);
        return cache[static_cast<std::size_t>(n)];
    }
}
// [/snippet]

static_assert(factorial11(5) == 120);
static_assert(factorial14(5) == 120);
static_assert(kFactorials[5] == 120);
static_assert(digits_in_factorial20(12) == 9);
static_assert(cached_or_computed(6) == 720);

int main() { std::println("{} {} {} {} {}", factorial11(5), factorial14(5), kFactorials[4], digits_in_factorial20(10), cached_or_computed(7)); }
