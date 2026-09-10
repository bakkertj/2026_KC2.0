// Demo: std::print and std::println (C++23)
// Session: s02
// Compiler Explorer: <add short link>
#include <cstdio>
#include <print>
#include <string>

// [snippet: before]
// C++11: printf casts and format letters; iostream manipulators that stick
void report_cpp11(std::FILE* out, const std::string& name, std::size_t n, double mean) {
    std::fprintf(out, "%-16s n=%lu mean=%.3f\n", name.c_str(), static_cast<unsigned long>(n), mean);
}
// [/snippet]

// [snippet: after]
// C++23: format-string safety, printf ergonomics, and a FILE* overload
void report(std::FILE* out, const std::string& name, std::size_t n, double mean) {
    std::println(out, "{:<16} n={} mean={:.3f}", name, n, mean);
}
// [/snippet]

int main() {
    report_cpp11(stdout, "rpm", 5, 4811.0);
    report(stdout, "rpm", 5, 4811.0);
    std::println("{} and {}", 1, "done");       // stdout, newline appended
    std::print("no newline; ");
    std::println(stderr, "to stderr");
}
