// Demo: fold expressions (C++17)
// Session: s03
// Compiler Explorer: https://godbolt.org/z/jz9oqPqdW
#include <print>
#include <string>

// [snippet: before]
// C++11: recursion with a base case, for every variadic function
int sum11() { return 0; }
template <typename T, typename... Rest>
int sum11(T first, Rest... rest) { return first + sum11(rest...); }
// [/snippet]

// [snippet: after]
// C++17: one expression
template <typename... Ts>
int sum(Ts... vs) { return (vs + ...); }                       // unary right fold: v1 + (v2 + (v3))

template <typename... Ts>
bool all_positive(Ts... vs) { return ((vs > 0) && ...); }      // any operator, including &&

template <typename... Ts>
std::string join(Ts... vs) {
    std::string out;
    ((out += std::to_string(vs) + ","), ...);                  // the comma fold: do this for each
    return out;
}

template <typename... Ts>
int sum_from_100(Ts... vs) { return (100 + ... + vs); }        // binary left fold with an init value
// [/snippet]

// [snippet: forms]
// ( pack op ... )          unary right   v1 op (v2 op (v3 ...))
// ( ... op pack )          unary left    ((v1 op v2) op v3) ...
// ( pack op ... op init )  binary right
// ( init op ... op pack )  binary left
// Empty packs: && gives true, || gives false, comma gives void; other operators need the binary form.
// [/snippet]

int main() { std::println("{} {} {} {} {}", sum11(1, 2, 3), sum(1, 2, 3), all_positive(1, 2), join(1, 2), sum_from_100(1, 2)); }
