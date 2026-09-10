# Session 2 deck outline: Vocabulary Types and the Standard Library

One line per slide. Badge in brackets; "2col" is before/after; "demo" is live; "evo" is a timeline slide. Target: 58 content slides for 90 minutes, then the exercise. Every "from the exercise" slide pulls its code from `exercises/s02-vocabulary-types/` starter and solution.

## 0. Opening (0:00, 5 slides)

1. Title
2. Agenda with minutes
3. Session 1 recap: what the solution looks like, the three things people got stuck on
4. The idea of a vocabulary type: a type that appears in interfaces so caller and callee agree without a comment; the Session 1 starter's five interface smells (bool + out-params, nullable pointer, precondition comment, pointer + length, printf)
5. Ownership vocabulary in one table: `T&`, `T*`, `unique_ptr`, `shared_ptr`, `string_view`, `span`, `optional`, `expected`: what each says about ownership, nullability, and lifetime

## 1. Views over data (0:10, 11 slides)

6. `std::string_view` [17] (2col, `split` from the exercise): a pointer and a length; the signature that accepts a literal, a `std::string`, and a slice with no allocation
7. `string_view` operations: `substr`, `remove_prefix`, `starts_with`/`ends_with` [20], `contains` [23], `find`; everything is O(1) or a view [17]
8. The dangling rule (feature): a view is a reference; four ways to dangle (`sv = std::string(...)` temporary, returning a view of a local, storing a view in a struct, `sv.data()` as a C string) [17]
9. `string_view` is not a C string (feature): no NUL terminator; the `strtod` fallback from the exercise; `std::string(sv)` when you must [17]
10. When to still take `const std::string&`: you will store it, you need `c_str()`, you pass it on to something that wants a `std::string`; and when to take `std::string` by value (you will keep it: sink parameter, then move) [17]
11. `std::span` [20] (2col, `value_range` from the exercise): `const std::vector<Record>&` said "a vector" when it meant "some records"; span accepts vector, array, sub-range
12. `span` mechanics: static vs dynamic extent, `first`/`last`/`subspan`, `as_bytes`/`as_writable_bytes`, `span<const T>` vs `span<T>` [20]
13. `span` in an embedded interface (feature): register maps, DMA buffers, `crc16(std::span<const std::byte>)` from the exercise, `std::byte` [17] as "raw memory, not a number"
14. `std::mdspan` preview [23]: a multidimensional view over the same contiguous memory; layout policies; one slide, matrix from a flat buffer (demo on Compiler Explorer with a trunk compiler if libstdc++ 14 lacks it)
15. Transparent comparators [14] and heterogeneous lookup: `std::map<std::string, T, std::less<>>::find(string_view)` without constructing a key; `unordered_map` needs C++20 for the same
16. Views takeaway: the lifetime rule is the whole cost of the feature

## 2. Maybe, either, anything (0:35, 12 slides)

17. `std::optional` [17] (2col, `find_sensor` from the exercise): a nullable pointer said "maybe" and "look elsewhere for the object"; optional says only "maybe"
18. `optional` mechanics: `has_value`/`operator bool`, `*`/`->` (unchecked), `value()` (throws), `value_or`, `std::nullopt`, `emplace`, `reset`; it holds the object inline, no allocation [17]
19. `optional` as a return type vs as a member vs as a parameter: return yes; member sometimes (a lazily computed field); parameter rarely (overloads are clearer) [17]
20. Monadic `optional` [23] (2col, the units lookup from the report): `and_then`, `transform`, `or_else` replace nested ifs; the pointer-to-member trap on a temporary
21. `std::variant` [17] (feature): a type-safe tagged union; `std::get`, `std::get_if`, `index`, `holds_alternative`; what it replaces (a `kind` enum plus a `union`, or `void*`)
22. `std::visit` and the overload-set visitor [17] (feature): `overloaded{ [](int){...}, [](std::string){...} }` with CTAD; exhaustive by construction
23. Variant in practice (feature): a message type for a telemetry link (`std::variant<Reading, Heartbeat, Fault>`); when `visit` beats a virtual hierarchy and when it does not (open vs closed set of types)
24. `variant` details worth knowing: `valueless_by_exception`, `std::monostate` for "empty", default constructs the first alternative, comparison and hashing work if the alternatives support them [17]
25. `std::any` [17]: type-erased anything; `any_cast`; heap-allocates for large types; the honest advice (plugin boundaries, scripting bridges, and not much else)
26. Choosing between them (table): optional = maybe one T; variant = exactly one of a fixed set; any = anything, you check at runtime; pointer = someone else owns it
27. Evolution slide (evo): where these came from (Boost.Optional 2003, Boost.Variant 2002, proposals through 2016) and why it took until C++17
28. Segment takeaway

