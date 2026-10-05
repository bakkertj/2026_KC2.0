// Demo: std::generator (C++23)
// Session: s05
// Compiler Explorer: https://godbolt.org/z/WKsn3cezn
// Availability: libstdc++ 14 ships <generator>; libc++ 18 does not. Gated on __cpp_lib_generator.
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#if __has_include(<generator>)
#include <generator>
#endif

#if defined(__cpp_lib_generator)

// [snippet: eager]
// C++11 to C++20: build the whole vector before anyone can look at the first element
std::vector<std::string> fields_eager(std::string_view line) {
    std::vector<std::string> out;
    for (auto piece : line | std::views::split(',')) out.emplace_back(piece.begin(), piece.end());
    return out;
}
// [/snippet]

// [snippet: lazy]
// C++23: a coroutine. Each co_yield hands one value to the consumer and suspends.
std::generator<std::string> fields(std::string_view line) {
    for (auto piece : line | std::views::split(','))
        co_yield std::string(piece.begin(), piece.end());
}                                               // returning ends the sequence

void use() {
    // std::generator is an input_range, so every Session 4 view composes with it
    for (const auto& f : fields("1,rpm,40,extra") | std::views::take(2))
        std::println("{}", f);                  // 1, rpm: the third field is never built
}
// [/snippet]

// [snippet: fib]
std::generator<long> fibonacci() {              // infinite: it is lazy, so that is fine
    long a = 0, b = 1;
    while (true) { co_yield a; std::tie(a, b) = std::pair{b, a + b}; }
}
// [/snippet]

int main() {
    use();
    for (auto x : fibonacci() | std::views::drop(10) | std::views::take(5)) std::print("{} ", x);
    std::println("");
    std::println("eager: {} fields", fields_eager("a,b,c").size());
}

#else
int main() { std::println("std::generator is not available on this standard library (libc++ 18)"); }
#endif
