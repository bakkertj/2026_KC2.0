// Demo: borrowed ranges and dangling (C++20)
// Session: s04
// Compiler Explorer: <add short link>
#include <algorithm>
#include <print>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

std::vector<int> make() { return {1, 2, 3}; }
std::vector<std::string_view> split(std::string_view s, char d) {
    return s | std::views::split(d) | std::views::transform([](auto&& p) { return std::string_view(p); }) | std::ranges::to<std::vector>();
}

int main() {
    // [snippet: borrowed]
    std::vector<int> v{1, 2, 3};
    auto a = std::ranges::find(v, 2);                 // v is an lvalue: iterator is safe
    auto b = std::ranges::find(std::span{v}, 2);      // span is a borrowed_range: safe even as a temporary
    auto c = std::ranges::find(make(), 2);            // temporary vector: std::ranges::dangling, cannot deref
    static_assert(std::same_as<decltype(c), std::ranges::dangling>);
    // [/snippet]

    // [snippet: views_dangle]
    auto fields = split(std::string("a,b"), ',');     // COMPILES. The views point into a temporary that
    // std::println("{}", fields[0]);                 // died at the ';'. Undefined behavior.
    std::string line = "a,b";
    auto ok = split(line, ',');                       // line outlives the views: fine
    auto owned = split(std::string("a,b"), ',')       // if it must outlive the source, materialize:
        | std::views::transform([](std::string_view s) { return std::string(s); })
        | std::ranges::to<std::vector>();
    // [/snippet]
    std::println("{} {} {} {}", *a, *b, ok[1], owned[0]);
    (void)fields;
}
