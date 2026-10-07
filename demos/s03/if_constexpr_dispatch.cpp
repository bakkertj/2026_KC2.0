// Demo: if constexpr replacing tag dispatch and enable_if (C++17)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/KMKPoxPYa
#include <print>
#include <string>
#include <type_traits>

// [snippet: before]
// C++11: two overloads selected by enable_if; the reader has to reassemble the logic
template <typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
std::string describe11(T v) { return "int " + std::to_string(v); }

template <typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
std::string describe11(T v) { return "float " + std::to_string(v); }
// [/snippet]

// [snippet: after]
// C++17: one function; the untaken branch is discarded, not compiled
template <typename T>
std::string describe(T v) {
    if constexpr (std::is_integral_v<T>) {
        return "int " + std::to_string(v);
    } else if constexpr (std::is_floating_point_v<T>) {
        return "float " + std::to_string(v);
    } else {
        return "other";                                 // v.foo() here would not be instantiated for int
    }
}
// [/snippet]

int main() { std::println("{} | {} | {}", describe11(1), describe(2.5), describe("x")); }
