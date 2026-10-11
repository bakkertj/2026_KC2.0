// Lab 5.3: deprecated and removed, fixed (C++14 through C++23)
// Session: s05
// Compiler Explorer: <add short link>
// Ten minutes. This file compiles under -std=c++23 with libstdc++, but every marked line
// uses something a later standard deprecated or removed, and the compiler says so. Goal:
// zero warnings, same output. Each line names its replacement in the comment; the slides
// have the rest. (libc++ removes several of these outright, so on that library this file
// does not compile at all: that is what "removed" means.)
// Finished version: lab3_deprecations_solution.cpp
// godbolt: -Wdeprecated
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <functional>
#include <iterator>
#include <memory>
#include <random>
#include <type_traits>
#include <vector>

struct Sensor { int id; };
bool by_id(const Sensor& a, const Sensor& b) { return a.id < b.id; }

// 1. std::iterator as a base class (deprecated C++17): spell the five typedefs instead.
struct CountingIt : std::iterator<std::forward_iterator_tag, int> {
    int n = 0;
    int operator*() const { return n; }
    CountingIt& operator++() { ++n; return *this; }
    bool operator!=(const CountingIt& o) const { return n != o.n; }
};

int main() {
    std::vector<Sensor> v{{3}, {1}, {2}};

    // 2. std::auto_ptr (removed C++17): std::unique_ptr.
    std::auto_ptr<Sensor> owner(new Sensor{7});

    // 3. std::random_shuffle (removed C++17): std::shuffle with an explicit engine.
    std::random_shuffle(v.begin(), v.end());

    // 4. std::bind2nd (removed C++17): a lambda.
    auto less_than_5 = std::bind2nd(std::less<int>(), 5);

    // 5. std::result_of (removed C++20): std::invoke_result_t. libstdc++ still accepts it
    //    without a word; libc++ does not. Fix it anyway.
    using R = std::result_of<decltype(&by_id)(Sensor, Sensor)>::type;
    static_assert(std::is_same<R, bool>::value, "by_id returns bool");

    // 6. std::aligned_storage (deprecated C++23): alignas(T) std::byte storage[sizeof(T)].
    std::aligned_storage_t<sizeof(Sensor), alignof(Sensor)> raw;

    // 7. ++ on a volatile (deprecated C++20): read, modify, write as three explicit steps.
    volatile int counter = 0;
    ++counter;

    std::sort(v.begin(), v.end(), by_id);
    int sum = 0;
    CountingIt end; end.n = 4;
    for (CountingIt it; it != end; ++it) sum += *it;
    std::printf("owner=%d first=%d lt5(3)=%d storage=%zu counter=%d sum=%d\n",
                owner->id, v.front().id, less_than_5(3) ? 1 : 0, sizeof(raw), counter, sum);
}
