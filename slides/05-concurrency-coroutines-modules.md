---
marp: true
theme: course
paginate: true
footer: 'The Evolution of C++ | Session 5: Concurrency, Coroutines, Modules, and Adoption'
---

<!-- _class: lead -->
<!-- _paginate: false -->

# The Evolution of C++
## Session 5: Concurrency, Coroutines, Modules, and Adoption

Three features that change how programs are built, and a plan for your own code

<!--
Notes: Last session. Three big topics, each of which changes something other than syntax:
concurrency changes lifetime and cancellation, coroutines change control flow, modules change the
build. Then the part that matters most: what each attendee does on Monday. The exercise adds two
threads and a coroutine to the program and it still prints the Session 1 report byte for byte.
-->

---

## Agenda

1. Recap and framing (10 min)
2. Concurrency, C++14 to C++23 (30 min)
3. Coroutines and `std::generator` (25 min)
4. Modules (20 min)
5. Deprecations and removals (5 min)
6. The adoption roadmap workshop (20 min)
7. C++26 and close (10 min)

Every demo opens in Compiler Explorer, preconfigured for GCC 14: `handouts/compiler-explorer-links.md`

<!--
Notes: Exercise README: exercises/s05-concurrency/README.md. The starter is the Session 4
solution. Tasks 1 to 3 (the bounded queue, the jthread pipeline, TSan as the check) are the core
of the at-home work; the in-class exercise today is the roadmap workshop (task 8), the only
exercise in the course without a solution.
-->

---

## Session 4 recap

The solution: projections everywhere, `top_n_by_value` as `filter | to<vector>`, `split` as a pipeline, `chunk_by` in the report, `records()` returning an `input_range`.

Where people got stuck:

- `views::split("")` yields zero pieces where the old loop yielded one; the tests decided the contract
- A `const filter_view` is not iterable: `begin()` on `filter_view` is non-`const` because it caches
- `views::enumerate` on libc++ 18: not there; `zip(iota(0), r)` is the portable spelling

<!--
Notes: Two of the three are the same lesson as today: the standard library is ahead of at least
one of your compilers, and a feature-test macro is how you write code that builds on both. The
support matrix handout is the reference; today adds a row for nearly everything we show.
-->

---

## Why these three are last

<p class="problem">Everything before today changed how a line of code is written. Today changes what a program is.</p>

| Feature | What it changes | Cost of adopting |
|---|---|---|
| **Concurrency** (`jthread`, `stop_token`, semaphores, `latch`, `barrier`) | Ownership and cancellation of threads | A design review, not a find-and-replace |
| **Coroutines** (`co_yield`, `std::generator`) | Control flow: a function that can pause | A new mental model; a library to hold it |
| **Modules** (`export module`, `import`) | Program structure and the build | Your build system, your CI, your toolchain floor |

And still: the exercise adds a producer thread, a consumer thread, and a coroutine to the telemetry program. The report is byte-identical to Session 1.

<!--
Notes: Set expectations. Nobody leaves today a concurrency expert; the point is to know what the
standard now provides so the next design does not reinvent a semaphore or a stop flag. Coroutines
and modules are "know what they are, adopt with care".
-->

---

## The course so far, in one program

<div class="evo">
<div class="step"><b>Session 1</b> raw <code>new</code>, <code>typedef</code>, iterator loops, sentinel returns &rarr; <code>unique_ptr</code>, <code>auto</code>, range-for, <code>optional</code>, <code>[[nodiscard]]</code>, <code>&lt;=&gt;</code></div>
<div class="step"><b>Session 2</b> <code>bool</code> + out-parameter, <code>const string&amp;</code>, <code>printf</code> &rarr; <code>expected</code>, <code>string_view</code>, <code>span</code>, <code>variant</code>, <code>format</code>, <code>print</code></div>
<div class="step"><b>Session 3</b> runtime CRC table, SFINAE, <code>enable_if</code> &rarr; <code>constexpr</code> table, <code>consteval</code> validator, concepts, deducing <code>this</code></div>
<div class="step"><b>Session 4</b> iterator pairs, comparator lambdas, temporary vectors &rarr; projections, views, pipelines, <code>chunk_by</code>, <code>to</code></div>
<div class="step"><b>Session 5</b> one thread &rarr; producer/consumer on <code>jthread</code> + semaphores, cancellation by <code>stop_token</code>, the parser as a <code>generator</code>, TSan as a test</div>
</div>

Same input, same report, every session. `ctest -R report_identical` has passed 5 times.

<!--
Notes: Thirty seconds. The point: modernization is incremental and behavior-preserving, or it is
a rewrite. Every session's tests carried forward.
-->

---

<!-- SEGMENT: Concurrency (0:10) -->

<!-- _class: feature -->

## What C++11 gave, and what it lacked <span class="badge cpp11">C++11</span>

<p class="problem">A memory model, threads, mutexes, atomics, futures. The basics, and only the basics.</p>

| Had | Lacked |
|---|---|
| `std::thread`, `std::mutex`, `lock_guard`, `unique_lock` | A thread that **joins itself**: forget `join()` and the destructor calls `std::terminate` |
| `condition_variable`, `std::atomic<T>`, the memory orders | **Cancellation**: every codebase invented an `atomic<bool> stop` |
| `future`, `promise`, `async`, `packaged_task` | **Semaphores**, **latches**, **barriers**: all hand-written on `condition_variable` |
| `call_once`, `thread_local` | A reader/writer lock; a way to lock two mutexes in one line |
| | Any way to wait on an atomic without spinning |

C++14 to C++23 fill the right-hand column. Executors and a thread pool are still C++26 work.

<!--
Notes: Ask who has written a semaphore on a condition_variable. Everyone. That is the theme:
the idioms you wrote by hand are now types with specified semantics and, importantly, with a
sanitizer that understands them.
-->

---

<!-- _class: feature dense -->

## Reader/writer locks: `shared_timed_mutex`, `shared_mutex` <span class="badge cpp14">C++14</span> <span class="badge cpp17">C++17</span>

<p class="problem">A configuration table read on every record and updated once an hour serialized all its readers on one mutex.</p>

<!-- snippet: demos/s05/shared_mutex.cpp#rwlock -->
```cpp
class SensorTable {
public:
    double limit(const std::string& name) const {
        std::shared_lock lock(mutex_);              // many readers at once
        auto it = limits_.find(name);
        return it == limits_.end() ? 0.0 : it->second;
    }
    void set(const std::string& name, double v) {
        std::unique_lock lock(mutex_);              // one writer, no readers
        limits_[name] = v;
    }
private:
    mutable std::shared_mutex mutex_;               // C++17; C++14: shared_timed_mutex
    std::map<std::string, double> limits_;
};
// Read-mostly data: readers no longer serialize on each other.
```

`shared_lock` for readers, `unique_lock` for writers. `shared_mutex` (C++17) drops the timed operations that `shared_timed_mutex` (C++14) must support, and is cheaper for it.

<!--
Notes: The first thing C++14 added to <thread>-land. Only pays for read-mostly data: a shared
mutex is more expensive to acquire than a plain one, so measure before converting. Demo file:
demos/s05/shared_mutex.cpp
-->

---

<!-- _class: twocol -->

## `std::scoped_lock` <span class="badge cpp17">C++17</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s05/scoped_lock.cpp#before -->
```cpp
// C++11: locking two mutexes safely needed std::lock plus two adopt_lock guards
void transfer11(Account& from, Account& to, double amount) {
    std::lock(from.m, to.m);                                      // deadlock-free ordering
    std::lock_guard<std::mutex> a(from.m, std::adopt_lock);
    std::lock_guard<std::mutex> b(to.m, std::adopt_lock);
    from.balance -= amount; to.balance += amount;
}
```

