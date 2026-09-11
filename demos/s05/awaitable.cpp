// Demo: co_await and a hand-written awaitable (C++20)
// Session: s05
// Compiler Explorer: <add short link>
// A task type that can suspend waiting for a value another thread supplies. Educational only.
#include <coroutine>
#include <exception>
#include <optional>
#include <print>
#include <semaphore>
#include <thread>

// [snippet: task]
// The simplest coroutine return type: fire and forget, runs eagerly, no result.
struct Task {
    struct promise_type {
        Task get_return_object() { return {}; }
        std::suspend_never initial_suspend() noexcept { return {}; }   // eager: runs until the first real suspension
        std::suspend_never final_suspend() noexcept { return {}; }     // frame freed when the body finishes
        void return_void() noexcept {}
        void unhandled_exception() { std::terminate(); }
    };
};
// [/snippet]

// [snippet: awaitable]
// co_await x calls: await_ready (skip?), await_suspend (park the handle), await_resume (the value)
std::jthread producer;                          // a real library would own this properly

class ValueFromThread {
public:
    bool await_ready() const noexcept { return value_.has_value(); }     // already there: do not suspend
    void await_suspend(std::coroutine_handle<> h) {
        producer = std::jthread([this, h] {                              // another thread produces the value...
            value_ = 42;
            h.resume();                                                  // ...and resumes the coroutine, on that thread
        });
    }
    int await_resume() const noexcept { return *value_; }
private:
    std::optional<int> value_;
};
// [/snippet]

// [snippet: use]
std::binary_semaphore done{0};

Task consumer() {
    std::println("before await on {}", std::this_thread::get_id());
    const int v = co_await ValueFromThread{};   // suspends here; the rest runs on the producing thread
    std::println("after await: {} on {}", v, std::this_thread::get_id());
    done.release();
}
// [/snippet]

int main() {
    consumer();                                 // returns at the first suspension
    done.acquire();                             // the resumed half signals when it is finished
    producer.join();
}
