// Demo: std::latch and std::barrier (C++20)
// Session: s05
// Compiler Explorer: https://godbolt.org/z/xd6qxYPY8
#include <barrier>
#include <latch>
#include <print>
#include <thread>
#include <vector>

int main() {
    // [snippet: latch]
    // latch: a one-shot countdown. "Start all workers, then wait until every one has checked in."
    constexpr int kWorkers = 4;
    std::latch ready(kWorkers);
    std::vector<std::jthread> workers;
    for (int i = 0; i < kWorkers; ++i)
        workers.emplace_back([&, i] { /* init */ ready.count_down(); std::println("worker {} ready", i); });
    ready.wait();                                   // blocks until the count reaches zero; cannot be reset
    std::println("all workers ready");
    workers.clear();
    // [/snippet]

    // [snippet: barrier]
    // barrier: reusable, with a completion function that runs once per phase when everyone arrives.
    int phase = 0;
    std::barrier sync(kWorkers, [&]() noexcept { std::println("phase {} complete", phase++); });
    std::vector<std::jthread> steppers;
    for (int i = 0; i < kWorkers; ++i)
        steppers.emplace_back([&] {
            for (int step = 0; step < 3; ++step) {
                /* compute this worker's slice of the step */
                sync.arrive_and_wait();             // no worker starts step+1 until all finish step
            }
        });
    // [/snippet]
}
