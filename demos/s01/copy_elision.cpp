// Demo: guaranteed copy elision (C++17)
// Session: s01
// Compiler Explorer: https://godbolt.org/z/fhE6jPGh6
#include <cstdio>
#include <mutex>

// [snippet: factory]
struct Pinned {                       // not copyable, not movable
    std::mutex m;
    int id;
    explicit Pinned(int i) : id(i) {}
    Pinned(const Pinned&) = delete;
    Pinned& operator=(const Pinned&) = delete;
};

Pinned make_pinned(int id) {
    return Pinned{id};                // C++17: legal. C++11/14: error.
}

int main() {
    Pinned p = make_pinned(7);        // no copy, no move: p is initialized in place
    std::printf("%d\n", p.id);
}
// [/snippet]