</div>
<div>

#### After (C++17)

<!-- snippet: demos/s05/scoped_lock.cpp#after -->
```cpp
// C++17: one line, same deadlock-avoidance algorithm, CTAD deduces the mutex types
void transfer(Account& from, Account& to, double amount) {
    std::scoped_lock lock(from.m, to.m);
    from.balance -= amount; to.balance += amount;
}
```

</div>
</div>

Same algorithm as `std::lock` underneath (deadlock-free for any number of mutexes). CTAD deduces the mutex types. With one mutex it is a `lock_guard`; use it everywhere and stop choosing.

<!--
Notes: The one-mutex case matters more than the two-mutex case: scoped_lock replaces lock_guard
as the default, so there is one spelling in the codebase. The trap: `std::scoped_lock lock;`
with no mutex compiles and locks nothing. GCC's -Wunused-variable catches it; clang-tidy's
bugprone-unused-raii catches the sibling `std::scoped_lock{m};` temporary. Demo file:
demos/s05/scoped_lock.cpp
-->

---

<!-- _class: feature -->

## False sharing and `hardware_destructive_interference_size` <span class="badge cpp17">C++17</span>

<p class="problem">Two counters, two threads, no shared data, and the program is four times slower than expected.</p>

<!-- snippet: demos/s05/false_sharing.cpp#false_sharing -->
```cpp
struct Packed { std::atomic<long> a{0}, b{0}; };   // one cache line: each write invalidates the other core's copy

struct Padded {
    alignas(kLine) std::atomic<long> a{0};   // kLine = hardware_destructive_interference_size (64 on x86)
    alignas(kLine) std::atomic<long> b{0};   // libc++ 18 lacks the constant; the demo falls back to 64
};
```

Measured in the demo: **packed 695 ms, padded 171 ms** for 20 million increments per thread (GCC 14, x86-64). The cache line is the unit of coherence, not the variable.

<!--
Notes: The two atomics share a 64-byte line; each core's write invalidates the other core's copy.
alignas to the interference size puts them on separate lines. The constant is 64 on x86 and
libstdc++; libc++ 18 does not define it (the value is ABI-affecting, and they declined), so the
demo falls back to 64. Also a Session 1 callback: alignas is C++11. Demo file:
demos/s05/false_sharing.cpp. Optional slide: cut if the segment runs long.
-->

---

<!-- _class: twocol -->

## `std::jthread` <span class="badge cpp20">C++20</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s05/jthread.cpp#before -->
```cpp
// C++11: forget join() and the destructor terminates. Stopping is a flag you invent.
void worker11(std::atomic<bool>& stop) { while (!stop) std::this_thread::sleep_for(1ms); }
void run11() {
    std::atomic<bool> stop{false};
    std::thread t(worker11, std::ref(stop));
    std::this_thread::sleep_for(5ms);
    stop = true;
    t.join();                                   // mandatory; an exception before it terminates
}
```

</div>
<div>

#### After (C++20)

<!-- snippet: demos/s05/jthread.cpp#after -->
```cpp
// C++20: joins in its destructor, requesting a stop first; the token arrives by itself
void run20() {
    std::jthread t([](std::stop_token st) {
        while (!st.stop_requested()) std::this_thread::sleep_for(1ms);
    });
    std::this_thread::sleep_for(5ms);
}                                               // ~jthread: request_stop(), then join()
```

</div>
</div>

From the exercise: `std::jthread producer([&](std::stop_token producer_stop) { ... });` The token is passed as the first argument automatically if the callable accepts one.

<!--
Notes: jthread = joining thread. Its destructor calls request_stop() then join(), in that order.
That turns "forgot to join" from terminate into correct behavior, and gives every thread a
cancellation channel. Replace std::thread with std::jthread everywhere; the cost is the extra
stop_source (a small allocation). One caveat: the destructor joins, so a thread that never checks
its token hangs the destructor instead of terminating the process. Demo file: demos/s05/jthread.cpp
-->

---

<!-- _class: feature dense -->

## Cooperative cancellation: `stop_token`, `stop_source`, `stop_callback` <span class="badge cpp20">C++20</span>

<p class="problem">Every codebase has an atomic<bool> stop flag. None of them compose.</p>

<!-- snippet: demos/s05/stop_token.cpp#stop -->
```cpp
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
```

<!--
Notes: Three types: stop_source requests, stop_token observes, stop_callback runs a function when
a stop is requested (on the requesting thread, or immediately if already stopped). The exercise's
load_and_compute takes a stop_token from its caller and forwards it to the producer with a
stop_callback that also closes the queue, so one request unwinds both threads. Versus a bool flag:
tokens are thread-safe by specification, forward across layers, and condition_variable_any::wait
accepts one directly. Demo file: demos/s05/stop_token.cpp
-->

---

<!-- _class: feature dense -->

## `counting_semaphore` and `binary_semaphore` <span class="badge cpp20">C++20</span>

<p class="problem">"Wait until there is a slot" and "wait until there is an item" were a condition_variable and a predicate you wrote yourself.</p>

From the exercise's `BoundedQueue<T, Capacity>`:

```cpp
std::counting_semaphore<Capacity> slots_{Capacity};     // free capacity: the producer waits on it
std::counting_semaphore<Capacity + 1> items_{0};        // queued elements: the consumer waits on it

bool push(T item) {
    slots_.acquire();                                   // blocks while full
    { std::lock_guard lock(mutex_); /* closed? */ queue_.push(std::move(item)); }
    items_.release();
    return true;
}
std::optional<T> pop(std::stop_token stop = {}) {
    while (!items_.try_acquire_for(10ms)) {             // poll, so a stop is noticed while blocked
        if (stop.stop_requested()) return std::nullopt;
        /* closed and drained? return nullopt */
    }
    ...                                                 // lock, pop, slots_.release()
}
```

`acquire`, `release(n)`, `try_acquire`, `try_acquire_for`. The count type is `std::ptrdiff_t`; `binary_semaphore` is `counting_semaphore<1>`.

<!--
Notes: Exercise task 1. The mutex still guards the std::queue; the semaphores only do the
blocking. The exercise's -Wsign-conversion catches size_t capacities. Hand-typed excerpt from
exercises/s05-concurrency/solution/include/telemetry/queue.h.
-->

---

<!-- _class: twocol dense -->

## The bounded queue, before and after <span class="badge cpp20">C++20</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s05/semaphore_queue.cpp#before -->
```cpp
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
```

</div>
<div>

#### After (C++20)

<!-- snippet: demos/s05/semaphore_queue.cpp#after -->
```cpp
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
```

</div>
</div>

<!--
Notes: Same structure, two fewer things to get wrong: no predicate, no lost wake-up. The
exercise's version adds close(): set a flag, then release one extra items_ count so a blocked
pop() wakes and sees it. That is why items_ is sized Capacity + 1. Demo file:
demos/s05/semaphore_queue.cpp
-->

---

<!-- _class: feature dense -->

## `std::latch` and `std::barrier` <span class="badge cpp20">C++20</span>

<p class="problem">"Wait for all N workers to start" and "no one begins step 2 until everyone finishes step 1".</p>

<!-- snippet: demos/s05/latch_barrier.cpp#latch -->
```cpp
// latch: a one-shot countdown. "Start all workers, then wait until every one has checked in."
constexpr int kWorkers = 4;
std::latch ready(kWorkers);
std::vector<std::jthread> workers;
for (int i = 0; i < kWorkers; ++i)
    workers.emplace_back([&, i] { /* init */ ready.count_down(); std::println("worker {} ready", i); });
ready.wait();                                   // blocks until the count reaches zero; cannot be reset
std::println("all workers ready");
workers.clear();
```

