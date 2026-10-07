// Demo: span<const std::byte> for raw memory (C++20, std::byte C++17)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/GbqP6ern7
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>

// [snippet: bytes]
std::uint8_t checksum(std::span<const std::byte> data) {     // any object's bytes, any buffer
    std::uint8_t sum = 0;
    for (std::byte b : data) sum = static_cast<std::uint8_t>(sum + std::to_integer<std::uint8_t>(b));
    return sum;
}

struct Packet { std::uint16_t id; std::uint16_t len; std::uint32_t crc; };

void demo(std::string_view text, const Packet& p, std::span<std::byte, 8> fixed) {
    checksum(std::as_bytes(std::span{text}));                 // a string's bytes
    checksum(std::as_bytes(std::span{&p, 1}));                // a struct's bytes
    checksum(fixed);                                          // static extent: exactly 8, checked at compile time
}
// [/snippet]

int main() {
    Packet p{1, 2, 3};
    std::byte buf[8]{};
    demo("abc", p, buf);
    std::printf("%u\n", checksum(std::as_bytes(std::span{"abc"})));
}
