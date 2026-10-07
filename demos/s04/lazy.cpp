// Demo: views are lazy (C++20)
// Session: s04
// Compiler Explorer: https://godbolt.org/z/d4z46YYf9
#include <print>
#include <ranges>
#include <vector>

int main() {
    // [snippet: lazy]
    int calls = 0;
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto pipeline = v
        | std::views::filter([&](int x) { ++calls; return x % 2 == 0; })
        | std::views::transform([](int x) { return x * x; })
        | std::views::take(2);
    std::println("after construction: {} predicate calls", calls);   // 0: nothing has run

    for (int x : pipeline) std::print("{} ", x);                      // 4 16
    std::println("\nafter iteration: {} predicate calls", calls);     // 6, not 10: it stopped early.
    // Why 6 and not 4: after yielding 4, ++ advances filter to the NEXT match (tests 5, 6)
    // before take's counter says stop. Lazy means "no more than needed", not "exactly".
    // [/snippet]
}
