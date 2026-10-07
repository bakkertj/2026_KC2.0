// Demo: sentinels (C++20)
// Session: s04
// Compiler Explorer: https://godbolt.org/z/bYoEqPf98
#include <algorithm>
#include <cstring>
#include <iterator>
#include <print>
#include <ranges>

// [snippet: sentinel]
// An end that is a predicate, not an iterator: "stop when *it == 0".
struct NulSentinel {
    friend bool operator==(const char* p, NulSentinel) { return *p == '\0'; }
};

int main() {
    const char* s = "hello, world";
    auto comma = std::ranges::find(s, NulSentinel{}, ',');           // no strlen first
    std::println("{}", comma - s);

    // Infinite ranges are fine when the end is a sentinel that never matches:
    auto squares = std::views::iota(1) | std::views::transform([](int x) { return x * x; });
    for (int sq : squares | std::views::take_while([](int x) { return x < 50; })) std::print("{} ", sq);
    std::println("");

    // std::unreachable_sentinel: "there is an end, trust me" (the search must succeed)
    auto it = std::ranges::find(s, std::unreachable_sentinel, 'w');   // no bounds check in the loop
    std::println("{}", it - s);
}
// [/snippet]
