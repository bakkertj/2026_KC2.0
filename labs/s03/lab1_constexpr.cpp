// Lab 3.1: work that belongs at compile time (constexpr, consteval, static_assert)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/Mqv7hP6PK
// Ten minutes, three TODOs in order. Build and run after each one. When a TODO is done
// right, a static_assert you uncomment passes; when it is wrong, the compiler tells you.
// Finished version: lab1_constexpr_solution.cpp
#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

// ---- TODO 1: the lookup table ----------------------------------------------------
// make_crc8_table runs at program start-up, every start-up, to compute 256 constants.
// Make it constexpr (C++14 relaxed constexpr allows the loops and the local array),
// make kCrc8 `constexpr`, and uncomment the static_assert. The table is then baked into
// the binary and nothing runs at start-up.
std::array<std::uint8_t, 256> make_crc8_table() {
    std::array<std::uint8_t, 256> t{};
    for (int i = 0; i < 256; ++i) {
        std::uint8_t c = static_cast<std::uint8_t>(i);
        for (int k = 0; k < 8; ++k) c = static_cast<std::uint8_t>((c & 0x80) ? (c << 1) ^ 0x07 : (c << 1));
        t[static_cast<std::size_t>(i)] = c;
    }
    return t;
}
const std::array<std::uint8_t, 256> kCrc8 = make_crc8_table();
// static_assert(kCrc8[1] == 0x07 && kCrc8[255] == 0xF3, "CRC-8 table is wrong");

// ---- TODO 2: validation that should never reach runtime -------------------------
// The sensor table is checked by a runtime function that nobody calls. Make validate()
// consteval (it may only ever run at compile time), make kSensors constexpr, and
// uncomment the static_assert. Then break a row (give two sensors the same id) and
// read the error.
struct Sensor { std::string_view name; int id; double lo, hi; };

std::string_view validate(const Sensor* begin, const Sensor* end) {
    for (const Sensor* a = begin; a != end; ++a) {
        if (a->name.empty()) return "sensor has no name";
        if (!(a->lo < a->hi)) return "sensor has lo >= hi";
        for (const Sensor* b = a + 1; b != end; ++b)
            if (a->id == b->id) return "duplicate sensor id";
    }
    return {};
}
const std::array<Sensor, 3> kSensors{{{"rpm", 1, 0, 12000}, {"temp", 2, -40, 125}, {"psi", 3, 0, 300}}};
// static_assert(validate(kSensors.data(), kSensors.data() + kSensors.size()).empty());

// ---- TODO 3: a parser that works in both worlds ---------------------------------
// parse_hex is plain runtime code. Make it constexpr and uncomment the static_assert:
// the same function then checks a protocol constant at compile time and parses frames
// at runtime. (No change to the body is needed; that is the point.)
std::uint32_t parse_hex(std::string_view s) {
    std::uint32_t v = 0;
    for (char c : s) {
        v <<= 4;
        if (c >= '0' && c <= '9')      v |= static_cast<std::uint32_t>(c - '0');
        else if (c >= 'a' && c <= 'f') v |= static_cast<std::uint32_t>(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') v |= static_cast<std::uint32_t>(c - 'A' + 10);
    }
    return v;
}
// static_assert(parse_hex("CAFE") == 0xCAFE);

int main() {
    std::printf("crc8[1]=%02X crc8[255]=%02X\n", kCrc8[1], kCrc8[255]);
    std::printf("sensors ok: %s\n", validate(kSensors.data(), kSensors.data() + kSensors.size()).empty() ? "yes" : "no");
    std::printf("parse_hex(\"BEEF\") = %u\n", parse_hex("BEEF"));
}
