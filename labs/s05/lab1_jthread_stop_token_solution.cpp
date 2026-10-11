// Lab 5.1 solution: jthread, stop_token, scoped_lock, semaphores
// Session: s05
// Compiler Explorer: https://godbolt.org/z/MTjWGoscr
#include <chrono>
#include <cstdio>
#include <mutex>
#include <semaphore>
#include <stop_token>
#include <thread>
using namespace std::chrono_literals;

// TODO 1 done: the stop request travels with the thread; no global flag, no explicit join.
void sampler(std::stop_token stop, int& samples) {
    while (!stop.stop_requested()) {
        ++samples;
        std::this_thread::sleep_for(1ms);
    }
}

// TODO 2 done: scoped_lock acquires both without deadlock, in any calling order.
struct Account { std::mutex m; int balance = 100; };

void transfer(Account& from, Account& to, int amount) {
    std::scoped_lock lock(from.m, to.m);
    from.balance -= amount;
    to.balance += amount;
}

// TODO 3 done: a semaphore blocks instead of spinning, and the ordering is built in.
std::binary_semaphore ready{0};

int main() {
    int samples = 0;
    {
        std::jthread t(sampler, std::ref(samples));     // the stop_token is passed automatically
        std::this_thread::sleep_for(20ms);
    }                                                   // ~jthread: request_stop(), then join()
    std::printf("sampled: %s\n", samples > 0 ? "yes" : "no");

    Account a, b;
    {
        std::jthread t1(transfer, std::ref(a), std::ref(b), 30);
        std::jthread t2(transfer, std::ref(b), std::ref(a), 10);
    }
    std::printf("balances: %d %d (sum %d)\n", a.balance, b.balance, a.balance + b.balance);

    std::jthread worker([] {
        std::this_thread::sleep_for(5ms);      // "initialization"
        ready.release();
    });
    ready.acquire();
    std::printf("worker ready\n");
}
