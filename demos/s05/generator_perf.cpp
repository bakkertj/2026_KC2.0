// Demo: what a generator costs (C++23)
// Session: s05
// Compiler Explorer: https://godbolt.org/z/es8cvdaMe
// Parses N comma-separated integers three ways and times each. Numbers are illustrative; build
// with -O2 (the repo defaults to Debug: cmake -DCMAKE_BUILD_TYPE=Release for real figures).
#include <charconv>
#include <chrono>
#include <cstdint>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#if __has_include(<generator>)
#include <generator>
#endif

namespace {

std::string make_input(int n) {
    std::string s;
    for (int i = 0; i < n; ++i) { s += std::to_string(i % 1000); s += ','; }
    return s;
}

int parse(std::string_view piece) {
    int v = 0;
    std::from_chars(piece.data(), piece.data() + piece.size(), v);
    return v;
}

// [snippet: three]
// 1. Eager: a vector of every value, then a loop over it. Two passes, one allocation per growth.
std::vector<int> values_eager(std::string_view in) {
    std::vector<int> out;
    for (auto p : in | std::views::split(',')) if (!p.empty()) out.push_back(parse(std::string_view(p)));
    return out;
}

// 2. A view pipeline: no coroutine, no allocation, fully inlinable. The Session 4 answer.
auto values_view(std::string_view in) {
    return in | std::views::split(',')
              | std::views::filter([](auto p) { return !p.empty(); })
              | std::views::transform([](auto p) { return parse(std::string_view(p)); });
}

#if defined(__cpp_lib_generator)
// 3. A generator: one frame allocation up front, then a suspend/resume pair per element.
std::generator<int> values_gen(std::string_view in) {
    for (auto p : in | std::views::split(',')) if (!p.empty()) co_yield parse(std::string_view(p));
}
#endif
// [/snippet]

template <typename F>
void time_it(std::string_view label, F f) {
    const auto t0 = std::chrono::steady_clock::now();
    const std::int64_t sum = f();
    const auto dt = std::chrono::steady_clock::now() - t0;
    std::println("{:<10} sum={} {:>8}", label, sum, std::chrono::duration_cast<std::chrono::microseconds>(dt));
}

}  // namespace

int main() {
    const std::string in = make_input(2'000'000);
    time_it("eager", [&] { std::int64_t s = 0; for (int v : values_eager(in)) s += v; return s; });
    time_it("view", [&] { std::int64_t s = 0; for (int v : values_view(in)) s += v; return s; });
#if defined(__cpp_lib_generator)
    time_it("generator", [&] { std::int64_t s = 0; for (int v : values_gen(in)) s += v; return s; });
#else
    std::println("generator: not available on this standard library");
#endif
}
