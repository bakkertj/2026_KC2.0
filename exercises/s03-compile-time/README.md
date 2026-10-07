# Session 3 exercise: compile time and concepts

## The program

`starter/` is the Session 2 solution. Three things in it were placed there for today:

- `crc.cpp` builds a 256-entry lookup table **at first use**, at runtime, behind a function-local static
- `config.cpp` holds the sensor table, and nothing checks that it is well-formed (unique names, `min_valid < max_valid`) until a bad entry misbehaves in production
- `serialize.h` is seven overloads plus two templates, and passing it something unsupported produces a page of candidates

This session moves the first two to compile time and turns the third into constrained templates. The report must stay byte-identical (`ctest -R s03_report_identical`).

    cmake -S ../.. -B ../../build && cmake --build ../../build
    ctest --test-dir ../../build -R s03 --output-on-failure

No local toolchain? The whole program also runs on Compiler Explorer as a single file with `data/sample.csv` on stdin: [starter](https://godbolt.org/z/oz4o9cMrf) and [solution](https://godbolt.org/z/G6a5Wjzv4). Edit there to experiment; the tests still need the repo build.

The target tests are in `solution/tests/`. Most of the new ones are `static_assert`s: if the file compiles, they passed.

## In class (about 20 minutes)

1. **The CRC table at compile time.** Write `constexpr std::array<std::uint16_t, 256> make_crc_table()` (the same loop as the `CrcTable` constructor; C++14 relaxed `constexpr` allows the loops and locals) and replace the function-local static with `inline constexpr auto kCrcTable = make_crc_table();` in the header. Make `crc16` itself `constexpr`. Then add `static_assert(crc16("123456789") == 0x29B1);`. One snag you will hit: `std::as_bytes` is not `constexpr`, because it is a `reinterpret_cast`. Write the core over `std::span<const Byte>` for any one-byte `Byte` and use `static_cast<unsigned char>` instead.

2. **Validate the configuration table at compile time.** Move `kSensors` into `config.h` as `inline constexpr std::array<SensorConfig, 6>`. Write `consteval std::string_view validate(std::span<const SensorConfig>)` that returns an empty view for a good table and a message for a bad one (empty name or units, `min_valid >= max_valid`, duplicate names). Then `static_assert(validate(kSensors).empty(), "sensor configuration table is invalid");`. Break the table on purpose and read the error.

3. **One `serialize`, constrained.** Replace the integer overloads with `std::string serialize(std::integral auto v)` and the `double` overload with `std::floating_point auto`. Write a `StringLike` concept (`std::convertible_to<T, std::string_view>`) and a `requires`-clause overload for it. Check that `serialize(42)`, `serialize(2.5)`, `serialize("x")` still pick the right one; the tests say what each must produce.

## At home

4. **Finish the concept set.** A `SerializableRange` concept (`begin()`/`end()`, and not `StringLike`, or a `std::string` call would be ambiguous between the string overload and the container overload, since neither constraint subsumes the other) for the container overload; a `requires std::same_as<T, Status> || std::same_as<T, Record>` overload for the formatter-backed types. Then define `Serializable<T>` as "some `serialize` overload accepts a `T` and returns `std::string`" and `static_assert` a few types in and out. `static_assert(!Serializable<ParseError>)` is the one that shows what concepts buy you: a question about your API, answered by the compiler.

5. **A fold expression.** `serialize_all(const Ts&... vs)` producing `[a, b, c]` for any number of serializable arguments. The comma-fold `((out += sep, out += serialize(vs), sep = ", "), ...)` is the whole implementation.

6. **`constexpr` lookups.** Make `find_sensor`, `sensors`, `to_string(Status)` and `parse_status` `constexpr` (they can live in the headers now). Then `static_assert(find_sensor("rpm")->max_valid == 12'000.0);`. `config.cpp` and `record.cpp` become empty; delete them.

7. **Deducing `this` (C++23).** Rewrite `SensorStats::add` as `template <typename Self> constexpr Self&& add(this Self&& self, double, Status)` returning `std::forward<Self>(self)`. One definition now serves `stats.add(v, s).add(v, s)` on an lvalue (returns `SensorStats&`) and `SensorStats{}.add(v, s).add(v, s)` on a temporary (returns `SensorStats&&`, no copy). Make it `constexpr` and evaluate a three-sample accumulator in a `static_assert`.

8. **Read the diagnostics.** Call `serialize(ParseError::EmptyLine)` once in the starter and once in the solution and compare the two error messages. The solution's names the concept that was not satisfied. That difference is the reason concepts exist.

## Checking your work

`solution/` is our version: 25 tests, of which about 30 assertions are `static_assert`s, plus the report-diff test. It is the Session 4 starter.

## What you should notice

Three functions ran at startup or per call and now run at build time, and one class of bug (a bad configuration table) can no longer reach a running program. The generic code got shorter and its error messages got readable. Nothing about the program's output changed.