<!-- snippet: demos/s05/latch_barrier.cpp#barrier -->
```cpp
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
```

<!--
Notes: latch: one-shot, count_down/wait, cannot reset; the fan-in half of fan-out/fan-in. barrier:
reusable, arrive_and_wait per phase, and a completion function that runs exactly once per phase
when the last thread arrives (must be noexcept). arrive_and_drop lets a thread leave the group.
Both replace a mutex + condition_variable + counter. Demo file: demos/s05/latch_barrier.cpp
-->

---

<!-- _class: feature dense -->

## `atomic::wait` / `notify` and `atomic_ref` <span class="badge cpp20">C++20</span>

<p class="problem">Waiting for an atomic to change meant spinning, or a mutex you only held to sleep on.</p>

<!-- snippet: demos/s05/atomic_wait.cpp#wait -->
```cpp
std::atomic<int> state{0};
std::jthread t([&] {
    state.wait(0);                              // block until state != 0 (a futex, not a spin)
    std::println("woke with state {}", state.load());
});
state.store(1);
state.notify_one();                             // wake one waiter (notify_all for everyone)
// C++11 needed a mutex + condition_variable for this. C++20: the atomic IS the wait point.
```

<!-- snippet: demos/s05/atomic_wait.cpp#atomic_ref -->
```cpp
Legacy obj;
std::atomic_ref<int> ref(obj.counter);          // atomic operations on a plain int, in place
std::jthread a([&] { for (int i = 0; i < 1000; ++i) ref.fetch_add(1); });
std::jthread b([&] { for (int i = 0; i < 1000; ++i) ref.fetch_add(1); });
a.join(); b.join();
std::println("{}", ref.load());                 // 2000; the object never changed type
// Rule: while any atomic_ref to an object exists, touch it ONLY through atomic_refs
// (so obj.counter is read through ref here, not directly).
```

<!--
Notes: wait(old) blocks until the value differs from old; implemented on futexes on Linux, so
it is a real sleep. notify_one/notify_all after the store. This is what semaphore, latch, and
barrier are built on in libstdc++. atomic_ref: atomic operations on an object you do not own the
type of (a struct from a C library, an element of a vector<int>). libc++ 18 lacks atomic_ref
(19 has it). Demo file: demos/s05/atomic_wait.cpp
-->

---

<!-- _class: feature dense -->

## `std::atomic<std::shared_ptr<T>>` <span class="badge cpp20">C++20</span>

<p class="problem">Hot-swappable configuration: readers on every record, a writer once an hour, no lock on the read path.</p>

<!-- snippet: demos/s05/atomic_shared_ptr.cpp#rcu -->
```cpp
struct Config { std::map<std::string, double> limits; };

std::atomic<std::shared_ptr<const Config>> g_config;   // readers copy it; writers swap it

double limit(const std::string& name) {
    auto cfg = g_config.load();                        // one atomic load; cfg keeps the Config alive
    auto it = cfg->limits.find(name);
    return it == cfg->limits.end() ? 0.0 : it->second;
}

void reload(std::map<std::string, double> fresh) {
    g_config.store(std::make_shared<const Config>(std::move(fresh)));   // in-flight readers keep the old one
}
// Read-copy-update with no lock on the read path.
```

The C++11 free functions `std::atomic_load(&sp)` and friends are **deprecated in C++20**: they made a non-atomic `shared_ptr` look atomic if you remembered to use them everywhere.

<!--
Notes: Read-copy-update. Readers copy the pointer (one atomic op plus a refcount increment);
the writer publishes a new object and the old one dies when the last reader drops it. Not
lock-free on most implementations (a spinlock inside), but the read path never waits on the
writer's work. libc++ 18 lacks it; libstdc++ 12+ has it. Optional slide: fold into the previous
one if short on time. Demo file: demos/s05/atomic_shared_ptr.cpp
-->

---

<!-- _class: feature -->

## `std::osyncstream` <span class="badge cpp20">C++20</span>

<p class="problem">Four threads log with <code>cout &lt;&lt; a &lt;&lt; b &lt;&lt; c</code>. The standard promises no data race; it promises nothing about the line.</p>

<!-- snippet: demos/s05/osyncstream.cpp#sync -->
```cpp
std::vector<std::jthread> threads;
for (int i = 0; i < 4; ++i)
    threads.emplace_back([i] {
        std::osyncstream(std::cout) << "thread " << i << " says hello" << '\n';   // one line, atomically
    });
// Without osyncstream, the four << calls from each thread interleave character by character.
// Each osyncstream buffers and emits on destruction (or on emit()).
```

Pair it with `std::format` from Session 2: `std::osyncstream(std::cout) << std::format("thread {} says hello\n", i);`, or `std::print` to a per-thread `osyncstream`.

<!--
Notes: A wrapper stream that buffers everything written to it and transfers the buffer to the
wrapped stream atomically on destruction or emit(). No locking around cout in your code. On
libc++ 18 it is behind -fexperimental-library, which the repo's CMake adds. Demo file:
demos/s05/osyncstream.cpp
-->

---

<!-- _class: feature -->

## `std::move_only_function` and task queues <span class="badge cpp23">C++23</span>

<p class="problem">A task queue of std::function cannot hold a task that owns a unique_ptr: std::function requires copyable callables.</p>

<!-- snippet: demos/s05/move_only_function_queue.cpp#tasks -->
```cpp
std::queue<std::move_only_function<void()>> tasks;    // std::function would refuse the unique_ptr capture
auto payload = std::make_unique<int>(42);
tasks.push([p = std::move(payload)] { std::println("task with {}", *p); });
tasks.push([] { std::println("plain task"); });
while (!tasks.empty()) { auto t = std::move(tasks.front()); tasks.pop(); t(); }
```

Also: `std::move_only_function<void() const noexcept>` carries the qualifiers in the signature, which `std::function` never did. libc++ 18 does not have it yet; the demo gates on `__cpp_lib_move_only_function`.

<!--
Notes: The type every thread pool needed. The old workaround was a shared_ptr in the capture,
which made the task copyable by lying about ownership. Session 2's "ownership in the type"
lesson applied to callables. Demo file: demos/s05/move_only_function_queue.cpp
-->

---

## What is still missing, and where it is going

<p class="problem">C++23 has no thread pool, no executor, no async I/O. Every project still picks one.</p>

- **`std::async`**: the future's destructor blocks; no pool, no priority, no cancellation. Fine for a one-off; do not build on it
- **Thread pools**: still a library choice (Intel TBB, Boost.Asio, folly, your own on `jthread` + the bounded queue)
- **`std::execution`** (P2300, senders/receivers): **C++26**. Composable asynchronous work with a scheduler abstraction; the model that `co_await` will plug into
- **Parallel algorithms** (Session 4): the standard's only built-in parallelism, and they need TBB on GCC
- **Hazard pointers and RCU** (`<hazard_pointer>`, `<rcu>`): C++26

What to do now: `jthread` for lifetime, `stop_token` for cancellation, a queue on semaphores for hand-off, and a pool you can replace later behind one interface.

<!--
Notes: Honest slide. The standard gives primitives, not an application-level model, until C++26.
The advice is to keep the pool behind an interface so std::execution can slot in.
-->

---

<!-- _class: demo -->

## ThreadSanitizer as a test

<!-- snippet: demos/s05/tsan_race.cpp#race -->
```cpp
    int shared = 0;                                  // plain int, written by one thread, read by another
    std::jthread writer([&] { for (int i = 0; i < 1000; ++i) ++shared; });
#ifdef SHOW_ERRORS
    std::println("{}", shared);                      // TSan: "WARNING: ThreadSanitizer: data race" with both stacks
#endif
    writer.join();                                   // join() is a synchronization point:
    std::println("{}", shared);                      // ...this read is ordered after every write. No race.
    // The exercise's lines_read counter is exactly this: written by the producer, read after join().
```

