#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace telemetry {

// CRC-16/CCITT-FALSE (polynomial 0x1021, initial value 0xFFFF).
//
// Session 3: the lookup table is computed at compile time and crc16 itself is
// constexpr, so a checksum of a constant is a constant and the well-known check
// value is a static_assert (see tests). Same 256 entries, zero runtime work.

inline constexpr std::uint16_t kCrcPolynomial = 0b0001'0000'0010'0001;  // 0x1021

// C++14 relaxed constexpr: loops and locals in a constant expression.
// C++17: std::array is usable in one. (C++20 would allow std::vector too.)
[[nodiscard]] constexpr std::array<std::uint16_t, 256> make_crc_table() {
    std::array<std::uint16_t, 256> table{};
    for (unsigned i = 0; i < 256; ++i) {
        auto crc = static_cast<std::uint16_t>(i << 8);
        for (int bit = 0; bit < 8; ++bit) {
            crc = static_cast<std::uint16_t>((crc & 0x8000) ? (crc << 1) ^ kCrcPolynomial : crc << 1);
        }
        table[i] = crc;
    }
    return table;
}

// inline constexpr: one table, in the header, initialized at compile time.
// No static-initialization-order problem, no first-use check.
inline constexpr auto kCrcTable = make_crc_table();

namespace detail {

// The core is generic over the byte type. std::as_bytes cannot be used here:
// it is a reinterpret_cast underneath, and reinterpret_cast is never allowed
// in a constant expression. static_cast from char or std::byte is.
template <typename Byte>
    requires(sizeof(Byte) == 1)
[[nodiscard]] constexpr std::uint16_t crc16_impl(std::span<const Byte> data) {
    std::uint16_t crc = 0xFFFF;
    for (const Byte b : data) {
        const auto index = static_cast<unsigned char>((crc >> 8) ^ static_cast<unsigned char>(b));
        crc = static_cast<std::uint16_t>((crc << 8) ^ kCrcTable[index]);
    }
    return crc;
}

}  // namespace detail

[[nodiscard]] constexpr std::uint16_t crc16(std::span<const std::byte> data) {
    return detail::crc16_impl(data);
}

[[nodiscard]] constexpr std::uint16_t crc16(std::string_view text) {
    return detail::crc16_impl(std::span<const char>{text});
}

}  // namespace telemetry
