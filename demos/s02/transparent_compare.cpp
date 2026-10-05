// Demo: transparent comparators and heterogeneous lookup (C++14 map, C++20 unordered_map)
// Session: s02
// Compiler Explorer: https://godbolt.org/z/14xnosoWv
#include <cstdio>
#include <map>
#include <string>
#include <string_view>

// [snippet: transparent]
std::map<std::string, int> plain;
std::map<std::string, int, std::less<>> transparent;   // std::less<> compares any two comparable types

void lookups(std::string_view key) {
    plain.find(std::string(key));      // must build a std::string to search: an allocation per lookup (past SSO)
    transparent.find(key);             // compares string_view to string directly: no allocation
}
// [/snippet]

int main() {
    transparent["rpm"] = 1;
    lookups("rpm");
    std::printf("%d\n", transparent.find("rpm")->second);
}
