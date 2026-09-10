// Demo: generic lambdas and init-capture (C++14)
// Session: s01
// Compiler Explorer: <add short link>
// Slide: slides/01-everyday-language.md
#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

// [snippet: generic]
// auto parameters: one lambda, any type with .size()
auto by_size = [](const auto& a, const auto& b) { return a.size() < b.size(); };
// [/snippet]

// [snippet: init_capture]
// init-capture moves the unique_ptr into the closure
auto make_printer(std::unique_ptr<std::string> owned) {
    return [s = std::move(owned)] { std::printf("%s\n", s->c_str()); };
}
// [/snippet]

int main() {
    std::vector<std::string> words{"delta", "a", "bbb", "cc"};
    std::sort(words.begin(), words.end(), by_size);
    for (const auto& w : words) std::printf("%s ", w.c_str());
    std::printf("\n");

    auto print = make_printer(std::make_unique<std::string>("captured by move"));
    print();
}
