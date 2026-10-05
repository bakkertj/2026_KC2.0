// Demo: the four ways to constrain a template (C++20)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/hdr6Gzx91
#include <concepts>
#include <print>
#include <string>
#include <string_view>

// [snippet: concept]
template <typename T>
concept StringLike = std::convertible_to<T, std::string_view>;   // a named predicate on types
// [/snippet]

// [snippet: four]
template <typename T>
    requires StringLike<T>                       // 1. requires-clause after the template head
std::size_t len1(const T& s) { return std::string_view{s}.size(); }

template <typename T>
std::size_t len2(const T& s) requires StringLike<T> { return std::string_view{s}.size(); }   // 2. trailing

template <StringLike T>                          // 3. constrained template parameter
std::size_t len3(const T& s) { return std::string_view{s}.size(); }

std::size_t len4(const StringLike auto& s) { return std::string_view{s}.size(); }   // 4. abbreviated
// [/snippet]

// [snippet: sfinae]
// The same constraint with C++11 enable_if (string_view itself is C++17). Read it aloud to a colleague.
template <typename T, typename std::enable_if<std::is_convertible<T, std::string_view>::value, int>::type = 0>
std::size_t len_old(const T& s) { return std::string_view{s}.size(); }
// [/snippet]

int main() {
    std::string s = "abc";
    std::println("{} {} {} {} {}", len1(s), len2("ab"), len3(std::string_view{"a"}), len4(s), len_old(s));
#ifdef SHOW_ERRORS
    len4(42);      // error: constraints not satisfied ... 'int' does not satisfy 'StringLike'
    len_old(42);   // error: no matching function for call to 'len_old' ... candidate template ignored ...
#endif
}
