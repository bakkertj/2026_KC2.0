// Lab 5.2: std::generator (lazy sequences as coroutines)
// Session: s05
// Compiler Explorer: https://godbolt.org/z/sThP6cG19
// Ten minutes, three TODOs in order. Build and run after each one.
// Needs libstdc++ 14 (GCC 14) for <generator>. Finished version: lab2_generator_solution.cpp
#include <cstdio>
#include <generator>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

// ---- TODO 1: an eager vector -> a lazy generator ---------------------------------
// squares() builds the whole vector even when the caller wants two values. Rewrite it as
//     std::generator<int> squares(int n) { for (...) co_yield i * i; }
// main's loop does not change; now only the values that are pulled are computed.
std::vector<int> squares(int n) {
    std::vector<int> out;
    for (int i = 1; i <= n; ++i) out.push_back(i * i);
    return out;
}

// ---- TODO 2: a parser that yields records as it finds them ------------------------
// parse_all splits a log into lines and parses every one up front. Turn it into
//     std::generator<Reading> readings(std::string_view log)
// that co_yields each Reading as soon as its line is parsed. Then change main to take
// only the first two with `| std::views::take(2)`, and watch the order of "parsed:" and
// "use:" lines. Is the third line parsed? (Keep the "parsed:" print; it is the evidence.)
struct Reading { std::string sensor; int value; };

std::vector<Reading> parse_all(std::string_view log) {
    std::vector<Reading> out;
    while (!log.empty()) {
        auto nl = log.find('\n');
        std::string_view line = log.substr(0, nl);
        log.remove_prefix(nl == std::string_view::npos ? log.size() : nl + 1);
        auto comma = line.find(',');
        std::printf("  parsed: %.*s\n", static_cast<int>(line.size()), line.data());
        out.push_back({std::string(line.substr(0, comma)), std::stoi(std::string(line.substr(comma + 1)))});
    }
    return out;
}

// ---- TODO 3: what happens to a suspended coroutine ---------------------------------
// Add a local object with a destructor that prints inside the generator body from TODO 1
// (a struct Guard { ~Guard() { std::puts("  generator frame destroyed"); } } will do), and
// a std::puts("  generator finished") AFTER the loop. Run with take(2): which of the two
// lines prints? Why? (Destroying a suspended coroutine runs destructors; it does not
// run the rest of the body.)

int main() {
    std::printf("squares:\n");
    for (int v : squares(10) | std::views::take(2)) std::printf("  %d\n", v);   // TODO 1: with a generator, 3..10 are never computed

    std::printf("readings:\n");
    for (const Reading& r : parse_all("rpm,3200\ntemp,88\npsi,31")) {         // TODO 2: readings(...) | std::views::take(2)
        std::printf("  use: %s=%d\n", r.sensor.c_str(), r.value);
    }
}
