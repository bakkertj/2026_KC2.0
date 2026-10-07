# Session 1 exercise: modernize the syntax

## The program

`starter/` is a small telemetry record processor written in careful C++11. It reads lines of the form

    <timestamp>,<sensor>,<value>[,<status>]

validates each against a sensor configuration table, computes per-sensor statistics, and prints a report. Try it:

    cmake -S ../.. -B ../../build && cmake --build ../../build
    ../../build/exercises/s01-modernize-syntax/s01_starter data/sample.csv

The same program is the exercise for every session of this course. This session changes only syntax. The behavior must not change, and the test suite in `tests/` is how you prove it:

    ctest --test-dir ../../build -R s01 --output-on-failure

No local toolchain? The whole program also runs on Compiler Explorer as a single file with `data/sample.csv` on stdin: [starter](https://godbolt.org/z/Tov79e9h1) and [solution](https://godbolt.org/z/bbahdxjPd). Edit there to experiment; the tests still need the repo build.

The starter is compiled as C++11 (see `CMakeLists.txt`). Once you start using C++14 or later features you will need to change its standard to 23, or work in a copy of `starter/` registered as its own variant.

## In class (about 20 minutes)

Do these in order. Run the tests after each one.

1. **`typedef` to `using`.** There are three: `Timestamp` in `record.h`, `StatsBySensor` in `stats.h`, and any others you find. Same meaning, reads left to right.

2. **Six comparison operators to one.** `record.h` and `record.cpp` declare and define `==`, `!=`, `<`, `>`, `<=`, `>=` for `Record`, ordering by timestamp, then sensor, then value, then status. That is exactly member order. Replace all six with a single defaulted `operator<=>` inside the struct and delete the definitions. You will need `<compare>`. The comparison test case is your proof.

3. **Structured bindings and `if` with initializer.** `load_stream` in `parser.cpp` and `compute_stats` in `stats.cpp` both use the `std::pair<iterator, bool>` returned by `map::insert`. Rewrite each with `auto [it, inserted] = ...`. In `load_stream`, put the binding in the `if` initializer so the names are scoped to the branch. Bonus: `try_emplace` (C++17) does what `insert(make_pair(...))` does without constructing a value you may throw away.

## At home

4. **`[[nodiscard]]`.** Add it to every function whose return value is the whole point of calling it (parsers, lookups, statistics, `serialize`, `crc16`). Build, and look at anything the compiler now flags. Then decide whether `write_report` should have it (it returns `void`, so no) and whether `status_from_string` should (it returns a success flag, so yes).

5. **`std::from_chars`.** Replace `strtoll` and `strtod` in `parser.cpp` with `std::from_chars` (C++17, `<charconv>`). Note what disappears: `errno`, the C string, the `end` pointer dance. The result's `ptr` and `ec` members are made for a structured binding. If you make it a function template, one function handles both timestamps and values. (`from_chars` for `double` needs libstdc++ 11+ or libc++ 20+.)

6. **Range-based `for` and lambdas.** Every iterator loop in the starter can be a range-based `for`. The two function objects in `stats.cpp` (`ValueDescending`, `TimestampLess`) can be lambdas; make the sort comparator a generic lambda (`const auto&` parameters, C++14).

7. **Inline variables.** `kMaxLineLength` and `kTopReadings` are declared `extern const` in a header and defined in a `.cpp`. Make each an `inline constexpr` in the header (C++17) and delete the definitions.

8. **`using enum`.** The `switch` statements over `Status` and `ParseError` repeat the enum name on every case. Add `using enum Status;` (C++20) at the top of the function and drop the qualifiers.

9. **Small things to notice.** `SensorStats` has a constructor that only sets defaults: use default member initializers instead. `sensor_count()` uses the `sizeof(a)/sizeof(a[0])` idiom: use `std::size` (C++17). The CRC polynomial and the rpm range are good places for binary literals and digit separators (C++14).

## Checking your work

`solution/` is our version. It passes the same tests and its report output is byte-identical to the starter's (`diff` them on `data/sample.csv`). It is also the starter for Session 2, so bring your own version next time or start from ours.

## What you should notice

Nothing in this session changed what the program does. Every change removed something: a duplicated name, five operator definitions, a `.first`/`.second` pair, an `errno` check, a function object, an `extern`/definition pair. That is the pattern for the rest of the course.