```
cmake -S . -B build-tsan -DCOURSE_SANITIZE=thread && cmake --build build-tsan
ctest --test-dir build-tsan -R s05 --output-on-failure
```

`WARNING: ThreadSanitizer: data race` with both stacks and both thread creation sites. Exit code non-zero, so it fails CTest.

<!--
Notes: Live: build tsan_race with -DSHOW_ERRORS under TSan and show the report; then the
exercise's solution under TSan, clean. Exercise task 3: reading lines_read before join() is
exactly this race. TSan understands every primitive shown today (it knows a join is a
synchronization point), which is why "every concurrent test runs under TSan in CI" is a policy,
not a hope. Slowdown 5 to 15x, memory 5 to 10x: a separate CI job, not the default build.
Demo file: demos/s05/tsan_race.cpp
-->

---

<!-- _class: takeaway -->

## Concurrency takeaway

**`jthread` + `stop_token`** for lifetime and cancellation. **Semaphores** for hand-off. **`latch` and `barrier`** for phases. **`scoped_lock`** for every lock. **`osyncstream`** for logging.

The hand-written versions of all of these are in your codebase now. Each replacement is a design review, and ThreadSanitizer is how the review ends.

<!--
Notes: Transition: the queue hands records between threads. The next segment hands them between
a function and its caller without a thread at all.
-->

---

<!-- SEGMENT: Coroutines (0:40) -->

<!-- _class: feature -->

## What a coroutine is <span class="badge cpp20">C++20</span>

<p class="problem">A function that can suspend in the middle, return to its caller, and later resume where it left off.</p>

- Any function whose body contains **`co_await`**, **`co_yield`**, or **`co_return`** is a coroutine. Nothing in the signature says so
- Its locals live in a **coroutine frame**, usually heap-allocated, that outlives each suspension
- Suspending is **not** blocking and **not** a thread: the caller gets control back, on the same thread
- C++20 shipped the **language** machinery (`<coroutine>`: `coroutine_handle`, `suspend_always`) and **no library types**. You wrote a promise type or used a library
- C++23 shipped the first library type: **`std::generator`**

<!--
Notes: The single most important sentence: a coroutine is a control-flow feature, not a
concurrency feature. It runs on whichever thread resumes it. The language gives the mechanism
and leaves the policy (what happens at suspend, who resumes) to a "promise type" you or a
library write. That design is why C++20 coroutines were nearly unusable without a library, and
why std::generator in C++23 matters.
-->

---

<!-- _class: feature -->

## Why they matter here <span class="badge cpp20">C++20</span>

| Problem | Without coroutines | With |
|---|---|---|
| A lazy sequence (the parser) | An iterator class with hand-kept state, or an eager vector | `co_yield` in a loop |
| A state machine (a protocol decoder) | An `enum State` and a `switch` in `feed()` | The state is the program counter |
| Asynchronous I/O | Callbacks, and the state they carry in captures | `co_await` on the operation; code reads top to bottom |
| A cooperative scheduler (embedded) | Hand-rolled with `setjmp` or an RTOS | Suspend/resume with no stack per task |

What they are **not**: parallelism. Two coroutines on one thread never run at the same time. Pair them with the previous segment for that.

<!--
Notes: The three rows we can show today: the parser (exercise task 5), the decoder (a demo),
and a taste of async (a hand-written awaitable). The I/O row is where most of the industry's
coroutine use is, in Asio, folly, cppcoro, and it is exactly where the standard library still
has nothing until std::execution.
-->

---

<!-- _class: twocol -->

## `std::generator` <span class="badge cpp23">C++23</span>

<div class="cols">
<div>

#### Before (C++11 to C++20, eager)

```cpp
// The exercise's Session 4 parser: build everything,
// then hand back the vector
LoadResult load_stream(std::istream& in) {
    LoadResult out;
    std::string line;
    while (std::getline(in, line)) {
        if (line.ends_with('\r')) line.pop_back();
        if (auto r = parse_record(line))
            out.records.push_back(std::move(*r));
        else
            ++out.rejected[r.error()];
    }
    return out;
}
```

</div>
<div>

#### After (C++23, lazy)

```cpp
// The Session 5 solution: one record per co_yield.
// The caller pulls; nothing runs until it does.
std::generator<Record> records(std::istream& in) {
    std::string line;
    while (std::getline(in, line)) {
        if (line.ends_with('\r')) line.pop_back();
        if (auto r = parse_record(line))
            co_yield std::move(*r);
    }
}

// It is an input_range: every Session 4 view composes
for (const auto& r : records(in) | std::views::take(2))
    std::println("{}", r.sensor);
```

</div>
</div>

<!--
Notes: Exercise task 5. Note what was lost: the rejected-line count, because a generator yields
one thing. That is a design question (yield a variant, as the queue does; or keep both entry
points, as the solution does). Hand-typed excerpts from exercises/s05-concurrency. Gated on
__cpp_lib_generator: libstdc++ 14 has it, libc++ 18 does not.
-->

---

<!-- _class: feature dense -->

## Generator mechanics: lazy, and the lookahead <span class="badge cpp23">C++23</span>

<!-- snippet: demos/s05/generator_basics.cpp#lazy -->
```cpp
// C++23: a coroutine. Each co_yield hands one value to the consumer and suspends.
std::generator<std::string> fields(std::string_view line) {
    for (auto piece : line | std::views::split(','))
        co_yield std::string(piece.begin(), piece.end());
}                                               // returning ends the sequence

void use() {
    // std::generator is an input_range, so every Session 4 view composes with it
    for (const auto& f : fields("1,rpm,40,extra") | std::views::take(2))
        std::println("{}", f);                  // 1, rpm: the third field is never built
}
```

<!-- snippet: demos/s05/generator_basics.cpp#fib -->
```cpp
std::generator<long> fibonacci() {              // infinite: it is lazy, so that is fine
    long a = 0, b = 1;
    while (true) { co_yield a; std::tie(a, b) = std::pair{b, a + b}; }
}
```

From the exercise's test: after `records(in) | views::take(2)`, the next unread line is line **4**, not line 3. `take(2)` resumes the coroutine once more than it uses (the same lookahead as Session 4's laziness demo). Then the generator is destroyed **while suspended**: locals are destroyed, and nothing after that `co_yield` ever runs.

<!--
Notes: Two consequences to say out loud. (1) A coroutine destroyed while suspended never
finishes: a lock held across a co_yield stays held until the frame is destroyed, and code after
the loop never runs. Put cleanup in destructors, not after the loop. (2) A generator body runs
on the consumer's thread, at the consumer's pace; there is no thread. Demo file:
demos/s05/generator_basics.cpp
-->

---

<!-- _class: feature dense -->

## What the compiler generates <span class="badge cpp20">C++20</span>

<p class="problem">A minimal generator by hand, so std::generator is not magic. Never write this in production.</p>

<!-- snippet: demos/s05/coroutine_machinery.cpp#promise -->
```cpp
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
```

<!-- snippet: demos/s05/coroutine_machinery.cpp#use -->
```cpp
Gen<int> counter(int n) {                       // a coroutine because its body contains co_yield
    for (int i = 0; i < n; ++i) co_yield i;     // ≈ co_await promise.yield_value(i)
}                                               // ≈ promise.return_void(); co_await promise.final_suspend()

int main() {
    auto g = counter(3);                        // frame allocated, promise constructed, suspended at initial_suspend
    while (g.next()) std::print("{} ", g.value());
    std::println("");
}                                               // ~Gen: handle.destroy() frees the frame
```

