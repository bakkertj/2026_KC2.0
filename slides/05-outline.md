# Session 5 deck outline: Concurrency, Coroutines, Modules, and Adoption

One line per slide. Badge in brackets; "2col" is before/after; "demo" is live; "evo" is a timeline. Target: 52 content slides for 90 minutes, then the roadmap workshop and close. "From the exercise" slides pull code from `exercises/s05-concurrency/` starter and solution.

## 0. Opening (0:00, 5 slides)

1. Title
2. Agenda with minutes
3. Session 4 recap: the solution, and the three stumbles (`views::split("")`, the `const filter_view`, `views::enumerate` on libc++)
4. Why these three are last (feature): concurrency, coroutines, and modules change build systems, control flow, and program structure, not syntax; the exercise adds threads and a coroutine to the program and still prints the same report
5. The course so far in one slide (evo): the telemetry processor's journey from Session 1 to Session 5, with what each session removed and what it added

## 1. Concurrency, C++14 to C++23 (0:10, 16 slides)

6. What C++11 gave and what it lacked (feature): `thread`, `mutex`, `condition_variable`, `atomic`, `future`; no cancellation, no join-on-destruction, no semaphores, no latch, a `thread` destructor that terminates
7. C++14 and C++17: `shared_timed_mutex` [14], `shared_mutex` [17] (2col): a reader/writer lock for the sensor table; `std::shared_lock` vs `std::unique_lock`
8. `std::scoped_lock` [17] (feature): locking two mutexes without deadlock, in one line; `std::lock` underneath; CTAD makes it `std::scoped_lock lk{m1, m2};`
9. `hardware_destructive_interference_size` [17] and false sharing (feature): `alignas(std::hardware_destructive_interference_size)` on per-thread counters; why two adjacent `atomic<int>`s are slow
10. `std::jthread` [20] (2col, from the exercise): `std::thread` + manual `join` vs `jthread` that joins in its destructor; the lambda receives a `stop_token` as its first parameter
11. Cooperative cancellation: `stop_token`, `stop_source`, `stop_callback` [20] (feature, from the exercise): the producer checks `stop_requested()` per line; the caller's token propagates via a `stop_callback`; why polling a token beats a `bool` flag (memory ordering, composition)
12. `std::counting_semaphore` and `binary_semaphore` [20] (feature, from the exercise): the bounded queue in thirty lines; `acquire`/`release`/`try_acquire_for`; `ptrdiff_t` counts; what C++11 needed instead (`condition_variable` and a predicate)
13. The bounded queue (2col, from the exercise): the C++11 `condition_variable` version vs the C++20 semaphore version; where each blocks; the `close()` wake-up trick
14. `std::latch` and `std::barrier` [20] (feature): one-shot countdown for "wait for N workers to start"; reusable barrier with a completion function for phased computation; a fan-out/fan-in over the sensor list
15. `atomic::wait`/`notify_one`/`notify_all` [20] and `std::atomic_ref` [20] (feature): a futex in the standard library; wait on a value change without a mutex; `atomic_ref` for atomic access to a plain `int` in a struct you do not own
16. `std::atomic<std::shared_ptr>` [20] and the deprecation of the free `atomic_load(shared_ptr*)` functions (feature): lock-free-ish RCU-style config swapping; the sensor table as a hot-swappable `atomic<shared_ptr<const Table>>`
17. `std::osyncstream` [20] (feature): `std::osyncstream(std::cout) << ...` for interleaving-free logging from many threads; the libc++ `-fexperimental-library` note
18. `std::move_only_function` [23] and task queues (feature): a queue of tasks that own `unique_ptr`s; what `std::function` could not hold
19. What is still missing and where it is going (feature): no executors, no standard thread pool, no async I/O in C++23; `std::execution` (senders/receivers, P2300) in C++26; `std::async` and why not to use it
20. ThreadSanitizer as a test (demo, from the exercise): `-DCOURSE_SANITIZE=thread`, the `lines_read` race if `join()` is skipped, what the report looks like, why every concurrent test runs under TSan in CI
21. Segment takeaway: `jthread` + `stop_token` for lifetime and cancellation, semaphores for signaling, `latch`/`barrier` for phases, TSan for proof

