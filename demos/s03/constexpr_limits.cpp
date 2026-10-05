// Demo: what a constant expression refuses (and why that is a feature)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/6GGTzK98P
#include <array>
#include <climits>
#include <cstdint>
#include <print>
#include <span>

// [snippet: ub]
constexpr int overflow(int x) { return x + 1; }
constexpr int at(const std::array<int, 3>& a, std::size_t i) { return a[i]; }

#ifdef SHOW_ERRORS
constexpr int uninit() { int x; return x; }       // (Clang rejects the read even before constant evaluation)
static_assert(overflow(INT_MAX) == INT_MIN);      // error: overflow in constant expression
static_assert(at({1, 2, 3}, 5) == 0);             // error: array subscript out of bounds
static_assert(uninit() == 0);                     // error: read of uninitialized object
#endif
// A constant expression cannot contain undefined behavior. The compiler is a sanitizer
// for every constexpr call it evaluates, with no runtime cost.
// [/snippet]

// [snippet: reinterpret]
constexpr std::uint16_t first_two_bytes(std::span<const char> s) {
#ifdef SHOW_ERRORS
    return *reinterpret_cast<const std::uint16_t*>(s.data());   // error: reinterpret_cast is never constexpr
#else
    return static_cast<std::uint16_t>((static_cast<unsigned char>(s[1]) << 8) | static_cast<unsigned char>(s[0]));
#endif
}
// This is why std::as_bytes is not constexpr, and why the exercise's crc16 uses static_cast per byte.
// [/snippet]

static_assert(overflow(1) == 2);
static_assert(first_two_bytes(std::span<const char>{"AB", 2}) == 0x4241);

int main() { std::println("{:04X}", first_two_bytes(std::span<const char>{"AB", 2})); }
