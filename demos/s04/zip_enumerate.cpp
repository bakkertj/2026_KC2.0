// Demo: zip, enumerate, and iota for indexing (C++23)
// Session: s04
// Compiler Explorer: <add short link>
#include <print>
#include <ranges>
#include <string>
#include <vector>
#include <version>

int main() {
    std::vector<std::string> names{"rpm", "temp", "pressure"};
    std::vector<double> values{4811.0, 42.1, 101.3};
    // [snippet: zip]
    // zip: iterate two (or more) ranges in lockstep, as tuples
    for (const auto& [name, value] : std::views::zip(names, values)) std::print("{}={} ", name, value);
    std::println("");

    // an index alongside: zip with iota (works everywhere), or enumerate (libc++ 20+)
    for (const auto& [i, name] : std::views::zip(std::views::iota(1uz), names)) std::print("#{} {} ", i, name);
    std::println("");
#ifdef __cpp_lib_ranges_enumerate                   // enumerate: from 0 (libc++ 20+)
    for (const auto& [i, name] : names | std::views::enumerate) std::print("[{}] {} ", i, name);
    std::println("");
#endif
    // [/snippet]

    // zip_transform: combine in lockstep. __cpp_lib_ranges_zip is defined only once zip, zip_transform,
    // adjacent and adjacent_transform are all present; libc++ has zip alone for several releases.
#ifdef __cpp_lib_ranges_zip
    for (auto s : std::views::zip_transform([](const std::string& n, double v) { return n + ":" + std::to_string(int(v)); }, names, values))
        std::print("{} ", s);
    std::println("");
#endif
}