## 2. Coroutines (0:40, 12 slides)

22. What a coroutine is (feature): a function that can suspend and resume; `co_await`, `co_yield`, `co_return` make a function a coroutine; the frame lives on the heap (usually); C++20 shipped the **language** machinery with no library types
23. Why they matter here (feature): lazy sequences (the parser), state machines (a protocol decoder), async I/O (without callbacks); what they do not do: they are not threads, and they do not make anything parallel
24. `std::generator` [23] (2col, from the exercise): the eager `load_stream` loop vs `records()` with `co_yield`; the result is an `input_range`, so it composes with every view from Session 4
25. Generator mechanics (feature, from the exercise): each `co_yield` suspends until the consumer asks; `take(2)` resumes the coroutine once more than it uses (the Session 4 lookahead); a coroutine destroyed while suspended never finishes; consequences for a coroutine holding a lock or a file
26. What the compiler generates (feature): the coroutine frame, the promise type, `initial_suspend`/`final_suspend`, `yield_value`, `return_void`; one slide of the machinery so `std::generator` is not magic
27. Awaitables and `co_await` (feature): `await_ready`, `await_suspend`, `await_resume`; a hand-written awaitable that suspends until a semaphore is released; why you should not hand-write coroutine types in production (use `std::generator`, a library like cppcoro or `folly::coro`, or C++26's `std::execution`)
28. A coroutine as a state machine (feature): a packet decoder that `co_await`s the next byte; the switch statement it replaces; where this pays in embedded protocol code
29. Coroutines and `expected` (feature): `co_await` on an `expected` (the Session 2 preview); an `expected`-returning coroutine that short-circuits on the first error, with a small custom promise; what C++26's `std::execution` does with this
30. Allocation and performance (feature): the frame allocation, HALO (heap allocation elision), when it happens (Clang, inlinable) and when it does not; `std::generator`'s allocator support; measuring a generator vs an eager loop on the exercise's parser
31. Availability (feature): `std::generator` on libstdc++ 14 only; the exercise gates it on `__cpp_lib_generator`; libc++ status; Compiler Explorer for a libc++ demo with a third-party generator
32. Where coroutines pay off and where they do not (feature): async I/O, lazy parsing, state machines: yes; hot inner loops, hard real-time with allocation limits: no, unless HALO is verified
33. Segment takeaway: coroutines are a control-flow tool; `std::generator` is the first standard use of it, and it is a range

## 3. Modules (1:05, 9 slides)

34. What modules fix (feature): header parsing cost (every TU re-parses `<vector>`), macro leakage, include-order dependence, ODR fragility; what they do not fix: they are not packages, not a build system, not faster link times
35. `export module`, `export`, `import` [20] (2col, from the demo): the `telemetry.crc` module interface unit vs the header; what is exported and what stays module-private (`kPolynomial`)
36. The global module fragment and `module;` (feature): `#include`s go above `export module`; why (the standard library is still headers until `import std`); the GCC 14 rule that `#include` must precede `import` in a consumer
37. Module partitions and implementation units (feature): `export module telemetry:crc;`, `module telemetry;` implementation units, how a large library is split; header units (`import <vector>;`) as the migration bridge
38. `import std;` [23] (feature): the whole standard library as one module; what it needs today (CMake 3.30+, libc++ 17+ or libstdc++ 15, MSVC 17.5+); measured compile-time wins
39. Build systems (feature): CMake 3.28's `FILE_SET CXX_MODULES`, the Ninja requirement, P1689 dependency scanning; why Make cannot do it; MSBuild and Meson status; the demo's `CMakeLists.txt`
40. Compiler status (dense): GCC 14 (works, `#include`-before-`import` rule, `-fmodules-ts`), Clang 16+ (works, best diagnostics), MSVC (most complete, `import std` first); the repo builds the demo on GCC 14 and Clang 18 with `-DCOURSE_MODULES=ON`
41. An adoption posture (feature): new leaf libraries as modules behind an option; header units for third-party headers; do not convert a working header-only library for its own sake; measure the build first; `import std` when your toolchain allows it
42. Segment takeaway: understand modules now, adopt them where the build system is ready, and expect `import std;` to be the first thing worth switching on

## 4. Deprecations and removals (1:25, 3 slides)

43. The cumulative list, C++14 through C++23 (dense): `auto_ptr`, `random_shuffle`, `std::iterator`, `throw()`, `bind1st`, `result_of`, `strstream`, `codecvt`, `volatile` compound ops (partially restored), `aligned_storage`, `atomic_init`, GC support, `std::async` (not deprecated, but); and their replacements
44. Finding them (demo): `-Wdeprecated`, clang-tidy `modernize-*` on the Session 1 starter vs the Session 5 solution (exercise task 7), the count as a metric
45. Segment takeaway

## 5. The adoption roadmap workshop (1:30, 5 slides)

46. Three tiers (feature): tier 1 mechanical and zero-risk (`make_unique`, `[[nodiscard]]`, structured bindings, `string_view` at boundaries, `optional` for sentinels, `using`, `<=>`); tier 2 interface changes (`span`, `expected`, concepts on public templates, `format`/`print`, `constexpr` tables); tier 3 architectural (ranges pipelines, `jthread` and stop tokens, coroutines, modules)
47. Tooling as a force multiplier (feature): clang-tidy `modernize-*` and `performance-*` with `-fix`; compiler flag progression (`-std=c++17` then `20` then `23`, warnings as errors per directory); feature-test macros for mixed toolchains; the support matrix as a living document
48. Writing the coding-standard update (feature): what to mandate (tier 1), what to allow (tier 2), what to pilot (tier 3); the C++ Core Guidelines as the reference; one page, not thirty
49. The worksheet (demo): `handouts/adoption-roadmap-template.md`; attendees fill in the tier-1 column for a codebase they own, ten minutes, then three volunteers read theirs
50. The capstone (feature): `exercises/capstone/README.md`, 300 to 500 lines of your own code, tiers 1 and 2, a before/after diff and one paragraph

## 6. Close (1:50, 3 slides)

51. What is coming in C++26 (feature): reflection (P2996), contracts, `std::execution`, hardened standard library, `std::inplace_vector`, `constexpr` exceptions, `#embed`; what to watch and what to wait for
52. Where to follow the language (feature): cppreference support table, WG21 papers and trip reports, the CppCon and ACCU talks from the syllabus reading list, isocpp.org
53. Course takeaway: the program is unchanged; everything about how it is written is different; the five "Monday morning" items from each session as the summary

## Demo files needed (`demos/s05/`)

- `shared_mutex.cpp`, `scoped_lock.cpp`, `false_sharing.cpp`, `jthread.cpp`, `stop_token.cpp`, `semaphore_queue.cpp` (the C++11 `condition_variable` version for the 2col), `latch_barrier.cpp`, `atomic_wait.cpp`, `atomic_shared_ptr.cpp`, `osyncstream.cpp`, `move_only_function_queue.cpp`, `tsan_race.cpp` (a deliberate race under `SHOW_ERRORS`, for the TSan demo)
- `generator_basics.cpp` (gated), `coroutine_machinery.cpp` (a minimal hand-written generator to show the promise type), `awaitable.cpp`, `state_machine.cpp`, `generator_perf.cpp`
- `modules/` (exists), plus `modules/partition.cppm` for slide 37 if time allows
- `deprecated.cpp` (uses under `SHOW_ERRORS` with `-Wdeprecated`)
- Exercise excerpts from `exercises/s05-concurrency/solution`

## Cut list

- Slide 16 (`atomic<shared_ptr>`) into 15
- Slide 29 (coroutines and `expected`) if the coroutine segment runs long
- Slide 37 (partitions) into 36
- Slide 9 (false sharing) if the concurrency segment runs long
