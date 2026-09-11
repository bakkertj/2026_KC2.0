// Demo: std::jthread (C++20)
// Session: s05
// Compiler Explorer: <add short link>
#include <atomic>
#include <chrono>
#include <print>
#include <stop_token>
#include <thread>

using namespace std::chrono_literals;

// [snippet: before]
// C++11: forget join() and the destructor terminates. Stopping is a flag you invent.
void worker11(std::atomic<bool>& stop) { while (!stop) std::this_thread::sleep_for(1ms); }
void run11() {
    std::atomic<bool> stop{false};
    std::thread t(worker11, std::ref(stop));
    std::this_thread::sleep_for(5ms);
    stop = true;
    t.join();                                   // mandatory; an exception before it terminates
}
// [/snippet]

// [snippet: after]
// C++20: joins in its destructor, requesting a stop first; the token arrives by itself
void run20() {
    std::jthread t([](std::stop_token st) {
        while (!st.stop_requested()) std::this_thread::sleep_for(1ms);
    });
    std::this_thread::sleep_for(5ms);
}                                               // ~jthread: request_stop(), then join()
// [/snippet]

#include <atomic>
int main() { run11(); run20(); std::println("both done"); }
