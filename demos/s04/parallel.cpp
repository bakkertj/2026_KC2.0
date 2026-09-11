// Demo: parallel algorithms (C++17 execution policies)
// Session: s04
// Compiler Explorer: <add short link>
// libstdc++ needs TBB (link -ltbb) for par to actually parallelize; without it the
// policies compile and run sequentially. libc++ 18: partial (-fexperimental-library).
#include <algorithm>
#include <chrono>
#include <execution>
#include <numeric>
#include <random>
#include <print>
#include <vector>
#include <version>

int main() {
#ifndef __cpp_lib_parallel_algorithm   // libc++ 18: needs -fexperimental-library
    std::println("parallel algorithms not available in this standard library build");
#else
    std::vector<double> v(5'000'000);
    std::iota(v.begin(), v.end(), 0.0);
    // [snippet: parallel]
    namespace ex = std::execution;
    std::mt19937 rng{42};
    std::shuffle(v.begin(), v.end(), rng);
    auto t0 = std::chrono::steady_clock::now();
    std::sort(ex::seq, v.begin(), v.end());                          // sequential: same as plain sort
    auto t1 = std::chrono::steady_clock::now();
    std::shuffle(v.begin(), v.end(), rng);                           // same input again, for a fair comparison
    t1 = std::chrono::steady_clock::now();
    std::sort(ex::par, v.begin(), v.end());                          // may use threads; elements must be independent
    auto t2 = std::chrono::steady_clock::now();
    double s = std::reduce(ex::par_unseq, v.begin(), v.end());       // may also vectorize: no locks, no allocation in the body
    auto t3 = std::chrono::steady_clock::now();
    // Not available for std::ranges:: algorithms until C++26. Iterator pairs only.
    // [/snippet]
    using ms = std::chrono::duration<double, std::milli>;
    std::println("seq {:.0f}ms  par {:.0f}ms  par_unseq reduce {:.0f}ms  sum {:.3e}", ms(t1 - t0).count(), ms(t2 - t1).count(), ms(t3 - t2).count(), s);
#endif
}
