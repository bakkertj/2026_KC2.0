// Demo: a named module (C++20). Interface unit for module telemetry.crc.
// Build: needs CMake 3.28+, the Ninja generator, and GCC 14 / Clang 16+ (see CMakeLists.txt).
module;                          // global module fragment: #includes go here, before the module name
#include <array>
#include <cstdint>
#include <span>
#include <string_view>

export module telemetry.crc;     // this file IS the module; nothing after this line leaks unless exported

namespace telemetry {

constexpr std::uint16_t kPolynomial = 0x1021;   // not exported: module-private

constexpr std::array<std::uint16_t, 256> make_table() {
    std::array<std::uint16_t, 256> t{};
    for (unsigned i = 0; i < 256; ++i) {
        auto crc = static_cast<std::uint16_t>(i << 8);
        for (int b = 0; b < 8; ++b) crc = static_cast<std::uint16_t>((crc & 0x8000) ? (crc << 1) ^ kPolynomial : crc << 1);
        t[i] = crc;
    }
    return t;
}
constexpr auto kTable = make_table();

export constexpr std::uint16_t crc16(std::string_view text) {   // exported: the module's interface
    std::uint16_t crc = 0xFFFF;
    for (char c : text) crc = static_cast<std::uint16_t>((crc << 8) ^ kTable[static_cast<unsigned char>((crc >> 8) ^ static_cast<unsigned char>(c))]);
    return crc;
}

}  // namespace telemetry
