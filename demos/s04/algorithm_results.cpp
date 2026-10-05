// Demo: what constrained algorithms return (C++20)
// Session: s04
// Compiler Explorer: https://godbolt.org/z/sW8EYha61
#include <algorithm>
#include <print>
#include <vector>

std::vector<int> make() { return {3, 1, 2}; }

int main() {
    std::vector<int> v{3, 1, 4, 1, 5}, out(5);
    // [snippet: results]
    auto [mn, mx] = std::ranges::minmax(v);                 // the elements, not iterators
    auto end = std::ranges::sort(v);                        // the end iterator (rarely needed)
    auto [in, o] = std::ranges::copy(v, out.begin());       // in_out_result: where both stopped
    auto [last, fn] = std::ranges::for_each(v, [s = 0](int x) mutable { s += x; return s; });
                                                            // in_fun_result: the end iterator AND the functor, with its state

    auto d = std::ranges::find(make(), 2);                  // on a temporary: std::ranges::dangling
    // *d;                                                  // error: dangling has no operator*
    // The algorithm refuses to hand back an iterator into an object that no longer exists.
    // [/snippet]
    std::println("{} {} {} {}", mn, mx, o - out.begin(), fn(0));   // fn(0) == 14: the accumulated state
    (void)end; (void)last; (void)in; (void)d;
}
