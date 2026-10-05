// Demo: requires expressions (C++20)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/f6zzfc56f
#include <concepts>
#include <print>
#include <string>
#include <vector>

// [snippet: requires]
template <typename T>
concept Range = requires(T& t) {
    t.begin();                                       // simple: this expression compiles
    t.end();
    typename T::value_type;                          // type: this type exists
    { t.size() } -> std::convertible_to<std::size_t>;   // compound: type satisfies a concept
    { t.empty() } noexcept -> std::same_as<bool>;    // ...and is noexcept
    requires !std::same_as<T, std::string>;          // nested: another constraint holds
};
static_assert(Range<std::vector<int>>);
static_assert(!Range<std::string>);                  // excluded by the nested requirement
static_assert(!Range<int>);
// [/snippet]

// [snippet: adhoc]
// A requires expression can also appear directly in a requires-clause (ad hoc, unnamed):
template <typename T>
    requires requires(T t) { t.size(); }
std::size_t count(const T& t) { return t.size(); }
// Prefer a named concept: it subsumes, it documents, and the error message uses its name.
// [/snippet]

int main() { std::println("{}", count(std::vector<int>{1, 2, 3})); }
