# Session 3 deck outline: Compile-Time and Generic Programming

One line per slide. Badge in brackets; "2col" is before/after; "evo" is a timeline slide; "demo" is live. Target: 51 slides as built (this list has 48 entries; items 11, 14, 20, and 32 each became two slides in the deck, and item 37 was cut) for 95 minutes, then the exercise. "From the exercise" slides pull code from `exercises/s03-compile-time/` starter and solution.

## 0. Opening (0:00, 5 slides)

1. Title
2. Agenda with minutes
3. Session 2 recap: the solution, and the three stumbles (`std::unexpected`, the `transform(&T::member)` trap, formatter as a template)
4. Why compile time matters here: tables, checks, and configuration with zero runtime cost, no static-initialization order, and bugs that cannot reach a running program; the three targets in the exercise (CRC table, config validation, `serialize`)
5. The two halves of today: making the compiler **compute** (`constexpr`, `consteval`) and making it **check** (concepts); templates as the bridge

## 1. The `constexpr` story (0:10, 13 slides)

6. `constexpr` in C++11 (evo, first step): one return statement, recursion or the ternary; the `factorial` from Session 1
7. C++14: relaxed `constexpr` [14] (2col): loops, locals, `if`, mutation; the CRC table loop becomes legal; `constexpr` member functions no longer implicitly `const`
8. Where `constexpr` values live (feature): `inline constexpr` in headers [17]; `constexpr` implies `const` for variables; the difference between a `constexpr` function called at runtime vs compile time (it is the same function)
9. C++17: `constexpr` lambdas and `if constexpr` [17] (feature): a lambda in a constant expression; branches discarded at compile time, replacing tag dispatch
10. C++17: `std::array` and friends usable in constant expressions [17] (feature, from the exercise): `make_crc_table()` returning `std::array<uint16_t, 256>`, `inline constexpr auto kCrcTable = make_crc_table()`
11. What is **not** allowed, and why (feature): `reinterpret_cast` (the `as_bytes` snag from the exercise), `goto`, undefined behavior (a constant expression cannot overflow, index out of bounds, or read uninitialized: the compiler is your sanitizer), non-literal types before C++20
12. C++20: dynamic allocation, `std::vector` and `std::string` at compile time [20] (feature): allocations must be freed before the constant expression ends ("transient allocation"); a compile-time string builder
13. C++20: `constexpr` virtual, `try`/`catch`, `dynamic_cast`, unions, `std::is_constant_evaluated` [20] (feature): `if (std::is_constant_evaluated())` and its `if constexpr` trap
14. C++20: `consteval` [20] (feature, from the exercise): an immediate function; `validate(kSensors)` **cannot** run at runtime, so a bad table is a build error; the error message technique (return a `string_view`, `static_assert` on `.empty()`)
15. C++20: `constinit` [20] (feature): guaranteed static initialization without `const`; the fix for the static initialization order fiasco; `constinit` globals in embedded startup code
16. C++23: `if consteval`, `static` and `thread_local` in `constexpr` functions, non-literal variables, `constexpr std::unique_ptr` [23] (feature): what is left that cannot be `constexpr` (almost nothing; C++26 adds `constexpr` exceptions and placement new)
17. The full timeline (evo): C++11 through C++26 on one slide, with the exercise's three functions placed on it
18. Segment takeaway: `constexpr` by default on any function that could be; `consteval` when it **must** be; `constinit` for globals; `static_assert` is your compile-time unit test

## 2. C++17 template quality of life (0:40, 7 slides)

19. The problem with C++11 templates (feature): SFINAE, `enable_if`, tag dispatch, recursive variadics; one real error message from the starter's `serialize(ParseError{})`
20. Fold expressions [17] (2col, from the exercise): `serialize_all(vs...)` with a comma fold vs the C++11 recursive base-case pair; the four fold forms
21. `if constexpr` replacing tag dispatch and `enable_if` [17] (2col): one function with branches instead of three overloads
22. Variable templates [14], the `_t` aliases [14], and the `_v` aliases [17] (feature): `std::is_integral_v<T>` instead of `std::is_integral<T>::value`; writing your own
23. CTAD and deduction guides [17] (feature): the `overloaded` visitor from Session 2 revisited: how `overloaded{lambda1, lambda2}` deduces; when you need a guide (C++17) and when you do not (C++20 aggregates)
24. `auto` non-type template parameters [17] (feature): `template <auto N>`, and a preview of class-type NTTPs
25. Segment takeaway: fold expressions and `if constexpr` remove most of the reasons templates used to be unreadable

## 3. C++20 concepts (0:55, 14 slides)

