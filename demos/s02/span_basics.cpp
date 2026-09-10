// Demo: std::span (C++20)
// Session: s02
// Compiler Explorer: <add short link>
#include <array>
#include <cstdio>
#include <span>
#include <vector>

// [snippet: before]
// C++11: says "a vector" when it means "some doubles"
double mean_cpp11(const std::vector<double>& v) {
    double s = 0;
    for (double x : v) s += x;
    return s / static_cast<double>(v.size());
}
// mean_cpp11(arr);            // error: a C array is not a vector
// mean_cpp11({v.begin()+1, v.end()});   // copies
// [/snippet]

// [snippet: after]
// C++20: any contiguous run of doubles, no copy, no ownership
double mean(std::span<const double> v) {
    double s = 0;
    for (double x : v) s += x;
    return s / static_cast<double>(v.size());
}
// [/snippet]

int main() {
    std::vector<double> v{1, 2, 3, 4};
    double arr[] = {10, 20, 30};
    std::array<double, 2> a{5, 7};
    // [snippet: calls]
    mean(v);                       // vector
    mean(arr);                     // C array: size deduced
    mean(a);                       // std::array
    mean(std::span{v}.subspan(1, 2));   // a slice: {2, 3}
    mean({v.data() + 2, 2});       // pointer + count, the C interface
    // [/snippet]
    std::printf("%.2f %.2f %.2f %.2f\n", mean(v), mean(arr), mean(a), mean(std::span{v}.subspan(1, 2)));
}
