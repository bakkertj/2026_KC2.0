#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

namespace telemetry {

// CRC-16/CCITT-FALSE (polynomial 0x1021, initial value 0xFFFF), the checksum
// used to fingerprint a record line. The lookup table is built on first use;
// Session 3 builds it at compile time.
[[nodiscard]] std::uint16_t crc16(const unsigned char* data, std::size_t len);
[[nodiscard]] std::uint16_t crc16(const std::string& text);

}  // namespace telemetry
