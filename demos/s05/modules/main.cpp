// Demo: importing a named module (C++20)
#include <print>                 // GCC 14: #includes must come BEFORE imports in the same file, or the
                                 // headers already attached to the module's global fragment collide.
import telemetry.crc;            // no header, no include guard, no macro leakage, parsed once

static_assert(telemetry::crc16("123456789") == 0x29B1);   // constexpr across the module boundary

int main() { std::println("{:04X}", telemetry::crc16("telemetry")); }
