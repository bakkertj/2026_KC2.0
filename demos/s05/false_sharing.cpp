// Demo: hardware_destructive_interference_size (C++17) and false sharing
// Session: s05
// Compiler Explorer: https://godbolt.org/z/ffG7nPnxf
#include <atomic>
#include <chrono>
#include <cstddef>
#include <new>
#include <print>
#include <thread>

// libc++ 18 does not define the interference sizes (the value is ABI-sensitive); fall back to 64.
#ifdef __cpp_lib_hardware_interference_size
constexpr std::size_t kLine = std::hardware_destructive_interference_size;
#else
constexpr std::size_t kLine = 64;
#endif

// [snippet: false_sharing]
struct Packed { std::atomic<long> a{0}, b{0}; };   // one cache line: each write invalidates the other core's copy

struct Padded {
    alignas(kLine) std::atomic<long> a{0};   // kLine = hardware_destructive_interference_size (64 on x86)
    alignas(kLine) std::atomic<long> b{0};   // libc++ 18 lacks the constant; the demo falls back to 64
};
// [/snippet]

template <typename C>
double run(C& c) {
    auto t0 = std::chrono::steady_clock::now();
    {
        std::jthread ta([&] { for (int i = 0; i < 20'000'000; ++i) c.a.fetch_add(1, std::memory_order_relaxed); });
        std::jthread tb([&] { for (int i = 0; i < 20'000'000; ++i) c.b.fetch_add(1, std::memory_order_relaxed); });
    }
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
}

int main() {
    Packed p; Padded q;
    std::println("packed {:.0f}ms  padded {:.0f}ms  (sizeof {} vs {})", run(p), run(q), sizeof p, sizeof q);
}
