// Demo: stop_token, stop_source, stop_callback (C++20)
// Session: s05
// Compiler Explorer: https://godbolt.org/z/dv5YErjs3
#include <chrono>
#include <print>
#include <stop_token>
#include <thread>

using namespace std::chrono_literals;

// [snippet: stop]
void pipeline(std::stop_token outer) {
    // A producer with its own token (from its jthread)...
    std::jthread producer([](std::stop_token st) {
        int n = 0;
        while (!st.stop_requested()) { ++n; std::this_thread::sleep_for(1ms); }
        std::println("producer stopped after {} iterations", n);
    });
    // ...and a callback that forwards the OUTER stop to it. Runs on whichever thread requests the stop.
    std::stop_callback forward(outer, [&] { producer.request_stop(); });

    while (!outer.stop_requested()) std::this_thread::sleep_for(1ms);
}   // producer joins here

int main() {
    std::stop_source source;                        // the handle that requests; tokens are its views
    std::jthread t([&] { pipeline(source.get_token()); });
    std::this_thread::sleep_for(20ms);
    source.request_stop();                          // one call unwinds both threads
}
// Why not a bool flag: a stop_token composes (callbacks, forwarding), is thread-safe by
// specification, and every std::jthread and condition_variable_any already understands it.
// [/snippet]
