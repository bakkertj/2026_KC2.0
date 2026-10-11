// Lab 2.1: views over data (string_view and span)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/oxPd8j78j
// Ten minutes, three TODOs in order. Build and run after each one: the output says
// what you fixed. Finished version: lab1_views_solution.cpp
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

// TODO 1: count_prefix takes two const std::string&. Every call below with a string
//         literal builds a temporary std::string (an allocation per call). Change both
//         parameters to std::string_view. Nothing else should need to change.
std::size_t count_prefix(const std::string& text, const std::string& prefix) {
    std::size_t n = 0;
    for (std::size_t pos = 0; pos < text.size(); ++pos) {
        if (text.compare(pos, prefix.size(), prefix) == 0) ++n;
    }
    return n;
}

// TODO 2: checksum takes a pointer and a length: nothing ties the two together, and
//         the caller has to spell both. Take std::span<const std::uint8_t> instead,
//         then update the three calls in main so they pass the container (or a
//         subspan), never a pointer-plus-count.
std::uint8_t checksum(const std::uint8_t* data, std::size_t n) {
    std::uint8_t sum = 0;
    for (std::size_t i = 0; i < n; ++i) sum = static_cast<std::uint8_t>(sum + data[i]);
    return sum;
}

// TODO 3: this function returns a view of a local std::string. GCC compiles it without a
//         word (Clang warns: -Wreturn-stack-address), it runs, and it is undefined
//         behavior: the buffer is gone when the caller looks.
//         Fix the return type. Rule: a view in a return type is only right when the
//         viewed data outlives the call.
std::string_view make_frame_label(int id) {
    std::string s = "frame-" + std::to_string(id);
    return s;                              // a view of s, which dies on the next line
}

int main() {
    std::printf("prefix count: %zu\n", count_prefix("sensor,sensor,rpm,sensor", "sensor"));

    std::vector<std::uint8_t> payload{0x10, 0x20, 0x30};
    std::array<std::uint8_t, 4> frame{0xAA, 0x03, 0x10, 0x20};
    std::uint8_t raw[] = {1, 2, 3, 4, 5, 6};
    std::printf("checksums: %02X %02X %02X\n",
                checksum(payload.data(), payload.size()),
                checksum(frame.data(), frame.size()),
                checksum(raw + 2, 3));          // the middle three bytes; a subspan after TODO 2

    auto label = make_frame_label(7);
    std::printf("label: %.*s\n", static_cast<int>(label.size()), label.data());
}
