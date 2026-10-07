// Demo: std::stacktrace (C++23)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/TjjxxTG3s
// godbolt: -lstdc++exp
// Availability: libstdc++ 12+ with -lstdc++exp (or -lstdc++_libbacktrace); libc++: not yet.
#include <print>
#include <version>
#ifdef __cpp_lib_stacktrace
#include <stacktrace>
#endif

void fail() {
#ifdef __cpp_lib_stacktrace
    // [snippet: stacktrace]
    auto trace = std::stacktrace::current();            // where am I, without a debugger
    std::println("{}", trace);                          // one frame per line: function, file, line
    std::println("caller: {}", trace[1].description()); // individual frames
    // [/snippet]
#else
    std::println("stacktrace not available in this standard library");
#endif
}

int main() { fail(); }
