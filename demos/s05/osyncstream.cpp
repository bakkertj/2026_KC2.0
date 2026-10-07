// Demo: std::osyncstream (C++20)
// Session: s05
// Compiler Explorer: https://godbolt.org/z/hjrfYe3Ef
#include <iostream>
#include <syncstream>
#include <thread>
#include <vector>

int main() {
    // [snippet: sync]
    std::vector<std::jthread> threads;
    for (int i = 0; i < 4; ++i)
        threads.emplace_back([i] {
            std::osyncstream(std::cout) << "thread " << i << " says hello" << '\n';   // one line, atomically
        });
    // Without osyncstream, the four << calls from each thread interleave character by character.
    // Each osyncstream buffers and emits on destruction (or on emit()).
    // [/snippet]
}
