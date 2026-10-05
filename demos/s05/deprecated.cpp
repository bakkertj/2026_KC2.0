// Demo: deprecations and removals, C++14 through C++23
// Session: s05
// Compiler Explorer: <add short link>
// Each block under SHOW_ERRORS uses something that a later standard deprecated or removed, next to
// its replacement. Build with -DSHOW_ERRORS -std=c++23 -Wdeprecated and read the diagnostics.
// libstdc++ keeps the removed names available with a deprecation warning; libc++ removes them
// (compile error), which is what "removed" means in the standard.
#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <print>
#include <random>
#include <type_traits>
#include <vector>

struct Sensor { int id; };
bool by_id(const Sensor& a, const Sensor& b) { return a.id < b.id; }
struct It {                                                   // was: struct It : std::iterator<std::forward_iterator_tag, int> (deprecated C++17)
    using iterator_category = std::forward_iterator_tag;
    using value_type = int; using difference_type = std::ptrdiff_t; using pointer = int*; using reference = int&;
};

// [snippet: replacements]

void modern(std::vector<Sensor>& v) {
    auto p = std::make_unique<Sensor>(1);                     // was: std::auto_ptr<Sensor> (removed in C++17)
    std::shuffle(v.begin(), v.end(), std::mt19937{42});       // was: std::random_shuffle (removed in C++17)
    auto less_than_5 = [](const Sensor& s) { return s.id < 5; };   // was: std::bind2nd(std::less<int>(), 5)
    using R = std::invoke_result_t<decltype(&by_id), Sensor, Sensor>;   // was: std::result_of (removed in C++20)
    static_assert(std::is_same_v<R, bool>);
    static_assert(std::is_same_v<std::iterator_traits<It>::value_type, int>);   // It: five typedefs, no std::iterator base
    alignas(Sensor) std::byte storage[sizeof(Sensor)];        // was: std::aligned_storage_t<sizeof(Sensor)> (deprecated C++23)
    (void)p; (void)less_than_5; (void)storage;
    std::println("{} sensors, shuffled deterministically", v.size());
}
// [/snippet]

#ifdef SHOW_ERRORS
// [snippet: legacy]
void legacy(std::vector<Sensor>& v) {
    std::auto_ptr<Sensor> p(new Sensor{1});                   // removed C++17 (libc++: error; libstdc++: warning)
    std::random_shuffle(v.begin(), v.end());                  // removed C++17
    auto lt5 = std::bind2nd(std::less<int>(), 5);             // removed C++17
    using R = std::result_of<decltype(&by_id)(Sensor, Sensor)>::type;   // removed C++20
    void f() throw();                                         // removed C++20: dynamic exception specification (GCC 13 still accepts it silently; Clang 18 warns)
    struct It : std::iterator<std::forward_iterator_tag, int> {};      // deprecated C++17
    std::aligned_storage_t<sizeof(Sensor), alignof(Sensor)> s;         // deprecated C++23
    volatile int counter = 0;
    ++counter;                                                // deprecated C++20 and still deprecated: C++23 (P2327R1) restored only |=, &=, ^=
}
// [/snippet]
#endif

int main() {
    std::vector<Sensor> v{{3}, {1}, {2}};
    modern(v);
}
