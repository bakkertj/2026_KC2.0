#pragma once
#include <chrono>
#include <cstddef>
#include <mutex>
#include <optional>
#include <queue>
#include <semaphore>
#include <stop_token>

namespace telemetry {

// A bounded, thread-safe queue for one producer and one consumer.
//
// Two counting semaphores do the blocking: `slots` counts free capacity (the
// producer waits on it), `items` counts queued elements (the consumer waits on
// it). A mutex protects the std::queue itself. C++20's semaphores make this a
// few lines; C++11 needed a condition_variable and a hand-written predicate.
//
// Shutdown is cooperative: close() marks the end of input, and pop() returns
// nullopt once the queue is both closed and drained. A stop_token lets a
// consumer give up early without waiting for close().
template <typename T, std::ptrdiff_t Capacity = 256>   // ptrdiff_t: counting_semaphore counts are signed
class BoundedQueue {
public:
    // Blocks while the queue is full. Returns false if the queue was closed.
    bool push(T item) {
        slots_.acquire();
        {
            std::lock_guard lock(mutex_);
            if (closed_) {
                slots_.release();
                return false;
            }
            queue_.push(std::move(item));
        }
        items_.release();
        return true;
    }

    // Blocks until an item is available, the queue is closed and empty, or a
    // stop is requested. Only the first case yields a value.
    std::optional<T> pop(std::stop_token stop = {}) {
        while (true) {
            // Poll with a timeout so a stop request is noticed even while blocked.
            if (!items_.try_acquire_for(std::chrono::milliseconds{10})) {
                if (stop.stop_requested()) return std::nullopt;
                std::lock_guard lock(mutex_);
                if (closed_ && queue_.empty()) return std::nullopt;
                continue;
            }
            std::lock_guard lock(mutex_);
            if (queue_.empty()) return std::nullopt;   // closed: the wake-up came from close()
            T item = std::move(queue_.front());
            queue_.pop();
            slots_.release();
            return item;
        }
    }

    // No more items will be pushed. Wakes a waiting consumer.
    void close() {
        {
            std::lock_guard lock(mutex_);
            closed_ = true;
        }
        items_.release();   // one extra count so a blocked pop() wakes and sees closed_
    }

private:
    std::mutex mutex_;
    std::queue<T> queue_;
    std::counting_semaphore<Capacity> slots_{Capacity};
    std::counting_semaphore<Capacity + 1> items_{0};
    bool closed_ = false;
};

}  // namespace telemetry
