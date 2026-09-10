// Demo: std::variant and std::visit (C++17)
// Session: s02
// Compiler Explorer: <add short link>
#include <cstdio>
#include <string>
#include <variant>

// [snippet: before]
// C++11: a tag plus a union; the compiler enforces nothing
struct ValueCpp11 {
    enum Kind { Int, Double, Text } kind;
    union { int i; double d; };
    std::string text;      // cannot live in the union, so it sits beside it
};
// [/snippet]

// [snippet: after]
// C++17: exactly one of these, and the compiler knows which
using Value = std::variant<int, double, std::string>;

void inspect(const Value& v) {
    if (std::holds_alternative<int>(v)) std::printf("int %d\n", std::get<int>(v));
    if (auto* d = std::get_if<double>(&v)) std::printf("double %f\n", *d);   // nullptr if not a double
    std::printf("index %zu\n", v.index());                                   // 0, 1, or 2
    // std::get<int>(v) on a string throws std::bad_variant_access
}
// [/snippet]

// [snippet: visitor]
template <class... Fs> struct overloaded : Fs... { using Fs::operator()...; };   // one object, all the overloads

std::string describe(const Value& v) {
    return std::visit(overloaded{
        [](int i)                { return "int " + std::to_string(i); },
        [](double d)             { return "double " + std::to_string(d); },
        [](const std::string& s) { return "text " + s; },
    }, v);
}
// Add a fourth alternative to Value and this stops compiling until you add a fourth lambda.
// [/snippet]

int main() {
    Value a = 1, b = 2.5, c = std::string("x");
    inspect(b);
    std::printf("%s %s %s\n", describe(a).c_str(), describe(b).c_str(), describe(c).c_str());
}
