// Demo: C++17 library odds and ends
// Session: s02
// Compiler Explorer: https://godbolt.org/z/Po9Kcnxbx
#include <algorithm>
#include <functional>
#include <map>
#include <numeric>
#include <print>
#include <random>
#include <string>
#include <tuple>
#include <vector>

int add(int a, int b) { return a + b; }

int main() {
    // [snippet: tour]
    std::println("{}", std::clamp(150, 0, 100));                 // 100
    std::println("{} {}", std::gcd(12, 18), std::lcm(4, 6));     // 6 12
    std::println("{}", std::invoke(add, 2, 3));                  // call anything callable uniformly
    std::println("{}", std::apply(add, std::tuple{2, 3}));      // unpack a tuple into arguments

    std::map<std::string, int> a{{"x", 1}}, b{{"y", 2}, {"x", 9}};
    a.merge(b);                                                  // splice "y" in; "x" stays in b
    auto node = a.extract("x");                                  // take a node out without reallocating
    node.key() = "z";                                            // change the key in place
    a.insert(std::move(node));
    std::println("{} {} {}", a.size(), a.count("z"), b.count("x"));   // 2 1 1

    std::vector<int> v{1, 2, 3, 4, 5, 6}, sample;
    std::sample(v.begin(), v.end(), std::back_inserter(sample), 2, std::mt19937{42});
    std::println("{} {}", sample.size(), std::reduce(v.begin(), v.end()));   // reduce: order-agnostic sum
    // [/snippet]
}
