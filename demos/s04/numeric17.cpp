// Demo: C++17 numeric algorithms
// Session: s04
// Compiler Explorer: https://godbolt.org/z/vxM3neq3j
#include <algorithm>
#include <functional>
#include <numeric>
#include <print>
#include <random>
#include <vector>

int main() {
    std::vector<double> v{1.5, 2.5, 3.0};
    // [snippet: numeric]
    double sum = std::reduce(v.begin(), v.end());                        // like accumulate, but order-agnostic:
                                                                         // parallelizable, and init defaults to T{}
    double dot = std::transform_reduce(v.begin(), v.end(), v.begin(), 0.0);   // sum of products, one pass
    std::vector<double> running(v.size());
    std::inclusive_scan(v.begin(), v.end(), running.begin());            // prefix sums: 1.5 4.0 7.0
    std::exclusive_scan(v.begin(), v.end(), running.begin(), 0.0);       // 0 1.5 4.0

    std::vector<int> pool(100), sample;
    std::iota(pool.begin(), pool.end(), 0);
    std::sample(pool.begin(), pool.end(), std::back_inserter(sample), 5, std::mt19937{42});   // 5 without replacement
    int c = std::clamp(150, 0, 100);
    // [/snippet]
    std::println("{} {} {} {} {}", sum, dot, running[2], sample.size(), c);
}
