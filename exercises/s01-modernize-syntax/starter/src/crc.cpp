#include "telemetry/crc.h"

namespace telemetry {

namespace {

const std::uint16_t kPolynomial = 0x1021;

struct CrcTable {
    std::uint16_t entries[256];

    CrcTable() {
        for (unsigned i = 0; i < 256; ++i) {
            std::uint16_t crc = static_cast<std::uint16_t>(i << 8);
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

std::uint16_t crc16(const unsigned char* data, std::size_t len) {
    const CrcTable& t = table();
    std::uint16_t crc = 0xFFFF;
    for (std::size_t i = 0; i < len; ++i) {
        const unsigned char index = static_cast<unsigned char>((crc >> 8) ^ data[i]);
        crc = static_cast<std::uint16_t>((crc << 8) ^ t.entries[index]);
    }
    return crc;
}

std::uint16_t crc16(const std::string& text) {
    return crc16(reinterpret_cast<const unsigned char*>(text.data()), text.size());
}

}  // namespace telemetry
