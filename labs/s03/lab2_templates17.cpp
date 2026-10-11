// Lab 3.2: C++17 templates (fold expressions, if constexpr, CTAD)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/8rvn4TTcs
// Ten minutes, three TODOs in order. Build and run after each one; the output must not
// change. Finished version: lab2_templates17_solution.cpp
#include <cstdio>
#include <string>
#include <type_traits>
#include <utility>

// ---- TODO 1: recursion -> fold expression ----------------------------------------
// C++11 variadic sum: a base case plus a recursive case. Replace both with ONE function:
//     template <class... Ts> auto sum(Ts... xs) { return (xs + ...); }
// Then do the same for count_true, which counts how many arguments are true
// (hint: fold over `+` after converting each to int, or fold with a lambda).
template <class T>
T sum(T x) { return x; }
template <class T, class... Ts>
auto sum(T x, Ts... xs) { return x + sum(xs...); }

int count_true() { return 0; }
template <class... Ts>
int count_true(bool b, Ts... rest) { return (b ? 1 : 0) + count_true(rest...); }

// ---- TODO 2: tag dispatch -> if constexpr ------------------------------------------
// Three overloads selected by a tag type computed from traits. Fold the whole thing into
// ONE function template that uses if constexpr on std::is_integral_v<T>,
// std::is_floating_point_v<T>, and else. (A branch that is not taken is not instantiated,
// so std::to_string on a const char* never has to compile.)
struct integral_tag {}; struct floating_tag {}; struct other_tag {};
template <class T> std::string describe_impl(T v, integral_tag) { return "int " + std::to_string(v); }
template <class T> std::string describe_impl(T v, floating_tag) { return "real " + std::to_string(v); }
template <class T> std::string describe_impl(T v, other_tag)    { return "text " + std::string(v); }
template <class T>
std::string describe(T v) {
    using tag = std::conditional_t<std::is_integral_v<T>, integral_tag,
                std::conditional_t<std::is_floating_point_v<T>, floating_tag, other_tag>>;
    return describe_impl(v, tag{});
}

// ---- TODO 3: spell the template arguments -> CTAD --------------------------------
// In main, delete the <int, double> on the pair and the <double> on the Range. The pair
// just works. The Range does not: Range{0, 1.0} deduces T from both arguments and they
// disagree. Add a deduction guide after the struct that picks the common type:
//     template <class A, class B> Range(A, B) -> Range<std::common_type_t<A, B>>;
template <class T>
struct Range {
    T lo, hi;
    Range(T l, T h) : lo(l), hi(h) {}
    bool contains(T v) const { return lo <= v && v <= hi; }
};

int main() {
    std::printf("sum: %d %.1f\n", sum(1, 2, 3, 4), sum(1.5, 2.5));
    std::printf("true count: %d\n", count_true(true, false, true, true));

    std::printf("%s | %s | %s\n", describe(42).c_str(), describe(2.5).c_str(), describe("hi").c_str());

    std::pair<int, double> p{3, 0.25};
    Range<double> r{0, 1.0};
    std::printf("pair %d %.2f, contains(0.5): %d\n", p.first, p.second, r.contains(0.5));
}
