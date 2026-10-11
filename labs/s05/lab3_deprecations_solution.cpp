// Lab 5.3 solution: deprecated and removed, fixed (C++14 through C++23)
// Session: s05
// Compiler Explorer: https://godbolt.org/z/bzMd3x6j9
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

// 1. The five typedefs, spelled out. (C++20 iterators need even less: iterator_traits
//    deduces most of them from the operators.)
struct CountingIt {
    using iterator_category = std::forward_iterator_tag;
    using value_type = int;
    using difference_type = std::ptrdiff_t;
    using pointer = int*;
    using reference = int&;
    int n = 0;
    int operator*() const { return n; }
    CountingIt& operator++() { ++n; return *this; }
    bool operator!=(const CountingIt& o) const { return n != o.n; }
};

int main() {
    std::vector<Sensor> v{{3}, {1}, {2}};

    // 2. unique_ptr: same cost, sane copy semantics.
    auto owner = std::make_unique<Sensor>(7);

    // 3. shuffle with an explicit engine: reproducible, and no hidden rand().
    std::shuffle(v.begin(), v.end(), std::mt19937{42});

    // 4. A lambda says what bind2nd only hinted at.
    auto less_than_5 = [](int x) { return x < 5; };

    // 5. invoke_result_t takes the callable type and the argument types separately.
    using R = std::invoke_result_t<decltype(&by_id), Sensor, Sensor>;
    static_assert(std::is_same_v<R, bool>, "by_id returns bool");

    // 6. Aligned raw storage without the trait (and without the trait's size pitfalls).
    alignas(Sensor) std::byte raw[sizeof(Sensor)];

    // 7. Three explicit steps make the non-atomicity visible, which was the point of
    //    deprecating the shorthand.
    volatile int counter = 0;
    int tmp = counter;
    ++tmp;
    counter = tmp;

    std::sort(v.begin(), v.end(), by_id);
    int sum = 0;
    CountingIt end; end.n = 4;
    for (CountingIt it; it != end; ++it) sum += *it;
    std::printf("owner=%d first=%d lt5(3)=%d storage=%zu counter=%d sum=%d\n",
                owner->id, v.front().id, less_than_5(3) ? 1 : 0, sizeof(raw), counter, sum);
}
