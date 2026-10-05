// Demo: std::is_constant_evaluated (C++20) and if consteval (C++23)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/Taevq7Wz1
#include <cmath>
#include <print>
#include <type_traits>

// [snippet: ice]
// One function, two implementations: exact at compile time, fast at runtime
constexpr double power(double base, int exp) {
    if (std::is_constant_evaluated()) {              // C++20 (C++23 spells it `if consteval`)
        double r = 1.0;
        for (int i = 0; i < exp; ++i) r *= base;     // constexpr-friendly loop
        return r;
    } else {
        return std::pow(base, exp);                  // libm; not constexpr everywhere
    }
}
static_assert(power(2.0, 10) == 1024.0);
// [/snippet]

// [snippet: trap]
constexpr int trap() {
#ifdef SHOW_ERRORS   // GCC 14 warns: always true in 'if constexpr'
    if constexpr (std::is_constant_evaluated()) {    // WRONG: the condition is itself
        return 1;                                    // constant-evaluated: always true
    }
#endif
    if (std::is_constant_evaluated()) return 1;      // C++20: a plain if. C++23: `if consteval`.
    return 2;
}
// [/snippet]

int main() { std::println("{} {}", power(2.0, 10), trap()); }
