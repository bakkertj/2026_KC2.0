// Lab 2.1 solution: views over data (string_view and span)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/1abGeMqcT
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// TODO 1 done: string_view accepts a literal, a std::string, or a slice, with no allocation.
std::size_t count_prefix(std::string_view text, std::string_view prefix) {
    std::size_t n = 0;
    for (std::size_t pos = 0; pos < text.size(); ++pos) {
        if (text.substr(pos, prefix.size()) == prefix) ++n;
    }
    return n;
}

// TODO 2 done: one parameter that carries pointer and length together.
std::uint8_t checksum(std::span<const std::uint8_t> data) {
    std::uint8_t sum = 0;
    for (std::uint8_t b : data) sum = static_cast<std::uint8_t>(sum + b);
    return sum;
}

// TODO 3 done: the function creates the string, so it must hand ownership out.
// (With guaranteed copy elision the return costs nothing extra.)
std::string make_frame_label(int id) {
    std::string s = "frame-" + std::to_string(id);
    return s;
}

int main() {
    std::printf("prefix count: %zu\n", count_prefix("sensor,sensor,rpm,sensor", "sensor"));

    std::vector<std::uint8_t> payload{0x10, 0x20, 0x30};
    std::array<std::uint8_t, 4> frame{0xAA, 0x03, 0x10, 0x20};
    std::uint8_t raw[] = {1, 2, 3, 4, 5, 6};
    std::printf("checksums: %02X %02X %02X\n",
                checksum(payload),
                checksum(frame),
                checksum(std::span(raw).subspan(2, 3)));   // the middle three bytes

    std::string label = make_frame_label(7);
    std::printf("label: %s\n", label.c_str());
}
