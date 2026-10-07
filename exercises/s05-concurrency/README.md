# Session 5 exercise: concurrency and coroutines

## The program

`starter/` is the Session 4 solution. It is single-threaded: `main` calls `load_stream`, then `compute_stats`, then `write_report`. This session splits the first two across a producer thread and a consumer thread, exposes the parser as a coroutine, and runs the whole thing under ThreadSanitizer. The report must stay byte-identical (`ctest -R s05_report_identical`).

    cmake -S ../.. -B ../../build && cmake --build ../../build
    ctest --test-dir ../../build -R s05 --output-on-failure

No local toolchain? The whole program also runs on Compiler Explorer as a single file with `data/sample.csv` on stdin: [starter](https://godbolt.org/z/5cKfGvs51) and [solution](https://godbolt.org/z/db9jsTc5M). Edit there to experiment; the tests still need the repo build.

Then, and this is the part that matters today:

    cmake -S ../.. -B ../../build-tsan -DCOURSE_SANITIZE=thread
    cmake --build ../../build-tsan && ctest --test-dir ../../build-tsan -R s05 --output-on-failure

A data race is a test failure. Clang users: the repo's CMake adds `-fexperimental-library` for libc++ 18, which is where `std::jthread` and `std::stop_token` live on that library.

## At home, first (the core of the exercise)

Unlike the earlier sessions, the in-class exercise today is the roadmap workshop (task 8, about 20 minutes on the clock). Tasks 1 to 3 are the heart of this exercise and are done at home; the next session's solution walk-through covers them.

1. **A bounded queue on semaphores.** Write `BoundedQueue<T, Capacity>` in a new `queue.h`: a `std::mutex`, a `std::queue<T>`, and two `std::counting_semaphore`s, `slots` (initialized to `Capacity`, the producer acquires one per push) and `items` (initialized to 0, the consumer acquires one per pop). `push` blocks when full; `pop` blocks when empty. Add `close()`: mark closed, release one extra `items` count so a blocked `pop` wakes, and have `pop` return `std::nullopt` once closed and drained. Note that `counting_semaphore`'s count is a `std::ptrdiff_t`, and the course's `-Wsign-conversion` will tell you if you forget.

2. **Producer and consumer on `std::jthread`.** In a new `pipeline.h`/`.cpp`, write `PipelineResult load_and_compute(std::istream&)`. The producer is a `std::jthread` that reads lines, parses them, and pushes a `std::variant<Record, ParseError>` (one queue, one message type, Session 2). The consumer is the calling thread: `while (auto m = queue.pop())`, `std::visit`, accumulate stats and records. Join the producer before reading anything it wrote (`lines_read`). Swap `main` over to it and run the diff test: the records must come out in input order.

3. **Run it under ThreadSanitizer.** Configure with `-DCOURSE_SANITIZE=thread` and run the tests. If you wrote `lines_read` from the producer and read it before `join()`, TSan says so. Fix it and note what the fix was (the join is a synchronization point).

## At home, after that

4. **Cooperative cancellation.** Add a `std::stop_token` parameter to `load_and_compute` and to `pop`. The producer's lambda takes a `std::stop_token` as its first parameter (a `jthread` passes its own automatically) and checks it per line. A `std::stop_callback` on the caller's token requests the producer's stop and closes the queue, so a stop from outside unwinds both threads. `pop` polls with `try_acquire_for` so a stop is noticed while blocked. The test requests a stop before starting and checks that fewer than all records arrive.

5. **The parser as a coroutine.** Where `__cpp_lib_generator` is defined (libstdc++ 14; not libc++ 18), write `std::generator<Record> records(std::istream&)`: a loop that `co_yield`s each accepted record. It is an `input_range`, so `records(in) | std::views::take(2)` works. The test checks how many lines were consumed: `take(2)` resumes the coroutine once more than the two records it uses (the same lookahead as Session 4's laziness demo), and the coroutine is then destroyed while suspended, never finishing. Think about what that means for a coroutine holding a lock or a file.

6. **Modules, experimentally.** `demos/s05/modules/` has `telemetry.crc` as a named module and a `main.cpp` that imports it. Build it with `cmake -G Ninja -DCOURSE_MODULES=ON`. On GCC 14, move the `#include <print>` in `main.cpp` after the `import` and read the error: includes must precede imports in that compiler. Then read `CMakeLists.txt` there for what `import std;` would additionally need.

7. **clang-tidy.** Run `clang-tidy -p build --checks='modernize-*' solution/src/*.cpp` (the repo's `.clang-tidy` already selects the checks). Fix what it flags. Then run it on the Session 1 starter and compare the count: that difference is this course.

8. **Your roadmap (in class).** Open `handouts/adoption-roadmap-template.md` and fill in the tier-1 column for a codebase you own. That is the last exercise of the course and the only one without a solution.

## Checking your work

`solution/` is our version: the Session 4 suite plus six concurrency and coroutine tests, clean under ThreadSanitizer on GCC 14 and Clang 18, and the same report as Session 1.

## What you should notice

The queue is thirty lines because `counting_semaphore` and `jthread` exist. Cancellation is a parameter, not a flag you invented. The coroutine version of the parser is shorter than the eager one and composes with every view from Session 4. And ThreadSanitizer turned "I think this is race-free" into a test result. The program still prints exactly what the C++11 version printed in Session 1.
