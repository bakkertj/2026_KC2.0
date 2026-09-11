// Demo: the cost model of views (C++20)
// Session: s04
// Compiler Explorer: <add short link>
#include <print>
#include <ranges>
#include <vector>

// [snippet: cost]
// Inlines to one loop with a branch: as fast as the hand-written loop.
int sum_even_squares(const std::vector<int>& v) {
    int s = 0;
    for (int x : v | std::views::filter([](int x) { return x % 2 == 0; })
                   | std::views::transform([](int x) { return x * x; })) s += x;
    return s;
}

// Evaluates the filter predicate TWICE per element in some cases: reverse of filter needs
// the end, and finding the end runs the predicate; iterating runs it again.
int reverse_of_filter(const std::vector<int>& v) {
    int s = 0;
    for (int x : v | std::views::filter([](int x) { return x % 2 == 0; }) | std::views::reverse) s += x;
    return s;
}

// join and split are forward-only: no size(), no random access, and a per-element branch.
// Fine for parsing a line; measure before putting them in a hot loop.
// [/snippet]

int main() {
    std::vector<int> v{1, 2, 3, 4, 5, 6};
    std::println("{} {}", sum_even_squares(v), reverse_of_filter(v));
}