## 3. Error handling: `std::expected` (1:00, 8 slides)

29. The error-handling landscape (feature): exceptions, error codes, `bool` + out-param, `optional` with no reason, `std::error_code` [11]; what each loses
30. `std::expected<T, E>` [23] (2col, `parse_record` from the exercise): the return type that carries the value or the reason
31. `expected` mechanics: `has_value`/`bool`, `*`/`->`, `value()` (throws `bad_expected_access<E>`), `error()`, `value_or`, `error_or` [23], `std::unexpected` [23]
32. Monadic `expected` (feature): `and_then` to chain parsers, `transform` on the value, `transform_error` to map error types across layers, `or_else` to recover; a three-stage pipeline in one expression [23]
33. Designing the error type: an `enum class` (the exercise), a struct with context, `std::error_code`, or a `variant` of error kinds; `[[nodiscard]]` on `expected`-returning functions [23]
34. `expected` vs exceptions (feature): the honest comparison; where exceptions still win (constructors, deep call stacks, truly exceptional), where `expected` wins (expected failures, hot paths, `-fno-exceptions` environments, embedded and safety-critical); what `expected` costs (size, the `if` at every call)
35. `expected<void, E>` and the "did it work" case; interaction with coroutines preview [23]
36. Segment takeaway: `expected` is the missing piece that makes "no exceptions" a defensible policy instead of a workaround

## 4. Formatting and output (1:15, 9 slides)

37. Why `printf` and `iostream` both lost (feature): `printf` is unchecked and non-extensible; `iostream` is stateful (`std::hex` sticks), slow, and verbose
38. `std::format` [20] (2col, `serialize(double)` from the exercise): compile-time-checked format strings, positional and named-by-index args, `{:.3f}`, `{:<16}`, `{:04X}`, `{:>8}`; the spec mini-language in one table
39. `std::print` and `std::println` [23] (2col, the report from the exercise): `printf` ergonomics with `format` safety; the `FILE*` and `ostream` overloads; Unicode-correct on Windows
40. `std::formatter<T>` for your own types [20] (feature, `Record` from the exercise): inherit from `formatter<string_view>` for enum-like types; write `parse` and `format` for the rest; `format` as a template on the context
41. Format specs for your type (feature): parsing `{:>10}` or a custom spec in `parse`, and forwarding to a nested formatter
42. Formatting ranges [23]: `std::println("{}", vec)` prints `[1, 2, 3]`; `{::.3f}` applies a spec to elements; maps and pairs; `std::format_join`-style options via `std::ranges::views`
43. `std::format_to`, `format_to_n`, `formatted_size`, `vformat` [20]: formatting into a buffer without allocation (embedded), runtime format strings, and why `std::runtime_format` [26] is coming
44. Migration (feature): `printf` to `println` mechanically (spec translation table); `iostream` to `print` (drop the manipulators); a `clang-tidy modernize-use-std-print` mention
45. Segment takeaway

## 5. Library tour (1:30, 10 slides, fast)

