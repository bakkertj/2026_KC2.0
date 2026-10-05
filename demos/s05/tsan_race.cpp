// Demo: a data race ThreadSanitizer catches (build with -DCOURSE_SANITIZE=thread)
// Session: s05
// Compiler Explorer: <add short link>
// godbolt: -fsanitize=thread -g
#include <print>
#include <thread>

int main() {
    // [snippet: race]
    int shared = 0;                                  // plain int, written by one thread, read by another
    std::jthread writer([&] { for (int i = 0; i < 1000; ++i) ++shared; });
#ifdef SHOW_ERRORS
    std::println("{}", shared);                      // TSan: "WARNING: ThreadSanitizer: data race" with both stacks
#endif
    writer.join();                                   // join() is a synchronization point:
    std::println("{}", shared);                      // ...this read is ordered after every write. No race.
    // The exercise's lines_read counter is exactly this: written by the producer, read after join().
    // [/snippet]
}
