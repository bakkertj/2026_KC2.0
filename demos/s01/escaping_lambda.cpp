// Demo: lambdas that outlive their captures (C++11 calibration)
// Session: s01
// Compiler Explorer: https://godbolt.org/z/e7hns9dWW
#include <cstdio>
#include <functional>
#include <string>

// [snippet: escape]
std::function<void()> make_logger_bad(const std::string& prefix) {
    std::string tag = "[" + prefix + "] ";
    return [&] { std::printf("%s\n", tag.c_str()); };   // by reference: dangles on return
}

std::function<void()> make_logger(const std::string& prefix) {
    std::string tag = "[" + prefix + "] ";
    return [tag] { std::printf("%s\n", tag.c_str()); };   // a copy: safe
}
// [/snippet]

int main() {
    auto log = make_logger("ok");
    log();
    // make_logger_bad("boom")();   // undefined behavior; try it under -fsanitize=address
}
