# Session 1 deck outline: The Everyday Language

One line per slide. Badge in brackets. "2col" means a before/after slide; "demo" means a live Compiler Explorer or terminal demo; "evo" means an evolution timeline slide. Target: 56 content slides for 90 minutes, then the exercise.

## 0. Opening (0:00, 6 slides)

1. Title
2. Agenda with minutes
3. Why not one session per standard (the size table)
4. How to read a feature slide: the badge, the problem line, the "what it replaces" line
5. The toolchain: `-std=c++23`, GCC 14 / Clang 18, Compiler Explorer, the Docker image; the two-minute environment check
6. The program we will modernize all course: the telemetry processor, what it does, run it once (demo)

## 1. C++11 calibration (0:10, 9 slides)

7. What this course assumes you know: the C++11 checklist
8. `auto` pitfalls: `auto` drops references and const; `auto&` and `const auto&`; `auto` with braces
9. When `std::move` does nothing: const objects, returning locals, the "moved-from is valid but unspecified" rule
10. Lambdas that escape: capture by reference and lifetime; capture by value and copies (2col)
11. `unique_ptr` vs `shared_ptr`: ownership, not "modern pointer"; `shared_ptr` is a design decision
12. `enum class`, `nullptr`, `override`, `final`: the ones everyone uses
13. `constexpr` in C++11: one return statement, no loops (preview of Session 3)
14. Uniform initialization: `{}` and the `initializer_list` trap with `vector`
15. Calibration takeaway: good C++11 is fine code, and every slide from here replaces some of it

## 2. C++14: the polish release (0:30, 11 slides)

16. C++14 in one slide: what it was for, why it is small, why it matters anyway
17. Generic lambdas: `auto` parameters make a lambda a template [14] (2col)
18. Lambda init-capture: moving into a closure, the `unique_ptr` case C++11 could not express [14]
19. Return type deduction for functions and `decltype(auto)` [14]; when not to use it (public headers)
20. `std::make_unique` and why "never write `new`" became possible [14] (2col)
21. Binary literals and digit separators [14]: the CRC polynomial and `12'000.0` from the exercise
22. Standard literals: `"abc"s`, `10ms`, `2h` [14]; the `using namespace std::literals` convention
23. `[[deprecated]]` [14]: your first attribute, and the pattern for the rest
24. Relaxed `constexpr`: loops and locals allowed [14] (evo preview, full story Session 3)
25. Small library: `std::exchange`, `std::integer_sequence`, `cbegin`/`cend`, `std::quoted` [14]
26. Removals: `gets` [14]; C++14 takeaway

## 3. C++17: syntax you will use daily (0:55, 15 slides)

27. C++17 in one slide: the "everyday" release
28. Structured bindings [17] (2col, from `compute_stats`)
29. Structured bindings on structs, arrays, and in range-for [17]
30. `if` and `switch` with initializer [17] (2col, from `load_stream`); scoping and the lock-guard idiom
31. `try_emplace` and `insert_or_assign` [17]: the map API C++11 should have had
32. `inline` variables [17] (2col, `kMaxLineLength`): header-only constants and singletons
33. Nested namespace definitions and `namespace a::b` [17]
34. `[[nodiscard]]` [17]: the attribute that finds bugs; where to put it and where not to
35. `[[maybe_unused]]` and `[[fallthrough]]` [17]
36. Guaranteed copy elision [17]: what "returns a prvalue" means, factories for non-movable types
37. Class template argument deduction [17]: `std::pair{a, b}`, `std::lock_guard lk{m}`, `std::vector v{1, 2}` and its trap
38. `std::from_chars` and `std::to_chars` [17] (2col, from `parse_record`): no errno, no locale, no NUL terminator
39. `std::string_view` preview [17]: one slide now, the full treatment in Session 2
40. Expression evaluation order and `noexcept` in the type system [17]: the quiet fixes
41. Removals: `auto_ptr`, `register`, trigraphs, dynamic exception specs, `random_shuffle` [17]; C++17 takeaway

## 4. C++20 and C++23: small but valuable (1:20, 13 slides)

42. Designated initializers [20] (2col): `SensorConfig{.name = ..., .units = ...}`
43. Three-way comparison, part 1: the problem (six operators) and `operator<=>` [20] (2col, from `Record`)
44. Three-way comparison, part 2: rewriting rules, `==` is separate and why, ordering categories [20]
45. Three-way comparison, part 3: defaulting, member order matters, `double` gives `partial_ordering` [20] (demo)
46. Range-for with initializer [20]: the temporary-lifetime bug it fixes
47. `using enum` [20] (2col, from `to_string(Status)`)
48. `[[likely]]`, `[[unlikely]]`, `[[no_unique_address]]` [20]: when they matter and when they are noise
49. `char8_t`, `__VA_OPT__`, `consteval`/`constinit` name-drop [20]: know they exist
50. `auto(x)` decay copy [23] and `size_t` literal `uz` [23]
51. `#elifdef`, `#warning`, `[[assume]]`, labels at end of blocks [23]
52. `std::to_underlying` and `std::unreachable` [23]: the last two small ones
53. Deprecations across 20/23 worth knowing today: `volatile` compound ops, `aligned_storage`
54. Segment takeaway: the "Monday morning" list from this segment

## 5. Exercise and close (1:35, 2 slides)

55. The exercise: `exercises/s01-modernize-syntax/README.md`, in-class tasks 1 to 3, the test command
56. Session takeaway and preview of Session 2 (vocabulary types)

## Demo files needed

- `demos/s01/structured_bindings.cpp` (exists)
- `demos/s01/spaceship.cpp` (exists), plus a `partial_ordering` and member-order variant
- `demos/s01/generic_lambda.cpp` (exists)
- `demos/s01/auto_pitfalls.cpp`, `move_does_nothing.cpp`, `escaping_lambda.cpp` (calibration)
- `demos/s01/copy_elision.cpp`, `ctad.cpp`, `from_chars.cpp`, `inline_variable.cpp`, `nodiscard.cpp`
- `demos/s01/designated_init.cpp`, `range_for_init.cpp`, `using_enum.cpp`, `small_cpp23.cpp`
- Exercise excerpts come straight from `exercises/s01-modernize-syntax/starter` and `solution`

## Cut list (write only if time allows in the dry run)

- `std::apply` and `std::invoke` (Session 2 library tour covers them)
- Template template parameter `auto` NTTP (Session 3)
- `__has_include` and `[[no_unique_address]]` details
