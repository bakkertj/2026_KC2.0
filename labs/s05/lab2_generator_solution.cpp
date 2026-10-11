// Lab 5.2 solution: std::generator (lazy sequences as coroutines)
// Session: s05
// Compiler Explorer: <add short link>
#include <cstdio>
#include <generator>
#include <ranges>
#include <string>
#include <string_view>

// TODO 1 and 3 done: a coroutine; each co_yield suspends with the value and resumes on
// the next pull. Guard shows what destruction of a suspended frame does.
struct Guard { ~Guard() { std::puts("  generator frame destroyed"); } };

std::generator<int> squares(int n) {
    Guard g;
    for (int i = 1; i <= n; ++i) co_yield i * i;
    std::puts("  generator finished");            // never printed with take(2): the body does not resume
}

// TODO 2 done: parse one line, hand it out, suspend. The caller decides how many.
struct Reading { std::string sensor; int value; };

std::generator<Reading> readings(std::string_view log) {
    while (!log.empty()) {
        auto nl = log.find('\n');
        std::string_view line = log.substr(0, nl);
        log.remove_prefix(nl == std::string_view::npos ? log.size() : nl + 1);
        auto comma = line.find(',');
        std::printf("  parsed: %.*s\n", static_cast<int>(line.size()), line.data());
        co_yield Reading{std::string(line.substr(0, comma)), std::stoi(std::string(line.substr(comma + 1)))};
    }
}

int main() {
    std::printf("squares:\n");
    for (int v : squares(10) | std::views::take(2)) std::printf("  %d\n", v);
    // Output: 1, 4, then "generator frame destroyed". The generator was suspended at the
    // second co_yield when take(2) stopped pulling; its destructor ran Guard's destructor
    // and skipped the rest of the body, so "generator finished" never appears.

    std::printf("readings:\n");
    for (const Reading& r : readings("rpm,3200\ntemp,88\npsi,31") | std::views::take(2)) {
        std::printf("  use: %s=%d\n", r.sensor.c_str(), r.value);
    }
    // "parsed:" and "use:" now interleave: each line is parsed only when the loop pulls it.
    // The third line IS parsed but never used: take(2)'s counted iterator advances the
    // generator once more before it compares the count. That is the lookahead from the
    // slides, and the reason a generator must not do irreversible work before co_yield.
}
