#pragma once
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace telemetry {

// CRC-16/CCITT-FALSE (polynomial 0x1021, initial value 0xFFFF), the checksum
// used to fingerprint a record line. Takes a view of bytes: any contiguous
// buffer, no pointer/length pair. Session 3 builds the table at compile time.
[[nodiscard]] std::uint16_t crc16(std::span<const std::byte> data);
[[nodiscard]] std::uint16_t crc16(std::string_view text);

}  // namespace telemetry
