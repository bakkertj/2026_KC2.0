// Lab 3.1 solution: work that belongs at compile time (constexpr, consteval, static_assert)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/48G96s5Kn
#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

// TODO 1 done: constexpr function + constexpr variable = the table is data in the binary.
constexpr std::array<std::uint8_t, 256> make_crc8_table() {
    std::array<std::uint8_t, 256> t{};
    for (int i = 0; i < 256; ++i) {
        std::uint8_t c = static_cast<std::uint8_t>(i);
        for (int k = 0; k < 8; ++k) c = static_cast<std::uint8_t>((c & 0x80) ? (c << 1) ^ 0x07 : (c << 1));
        t[static_cast<std::size_t>(i)] = c;
    }
    return t;
}
constexpr std::array<std::uint8_t, 256> kCrc8 = make_crc8_table();
static_assert(kCrc8[1] == 0x07 && kCrc8[255] == 0xF3, "CRC-8 table is wrong");

// TODO 2 done: consteval means this cannot be called at runtime; the only way to use it
// is static_assert or a constexpr initializer, so a bad table cannot reach a build.
struct Sensor { std::string_view name; int id; double lo, hi; };

consteval std::string_view validate(const Sensor* begin, const Sensor* end) {
    for (const Sensor* a = begin; a != end; ++a) {
        if (a->name.empty()) return "sensor has no name";
        if (!(a->lo < a->hi)) return "sensor has lo >= hi";
        for (const Sensor* b = a + 1; b != end; ++b)
            if (a->id == b->id) return "duplicate sensor id";
    }
    return {};
}
constexpr std::array<Sensor, 3> kSensors{{{"rpm", 1, 0, 12000}, {"temp", 2, -40, 125}, {"psi", 3, 0, 300}}};
static_assert(validate(kSensors.data(), kSensors.data() + kSensors.size()).empty());

// TODO 3 done: one function, two worlds. The body did not change.
constexpr std::uint32_t parse_hex(std::string_view s) {
    std::uint32_t v = 0;
    for (char c : s) {
        v <<= 4;
        if (c >= '0' && c <= '9')      v |= static_cast<std::uint32_t>(c - '0');
        else if (c >= 'a' && c <= 'f') v |= static_cast<std::uint32_t>(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') v |= static_cast<std::uint32_t>(c - 'A' + 10);
    }
    return v;
}
static_assert(parse_hex("CAFE") == 0xCAFE);

int main() {
    std::printf("crc8[1]=%02X crc8[255]=%02X\n", kCrc8[1], kCrc8[255]);
    // validate() is consteval, so this call is evaluated by the compiler too:
    std::printf("sensors ok: %s\n", validate(kSensors.data(), kSensors.data() + kSensors.size()).empty() ? "yes" : "no");
    char frame[] = "BEEF";                                    // runtime data
    std::printf("parse_hex(\"%s\") = %u\n", frame, parse_hex(frame));
}
