// Demo: structured bindings and if-with-initializer (C++17)
// Session: s01
// Compiler Explorer: https://godbolt.org/z/Ysbq9zd6v
// Slide: slides/01-everyday-language.md
#include <cstdio>
#include <map>
#include <string>

std::map<std::string, int> counts;

// [snippet: before]
// C++11: insert returns a pair; name both halves
void record_cpp11(const std::string& key) {
    std::pair<std::map<std::string, int>::iterator, bool> r =
        counts.insert({key, 1});
    if (!r.second) {
        ++r.first->second;
    }
}
// [/snippet]

// [snippet: after]
// C++17: bindings name the halves; scoped to the if
void record_cpp17(const std::string& key) {
    if (auto [it, inserted] = counts.insert({key, 1});
        !inserted) {
        ++it->second;
    }
}
// [/snippet]

int main() {
    record_cpp11("a");
    record_cpp17("a");
    record_cpp17("b");
    for (const auto& [key, n] : counts) {
        std::printf("%s=%d\n", key.c_str(), n);
    }
}
