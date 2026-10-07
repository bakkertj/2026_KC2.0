// Demo: a string_view is not a C string (C++17)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/5zMh8eEKr
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>

// [snippet: nul]
double bad(std::string_view field) {
    return std::strtod(field.data(), nullptr);   // WRONG: reads past field.size() to find a NUL
}

double ok(std::string_view field) {
    return std::strtod(std::string(field).c_str(), nullptr);   // copy when a C API needs a NUL
}
// [/snippet]

int main() {
    std::string line = "41.25,99";
    std::string_view field = std::string_view(line).substr(0, 5);   // "41.25", no NUL after it
    std::printf("%f %f\n", bad(field), ok(field));                 // bad() parses "41.25,99" -> 41.25 by luck
}
