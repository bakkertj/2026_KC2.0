// Demo: atomic wait/notify and atomic_ref (C++20)
// Session: s05
// Compiler Explorer: https://godbolt.org/z/Wx1Toseo9
#include <atomic>
#include <print>
#include <thread>

struct Legacy { int counter = 0; };                 // a struct you do not own and cannot make atomic

int main() {
    // [snippet: wait]
    std::atomic<int> state{0};
    std::jthread t([&] {
        state.wait(0);                              // block until state != 0 (a futex, not a spin)
        std::println("woke with state {}", state.load());
    });
    state.store(1);
    state.notify_one();                             // wake one waiter (notify_all for everyone)
    // C++11 needed a mutex + condition_variable for this. C++20: the atomic IS the wait point.
    // [/snippet]
    t.join();

#ifdef __cpp_lib_atomic_ref                         // libc++ 18 lacks atomic_ref (libc++ 19)
    // [snippet: atomic_ref]
    Legacy obj;
    std::atomic_ref<int> ref(obj.counter);          // atomic operations on a plain int, in place
    std::jthread a([&] { for (int i = 0; i < 1000; ++i) ref.fetch_add(1); });
    std::jthread b([&] { for (int i = 0; i < 1000; ++i) ref.fetch_add(1); });
    a.join(); b.join();
    std::println("{}", ref.load());                 // 2000; the object never changed type
    // Rule: while any atomic_ref to an object exists, touch it ONLY through atomic_refs
    // (so obj.counter is read through ref here, not directly).
    // [/snippet]
#else
    std::println("atomic_ref: not available on this standard library");
#endif
}
