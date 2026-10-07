# Session 2 exercise: vocabulary types

## The program

`starter/` is the Session 1 solution: the telemetry processor with modern syntax but C++11 interfaces. Look at how information crosses function boundaries today:

- `bool parse_record(const std::string&, Record* out, ParseError* err)`: a success flag plus two out-parameters
- `const SensorConfig* find_sensor(const std::string&)`: a pointer that might be null
- `std::pair<double, double> value_range(const std::vector<Record>&)`: a precondition in a comment ("must not be empty")
- `std::uint16_t crc16(const unsigned char*, std::size_t)`: a pointer and a length
- `std::fprintf(out, "%-16s %lu", ...)`: a format string the compiler cannot check

This session replaces each with the type that says what it means. Behavior must not change: the CTest `s02_report_identical` diffs your program's report against the starter's on `data/sample.csv`.

Build and test:

    cmake -S ../.. -B ../../build && cmake --build ../../build
    ctest --test-dir ../../build -R s02 --output-on-failure

No local toolchain? The whole program also runs on Compiler Explorer as a single file with `data/sample.csv` on stdin: [starter](https://godbolt.org/z/e7Pvo6v15) and [solution](https://godbolt.org/z/G6xEfTEhb). Edit there to experiment; the tests still need the repo build.

Because the interfaces change, the tests change too. `solution/tests/` is the target suite; write toward it. (Clang users: the repo's CMake selects libc++, which is where Clang 18 has `std::expected` and `std::print`.)

## In class (about 15 minutes)

1. **`std::expected` for the parser.** Change `parse_record` to `std::expected<Record, ParseError> parse_record(std::string_view line)` and delete `ParseError::None` (an error enum no longer needs a "not an error" value). Each early `return false` becomes `return std::unexpected(Reason)`. Update `load_stream`: `if (auto parsed = parse_record(line))` then `parsed.error()` on the else branch.

2. **`std::optional` for lookups.** `find_sensor` returns `std::optional<SensorConfig>` instead of a pointer; `status_from_string(text, &out)` becomes `std::optional<Status> parse_status(std::string_view)`. In the parser, `if (!cfg) return std::unexpected(UnknownSensor);` reads the way the sentence does.

3. **`std::print` in the report.** Replace every `fprintf` in `report.cpp` with `std::println(out, ...)`. Translate the format specs: `%-16s` is `{:<16}`, `%.3f` is `{:.3f}`, `%04X` is `{:04X}`, `%lu` is just `{}` (no more casts to `unsigned long`). Run the diff test after this one; a single spacing difference will fail it, which is the point.

## At home

4. **`std::formatter` specializations.** Add `std::formatter<Status>`, `std::formatter<ParseError>` (both can inherit from `std::formatter<std::string_view>`), and `std::formatter<Record>` producing `{ts,"sensor",value,status}`. Then `serialize(const Record&)` is one line: `std::format("{}", v)`. Make the `format` member a template on the context type; libc++ requires it and the standard allows any `basic_format_context`.

5. **`std::string_view` at every read-only boundary.** `split` should return `std::vector<std::string_view>` into the caller's buffer; `parse_number` and `find_sensor` should take views. Then the parser never allocates until it builds the `Record`. Note the one place a copy is still needed: the `strtod` fallback under `#ifndef __cpp_lib_to_chars`, because `strtod` needs a NUL terminator. That is the lesson: views are not C strings.

6. **`std::span` for every contiguous-input parameter.** `compute_stats`, `value_range`, `top_n_by_value`, `first_at_or_after` take `std::span<const Record>`; `crc16` takes `std::span<const std::byte>` with a `std::string_view` overload via `std::as_bytes`. Replace `sensor_count()`/`sensor_at(i)` with one `std::span<const SensorConfig> sensors()`. The tests pass a C array and a sub-span to prove the point.

7. **A precondition becomes a return type.** `value_range` returns `std::optional<std::pair<double, double>>`; the report's "no accepted records" branch now falls out of `if (!range)`.

8. **Monadic operations (C++23).** In the report, the units lookup becomes `find_sensor(name).transform([](const SensorConfig& c) { return c.units; }).value_or("")`. Try `&SensorConfig::units` as the projection first and read the compiler error: a pointer-to-member yields a reference to the member (rvalue on a temporary, lvalue on a named `optional`), and `optional` cannot hold a reference in C++23. In the tests, `parse_record(line).value_or(fallback)` and `.error_or(...)` show the `expected` side.

9. **Small things to notice.** `std::map<std::string, SensorStats, std::less<>>` lets `find("rpm")` work without constructing a `std::string`. `std::string::ends_with` and `contains` (C++20/23) replace the `line[line.size() - 1] == '\r'` idiom. `SensorConfig::name` can be a `std::string_view` because the table is `constexpr` and the strings are literals.

## Checking your work

`solution/` is our version. Its report is byte-identical to the starter's (the diff test) and its tests are the target for yours. It is the Session 3 starter.

## What you should notice

Nothing changed in what the program computes. What changed is that the function signatures now carry the information that used to live in comments, sentinel values, and out-parameters: a caller can read `std::expected<Record, ParseError>` and know everything `parse_record` can do.
