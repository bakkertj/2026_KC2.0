// Lab 5.1: jthread, stop_token, scoped_lock, semaphores
// Session: s05
// Compiler Explorer: https://godbolt.org/z/b48KPoeYf
// Ten minutes, three TODOs in order. Build and run after each one; the output must not
// change. (On Compiler Explorer the execution pane runs real threads.)
// Finished version: lab1_jthread_stop_token_solution.cpp
#include <atomic>
#include <chrono>
#include <cstdio>
#include <mutex>
#include <semaphore>
#include <stop_token>
#include <thread>
using namespace std::chrono_literals;

// ---- TODO 1: thread + flag + join -> jthread + stop_token --------------------------
// The C++11 shape: an atomic<bool> the worker polls, set by the owner, then an explicit
// join that is easy to forget (and that std::terminate()s you if you do). Change the
// worker to take a std::stop_token as its first parameter, poll stop_requested(), and
// make it a std::jthread. Delete the flag and the join: the destructor requests stop and
// joins.
std::atomic<bool> g_running{true};

void sampler(int& samples) {
    while (g_running) {
        ++samples;
        std::this_thread::sleep_for(1ms);
    }
}

// ---- TODO 2: two mutexes locked by hand -> std::scoped_lock -----------------------
// transfer() locks a then b. Called from two threads in opposite orders, that deadlocks
// (the classic). std::lock(a, b) plus two adopt_lock guards is the C++11 fix; the C++17
// one is a single line:  std::scoped_lock lock(a.m, b.m);
struct Account { std::mutex m; int balance = 100; };

void transfer(Account& from, Account& to, int amount) {
    std::lock_guard<std::mutex> l1(from.m);
    std::lock_guard<std::mutex> l2(to.m);     // deadlock-prone: the other thread may hold `to` and want `from`
    from.balance -= amount;
    to.balance += amount;
}

// ---- TODO 3: "wait until the worker is ready" with a spin -> binary_semaphore -------
// The owner spins on an atomic until the worker signals readiness: it burns a core, and
// it is easy to get the memory order wrong. Replace g_ready with
//     std::binary_semaphore ready{0};
// the worker calls ready.release(); the owner calls ready.acquire().
std::atomic<bool> g_ready{false};

int main() {
    // TODO 1
    int samples = 0;
    std::thread t(sampler, std::ref(samples));
    std::this_thread::sleep_for(20ms);
    g_running = false;
    t.join();
    std::printf("sampled: %s\n", samples > 0 ? "yes" : "no");

    // TODO 2: the two transfers run one after the other here, because concurrently they
    //         can deadlock. Once transfer() uses scoped_lock, start t2 BEFORE joining t1
    //         (move the `t1.join()`): it cannot deadlock, whichever order they run.
    Account a, b;
    std::thread t1(transfer, std::ref(a), std::ref(b), 30);
    t1.join();
    std::thread t2(transfer, std::ref(b), std::ref(a), 10);
    t2.join();
    std::printf("balances: %d %d (sum %d)\n", a.balance, b.balance, a.balance + b.balance);

    // TODO 3
    std::thread worker([] {
        std::this_thread::sleep_for(5ms);      // "initialization"
        g_ready = true;
    });
    while (!g_ready) { /* spin */ }
    std::printf("worker ready\n");
    worker.join();
}
