// Demo: auto non-type template parameters (C++17) and class-type NTTPs (C++20)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/xPWv9Y5c5
#include <algorithm>
#include <array>
#include <print>
#include <string_view>

// [snippet: auto_nttp]
template <auto N>                              // C++17: the type of N is deduced from the argument
struct Constant { static constexpr auto value = N; };
static_assert(Constant<42>::value == 42);      // int
static_assert(Constant<'x'>::value == 'x');    // char
// [/snippet]

// [snippet: fixed_string]
// C++20: a class type as a template argument, if it is "structural" (all public members, no
// private state, and every member is itself structural). A string_view is NOT (private pointer).
template <std::size_t N>
struct FixedString {
    char data[N]{};
    constexpr FixedString(const char (&s)[N]) { std::copy_n(s, N, data); }
    constexpr std::string_view view() const { return {data, N - 1}; }
};

template <FixedString Name>                    // a compile-time string as a template argument
struct Sensor {
    static constexpr std::string_view name = Name.view();
};
static_assert(Sensor<"rpm">::name == "rpm");
// Why the exercise's SensorConfig cannot be an NTTP: its string_view members are not structural.
// [/snippet]

int main() { std::println("{} {}", Constant<7>::value, Sensor<"temp_core">::name); }
