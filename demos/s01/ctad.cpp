// Demo: class template argument deduction (C++17)
// Session: s01
// Compiler Explorer: <add short link>
#include <cstdio>
#include <mutex>
#include <utility>
#include <vector>

std::mutex m;

int main() {
    // [snippet: ctad]
    std::pair p{1, 2.5};              // pair<int, double>; no more make_pair
    std::lock_guard lk{m};            // lock_guard<std::mutex>: the killer use
    std::vector v{1, 2, 3};           // vector<int>, three elements

    std::vector<int> a(3, 7);         // three sevens
    std::vector b{3, 7};              // TRAP: two elements, not three sevens
    // [/snippet]
    std::printf("%d %.1f %zu %zu %zu\n", p.first, p.second, v.size(), a.size(), b.size());
}
