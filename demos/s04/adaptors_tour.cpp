// Demo: the core view adaptors (C++20)
// Session: s04
// Compiler Explorer: https://godbolt.org/z/rr9b931Eq
#include <map>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

template <std::ranges::input_range R>
void show(std::string_view label, R&& r) {
    std::print("{:<14}", label);
    for (auto&& x : r) std::print("{} ", x);
    std::println("");
}

int main() {
    namespace v = std::views;
    std::vector<int> n{1, 2, 3, 4, 5, 6};
    std::map<std::string, int> m{{"a", 1}, {"b", 2}};
    // [snippet: tour]
    show("filter",     n | v::filter([](int x) { return x % 2 == 0; }));     // 2 4 6
    show("transform",  n | v::transform([](int x) { return x * 10; }));      // 10 20 ... 60
    show("take",       n | v::take(3));                                      // 1 2 3
    show("drop",       n | v::drop(4));                                      // 5 6
    show("take_while", n | v::take_while([](int x) { return x < 4; }));      // 1 2 3
    show("drop_while", n | v::drop_while([](int x) { return x < 4; }));      // 4 5 6
    show("reverse",    n | v::reverse);                                      // 6 5 ... 1
    show("iota",       v::iota(10, 15));                                     // 10 11 12 13 14
    show("keys",       m | v::keys);                                         // a b
    show("values",     m | v::values);                                       // 1 2
    show("split",      std::string_view{"a,b,c"} | v::split(',') | v::transform([](auto&& p) { return std::string_view(p); }));
    show("join",       std::vector<std::string>{"ab", "cd"} | v::join);      // a b c d (chars)
    show("chain",      n | v::reverse | v::filter([](int x) { return x > 2; }) | v::take(2));   // 6 5
    // [/snippet]
}
