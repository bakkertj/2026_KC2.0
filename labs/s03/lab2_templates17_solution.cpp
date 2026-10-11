// Lab 3.2 solution: C++17 templates (fold expressions, if constexpr, CTAD)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/4M5cnvbzc
#include <cstdio>
#include <string>
#include <type_traits>
#include <utility>

// TODO 1 done: a fold expression is the recursion, written once, with no base case.
template <class... Ts>
auto sum(Ts... xs) { return (xs + ...); }

template <class... Ts>
int count_true(Ts... bs) { return (0 + ... + (bs ? 1 : 0)); }   // binary fold: an empty pack gives 0

// TODO 2 done: one template; the compiler discards the branches that do not apply.
template <class T>
std::string describe(T v) {
    if constexpr (std::is_integral_v<T>)            return "int " + std::to_string(v);
    else if constexpr (std::is_floating_point_v<T>) return "real " + std::to_string(v);
    else                                            return "text " + std::string(v);
}

// TODO 3 done: a deduction guide resolves the int/double disagreement.
template <class T>
struct Range {
    T lo, hi;
    Range(T l, T h) : lo(l), hi(h) {}
    bool contains(T v) const { return lo <= v && v <= hi; }
};
template <class A, class B> Range(A, B) -> Range<std::common_type_t<A, B>>;

int main() {
    std::printf("sum: %d %.1f\n", sum(1, 2, 3, 4), sum(1.5, 2.5));
    std::printf("true count: %d\n", count_true(true, false, true, true));

    std::printf("%s | %s | %s\n", describe(42).c_str(), describe(2.5).c_str(), describe("hi").c_str());

    std::pair p{3, 0.25};                 // pair<int, double>
    Range r{0, 1.0};                      // Range<double>, via the guide
    std::printf("pair %d %.2f, contains(0.5): %d\n", p.first, p.second, r.contains(0.5));
}