46. C++17 tour, part 1: `std::filesystem` (path, `exists`, `directory_iterator`, `file_size`; the `main.cpp` file-open could use it) [17]
47. C++17 tour, part 2: `std::byte`, `to_chars`/`from_chars` (seen), `std::invoke`, `std::apply`, `std::clamp`, `std::gcd`/`lcm`, `std::size`/`data`/`empty`, `std::as_const`, `std::not_fn` [17]
48. C++17 tour, part 3: map `try_emplace`/`insert_or_assign` (seen), `extract`/`merge`, `std::sample`, `std::reduce`/`transform_reduce` preview for Session 4 [17]
49. C++20 tour, part 1: `<bit>` (`std::bit_cast`, `popcount`, `rotl`, `has_single_bit`, `std::endian`); `std::source_location` for logging without macros [20]
50. C++20 tour, part 2: `std::erase`/`erase_if` for every container; `starts_with`/`ends_with`; `contains` for associative containers; `std::ssize`; `std::to_array`; `std::midpoint`/`lerp`; `std::numbers::pi` [20]
51. C++20 tour, part 3: `<chrono>` calendars and time zones: `year_month_day`, `sys_days`, `zoned_time`, formatting timestamps with `std::format("{:%F %T}")`; the exercise's raw `long long` timestamp could be `sys_time<milliseconds>` [20]
52. C++23 tour, part 1: `std::flat_map`/`flat_set` (sorted vectors with a map interface; cache-friendly; the sensor table as a `flat_map`); `std::stacktrace` (`std::stacktrace::current()` in an error path); `std::move_only_function` [23]
53. C++23 tour, part 2: `std::string::contains` (seen), `resize_and_overwrite`, `std::byteswap`, `std::out_ptr`/`inout_ptr` for C APIs that fill a pointer, `std::forward_like`, `std::to_underlying` (seen), `std::spanstream` [23]
54. Deprecated and removed in the library: `std::iterator`, `std::result_of`, `random_shuffle`, `<codecvt>`, `std::aligned_storage` (C++23), `strstream` (C++26); `-Wdeprecated` finds them [17/20/23]
55. Support matrix for this session (table): `expected` on Clang 18 needs libc++; `from_chars<double>` missing on libc++ < 20; `flat_map`, `mdspan`, `stacktrace` per compiler; feature-test macros and `<version>` as the tool

## 6. Exercise and close (1:40, 3 slides)

56. Interface design checklist (the one-slide summary attendees will screenshot): which vocabulary type for which situation, with the ownership table from slide 5 completed
57. The exercise: `exercises/s02-vocabulary-types/README.md`, in-class tasks 1 to 3, the report-diff test
58. Session takeaway and preview of Session 3 (compile-time and concepts; the CRC table and the `serialize` overload set are the targets)

## Demo files needed (`demos/s02/`)

- `string_view_dangling.cpp` (the four dangling patterns, buggy lines under `SHOW_ERRORS` or in comments)
- `string_view_not_cstring.cpp`, `span_basics.cpp`, `span_bytes.cpp`, `mdspan.cpp` (Compiler Explorer only if missing locally)
- `optional_basics.cpp`, `optional_monadic.cpp`, `variant_visit.cpp`, `variant_messages.cpp`, `any.cpp`
- `expected_basics.cpp`, `expected_pipeline.cpp`, `expected_void.cpp`
- `format_specs.cpp`, `print.cpp`, `formatter_custom.cpp`, `format_ranges.cpp`, `format_to_buffer.cpp`
- `tour_filesystem.cpp`, `tour_bit.cpp`, `tour_chrono.cpp`, `tour_flat_map.cpp`, `tour_stacktrace.cpp` (GCC-only with `-lstdc++exp`), `tour_out_ptr.cpp`
- Exercise excerpts from `exercises/s02-vocabulary-types/{starter,solution}`

## Cut list

- Slide 27 (history) if time is short
- Slide 43 (`format_to` and friends) can become two bullets on slide 38
- Slide 35 (`expected<void>`) can fold into 31
