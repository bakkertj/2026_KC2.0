// Demo: C++20 library odds and ends
// Session: s02
// Compiler Explorer: https://godbolt.org/z/YKodx7Kz1
#include <array>
#include <cmath>
#include <map>
#include <numbers>
#include <numeric>
#include <print>
#include <set>
#include <string>
#include <vector>

int main() {
    // [snippet: tour]
    std::vector<int> v{1, 2, 3, 4, 5, 6};
    std::erase_if(v, [](int x) { return x % 2 == 0; });        // finally: no erase(remove_if(...))
    std::println("{} {} {}", v[0], v[1], v[2]);                 // 1 3 5 (range formatting: GCC 15 / libc++)

    std::string s = "temp_core";
    std::println("{} {}", s.starts_with("temp"), s.ends_with("_core"));
    std::set<int> primes{2, 3, 5};
    std::println("{}", primes.contains(3));                     // no more find() != end()

    auto arr = std::to_array({1, 2, 3});                        // std::array<int, 3> from a literal
    std::println("{} {}", std::ssize(v), arr.size());           // signed size: no -Wsign-compare
    std::println("{} {}", std::midpoint(1, 4), std::lerp(0.0, 10.0, 0.25));   // overflow-safe midpoint
    std::println("{:.5f} {:.5f}", std::numbers::pi, std::numbers::sqrt2);
    // [/snippet]
}
