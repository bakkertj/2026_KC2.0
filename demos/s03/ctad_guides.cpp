// Demo: CTAD and deduction guides (C++17), aggregate CTAD (C++20)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/fPxbhsvTT
#include <print>
#include <string>
#include <variant>

// [snippet: guides]
template <class... Fs> struct overloaded : Fs... { using Fs::operator()...; };
// C++17 needed this deduction guide for overloaded{l1, l2} to deduce Fs...:
// template <class... Fs> overloaded(Fs...) -> overloaded<Fs...>;
// C++20 deduces it for aggregates automatically, so the guide is gone.

template <typename T>
struct Wrapper {
    T value;
    explicit Wrapper(const T& v) : value(v) {}
};
Wrapper(const char*) -> Wrapper<std::string>;   // a guide: a literal deduces std::string, not const char*
// [/snippet]

int main() {
    Wrapper w{"text"};                          // Wrapper<std::string>
    std::variant<int, std::string> v = 3;
    auto s = std::visit(overloaded{[](int i) { return std::to_string(i); }, [](const std::string& x) { return x; }}, v);
    std::println("{} {}", w.value, s);
}
