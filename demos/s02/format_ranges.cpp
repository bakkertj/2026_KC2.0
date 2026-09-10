// Demo: formatting ranges (C++23)
// Session: s02
// Compiler Explorer: <add short link>
// Availability: libc++ 17+; libstdc++ 15 (GCC 14 lacks it). Feature-test macro: __cpp_lib_format_ranges.
#include <map>
#include <print>
#include <string>
#include <vector>
#include <version>

int main() {
#ifdef __cpp_lib_format_ranges
    std::vector<double> v{1.5, 2.25, 3};
    std::map<std::string, int> m{{"rpm", 2}, {"temp", 1}};
    // [snippet: ranges]
    std::println("{}", v);           // [1.5, 2.25, 3]
    std::println("{::.1f}", v);      // [1.5, 2.2, 3.0]   the spec after :: applies to each element
    std::println("{:n}", v);         // 1.5, 2.25, 3      no brackets
    std::println("{}", m);           // {"rpm": 2, "temp": 1}
    std::println("{}", std::pair{1, "x"});   // (1, "x")
    // [/snippet]
#else
    std::println("range formatting not available in this standard library");
#endif
}