26. The problem concepts solve (demo): the starter's `serialize(ParseError{})` error (candidates, no reason) vs the solution's (the unmet constraint, by name); side by side on Compiler Explorer
27. A concept is a named predicate on types [20] (feature, from the exercise): `template <typename T> concept StringLike = std::convertible_to<T, std::string_view>;`
28. Four ways to constrain [20] (feature): `requires`-clause after the template head, `requires`-clause after the parameter list, constrained template parameter (`template <StringLike T>`), abbreviated function template (`std::integral auto v`); when to use which
29. Abbreviated function templates [20] (2col, from the exercise): `serialize(std::integral auto v)` vs `template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>`
30. `requires` expressions [20] (feature, from the exercise): `requires(const T& t) { t.begin(); t.end(); }`; simple, type, compound (`{ expr } -> concept`), and nested requirements
31. The standard concepts library [20] (feature): `<concepts>` (`same_as`, `convertible_to`, `integral`, `floating_point`, `invocable`, `predicate`, `equality_comparable`, `totally_ordered`, `copyable`, `movable`, `regular`), `<iterator>` and `<ranges>` concepts (Session 4)
32. Subsumption: how the compiler picks the more constrained overload [20] (feature): `SerializableRange` is `!StringLike && has begin/end`, so a `std::string` picks the `StringLike` overload; why concepts written as conjunctions of named concepts subsume and ad-hoc `requires` expressions do not
33. Concepts as questions [20] (feature, from the exercise): `Serializable<T>` defined as "some `serialize` accepts a `T`"; `static_assert(!Serializable<ParseError>)`; concepts in `if constexpr`; concepts as documentation of an API
34. Concept design guidance (feature): semantic requirements the compiler cannot check (`std::regular` promises more than its syntax); prefer standard concepts; name the concept for the **capability**, not the type; do not over-constrain generic code
35. Template lambdas [20] and lambdas in unevaluated contexts [20] (feature): `[]<typename T>(std::span<T> s) {}` when `auto` is not enough; `decltype([]{})` as a unique type
36. Class types as non-type template parameters [20] (feature): `template <FixedString Name>`; a compile-time string as a template argument; why `SensorConfig` cannot be one (a `string_view` member is not a structural type) and what would make it one
37. (cut, not in the deck) `constexpr` and concepts together (feature): a `consteval` function constrained by a concept; `static_assert` with a concept as the message; a compile-time registry pattern
38. Concepts and SFINAE side by side (2col): the same constraint written both ways, with the two error messages
39. Segment takeaway: constrain every template parameter with the weakest concept that makes the body compile; the error message is the feature

## 4. C++23: deducing `this` (1:20, 6 slides)

40. The three problems it solves (feature): `const`/non-`const` member duplication, CRTP boilerplate, recursive lambdas
41. Explicit object parameter [23] (2col, from the exercise): `SensorStats::add` before (`SensorStats& add(...)`, cannot chain on a temporary without a copy) and after (`template <typename Self> Self&& add(this Self&& self, ...)`)
42. Deduplicating `const` overloads [23] (feature): one `get(this auto&& self)` instead of two; `std::forward_like` for the member
43. CRTP without the template parameter [23] (2col): `struct Base { void interface(this auto&& self) { self.impl(); } };` vs `template <typename Derived> struct Base`
44. Recursive lambdas [23] (feature): `auto fib = [](this auto self, int n) { return n < 2 ? n : self(n-1) + self(n-2); };`
45. Segment takeaway and the by-value trick: `this auto self` (by value) for small types passed in registers

## 5. Exercise and close (1:35, 3 slides)

46. Reading the diagnostics (demo): the exercise's task 8, starter vs solution error for `serialize(ParseError{})`, live
47. The exercise: `exercises/s03-compile-time/README.md`, in-class tasks 1 to 3 (constexpr CRC table with `static_assert(crc16("123456789") == 0x29B1)`, `consteval validate`, constrained `serialize`), the report-diff test
48. Session takeaway and preview of Session 4 (ranges: the `SerializableRange` concept becomes `std::ranges::input_range`, and the report loop becomes a pipeline)

## Demo files needed (`demos/s03/`)

- `constexpr_evolution.cpp` (the same function written under 11/14/17/20/23 rules, with `#if __cplusplus` guards or separate functions)
- `constexpr_limits.cpp` (UB caught at compile time: overflow, out-of-bounds; `reinterpret_cast` under `SHOW_ERRORS`)
- `constexpr_alloc.cpp` (compile-time `std::vector`/`std::string`, transient allocation)
- `consteval_constinit.cpp`, `is_constant_evaluated.cpp` (and the `if consteval` form)
- `fold_expressions.cpp`, `if_constexpr_dispatch.cpp`, `variable_templates.cpp`, `ctad_guides.cpp`, `auto_nttp.cpp`
- `concepts_basics.cpp` (the four spellings), `requires_expressions.cpp`, `subsumption.cpp`, `concepts_vs_sfinae.cpp` (both forms, errors under `SHOW_ERRORS`)
- `template_lambdas.cpp` (`FixedString` lives in `auto_nttp.cpp`)
- `deducing_this.cpp` (const dedup, CRTP, recursive lambda)
- Exercise excerpts from `exercises/s03-compile-time/{starter,solution}`

## Cut list

- Slide 36 (class-type NTTPs) can become a bullet on 35
- Slide 37 (constexpr + concepts registry) if the concepts segment runs long
- Slide 24 (`auto` NTTP) folds into 36
