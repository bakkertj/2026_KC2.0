// Demo: auto pitfalls (C++11 calibration)
// Session: s01
// Compiler Explorer: https://godbolt.org/z/vK655KbjE
#include <cstdio>
#include <string>
#include <vector>

struct Config {
    std::string name = "cfg";
    const std::string& get() const { return name; }
};

int main() {
    Config c;
    // [snippet: drops]
    auto a = c.get();         // std::string: a COPY. auto drops the & and the const
    const auto& b = c.get();  // const std::string&: what you meant
    auto& d = c.name;         // std::string&: a mutable alias into c
    // [/snippet]
    d += "!";

    // [snippet: braces]
    auto x{42};        // int (N3922; GCC and Clang apply it back to C++11)
    auto y = {1, 2};   // initializer_list<int>, still
    std::vector<int> v(3, 7);   // three sevens
    std::vector<int> w{3, 7};   // the values 3 and 7
    // [/snippet]
    std::printf("%s %s %s %d %zu %zu %zu\n", a.c_str(), b.c_str(), d.c_str(), x, y.size(), v.size(), w.size());
}
