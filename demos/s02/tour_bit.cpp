// Demo: <bit> and std::source_location (C++20)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/P8z77EK7z
#include <bit>
#include <cstdint>
#include <cstring>
#include <print>
#include <source_location>

void log(std::string_view msg, std::source_location loc = std::source_location::current()) {
    std::println("{}:{} {}: {}", loc.file_name(), loc.line(), loc.function_name(), msg);
}

int main() {
    // [snippet: bit]
    float f = 1.0f;
    auto bits = std::bit_cast<std::uint32_t>(f);          // the bytes, reinterpreted; constexpr; no UB
    // (C++11 spelling: std::memcpy(&bits, &f, sizeof f), or a union, or a UB pointer cast)

    std::println("{:08X}", bits);                          // 3F800000
    std::println("{}", std::popcount(0xF0u));              // 4
    std::println("{}", std::has_single_bit(64u));          // true: a power of two
    std::println("{}", std::bit_width(255u));              // 8
    std::println("{:08X}", std::rotl(0x80000001u, 1));     // 00000003
    std::println("{}", std::endian::native == std::endian::little);
    // [/snippet]

    // [snippet: source_location]
    log("no macros: the caller's file and line come from the default argument");
    // [/snippet]
}
