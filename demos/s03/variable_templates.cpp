// Demo: variable templates (C++14) and _v/_t aliases (C++17)
// Session: s03
// Compiler Explorer: <add short link>
#include <print>
#include <type_traits>

// [snippet: vt]
template <typename T>
constexpr T pi = T(3.1415926535897932385L);           // one constant, every precision

template <typename T>
constexpr bool is_small_v = sizeof(T) <= sizeof(void*);   // your own _v trait

static_assert(pi<float> > pi<double>);            // float rounds pi UP: a compile-time float lesson for free
static_assert(is_small_v<int> && !is_small_v<long double>);

// C++11:  std::is_integral<T>::value      std::remove_const<T>::type
// C++17:  std::is_integral_v<T>           std::remove_const_t<T>
static_assert(std::is_integral_v<int> && std::is_same_v<std::remove_const_t<const int>, int>);
// [/snippet]

int main() { std::println("{:.7f} {:.15f}", pi<float>, pi<double>); }
