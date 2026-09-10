#include "telemetry/crc.h"

namespace telemetry {

namespace {

constexpr std::uint16_t kPolynomial = 0b0001'0000'0010'0001;  // 0x1021

struct CrcTable {
    std::uint16_t entries[256];

    CrcTable() {
        for (unsigned i = 0; i < 256; ++i) {
            auto crc = static_cast<std::uint16_t>(i << 8);
            for (int bit = 0; bit < 8; ++bit) {
                if (crc & 0x8000) {
                    crc = static_cast<std::uint16_t>((crc << 1) ^ kPolynomial);
                } else {
                    crc = static_cast<std::uint16_t>(crc << 1);
                }
            }
            entries[i] = crc;
        }
    }
};

const CrcTable& table() {
    static const CrcTable t;
    return t;
}

}  // namespace

std::uint16_t crc16(std::span<const std::byte> data) {
    const auto& t = table();
    std::uint16_t crc = 0xFFFF;
    for (const std::byte b : data) {
        const auto index = static_cast<unsigned char>((crc >> 8) ^ std::to_integer<unsigned>(b));
        crc = static_cast<std::uint16_t>((crc << 8) ^ t.entries[index]);
    }
    return crc;
}

std::uint16_t crc16(std::string_view text) { return crc16(std::as_bytes(std::span{text})); }

}  // namespace telemetry
