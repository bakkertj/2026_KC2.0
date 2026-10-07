// Demo: chunk, slide, stride, adjacent (C++23)
// Session: s04
// Compiler Explorer: https://godbolt.org/z/GqEcdhbMr
// Availability: libstdc++ 13+. libc++ (including Apple Clang 17 / Xcode 26) lacks some or all of
// chunk/slide/stride/adjacent; gated on the feature-test macros rather than a library version.
#include <print>
#include <ranges>
#include <vector>
#include <version>

int main() {
#if defined(__cpp_lib_ranges_chunk) && defined(__cpp_lib_ranges_slide) && defined(__cpp_lib_ranges_stride) && defined(__cpp_lib_ranges_zip)
    std::vector<double> signal{1, 2, 4, 8, 16, 32, 64};
    namespace v = std::views;
    // [snippet: windows]
    for (auto c : signal | v::chunk(3)) std::print("[{} .. {}] ", c.front(), c.back());   // [1..4] [8..32] [64..64]
    std::println("");

    // a moving average: slide(3) yields overlapping windows of 3
    for (auto w : signal | v::slide(3)) {
        double sum = 0; for (double x : w) sum += x;
        std::print("{:.1f} ", sum / 3);                                                   // 2.3 4.7 9.3 ...
    }
    std::println("");

    for (double x : signal | v::stride(2)) std::print("{} ", x);                          // 1 4 16 64
    std::println("");

    for (auto [a, b] : signal | v::adjacent<2>) std::print("{} ", b - a);                 // deltas: 1 2 4 8 16 32
    std::println("");
    for (auto [a, b] : signal | v::pairwise) std::print("{} ", b / a);                    // ratios: pairwise = adjacent<2>
    std::println("");
    // [/snippet]
#else
    std::println("window views not available in this standard library");
#endif
}
