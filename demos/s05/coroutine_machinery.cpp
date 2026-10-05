// Demo: what the compiler generates for a coroutine (C++20)
// Session: s05
// Compiler Explorer: https://godbolt.org/z/dc9q3hf8b
// A minimal hand-written generator, so std::generator is not magic. Not for production use:
// no allocator support, no exceptions, no nested yields. Use std::generator (C++23) instead.
#include <coroutine>
#include <exception>
#include <print>
#include <utility>

// [snippet: promise]
template <typename T>
class Gen {
public:
    // The compiler looks for this nested type by name. Every customization point lives here.
    struct promise_type {
        T value{};
        Gen get_return_object() { return Gen{Handle::from_promise(*this)}; }
        std::suspend_always initial_suspend() noexcept { return {}; }   // lazy: do nothing until first resume
        std::suspend_always final_suspend() noexcept { return {}; }     // keep the frame so done() can be read
        std::suspend_always yield_value(T v) noexcept { value = std::move(v); return {}; }   // co_yield v
        void return_void() noexcept {}                                  // falling off the end
        void unhandled_exception() { std::terminate(); }
    };
    using Handle = std::coroutine_handle<promise_type>;
// [/snippet]

// [snippet: handle]
    // The generator object owns the coroutine frame through its handle.
    explicit Gen(Handle h) : h_(h) {}
    Gen(Gen&& o) noexcept : h_(std::exchange(o.h_, {})) {}
    Gen(const Gen&) = delete;
    Gen& operator=(const Gen&) = delete;
    Gen& operator=(Gen&&) = delete;
    ~Gen() { if (h_) h_.destroy(); }            // destroying a suspended coroutine: locals' destructors run, nothing after the co_yield does

    bool next() {                               // resume until the next co_yield or the end
        h_.resume();
        return !h_.done();
    }
    const T& value() const { return h_.promise().value; }

private:
    Handle h_;
};
// [/snippet]

// [snippet: use]
Gen<int> counter(int n) {                       // a coroutine because its body contains co_yield
    for (int i = 0; i < n; ++i) co_yield i;     // ≈ co_await promise.yield_value(i)
}                                               // ≈ promise.return_void(); co_await promise.final_suspend()

int main() {
    auto g = counter(3);                        // frame allocated, promise constructed, suspended at initial_suspend
    while (g.next()) std::print("{} ", g.value());
    std::println("");
}                                               // ~Gen: handle.destroy() frees the frame
// [/snippet]
