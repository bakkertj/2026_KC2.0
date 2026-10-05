// Demo: subsumption, how the more constrained overload wins (C++20)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/ajf3YhKx5
#include <concepts>
#include <print>
#include <string>
#include <string_view>
#include <vector>

// [snippet: subsumption]
template <typename T>
concept HasSize = requires(const T& t) { t.size(); };

template <typename T>
concept Container = HasSize<T> && requires(const T& t) { t.begin(); t.end(); };   // Container subsumes HasSize

const char* describe(const HasSize auto&)   { return "has size"; }
const char* describe(const Container auto&) { return "container"; }    // more constrained: wins when both match

// std::string satisfies both; the compiler picks "container" because Container = HasSize && more.
// It can only see that because Container is written as a conjunction of NAMED concepts.
// [/snippet]

// [snippet: no_subsumption]
template <typename T>
concept Container2 = requires(const T& t) { t.size(); t.begin(); t.end(); };   // one ad hoc block

const char* describe2(const HasSize auto&)    { return "has size"; }
const char* describe2(const Container2 auto&) { return "container"; }
// describe2(std::string{});   // error: ambiguous. Container2 does not subsume HasSize: two
//                             // requires-expressions are never compared for subsumption.
// [/snippet]

struct Sized { std::size_t size() const { return 0; } };

int main() { std::println("{} {}", describe(std::string{}), describe(Sized{})); }
