// Demo: the error messages, side by side (C++11 vs C++20)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/T9oj4s9Kv
#include <concepts>
#include <print>
#include <type_traits>

// [snippet: both]
template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
T twice_sfinae(T v) { return v * 2; }

auto twice_concept(std::integral auto v) { return v * 2; }

#ifdef SHOW_ERRORS
auto a = twice_sfinae(2.5);
// GCC: "no matching function for call to 'twice_sfinae(double)'"
//      "candidate: template<class T, typename std::enable_if<is_integral_v<T>, int>::type <anonymous> >"
//      "template argument deduction/substitution failed: ... no type named 'type' in 'struct std::enable_if<false, int>'"
auto b = twice_concept(2.5);
// GCC: "no matching function for call to 'twice_concept(double)'"
//      "constraints not satisfied ... the expression 'is_integral_v<T>' evaluated to 'false'"
//      "note: 'double' does not satisfy 'integral'"
#endif
// [/snippet]

int main() { std::println("{} {}", twice_sfinae(2), twice_concept(3)); }
