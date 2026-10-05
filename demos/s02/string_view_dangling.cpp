// Demo: the four ways a string_view dangles (C++17)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/qsc6eqsTE
#include <cstdio>
#include <string>
#include <string_view>

std::string make() { return "temporary"; }

// [snippet: dangling]
struct Config {
    std::string_view name;          // (3) stored view: safe only if the owner outlives Config
};

#ifdef SHOW_ERRORS                  // Clang 18 rejects all three below under -Werror:
std::string_view first_word() {     //   -Wreturn-stack-address, -Wdangling-gsl
    std::string s = "hello world";
    return std::string_view(s).substr(0, 5);   // (2) view of a local: dangles at return
}

void dangling() {
    std::string_view a = make();    // (1) view of a temporary: dead at the end of this line
    const char* p = std::string("x").c_str();   // (4) same bug, C++98 edition
    (void)a; (void)p;
}
#endif
// [/snippet]

// [snippet: safe]
std::string_view sensor_of(std::string_view line) {   // view in, view of the SAME buffer out: fine
    const auto comma = line.find(',');
    return line.substr(comma + 1);
}
// [/snippet]

int main() {
    std::string line = "1,rpm,3";
    auto v = sensor_of(line);        // line outlives v
    std::printf("%.*s\n", static_cast<int>(v.size()), v.data());
    // dangling();  // run under -fsanitize=address to watch (1) and (4) fail
}
