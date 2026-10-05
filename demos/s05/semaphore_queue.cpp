// Demo: a bounded queue, C++11 vs C++20 (counting_semaphore)
// Session: s05
// Compiler Explorer: <add short link>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <print>
#include <queue>
#include <semaphore>
#include <thread>
#include <utility>

// [snippet: before]
// C++11: a mutex, two condition_variables, two predicates by hand
template <typename T, std::size_t N>
class Queue11 {
public:
    void push(T v) {
        std::unique_lock<std::mutex> lock(m_);   // C++11: no CTAD yet
        not_full_.wait(lock, [&] { return q_.size() < N; });
        q_.push(std::move(v));
        not_empty_.notify_one();
    }
    T pop() {
        std::unique_lock<std::mutex> lock(m_);
        not_empty_.wait(lock, [&] { return !q_.empty(); });
        T v = std::move(q_.front()); q_.pop();
        not_full_.notify_one();
        return v;
    }
private:
    std::mutex m_;
    std::condition_variable not_full_, not_empty_;   // two, and a predicate each
    std::queue<T> q_;
};
// [/snippet]

// [snippet: after]
// C++20: semaphores do the blocking; the mutex only guards the queue
template <typename T, std::ptrdiff_t N>
class Queue20 {
public:
    void push(T v) {
        slots_.acquire();                       // wait for a free slot
        { std::lock_guard lock(m_); q_.push(std::move(v)); }
        items_.release();                       // announce an item
    }
    T pop() {
        items_.acquire();                       // wait for an item
        T v;
        { std::lock_guard lock(m_); v = std::move(q_.front()); q_.pop(); }
        slots_.release();                       // free a slot
        return v;
    }
private:
    std::mutex m_;
    std::queue<T> q_;
    std::counting_semaphore<N> slots_{N}, items_{0};   // counts are ptrdiff_t
};
// [/snippet]

int main() {
    Queue20<int, 4> q;
    std::jthread p([&] { for (int i = 0; i < 10; ++i) q.push(i); });
    int sum = 0;
    for (int i = 0; i < 10; ++i) sum += q.pop();
    std::println("{}", sum);
}
