// Demo: std::flat_map (C++23)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/n5rh7b6nx
// Availability: libstdc++ 15, libc++ 20 (flat_set: libc++ 21). Kept out of the default build
// when the header is missing; the slide uses Compiler Explorer.
#include <print>
#include <string_view>
#include <version>
#ifdef __cpp_lib_flat_map
#include <flat_map>
#endif

int main() {
#ifdef __cpp_lib_flat_map
    // [snippet: flat_map]
    // A sorted vector of keys and a sorted vector of values, with a map interface.
    // Contiguous, cache-friendly, cheap to iterate, expensive to insert in the middle.
    std::flat_map<std::string_view, double> limits{{"rpm", 12000.0}, {"temp_core", 125.0}};
    limits["pressure"] = 400.0;
    for (const auto& [name, max] : limits) std::println("{} {}", name, max);
    std::println("{}", limits.keys().size());          // the underlying containers are exposed
    // [/snippet]
#else
    std::println("flat_map not available in this standard library");
#endif
}
