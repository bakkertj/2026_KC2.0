// Demo: template lambdas and lambdas in unevaluated contexts (C++20)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/cv3hvjPeb
#include <cstdio>
#include <memory>
#include <print>
#include <set>
#include <span>
#include <string>
#include <vector>

int main() {
    // [snippet: template_lambda]
    // C++14 generic lambda: `auto` gives you a type you cannot name inside the body.
    auto sum14 = [](const auto& s) { double t = 0; for (auto x : s) t += x; return t; };

    // C++20 template lambda: name the parameter types, constrain them, use them in the body.
    auto sum20 = []<typename T>(std::span<const T> s) {
        T t{};                                          // T is nameable now
        for (auto x : s) t += x;
        return t;
    };
    std::vector<int> v{1, 2, 3};
    std::println("{} {}", sum14(v), sum20(std::span<const int>{v}));
    // [/snippet]

    // [snippet: unevaluated]
    // C++20: a lambda may appear in decltype, so a stateless comparator needs no named type.
    auto less_by_size = [](const std::string& a, const std::string& b) { return a.size() < b.size(); };
    std::set<std::string, decltype(less_by_size)> by_size;   // default-constructible closure (C++20)
    by_size.insert("ccc"); by_size.insert("a");
    std::unique_ptr<FILE, decltype([](FILE* f) { if (f) std::fclose(f); })> file{std::fopen("/dev/null", "r")};
    std::println("{} {}", *by_size.begin(), file != nullptr);
    // [/snippet]
}
