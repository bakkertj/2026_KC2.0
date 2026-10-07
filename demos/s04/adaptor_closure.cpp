// Demo: storing and composing adaptors; your own adaptor (C++20 / C++23)
// Session: s04
// Compiler Explorer: https://godbolt.org/z/h5ecMfcEa
#include <print>
#include <ranges>
#include <vector>
#include <version>

// [snippet: closure]
// A range adaptor closure is a value: store it, name it, reuse it, compose it.
inline constexpr auto evens = std::views::filter([](int x) { return x % 2 == 0; });
inline constexpr auto squared = std::views::transform([](int x) { return x * x; });
inline constexpr auto top2_even_squares = evens | squared | std::views::take(2);   // composed, no range yet
// [/snippet]

// range_adaptor_closure (C++23, P2387): libstdc++ 13+, libc++ 19+. No feature-test macro of its own.
#if defined(__GLIBCXX__) || (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 190000)
#define HAVE_RANGE_ADAPTOR_CLOSURE 1
// [snippet: own]
// C++23: make your own function pipeable by deriving from range_adaptor_closure
struct sum_fn : std::ranges::range_adaptor_closure<sum_fn> {
    template <std::ranges::input_range R>
    int operator()(R&& r) const { int s = 0; for (auto&& x : r) s += x; return s; }
};
inline constexpr sum_fn sum;                    // v | evens | sum
// [/snippet]
#endif

int main() {
    std::vector<int> v{1, 2, 3, 4, 5, 6};
    for (int x : v | top2_even_squares) std::print("{} ", x);      // 4 16
#ifdef HAVE_RANGE_ADAPTOR_CLOSURE
    std::println("| {}", v | evens | sum);                          // 12
#else
    std::println("| (range_adaptor_closure not available)");
#endif
}
