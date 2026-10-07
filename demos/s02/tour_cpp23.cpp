// Demo: C++23 library odds and ends
// Session: s02
// Compiler Explorer: https://godbolt.org/z/rqcfM8rqv
#include <bit>
#include <cstdint>
#include <functional>
#include <memory>
#include <print>
#include <string>
#include <utility>
#include <version>

// A C API that fills a pointer, the way most of them do.
int c_open(const char*, void** out) { *out = new int{7}; return 0; }

int main() {
    // [snippet: tour]
    std::string s = "temp_core";
    std::println("{}", s.contains("_"));                          // finally

    std::println("{:04X}", std::byteswap(std::uint16_t{0x1234}));   // 3412; constexpr

    s.resize_and_overwrite(16, [](char* buf, std::size_t n) {     // write into uninitialized capacity
        return static_cast<std::size_t>(std::snprintf(buf, n, "rpm=%d", 4811));
    });
    std::println("{}", s);
    // [/snippet]

#ifdef __cpp_lib_move_only_function                               // libstdc++ 12+; libc++: not yet (18)
    std::move_only_function<int()> task = [p = std::make_unique<int>(5)] { return *p; };
    std::println("{}", task());
#endif

#ifdef __cpp_lib_out_ptr                                          // libstdc++ 14+; libc++ 19+
    // [snippet: out_ptr]
    std::unique_ptr<int> handle;
    c_open("x", std::out_ptr(handle));       // adapts a smart pointer to a T** C API
    // [/snippet]
    std::println("{}", *handle);
#endif
}
