// Demo: the const view trap (C++20)
// Session: s04
// Compiler Explorer: https://godbolt.org/z/a6vTKTY3K
#include <print>
#include <ranges>
#include <vector>

// [snippet: trap]
// filter_view::begin() finds the first match and CACHES it (so begin() is amortized O(1)),
// so begin() is not const, so a const filter_view is not a range.
template <typename R>
std::size_t count_const(const R& r) {          // const&: fails for filter_view
    std::size_t n = 0;
    for ([[maybe_unused]] auto&& x : r) ++n;
    return n;
}

template <typename R>
std::size_t count(R&& r) {                     // R&&: how the standard algorithms take ranges
    std::size_t n = 0;
    for ([[maybe_unused]] auto&& x : r) ++n;
    return n;
}
// const-iterable:     transform, take, drop (of random-access), iota, all
// NOT const-iterable: filter, drop_while, split, chunk_by, join (sometimes)
// [/snippet]

int main() {
    std::vector<int> v{1, 2, 3, 4};
    auto evens = v | std::views::filter([](int x) { return x % 2 == 0; });
    std::println("{}", count(evens));                     // 2
    std::println("{}", count_const(v | std::views::transform([](int x) { return x; })));   // transform_view: fine
#ifdef SHOW_ERRORS
    std::println("{}", count_const(evens));               // error: 'begin' ... but function is not marked const
#endif
}