<!--
Notes: The compiler rewrites the body: allocate a frame, construct promise_type inside it, call
get_return_object (that is what the caller receives), co_await initial_suspend, run the body
where co_yield v becomes co_await promise.yield_value(v), then return_void and co_await
final_suspend. Every customization point is a member of promise_type found by name. The
Gen class's job is to own the coroutine_handle and call resume/destroy. Demo file:
demos/s05/coroutine_machinery.cpp (the handle half is in the file).
-->

---

<!-- _class: feature dense -->

## Awaitables and `co_await` <span class="badge cpp20">C++20</span>

<p class="problem">co_await x asks x three questions: ready? where do I park? what is my value?</p>

<!-- snippet: demos/s05/awaitable.cpp#awaitable -->
```cpp
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
```

<!-- snippet: demos/s05/awaitable.cpp#use -->
```cpp
std::binary_semaphore done{0};

Task consumer() {
    std::println("before await on {}", std::this_thread::get_id());
    const int v = co_await ValueFromThread{};   // suspends here; the rest runs on the producing thread
    std::println("after await: {} on {}", v, std::this_thread::get_id());
    done.release();
}
```

<!--
Notes: The demo prints two thread ids: the code after co_await runs on the producing thread,
because that is who called resume(). That is the whole model of async coroutines: whoever
completes the operation continues the function. It is also why hand-writing these is dangerous
(lifetimes, exceptions, which thread you are on). In production use std::generator, a library
(cppcoro, folly::coro, Asio's awaitables, libunifex), or C++26's std::execution. Demo file:
demos/s05/awaitable.cpp
-->

---

<!-- _class: twocol dense -->

## A coroutine as a state machine <span class="badge cpp20">C++20</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s05/state_machine.cpp#switch -->
```cpp
// C++11: the state is data; every transition is a line you must not forget
class SwitchDecoder {
public:
    std::optional<Bytes> feed(std::uint8_t b) {
        using enum State;                       // C++20
        switch (state_) {
        case Sync:    { if (b == 0xAA) { state_ = Len; } break; }
        case Len:     { len_ = b; payload_.clear(); sum_ = 0;
                        state_ = len_ ? Payload : Check; break; }
        case Payload: { payload_.push_back(b); sum_ = add(sum_, b);
                        if (payload_.size() == len_) { state_ = Check; } break; }
        case Check:   { state_ = Sync; if (b == sum_) { return payload_; } break; }
        }
        return std::nullopt;
    }
private:
    static std::uint8_t add(std::uint8_t a, std::uint8_t b) { return static_cast<std::uint8_t>(a + b); }
    enum class State { Sync, Len, Payload, Check } state_ = State::Sync;
    std::size_t len_ = 0;
    std::uint8_t sum_ = 0;
    Bytes payload_;
};
```

</div>
<div>

#### After (C++20)

<!-- snippet: demos/s05/state_machine.cpp#coroutine -->
```cpp
// C++20: the state is where you are in the function; it reads like the spec
Decoder decode() {
    using next = Decoder::next_byte;
    while (true) {
        while (co_await next{} != 0xAA) {}      // Sync
        const std::uint8_t len = co_await next{};
        Bytes payload;
        std::uint8_t sum = 0;
        for (std::uint8_t i = 0; i < len; ++i) {
            const auto b = co_await next{};
            payload.push_back(b);
            sum = static_cast<std::uint8_t>(sum + b);
        }
        if (co_await next{} == sum) co_yield payload;   // Check: hand the frame to the caller
    }
}
```

</div>
</div>

<!--
Notes: Frame format: 0xAA, length, payload, checksum. The switch version's state is an enum, a
count, and a partial buffer, all of which must be reset correctly on every transition. The
coroutine version's state is where you are in the function; the `for` loop IS the Payload state.
The forty lines of Decoder plumbing (co_await next_byte, co_yield frame) are in the demo file and
are written once per project. Where this pays: embedded protocol decoders, parsers, anything
with "wait for the next input". Demo file: demos/s05/state_machine.cpp
-->

---

<!-- _class: feature -->

## Coroutines and `expected` <span class="badge cpp23">C++23</span>

<p class="problem">Session 2's preview: an expected-returning function that stops at the first error, without and_then chains.</p>

```cpp
// Not standard C++23: a custom promise type makes `co_await expected` mean
// "unwrap the value, or return the error from the whole function".
Expected<Report, ParseError> build(std::istream& in) {
    auto header = co_await parse_header(in);        // Expected<Header, ParseError>: value, or co_return the error
    auto body   = co_await parse_body(in, header);
    co_return Report{header, body};
}
```

- The promise's `await_transform(expected<T, E>)` returns an awaitable: `await_ready` is `has_value()`; on error, `await_suspend` stores the error and destroys the frame
- Boost.Outcome, `tl::expected`'s examples, and P2561 (`?`-style operator) explore this; nothing is standard yet
- What C++26's `std::execution` does instead: errors are a **channel** (`set_error`), not a return value, so `co_await` on a sender propagates them

<!--
Notes: Optional slide; cut first if the segment runs long. The point is the shape: a coroutine
can implement "early return on error" with no macro, because the promise type sees every
co_await. Keep it conceptual; no demo file.
-->

---

<!-- _class: feature dense -->

## Allocation and performance <span class="badge cpp23">C++23</span>

<!-- snippet: demos/s05/generator_perf.cpp#three -->
```cpp
// 1. Eager: a vector of every value, then a loop over it. Two passes, one allocation per growth.
std::vector<int> values_eager(std::string_view in) {
    std::vector<int> out;
    for (auto p : in | std::views::split(',')) if (!p.empty()) out.push_back(parse(std::string_view(p)));
    return out;
}

// 2. A view pipeline: no coroutine, no allocation, fully inlinable. The Session 4 answer.
auto values_view(std::string_view in) {
    return in | std::views::split(',')
              | std::views::filter([](auto p) { return !p.empty(); })
              | std::views::transform([](auto p) { return parse(std::string_view(p)); });
}

#if defined(__cpp_lib_generator)
// 3. A generator: one frame allocation up front, then a suspend/resume pair per element.
std::generator<int> values_gen(std::string_view in) {
    for (auto p : in | std::views::split(',')) if (!p.empty()) co_yield parse(std::string_view(p));
}
#endif
```

Demo (2 million integers, GCC 14, Debug build): **eager 768 ms, view 936 ms, generator 926 ms**. In a Release build the view wins; the generator costs one frame allocation plus a suspend/resume per element.

<!--
Notes: Three real numbers, then the caveats: Debug build, so take the ordering not the ratios;
the eager version's allocation is amortized over millions of elements. What matters: a
generator is roughly the cost of an indirect call per element, plus one allocation for the
frame. HALO (heap allocation elision) can remove the allocation when the coroutine is inlined
into its consumer; Clang does it sometimes, GCC has no such pass (do not expect it); never rely on it for a hard real-time
budget. std::generator takes an allocator argument if you need control. Demo file:
demos/s05/generator_perf.cpp (build Release for real numbers).
-->

---

<!-- _class: feature -->

## Availability <span class="badge cpp23">C++23</span>

| | GCC 14 / libstdc++ | Clang 18 / libc++ 18 | MSVC 19.3x |
|---|---|---|---|
| Language (`co_await`, `<coroutine>`) | yes | yes | yes |
| `std::generator` | **yes** | no (in progress) | no |
| `__cpp_lib_generator` | defined | not defined | not defined |
| HALO | do not expect it | sometimes | sometimes |

The exercise: `#if __has_include(<generator>) && defined(__cpp_lib_generator)` defines `TELEMETRY_HAS_GENERATOR`; the coroutine parser and its test compile only there. For libc++ today: a third-party generator (cppcoro, `std::generator` reference implementation from the paper) is a header.

<!--
Notes: The language part is everywhere; the library part is one implementation. This is the
strongest case in the course for feature-test macros: the same source builds on both
toolchains and the test suite differs by one test.
-->

---

<!-- _class: feature -->

## Where coroutines pay off, and where they do not <span class="badge cpp20">C++20</span>

| Use | Verdict |
|---|---|
| Lazy sequences over I/O or expensive parsing | **Yes**: `std::generator`, today, on libstdc++ |
| Protocol decoders and state machines | **Yes**, with one small hand-written or library type |
| Asynchronous I/O | **Yes**, through a library (Asio, folly, cppcoro), or wait for `std::execution` |
| A hot inner loop over a vector | **No**: a view or a plain loop; the suspend/resume is pure overhead |
| Hard real-time with an allocation budget | **Not unless** HALO is verified in the generated code, or a custom allocator is used |
| Replacing every callback in an existing codebase | **Not yet**: pick one subsystem, measure, and keep the callback interface behind it |

<!--
Notes: The middle column is the honest answer to "should we use coroutines". The lazy sequence
and the state machine are safe first uses because they are single-threaded and the type is
small.
-->

---

<!-- _class: takeaway -->

## Coroutines takeaway

A coroutine is a **control-flow** tool: a function that can pause. **`std::generator`** is the standard's first use of it, and it is a **range**, so Session 4 applies.

The machinery is one promise type and three awaiter methods. Use a library's, not your own, except to learn.

<!--
Notes: Transition: the last feature is the one that changes the build.
-->

---

<!-- SEGMENT: Modules (1:05) -->

<!-- _class: feature -->

## What modules fix <span class="badge cpp20">C++20</span>

<p class="problem">Every translation unit re-parses <code>&lt;vector&gt;</code>. Every macro in every header leaks into every file after it.</p>

| Problem | Header | Module |
|---|---|---|
| Parse cost | Every TU re-parses every header it includes, transitively | The interface is compiled **once** to a BMI and imported |
| Macro leakage | `#define min` in one header breaks the next | Macros do not cross `import` |
| Include order | Header A works only if B came first | Imports are order-independent |
| Private helpers | `namespace detail`, and hope | Not exported means **not visible** |
| ODR | Inline functions with slightly different definitions across TUs | One definition, one owner |

What they do **not** fix: they are not packages, not a build system, not faster **link** times, and not a reason to rewrite a working library.

<!--
Notes: The pitch is compile time and hygiene. The catch is the build system, which is the next
three slides. Honest framing: modules are the least-adopted feature in this course as of 2026,
and the reasons are tooling, not the language.
-->

---

<!-- _class: twocol -->

## `export module`, `export`, `import` <span class="badge cpp20">C++20</span>

<div class="cols">
<div>

#### Before (a header)

```cpp
// crc.h
#pragma once
#include <array>
#include <cstdint>
#include <string_view>

namespace telemetry {
namespace detail {                       // "please do not use"
    constexpr std::uint16_t kPolynomial = 0x1021;
    constexpr std::array<std::uint16_t, 256> make_table();
}
constexpr std::uint16_t crc16(std::string_view text);
}
// every #include of this re-parses <array> and <string_view>
```

</div>
<div>

#### After (a module interface unit)

```cpp
// crc.cppm
module;                                  // global module fragment: includes go here
#include <array>
#include <cstdint>
#include <string_view>

export module telemetry.crc;             // this file IS the module

namespace telemetry {
constexpr std::uint16_t kPolynomial = 0x1021;   // module-private: invisible to importers
constexpr auto kTable = make_table();

export constexpr std::uint16_t crc16(std::string_view text) { ... }
}
```

```cpp
// main.cpp
import telemetry.crc;                    // parsed once; no macros; no include guard
static_assert(telemetry::crc16("123456789") == 0x29B1);
```

</div>
</div>

<!--
Notes: demos/s05/modules/crc.cppm and main.cpp, abridged. Three keywords: `module;` opens the
global module fragment, `export module name;` names this unit, `export` marks what importers
see. Everything else in the file is private without a detail namespace. The static_assert
shows constexpr crosses the module boundary. Hand-typed excerpts.
-->

---

<!-- _class: feature -->

## The global module fragment, and a GCC 14 rule <span class="badge cpp20">C++20</span>

<p class="problem">The standard library is still headers until <code>import std;</code>. Where do the includes go?</p>

```cpp
module;                       // 1. global module fragment: #include only, nothing else
#include <array>              //    these declarations attach to the global module, not to telemetry.crc
#include <string_view>

export module telemetry.crc;  // 2. the module purview begins; from here, everything belongs to the module
import telemetry.util;        // 3. imports come first inside the purview
export namespace telemetry { ... }
```

In a **consumer** on GCC 14, `#include` must come **before** `import`:

```cpp
#include <print>              // GCC 14: an include after an import fails ("import ... appears after a
import telemetry.crc;         // declaration" or duplicate attachment errors). Clang 18 accepts both orders.
```

<!--
Notes: Exercise task 6 asks them to swap the two lines in main.cpp and read the GCC error. The
rule exists because GCC 14 treats a header included after an import as possibly re-declaring
things the module's global fragment already attached. Clang is more forgiving. Write includes
first everywhere and both compilers are happy. Hand-typed excerpts.
-->

---

<!-- _class: feature dense -->

## Partitions, implementation units, header units <span class="badge cpp20">C++20</span>

<p class="problem">One interface file per library does not scale. Modules have three ways to split.</p>

```cpp
// telemetry-crc.cppm: a partition. Part of module telemetry, compiled separately.
export module telemetry:crc;
export constexpr std::uint16_t crc16(std::string_view);

// telemetry.cppm: the primary interface re-exports its partitions.
export module telemetry;
export import :crc;
export import :parse;

// telemetry-parse.cpp: an implementation unit. No export; sees everything in module telemetry.
module telemetry;
std::expected<Record, ParseError> parse_record(std::string_view line) { ... }
```

```cpp
import <vector>;              // a header unit: a header compiled as if it were a module. Macros DO cross.
                              // The migration bridge for third-party headers, where the build system supports it.
```

<!--
Notes: Optional slide; fold into the previous one if short on time. Partitions split the
interface; implementation units split the definitions (like .cpp files); header units are the
compatibility mechanism and are the least supported piece (CMake has no official header-unit
support as of 3.30). Hand-typed excerpts; no demo file.
-->

---

<!-- _class: feature -->

## `import std;` <span class="badge cpp23">C++23</span>

<p class="problem">The whole standard library as one module. The first modules feature most codebases will switch on.</p>

```cpp
import std;                   // everything in namespace std, from <algorithm> to <vector>
import std.compat;            // plus the C library's global names (::printf, ::size_t)

int main() { std::println("{}", std::ranges::max(std::vector{1, 2, 3})); }
```

| Needs | Status (2026) |
|---|---|
| Compiler + library | libc++ 17+, MSVC 17.5+, libstdc++ **15** (not 14) |
| CMake | 3.30+ with `CMAKE_EXPERIMENTAL_CXX_IMPORT_STD` and `CXX_MODULE_STD ON` |
| Generator | Ninja or Visual Studio |
| Measured | Microsoft: a hello-world 10x faster to compile; a real TU typically 20 to 50% |

<!--
Notes: The pragmatic entry point: no code changes beyond replacing a block of includes, and the
compile-time win is immediate. The blocker in this room is libstdc++ 14; GCC 15 adds it. Clang
18 with libc++ can do it today with the CMake experimental flag. Not enabled in the repo because
CMake 3.28 is the floor there. Hand-typed excerpt; no demo file.
-->

---

<!-- _class: feature dense -->

## Build systems <span class="badge cpp20">C++20</span>

<p class="problem">A module must be compiled before anything that imports it. The build system has to discover that order from the source.</p>

```cmake
# demos/s05/modules/CMakeLists.txt (CMake 3.28+, Ninja)
add_executable(demo_s05_modules main.cpp)
target_sources(demo_s05_modules PUBLIC FILE_SET CXX_MODULES FILES crc.cppm)
```

```
cmake -S . -B build-mod -G Ninja -DCOURSE_MODULES=ON && cmake --build build-mod
```

- **P1689** dependency scanning: the compiler emits, per source, what it exports and imports; CMake builds the graph before compiling. That is why **Ninja** (or MSBuild) is required: Make cannot express dynamically discovered dependencies
- **Meson**, **Bazel**: partial; **build2**: full; **xmake**: full
- The BMI (built module interface) is compiler-specific and flag-specific: no sharing across compilers or across `-std` flags

<!--
Notes: Live: configure with Ninja and COURSE_MODULES=ON, build, run, then `ninja -t deps` or
open the .ddi files to show the scan output. The one-line CMake is the good news; the
generator requirement is what blocks Make-based projects. The CMake block is a hand-typed
excerpt from demos/s05/modules/CMakeLists.txt.
-->

---

<!-- _class: dense -->

## Compiler status, and what the repo does

| | GCC 14 | Clang 18 | MSVC 19.3x |
|---|---|---|---|
| Named modules | yes; `#include` before `import` in consumers | yes, best diagnostics | yes, most complete |
| Partitions, implementation units | yes | yes | yes |
| Header units | partial (`-fmodule-header`) | yes (`-fmodule-header`) | yes |
| `import std;` | GCC 15 | libc++ 17+ with CMake 3.30 | yes (17.5+) |
| CMake `FILE_SET CXX_MODULES` | 3.28+ | 3.28+ | 3.28+ |
| Diagnostics quality | improving | good | good |

The repo: `demos/s05/modules/` builds `telemetry.crc` on **both** GCC 14 and Clang 18 with `-G Ninja -DCOURSE_MODULES=ON`. Off by default so the Make-based default build keeps working.

<!--
Notes: The table is the support matrix row for modules. The message: the compilers are ready;
the reason to hesitate is your build and your dependencies' headers.
-->

---

<!-- _class: feature -->

## An adoption posture <span class="badge cpp20">C++20</span>

1. **Measure the build first.** `-ftime-report`, ClangBuildAnalyzer. If headers are not the cost, modules will not be the win
2. **`import std;`** as soon as the toolchain floor allows it. Zero source change beyond the includes; the biggest single win
3. **New leaf libraries as modules**, behind a build option, with a header fallback until every consumer's build can import
4. **Header units for third-party headers** where the build system supports them; otherwise leave them in the global module fragment
5. **Do not convert a working header-only library** for its own sake
6. **One compiler at a time**: BMIs are not portable, so a mixed-toolchain CI doubles the module build

<!--
Notes: The order matters: measure, then the cheapest win, then new code only. Nobody in this
room should plan a module conversion of an existing library this year unless the build
measurement says headers dominate.
-->

---

<!-- _class: takeaway -->

## Modules takeaway

Understand modules now: **`module;`**, **`export module`**, **`export`**, **`import`**, and the build order the compiler must discover.

Adopt them where the build is ready, and expect **`import std;`** to be the first thing worth switching on.

<!--
Notes: Transition: five minutes on what has been removed from the language while all this was
added.
-->

---

<!-- SEGMENT: Deprecations (1:25) -->

<!-- _class: dense -->

## Deprecated and removed, C++14 through C++23

| Gone | Since | Use instead |
|---|---|---|
| `std::auto_ptr`, `random_shuffle`, `bind1st`/`bind2nd`, `ptr_fun`, `unary_function` | removed C++17 | `unique_ptr`, `shuffle`, lambdas |
| `throw()` dynamic exception specifications | removed C++17/C++20 | `noexcept` |
| `std::iterator<...>` base class | deprecated C++17 | the five member typedefs, or C++20 iterator concepts |
| `std::result_of`, `std::uncaught_exception`, `raw_storage_iterator` | removed C++20 | `invoke_result`, `uncaught_exceptions` |
| `<codecvt>`, `wstring_convert` | deprecated C++17, removed C++26 | a library (ICU, simdutf) |
| `volatile` compound ops (`v++`, `v += 1`) | deprecated C++20; C++23 restored only `\|=`, `&=`, `^=` | `v = v + 1`, or an atomic |
| `std::aligned_storage`, `aligned_union` | deprecated C++23 | `alignas(T) std::byte buf[sizeof(T)]` |
| `std::atomic_init`, `ATOMIC_VAR_INIT`, free `atomic_load(shared_ptr*)` | deprecated C++20 | constructors, `atomic<shared_ptr>` |
| Garbage-collection support (`declare_reachable`) | removed C++23 | nothing; nobody used it |
| `std::strstream` | deprecated C++98, removed C++26 | `spanstream` (C++23) |

<!--
Notes: libstdc++ keeps most removed names available with a deprecation warning; libc++ removes
them, so a codebase that only builds on GCC may be carrying auto_ptr without knowing. The
deprecated.cpp demo shows both behaviors: warnings on GCC, hard errors on Clang.
-->

---

<!-- _class: demo dense -->

## Finding them

<!-- snippet: demos/s05/deprecated.cpp#replacements -->
```cpp

void modern(std::vector<Sensor>& v) {
    auto p = std::make_unique<Sensor>(1);                     // was: std::auto_ptr<Sensor> (removed in C++17)
    std::shuffle(v.begin(), v.end(), std::mt19937{42});       // was: std::random_shuffle (removed in C++17)
    auto less_than_5 = [](const Sensor& s) { return s.id < 5; };   // was: std::bind2nd(std::less<int>(), 5)
    using R = std::invoke_result_t<decltype(&by_id), Sensor, Sensor>;   // was: std::result_of (removed in C++20)
    static_assert(std::is_same_v<R, bool>);
    static_assert(std::is_same_v<std::iterator_traits<It>::value_type, int>);   // It: five typedefs, no std::iterator base
    alignas(Sensor) std::byte storage[sizeof(Sensor)];        // was: std::aligned_storage_t<sizeof(Sensor)> (deprecated C++23)
    (void)p; (void)less_than_5; (void)storage;
    std::println("{} sensors, shuffled deterministically", v.size());
}
```

```
g++-14 -std=c++23 -DSHOW_ERRORS -Wdeprecated -fsyntax-only demos/s05/deprecated.cpp      # warnings
clang++ -std=c++23 -stdlib=libc++ -DSHOW_ERRORS -fsyntax-only demos/s05/deprecated.cpp   # errors
clang-tidy -p build --checks='modernize-*' exercises/s01-*/starter/src/*.cpp | grep -c warning
```

Exercise task 7: the clang-tidy count on the Session 1 starter versus the Session 5 solution. That difference is the course.

<!--
Notes: Live: the two compiler runs, side by side. Then clang-tidy on the Session 1 starter (many
warnings) and on the Session 5 solution (near zero). Library deprecations warn through
-Wdeprecated-declarations, on by default in both compilers; -Wdeprecated is a separate group
(language deprecations such as volatile ++) and not on by default in Clang.
-Werror=deprecated-declarations makes it a build break,
which is the right setting once the count reaches zero. Demo file: demos/s05/deprecated.cpp
-->

---

<!-- _class: takeaway -->

## Deprecations takeaway

Removed names are a **toolchain test**: libstdc++ warns, libc++ refuses. Build on both.

`-Werror=deprecated-declarations` once the count is zero, and clang-tidy `modernize-*` as the metric that says how far you have come.

<!--
Notes: Transition to the workshop.
-->

---

<!-- SEGMENT: Roadmap workshop (1:30) -->

<!-- _class: feature dense -->

## Three tiers <span class="badge cpp23">C++23</span>

| Tier | Character | Contents |
|---|---|---|
| **1. Mechanical, zero risk** (this month) | Local edits; no interface changes; clang-tidy does most of it | `make_unique`, `[[nodiscard]]`, structured bindings, `string_view` at read-only boundaries, `optional` for sentinels, `using` over `typedef`, `<=>`, `scoped_lock` for `lock_guard`, `jthread` for `thread` |
| **2. Interface changes** (this quarter) | Callers change; tests must exist first | `span` for buffer parameters, `expected` for error returns, concepts on public templates, `format`/`print` replacing `printf`/`iostream`, `constexpr` tables, projections and `ranges::` algorithms |
| **3. Architectural** (evaluate, pilot one) | Design reviews; one subsystem at a time | Views pipelines in hot paths, `stop_token` cancellation, a semaphore queue replacing a hand-written one, `std::generator` for a lazy source, modules for one new library |

Tier 1 is the exercise from Session 1. Tier 2 is Sessions 2 to 4. Tier 3 is today.

<!--
Notes: The tiers map to the course. Tier 1 needs no meeting. Tier 2 needs the tests the course
kept insisting on (report_identical). Tier 3 needs a pilot and a measurement. The one tier 1
item with a caveat: jthread's destructor joins, so a thread that never checks its token hangs
rather than terminates; check the loop before swapping.
-->

---

<!-- _class: feature -->

## Tooling as a force multiplier <span class="badge cpp23">C++23</span>

- **clang-tidy** `modernize-*`, `performance-*`, `readability-*` with `-fix`; the repo's `.clang-tidy` is a starting point. Run it per directory, commit per check
- **Flag progression**: `-std=c++17`, then `20`, then `23`, one CI job ahead of the default; fix what breaks before switching the default
- **Warnings as errors per directory**: the course's `course_warnings` target, applied to new code first, then outward
- **Feature-test macros** (`__cpp_lib_generator`, `__cpp_lib_expected`) for the mixed-toolchain years; the exercise does this in three places
- **Sanitizers in CI**: ASan/UBSan on every test, TSan on every concurrent test, as separate jobs
- **The support matrix** (`handouts/toolchain-support-matrix.md`): a living document; add a row when a feature is gated, delete it when the floor rises

<!--
Notes: The one to underline is the flag progression: a CI job on the next standard, allowed to
fail, is how you find out what the switch will cost before you commit to it.
-->

---

<!-- _class: feature -->

## Writing the coding-standard update <span class="badge cpp23">C++23</span>

<p class="problem">One page. Three verbs.</p>

| Verb | Applies to | Example lines |
|---|---|---|
| **Mandate** | Tier 1 | "New code uses `make_unique`; `new` outside a constructor is a review comment." "Value-returning functions are `[[nodiscard]]`." |
| **Allow** | Tier 2 | "`std::expected` is the error type for new interfaces; existing `bool` + out-parameter APIs are converted when touched." |
| **Pilot** | Tier 3 | "Ranges pipelines in `telemetry/report` through Q4; a decision by the retrospective." |

Reference the **C++ Core Guidelines** by rule number (R.11, F.20, ES.20) instead of restating them. Reject anything that needs more than one page: it will not be read.

<!--
Notes: Attendees have seen thirty-page coding standards. The point is that a one-page document
with three verbs gets applied in review; anything longer gets linked and ignored.
-->

---

<!-- _class: demo -->

## The worksheet

Open `handouts/adoption-roadmap-template.md`. Eight minutes:

1. Name a codebase you own and its current `-std=` flag
2. Fill in the **tier 1** column: for each row, where (a directory or module), who, and whether clang-tidy can do it
3. Pick **one** tier 2 row and **one** tier 3 row, and write the pilot area

Then three volunteers read theirs. The rest of us ask one question each: **what would break?**

<!--
Notes: Exercise task 8. Circulate. Push back on anyone with more than one tier 3 item; push
back on anyone with zero tier 1 items (there is always a `new`). The "what would break"
question is the review the roadmap needs before it becomes a ticket.
-->

---

<!-- _class: feature -->

## The capstone <span class="badge cpp23">C++23</span>

`exercises/capstone/README.md`:

1. Pick **300 to 500 lines** of code you own, with tests, or write tests first
2. Apply **tier 1**, then **tier 2**, one commit per feature
3. Deliver a **before/after diff** and **one paragraph**: what changed, what the tests said, what you would not do again

Two weeks. Bring the paragraph to the retrospective. The course's five exercises are the worked example: the same program, five diffs, one unchanged report.

<!--
Notes: The capstone is what makes the course stick. Small enough to finish, real enough to
matter. Offer to review the diffs.
-->

---

<!-- SEGMENT: Close (1:50) -->

<!-- _class: feature -->

## What is coming in C++26 <span class="badge cpp23">C++23</span>

| Feature | What it is | Watch or wait |
|---|---|---|
| **Reflection** (P2996) | `^^T`, `[: :]`, `std::meta`: enumerate members, generate code at compile time | Watch: the biggest change since templates; serialization and enum-to-string first |
| **Contracts** | `pre`, `post`, `contract_assert` with enforcement modes | Watch: the replacement for `assert` and for half of your comments |
| **`std::execution`** (P2300) | Senders, receivers, schedulers; the async model | Wait for a library implementation (stdexec) to settle |
| **Hardened standard library** | Bounds-checked `operator[]` and friends as a build mode | Turn on the day your library ships it |
| **`std::inplace_vector`**, **`std::hive`** | Fixed-capacity vector; a stable-address bucket container | Use: embedded and game codebases have wanted these for years |
| **`#embed`**, `constexpr` exceptions, `std::simd` | Binary resources; `throw` in constant evaluation; portable SIMD | Use when available |

<!--
Notes: Reflection and contracts are the two that change how code is written; execution is the
one that changes this session's concurrency story. Hardened library is the free win. C++26 is
feature-complete as of 2025 and the first compiler support is landing; check the cppreference
table.
-->

---

<!-- _class: feature -->

## Where to follow the language <span class="badge cpp23">C++23</span>

- **cppreference compiler support table**: the first page to open when a feature does not compile
- **isocpp.org**: WG21 trip reports after each meeting (three a year); what passed and what did not
- **The papers**: `wg21.link/pNNNN` for anything on today's slides (P2300 execution, P2996 reflection, P2502 generator)
- **Talks**: the CppCon and ACCU list in the syllabus; one talk per session topic
- **Tooling**: Compiler Explorer for "does this compile on X", the `-std=c++26` CI job for "does our code"
- **This repo**: `handouts/feature-timeline.md`, the support matrix, the cheat sheets, and five exercises with solutions

<!--
Notes: The cppreference support table answers most questions in this course's Q&A. The papers
are readable; the abstracts and the motivation sections are written for practitioners.
-->

---

<!-- _class: takeaway -->

## Course takeaway

The program is **unchanged**. Five sessions, five diffs, one report, byte for byte.

Everything about how it is written is different: **ownership in the type**, **errors in the return**, **work at compile time**, **algorithms without iterator pairs**, and **threads that join and cancel themselves**.

Monday morning: one tier 1 item, one clang-tidy run, one CI job on the next standard.

<!--
Notes: End on the five phrases, one per session. Thank them, point at the capstone, offer to
review diffs.
-->
