# Session 1 exercise: modernize the syntax

Goal: carry a well-written C++11 program to modern syntax without changing its behavior. The test suite in `tests/` must pass before and after.

Build and test:

    cmake -S ../.. -B ../../build && cmake --build ../../build && ctest --test-dir ../../build -R s01

In class (about 20 minutes):

1. Replace every `typedef` with `using`.
2. Replace the hand-written `operator==` and `operator<` on `Record` with a defaulted `operator<=>`. Check the tests still pass.
3. In `count_by_sensor`, replace the `std::pair<iterator,bool>` idiom with a structured binding inside an `if` with initializer.

At home:

4. Add `[[nodiscard]]` to every value-returning function. Build; fix whatever the compiler now flags.
5. Replace `strtoll`/`strtod` in `parse_record` with `std::from_chars` (C++17) and structured bindings on its result.
6. Replace the iterator loop in `count_by_sensor` with a range-based `for`.

Hints: `operator<=>` needs `<compare>`; `from_chars` needs `<charconv>` and, for `double`, GCC 11+ or Clang 14+ with libc++ 20+.

The `solution/` directory is the Session 2 starter, so bring your version or use ours next time.
