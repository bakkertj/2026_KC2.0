---
marp: true
theme: course
paginate: true
footer: 'The Evolution of C++ | Session 3: Compile-Time and Generic Programming'
---

<!-- _class: lead -->
<!-- _paginate: false -->

# The Evolution of C++
## Session 3: Compile-Time and Generic Programming

Moving work to compile time, and writing templates a human can read

<!--
Notes: Two halves: making the compiler compute (constexpr, consteval) and making it check
(concepts). The exercise has one target for each half plus deducing this.
-->

---

## Agenda

1. Recap and framing (10 min)
2. The `constexpr` story, C++11 to C++26 (30 min)
3. C++17 template quality of life (15 min)
4. C++20 concepts (25 min)
5. C++23: deducing `this` (15 min)
6. Guided exercise (20 min)

<!--
Notes: Exercise README: exercises/s03-compile-time/README.md. The starter is the Session 2
solution.
-->

---

## Session 2 recap

The solution: `expected` parser, `optional` lookups, `span` parameters, `string_view` fields, `std::print` report with three `formatter`s. Same report, byte for byte.

Where people got stuck:

- Forgetting `std::unexpected(...)` around the error; a bare `return ParseError::X` does not convert
- `find_sensor(name).transform(&SensorConfig::units)`: an rvalue-reference result that `optional` cannot hold. A lambda fixes it.
- A `formatter::format` that is not a template on the context: fine on libstdc++, rejected by libc++

<!--
Notes: The transform trap is a value-category fact; the supplemental deck covers it.
-->

---

## Why compile time matters here

Three things in the starter do work at runtime that never changes:

| In the starter | What it costs | After today |
|---|---|---|
| `crc.cpp` builds a 256-entry table behind a function-local static | first-call latency, a guard check on every call, one more thing at startup | `inline constexpr auto kCrcTable = make_crc_table();` |
| `config.cpp` holds the sensor table; nothing validates it | a bad entry ships and misbehaves in the field | `static_assert(validate(kSensors).empty())` |
| `serialize.h` is seven overloads | an unsupported type gives a page of candidates | four constrained templates and a `Serializable` concept |

**Zero runtime cost, no static-initialization order, and one class of bug that cannot reach a running program.**

<!--
Notes: The embedded framing: startup code and configuration tables are exactly where these
features pay. Every row is a task in the exercise.
-->

---

## The two halves of today

**Making the compiler compute:** `constexpr` (may run at compile time), `consteval` (must), `constinit` (initialized at compile time). Result: tables, checks, hashes, and configuration that cost nothing at runtime, and `static_assert` as a unit test.

**Making the compiler check:** concepts. Result: templates whose requirements are in the signature, and error messages that name what went wrong.

**The bridge:** templates got readable in C++17 (fold expressions, `if constexpr`) before concepts made them checkable in C++20, and deducing `this` (C++23) removes the last of the boilerplate.

<!--
Notes: Sixty seconds. Then the constexpr timeline, which is the longest single story in the
course.
-->

---

<!-- SEGMENT: constexpr (0:10) -->

<!-- _class: feature -->

## `constexpr` in C++11: where it started <span class="badge cpp11">C++11</span>

<p class="problem">A function that may run at compile time, restricted to a single return statement.</p>

<!-- snippet: demos/s03/constexpr_evolution.cpp#cpp11 -->
```cpp
// C++11: one return statement. Loops are spelled as recursion.
constexpr int factorial11(int n) { return n <= 1 ? 1 : n * factorial11(n - 1); }
```

Loops are spelled as recursion. No locals, no `if`, no mutation. Useful for array bounds and little else.

<!--
Notes: The C++11 form was a toy. It exists so that the rest of the story has a starting point.
Demo file: demos/s03/constexpr_evolution.cpp
-->

---

<!-- _class: twocol -->

## Relaxed `constexpr` <span class="badge cpp14">C++14</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s03/constexpr_evolution.cpp#cpp11 -->
```cpp
// C++11: one return statement. Loops are spelled as recursion.
constexpr int factorial11(int n) { return n <= 1 ? 1 : n * factorial11(n - 1); }
```

</div>
<div>

#### After (C++14)

<!-- snippet: demos/s03/constexpr_evolution.cpp#cpp14 -->
```cpp
// C++14: loops, locals, if, mutation. It looks like a normal function.
constexpr int factorial14(int n) {
    int r = 1;
    for (int i = 2; i <= n; ++i) r *= i;
    return r;
}
```

</div>
</div>

Loops, locals, `if`, mutation of locals, multiple returns. Also: `constexpr` member functions are no longer implicitly `const`.

<!--
Notes: From C++14 on, a constexpr function looks like a normal function with a keyword in
front. The CRC table loop in the exercise is legal from here. Demo file:
demos/s03/constexpr_evolution.cpp
-->

---

## Where `constexpr` values live

```cpp
constexpr int kMax = 256;                    // a constant; constexpr on a variable implies const
inline constexpr auto kTable = make();       // C++17: one definition in a header, computed at compile time

constexpr int f(int n) { ... }               // MAY run at compile time
int a = f(3);                                // runtime call (a is not constexpr)... unless the optimizer folds it
constexpr int b = f(3);                      // compile-time call, guaranteed
static_assert(f(3) == 6);                    // compile-time call, and a unit test
```

A `constexpr` function is **one function**: the same body serves both contexts. It runs at compile time only when a constant is **required** (a `constexpr` variable, a template argument, an array bound, a `static_assert`).

<!--
Notes: The most common misunderstanding: people think constexpr on a function forces
compile-time evaluation. It permits it. consteval (later) forces it. Hand-typed.
-->

---

<!-- _class: feature -->

## `constexpr` lambdas and `if constexpr` <span class="badge cpp17">C++17</span>

<p class="problem">Lambdas join constant expressions, and branches can be selected at compile time.</p>

<!-- snippet: demos/s03/constexpr_evolution.cpp#cpp17 -->
```cpp
// C++17: std::array works in a constant expression; lambdas can be constexpr.
constexpr std::array<int, 6> factorials17() {
    std::array<int, 6> out{};
    auto fact = [](int n) constexpr { return factorial14(n); };
    for (int i = 0; i < 6; ++i) out[static_cast<std::size_t>(i)] = fact(i);
    return out;
}
inline constexpr auto kFactorials = factorials17();   // computed once, at compile time, in the header
```

Lambdas are implicitly `constexpr` when they can be. `if constexpr` is covered in the templates segment.

<!--
Notes: The `constexpr` keyword on the lambda is optional (implicit since C++17); it is there to
make the intent visible and to get an error if the body cannot be constant-evaluated. Demo file:
demos/s03/constexpr_evolution.cpp
-->

---

<!-- _class: feature -->

## `std::array` at compile time <span class="badge cpp17">C++17</span>

<p class="problem">The exercise's CRC table, computed once by the compiler and placed in read-only data.</p>

```cpp
constexpr std::array<std::uint16_t, 256> make_crc_table() {
    std::array<std::uint16_t, 256> table{};
    for (unsigned i = 0; i < 256; ++i) {
        auto crc = static_cast<std::uint16_t>(i << 8);
        for (int bit = 0; bit < 8; ++bit)
            crc = static_cast<std::uint16_t>((crc & 0x8000) ? (crc << 1) ^ kCrcPolynomial : crc << 1);
        table[i] = crc;
    }
    return table;
}
inline constexpr auto kCrcTable = make_crc_table();      // 512 bytes in .rodata, no startup code
static_assert(crc16("123456789") == 0x29B1);              // the standard check value, as a unit test
```

<!--
Notes: Exercise task 1. From exercises/s03-compile-time/solution/include/telemetry/crc.h. Show
the disassembly on Compiler Explorer: kCrcTable is a data symbol, and crc16 of a literal folds to
an immediate. Hand-typed excerpt of the solution.
-->

---

<!-- _class: feature -->

## What a constant expression refuses <span class="badge cpp11">C++11</span>

<p class="problem">Undefined behavior is not allowed in a constant expression. The compiler is a sanitizer that costs nothing.</p>

<!-- snippet: demos/s03/constexpr_limits.cpp#ub -->
```cpp
constexpr int overflow(int x) { return x + 1; }
constexpr int at(const std::array<int, 3>& a, std::size_t i) { return a[i]; }

#ifdef SHOW_ERRORS
constexpr int uninit() { int x; return x; }       // (Clang rejects the read even before constant evaluation)
static_assert(overflow(INT_MAX) == INT_MIN);      // error: overflow in constant expression
static_assert(at({1, 2, 3}, 5) == 0);             // error: array subscript out of bounds
static_assert(uninit() == 0);                     // error: read of uninitialized object
#endif
// A constant expression cannot contain undefined behavior. The compiler is a sanitizer
// for every constexpr call it evaluates, with no runtime cost.
```

<!--
Notes: The argument that converts embedded engineers: every constexpr call the compiler
evaluates is checked for overflow, out-of-bounds, uninitialized reads, and lifetime errors, at
build time. Write your lookup tables and protocol constants constexpr and you get this for free.
Demo file: demos/s03/constexpr_limits.cpp (errors under SHOW_ERRORS)
-->

---

<!-- _class: feature -->

## Also refused: `reinterpret_cast` <span class="badge cpp11">C++11</span>

<p class="problem">The snag from the exercise: std::as_bytes is not constexpr because it is a reinterpret_cast.</p>

<!-- snippet: demos/s03/constexpr_limits.cpp#reinterpret -->
```cpp
constexpr std::uint16_t first_two_bytes(std::span<const char> s) {
#ifdef SHOW_ERRORS
    return *reinterpret_cast<const std::uint16_t*>(s.data());   // error: reinterpret_cast is never constexpr
#else
    return static_cast<std::uint16_t>((static_cast<unsigned char>(s[1]) << 8) | static_cast<unsigned char>(s[0]));
#endif
}
// This is why std::as_bytes is not constexpr, and why the exercise's crc16 uses static_cast per byte.
```

Also not allowed: `goto`, `asm`, non-literal types (pre-C++20), calling non-`constexpr` functions, `std::to_string` (until C++26).

<!--
Notes: Exercise task 1 hits this. The fix in the solution: a template over the byte type with
static_cast per byte. reinterpret_cast is the one cast that reinterprets memory; a constant
expression has no memory to reinterpret, only values. Demo file: demos/s03/constexpr_limits.cpp
-->

---

<!-- _class: feature -->

## Dynamic allocation at compile time <span class="badge cpp20">C++20</span>

<p class="problem">std::vector and std::string in a constant expression, as long as they are freed before it ends.</p>

<!-- snippet: demos/s03/constexpr_alloc.cpp#alloc -->
```cpp
// Build a sorted list of names at compile time. The vector lives and dies inside
// the constant expression ("transient allocation"); only the result escapes.
constexpr std::array<std::string_view, 4> sorted_names() {
    std::vector<std::string_view> v{"rpm", "pressure", "temp_core", "current_bus"};
    std::sort(v.begin(), v.end());                      // constexpr algorithms, C++20
    std::array<std::string_view, 4> out{};
    std::copy(v.begin(), v.end(), out.begin());
    return out;                                          // v is freed here, before the expression ends
}
inline constexpr auto kSortedNames = sorted_names();
static_assert(kSortedNames[0] == "current_bus");

// constexpr std::vector<int> kNotAllowed = {1, 2, 3};   // error: the allocation would outlive the expression
```

<!--
Notes: "Transient allocation": the vector is a scratch buffer inside the computation; only the
std::array result escapes. A constexpr std::vector VARIABLE is still an error, because its
allocation would have to survive into the program. C++26 is working on that. Demo file:
demos/s03/constexpr_alloc.cpp
-->

---

<!-- _class: feature dense -->

## The rest of C++20 `constexpr` <span class="badge cpp20">C++20</span>

<p class="problem">Virtual calls, try/catch, dynamic_cast, unions, and knowing which mode you are in.</p>

<!-- snippet: demos/s03/is_constant_evaluated.cpp#ice -->
```cpp
// One function, two implementations: exact at compile time, fast at runtime
constexpr double power(double base, int exp) {
    if consteval {                                   // C++23: a real branch on evaluation mode
        double r = 1.0;
        for (int i = 0; i < exp; ++i) r *= base;     // constexpr-friendly loop
        return r;
    } else {
        return std::pow(base, exp);                  // libm; not constexpr everywhere
    }
}
static_assert(power(2.0, 10) == 1024.0);
```

<!-- snippet: demos/s03/is_constant_evaluated.cpp#trap -->
```cpp
constexpr int trap() {
#ifdef SHOW_ERRORS   // GCC 14 warns: always true in 'if constexpr'
    if constexpr (std::is_constant_evaluated()) {    // WRONG: the condition is itself
        return 1;                                    // constant-evaluated: always true
    }
#endif
    if (std::is_constant_evaluated()) return 1;      // C++20: a plain if. C++23: `if consteval`.
    return 2;
}
```

<!--
Notes: The pattern: exact-but-slow at compile time, fast at runtime, one function. The trap:
inside `if constexpr` the condition is itself constant-evaluated, so is_constant_evaluated() is
always true there; GCC 14 warns. C++23's `if consteval` cannot be misused that way. Demo file:
demos/s03/is_constant_evaluated.cpp
-->

---

<!-- _class: feature -->

## `consteval`: must run at compile time <span class="badge cpp20">C++20</span>

<p class="problem">An immediate function. Calling it at runtime is a compile error, so it can never leak into startup or a hot loop.</p>

<!-- snippet: demos/s03/consteval_constinit.cpp#consteval -->
```cpp
// consteval: an immediate function. It CANNOT be called at runtime.
consteval std::uint32_t fnv1a(std::string_view s) {
    std::uint32_t h = 2166136261u;
    for (char c : s) h = (h ^ static_cast<unsigned char>(c)) * 16777619u;
    return h;
}

constexpr auto kRpmId = fnv1a("rpm");          // fine: a constant
// std::uint32_t id(std::string_view s) { return fnv1a(s); }   // error: s is not a constant
// A hash that is guaranteed never to run at startup or in a hot loop, and cannot be
// misused to do so.
```

<!--
Notes: Use consteval for things that should NEVER cost runtime: hashes of literals, table
generation, validation. The compile-time hash is a classic: string switch-cases by hash, with
no hash computed at runtime. Demo file: demos/s03/consteval_constinit.cpp
-->

---

<!-- _class: feature -->

## `consteval` with a message <span class="badge cpp20">C++20</span>

<p class="problem">The exercise's config validator: return why it failed, and let static_assert print it.</p>

<!-- snippet: demos/s03/consteval_constinit.cpp#validate -->
```cpp
// consteval with a message: return why it failed, and let static_assert print it.
struct Limit { std::string_view name; double lo, hi; };
consteval std::string_view validate(const Limit& l) {
    if (l.name.empty()) return "limit has no name";
    if (!(l.lo < l.hi)) return "limit has lo >= hi";
    return {};
}
constexpr Limit kRpm{"rpm", 0.0, 12000.0};
static_assert(validate(kRpm).empty(), "sensor limit is invalid");
```

In the exercise: `validate(std::span<const SensorConfig>)` checks names, units, ranges, and duplicates, and `static_assert(validate(kSensors).empty(), "sensor configuration table is invalid");` guards the table.

<!--
Notes: Exercise task 2. Break the table live and read the error: the static_assert message plus
the returned string_view in the "evaluated to" note. C++26 lets static_assert take a
constexpr string directly. Demo file: demos/s03/consteval_constinit.cpp
-->

---

<!-- _class: feature -->

## `constinit`: initialized at compile time, mutable at runtime <span class="badge cpp20">C++20</span>

<p class="problem">The static initialization order fiasco, ended: a global whose initializer is guaranteed to be a constant.</p>

<!-- snippet: demos/s03/consteval_constinit.cpp#constinit -->
```cpp
// constinit: initialized at compile time, mutable at runtime. No static-init-order fiasco.
struct Counters { std::uint32_t parsed = 0; std::uint32_t rejected = 0; };
constinit Counters g_counters{};                        // zero-initialized before any code runs, guaranteed

// constinit int bad = fnv1a_runtime("x");             // error if the initializer is not a constant
// A constexpr global would be immutable; a plain global might be dynamically initialized
// after something else already used it. constinit is the third option.
```

<!--
Notes: Three kinds of global: constexpr (immutable, constant-initialized), constinit
(mutable, constant-initialized), plain (mutable, maybe dynamically initialized, in an order you
do not control across translation units). Embedded startup code wants the middle one. Demo
file: demos/s03/consteval_constinit.cpp
-->

---

<!-- _class: feature -->

## Almost nothing left <span class="badge cpp23">C++23</span>

<p class="problem">if consteval, static locals, non-literal variables, constexpr std::unique_ptr.</p>

<!-- snippet: demos/s03/constexpr_evolution.cpp#cpp23 -->
```cpp
// C++23: a static local in a constexpr function (as long as it is not touched during constant evaluation).
constexpr int cached_or_computed(int n) {
    if consteval {
        return factorial14(n);                          // constant evaluation: just compute
    } else {
        static std::array<int, 13> cache{};             // runtime: memoize
        if (cache[static_cast<std::size_t>(n)] == 0) cache[static_cast<std::size_t>(n)] = factorial14(n);
        return cache[static_cast<std::size_t>(n)];
    }
}
```

Still not `constexpr` in C++23: exceptions being thrown (C++26), placement `new` (C++26), `std::to_string` (C++26), most of `<cmath>` (C++26), `reinterpret_cast` (never).

<!--
Notes: C++23 removed the "all paths must be constant-evaluable" restriction: a function may be
constexpr as long as SOME call could be a constant expression. The static local is fine as long
as the compile-time path does not touch it. Demo file: demos/s03/constexpr_evolution.cpp
-->

---

## The full timeline

<div class="evo">
<div class="step"><b>C++11</b> single return statement, recursion; array bounds and little else</div>
<div class="step"><b>C++14</b> loops, locals, if, mutation: normal-looking functions <span style="color:var(--muted)">(the CRC loop)</span></div>
<div class="step"><b>C++17</b> lambdas, <code>if constexpr</code>, <code>std::array</code>, <code>inline constexpr</code> variables <span style="color:var(--muted)">(the CRC table)</span></div>
<div class="step"><b>C++20</b> allocation, <code>vector</code>/<code>string</code>, virtual, try/catch, <code>consteval</code>, <code>constinit</code>, <code>is_constant_evaluated</code> <span style="color:var(--muted)">(the config validator)</span></div>
<div class="step"><b>C++23</b> <code>if consteval</code>, static locals, non-literal variables, <code>unique_ptr</code>, <code>optional</code>/<code>string_view</code> everywhere <span style="color:var(--muted)">(find_sensor, parse_status)</span></div>
<div class="step"><b>C++26</b> exceptions, placement new, <code>to_string</code>, <code>&lt;cmath&gt;</code>, static_assert with a computed message</div>
</div>

<!--
Notes: The evolution slide. Twelve years from "a glorified macro" to "the whole language at
compile time". Point at where each exercise function sits.
-->

---

<!-- _class: takeaway -->

## `constexpr` takeaway

**`constexpr` by default** on any function that could be: it costs nothing and unlocks `static_assert`.
**`consteval`** when it must never run at runtime. **`constinit`** for mutable globals.
**`static_assert` is your compile-time unit test**, and undefined behavior in a constant expression is a compile error.

**Monday morning:** find one lookup table built at startup and make it `inline constexpr`.

<!--
Notes: 0:40. Templates next; shorter segment.
-->

---

<!-- SEGMENT: C++17 templates (0:40) -->

## The problem with C++11 templates

```
error: no matching function for call to 'serialize(telemetry::ParseError)'
note: candidate: 'std::string telemetry::serialize(int)'
note:   no known conversion for argument 1 from 'ParseError' to 'int'
note: candidate: 'std::string telemetry::serialize(long long int)'
note: candidate: 'std::string telemetry::serialize(unsigned long)'
note: candidate: 'std::string telemetry::serialize(double)'
note: candidate: 'std::string telemetry::serialize(std::string_view)'
note: candidate: 'std::string telemetry::serialize(telemetry::Status)'
note: candidate: 'std::string telemetry::serialize(const telemetry::Record&)'
note: candidate: 'template<class T> std::string telemetry::serialize(std::span<const T>)'
note:   template argument deduction/substitution failed:
...
```

Seven candidates, no reason. And that is the **simple** case; `enable_if` errors run to pages.

<!--
Notes: This is the real error from the Session 2 starter. Keep it on screen while introducing
the fixes; it returns on the concepts segment with the C++20 version.
-->

---

<!-- _class: twocol -->

## Fold expressions <span class="badge cpp17">C++17</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s03/fold_expressions.cpp#before -->
```cpp
// C++11: recursion with a base case, for every variadic function
int sum11() { return 0; }
template <typename T, typename... Rest>
int sum11(T first, Rest... rest) { return first + sum11(rest...); }
```

</div>
<div>

#### After (C++17)

<!-- snippet: demos/s03/fold_expressions.cpp#after -->
```cpp
// C++17: one expression
template <typename... Ts>
int sum(Ts... vs) { return (vs + ...); }                       // unary right fold: v1 + (v2 + (v3))

template <typename... Ts>
bool all_positive(Ts... vs) { return ((vs > 0) && ...); }      // any operator, including &&

template <typename... Ts>
std::string join(Ts... vs) {
    std::string out;
    ((out += std::to_string(vs) + ","), ...);                  // the comma fold: do this for each
    return out;
}

template <typename... Ts>
int sum_from_100(Ts... vs) { return (100 + ... + vs); }        // binary left fold with an init value
```

</div>
</div>

<!--
Notes: Every variadic function in C++11 was a recursive pair with a base case. A fold applies an
operator across the pack in one expression. The comma fold ("do this for each element") is the
exercise's serialize_all (task 5). Demo file: demos/s03/fold_expressions.cpp
-->

---

## The four fold forms <span class="badge cpp17">C++17</span>

<!-- snippet: demos/s03/fold_expressions.cpp#forms -->
```cpp
// ( pack op ... )          unary right   v1 op (v2 op (v3 ...))
// ( ... op pack )          unary left    ((v1 op v2) op v3) ...
// ( pack op ... op init )  binary right
// ( init op ... op pack )  binary left
// Empty packs: && gives true, || gives false, comma gives void; other operators need the binary form.
```

<!--
Notes: Right vs left rarely matters for associative operators; it matters for `-` and for
`<<`. The empty-pack rule is the gotcha: `(vs + ...)` with no arguments is an error, so use the
binary form with an init value when the pack can be empty. Demo file: demos/s03/fold_expressions.cpp
-->

---

<!-- _class: twocol -->

## `if constexpr` replacing dispatch <span class="badge cpp17">C++17</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s03/if_constexpr_dispatch.cpp#before -->
```cpp
// C++11: two overloads selected by enable_if; the reader has to reassemble the logic
template <typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
std::string describe11(T v) { return "int " + std::to_string(v); }

template <typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
std::string describe11(T v) { return "float " + std::to_string(v); }
```

</div>
<div>

#### After (C++17)

<!-- snippet: demos/s03/if_constexpr_dispatch.cpp#after -->
```cpp
// C++17: one function; the untaken branch is discarded, not compiled
template <typename T>
std::string describe(T v) {
    if constexpr (std::is_integral_v<T>) {
        return "int " + std::to_string(v);
    } else if constexpr (std::is_floating_point_v<T>) {
        return "float " + std::to_string(v);
    } else {
        return "other";                                 // v.foo() here would not be instantiated for int
    }
}
```

</div>
</div>

<!--
Notes: The untaken branch is discarded, not instantiated: code in it may not even compile for
the other type. This is what tag dispatch and enable_if overload pairs were for. Concepts (next
segment) are the other half: if constexpr for branching inside one function, concepts for
choosing between functions. Demo file: demos/s03/if_constexpr_dispatch.cpp
-->

---

<!-- _class: feature -->

## Variable templates and the `_v` / `_t` aliases <span class="badge cpp14">C++14</span>

<p class="problem">A constant parameterized on a type, and the end of ::value and ::type.</p>

<!-- snippet: demos/s03/variable_templates.cpp#vt -->
```cpp
template <typename T>
constexpr T pi = T(3.1415926535897932385L);           // one constant, every precision

template <typename T>
constexpr bool is_small_v = sizeof(T) <= sizeof(void*);   // your own _v trait

static_assert(pi<float> > pi<double>);            // float rounds pi UP: a compile-time float lesson for free
static_assert(is_small_v<int> && !is_small_v<long double>);

// C++11:  std::is_integral<T>::value      std::remove_const<T>::type
// C++17:  std::is_integral_v<T>           std::remove_const_t<T>
static_assert(std::is_integral_v<int> && std::is_same_v<std::remove_const_t<const int>, int>);
```

<!--
Notes: The _v aliases are C++17 (the variable-template feature they use is C++14). The pi
example carries a free lesson: float rounds pi up, so pi<float> > pi<double>, and the compiler
told us so via a failed static_assert while this demo was being written. Demo file:
demos/s03/variable_templates.cpp
-->

---

<!-- _class: feature -->

## CTAD and deduction guides <span class="badge cpp17">C++17</span>

<p class="problem">How overloaded{l1, l2} from Session 2 knows its template arguments, and when you have to tell it.</p>

<!-- snippet: demos/s03/ctad_guides.cpp#guides -->
```cpp
template <class... Fs> struct overloaded : Fs... { using Fs::operator()...; };
// C++17 needed this deduction guide for overloaded{l1, l2} to deduce Fs...:
// template <class... Fs> overloaded(Fs...) -> overloaded<Fs...>;
// C++20 deduces it for aggregates automatically, so the guide is gone.

template <typename T>
struct Wrapper {
    T value;
    explicit Wrapper(const T& v) : value(v) {}
};
Wrapper(const char*) -> Wrapper<std::string>;   // a guide: a literal deduces std::string, not const char*
```

<!--
Notes: A deduction guide is a rule: "when constructed from these argument types, deduce these
template arguments". Most types never need one; aggregates get CTAD for free from C++20, which is
why the overloaded guide disappeared. The Wrapper guide shows the case where you want a
different type than the argument's. Demo file: demos/s03/ctad_guides.cpp
-->

---

<!-- _class: feature -->

## `auto` non-type template parameters <span class="badge cpp17">C++17</span>

<p class="problem">A template parameter that is a value of deduced type.</p>

<!-- snippet: demos/s03/auto_nttp.cpp#auto_nttp -->
```cpp
template <auto N>                              // C++17: the type of N is deduced from the argument
struct Constant { static constexpr auto value = N; };
static_assert(Constant<42>::value == 42);      // int
static_assert(Constant<'x'>::value == 'x');    // char
```

Preview: C++20 allows class-type values too (coming up in the concepts segment).

<!--
Notes: Short. `template <auto N>` replaces the C++11 dance of `template <typename T, T N>`.
Demo file: demos/s03/auto_nttp.cpp
-->

---

<!-- _class: takeaway -->

## C++17 templates takeaway

**Fold expressions** end recursive variadics. **`if constexpr`** ends tag dispatch. **`_v` and `_t`** end `::value` and `::type`. **CTAD** ends `make_*` helpers.

Templates stopped being a specialist skill in C++17. C++20 makes them **checkable**.

**Monday morning:** find one recursive variadic template and fold it.

<!--
Notes: 0:55. Concepts: the biggest segment after constexpr.
-->

---

<!-- SEGMENT: Concepts (0:55) -->

<!-- _class: demo -->

## The problem concepts solve

Compiler Explorer, two panes, same call: `serialize(ParseError::EmptyLine)`

**Left:** the Session 2 starter (overloads). Seven candidates, no reason.

**Right:** the Session 3 solution (constrained templates).
```
error: no matching function for call to 'serialize(telemetry::ParseError)'
note: candidate: 'std::string telemetry::serialize(const T&) requires StringLike<T>'
note:   constraints not satisfied
note:   'telemetry::ParseError' does not satisfy 'StringLike'
```

The error names the **requirement**. That is the feature.

<!--
Notes: Exercise task 8 is exactly this comparison. Do it live; it is the most persuasive thirty
seconds in the session.
-->

---

<!-- _class: feature -->

## A concept is a named predicate on types <span class="badge cpp20">C++20</span>

<p class="problem">A compile-time boolean, parameterized on a type, with a name you can use in a signature.</p>

<!-- snippet: demos/s03/concepts_basics.cpp#concept -->
```cpp
template <typename T>
concept StringLike = std::convertible_to<T, std::string_view>;   // a named predicate on types
```

```cpp
static_assert(StringLike<std::string>);
static_assert(StringLike<const char*>);
static_assert(!StringLike<int>);
```

In the exercise: `StringLike`, `SerializableRange`, and `Serializable` (task 3 and 4).

<!--
Notes: A concept is nothing more than a constexpr bool template with a name and a place in the
grammar. Everything else follows from being able to write that name in four places. Demo file:
demos/s03/concepts_basics.cpp
-->

---

<!-- _class: feature -->

## Four ways to constrain <span class="badge cpp20">C++20</span>

<p class="problem">Same constraint, four spellings. Pick by how much you need to say.</p>

<!-- snippet: demos/s03/concepts_basics.cpp#four -->
```cpp
template <typename T>
    requires StringLike<T>                       // 1. requires-clause after the template head
std::size_t len1(const T& s) { return std::string_view{s}.size(); }

template <typename T>
std::size_t len2(const T& s) requires StringLike<T> { return std::string_view{s}.size(); }   // 2. trailing

template <StringLike T>                          // 3. constrained template parameter
std::size_t len3(const T& s) { return std::string_view{s}.size(); }

std::size_t len4(const StringLike auto& s) { return std::string_view{s}.size(); }   // 4. abbreviated
```

<!--
Notes: 4 (abbreviated) when the constraint is one concept on one parameter. 3 when you need the
type name in the body. 1 when the constraint involves several parameters or is compound. 2
almost never (it exists for member functions of class templates). Demo file:
demos/s03/concepts_basics.cpp
-->

---

<!-- _class: twocol -->

## Abbreviated function templates <span class="badge cpp20">C++20</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s03/concepts_basics.cpp#sfinae -->
```cpp
// The same constraint in C++11/14. Read it aloud to a colleague.
template <typename T, typename std::enable_if<std::is_convertible<T, std::string_view>::value, int>::type = 0>
std::size_t len_old(const T& s) { return std::string_view{s}.size(); }
```

</div>
<div>

#### After (C++20)

```cpp
std::size_t len(const StringLike auto& s) {
    return std::string_view{s}.size();
}

// from the exercise:
std::string serialize(std::integral auto v);
std::string serialize(std::floating_point auto v);
```

</div>
</div>

<!--
Notes: `Concept auto` in a parameter list is a template parameter with a constraint; there is
no `template<>` line. Exercise task 3. Demo file: demos/s03/concepts_basics.cpp
-->

---

<!-- _class: feature dense -->

## `requires` expressions <span class="badge cpp20">C++20</span>

<p class="problem">Concepts defined by what must compile: expressions, types, and their properties.</p>

<!-- snippet: demos/s03/requires_expressions.cpp#requires -->
```cpp
template <typename T>
concept Range = requires(T& t) {
    t.begin();                                       // simple: this expression compiles
    t.end();
    typename T::value_type;                          // type: this type exists
    { t.size() } -> std::convertible_to<std::size_t>;   // compound: type satisfies a concept
    { t.empty() } noexcept -> std::same_as<bool>;    // ...and is noexcept
    requires !std::same_as<T, std::string>;          // nested: another constraint holds
};
static_assert(Range<std::vector<int>>);
static_assert(!Range<std::string>);                  // excluded by the nested requirement
static_assert(!Range<int>);
```

<!--
Notes: Four kinds of requirement: simple (compiles), type (exists), compound (compiles and its
type satisfies a concept, optionally noexcept), nested (another constraint). The exercise's
SerializableRange uses simple requirements plus a nested !StringLike. Demo file:
demos/s03/requires_expressions.cpp
-->

---

## The standard concepts library <span class="badge cpp20">C++20</span>

| Header | Concepts |
|---|---|
| `<concepts>` core | `same_as`, `derived_from`, `convertible_to`, `common_with`, `integral`, `signed_integral`, `floating_point`, `assignable_from`, `swappable` |
| `<concepts>` objects | `destructible`, `constructible_from`, `default_initializable`, `move_constructible`, `copy_constructible`, `movable`, `copyable`, `semiregular`, `regular` |
| `<concepts>` comparison | `equality_comparable`, `totally_ordered`, `three_way_comparable` |
| `<concepts>` callables | `invocable`, `regular_invocable`, `predicate`, `relation`, `strict_weak_order` |
| `<iterator>` | `input_iterator`, `forward_iterator`, ..., `sentinel_for`, `indirectly_readable` |
| `<ranges>` | `range`, `input_range`, `sized_range`, `contiguous_range`, `view`, `borrowed_range` (Session 4) |

Prefer these over your own. They subsume each other correctly and their names are known.

<!--
Notes: regular is the important semantic one: copyable + default_initializable +
equality_comparable, and it PROMISES that copies compare equal, which the compiler cannot check.
-->

---

<!-- _class: feature -->

## Subsumption: the more constrained overload wins <span class="badge cpp20">C++20</span>

<p class="problem">When two constrained overloads both match, the compiler picks the one whose constraints include the other's.</p>

<!-- snippet: demos/s03/subsumption.cpp#subsumption -->
```cpp
template <typename T>
concept HasSize = requires(const T& t) { t.size(); };

template <typename T>
concept Container = HasSize<T> && requires(const T& t) { t.begin(); t.end(); };   // Container subsumes HasSize

const char* describe(const HasSize auto&)   { return "has size"; }
const char* describe(const Container auto&) { return "container"; }    // more constrained: wins when both match

// std::string satisfies both; the compiler picks "container" because Container = HasSize && more.
// It can only see that because Container is written as a conjunction of NAMED concepts.
```

<!--
Notes: This is why the exercise's SerializableRange is `!StringLike<T> && requires {...}`: a
std::string satisfies both the string overload and (has begin/end) the range overload, and
without the exclusion it would be ambiguous. Demo file: demos/s03/subsumption.cpp
-->

---

<!-- _class: feature -->

## Subsumption only sees named concepts <span class="badge cpp20">C++20</span>

<p class="problem">Two requires-expressions are never compared, even if they are textually identical.</p>

<!-- snippet: demos/s03/subsumption.cpp#no_subsumption -->
```cpp
template <typename T>
concept Container2 = requires(const T& t) { t.size(); t.begin(); t.end(); };   // one ad hoc block

const char* describe2(const HasSize auto&)    { return "has size"; }
const char* describe2(const Container2 auto&) { return "container"; }
// describe2(std::string{});   // error: ambiguous. Container2 does not subsume HasSize: two
//                             // requires-expressions are never compared for subsumption.
```

Rule: build concepts as conjunctions of **named** concepts. Ad hoc `requires` blocks are leaves, not layers.

<!--
Notes: The one subsumption rule that bites in practice. The fix is always the same: name the
pieces. Demo file: demos/s03/subsumption.cpp
-->

---

<!-- _class: feature -->

## Concepts as questions about your API <span class="badge cpp20">C++20</span>

<p class="problem">A concept can be defined in terms of your own functions, and then asked.</p>

```cpp
template <typename T>
concept Serializable = requires(const T& t) {
    { serialize(t) } -> std::same_as<std::string>;     // "some serialize overload accepts a T"
};

static_assert(Serializable<int>);
static_assert(Serializable<std::vector<std::vector<int>>>);
static_assert(!Serializable<ParseError>);              // documented, and enforced, in one line
```

Also usable in `if constexpr (Serializable<T>)`, as a constraint on other templates, and as the message in an error.

<!--
Notes: Exercise task 4. This is the slide that shows concepts are not only about constraining:
they are a way to ASK the compiler about your own code. Hand-typed from the solution.
-->

---

## Concept design guidance

- **Name the capability, not the type:** `Serializable`, `Hashable`, `Range`; not `IsVector`
- **Prefer standard concepts** where one fits; they subsume correctly and readers know them
- **Constrain with the weakest concept that makes the body compile.** Over-constraining rejects valid callers; under-constraining gives bad errors, not wrong code
- **Semantic requirements exist.** `std::regular` promises copies compare equal; `totally_ordered` promises transitivity. The compiler checks syntax only. Document the rest.
- **Do not constrain for its own sake.** A function template used with two types in one file does not need a concept. A library boundary does.

<!--
Notes: Opinionated; say so. The third bullet is the one people argue about; the position here is
the same as the Core Guidelines (T.10 through T.26).
-->

---

<!-- _class: feature -->

## Template lambdas and unevaluated lambdas <span class="badge cpp20">C++20</span>

<p class="problem">Name a lambda's parameter types, and use a lambda where only a type is wanted.</p>

<!-- snippet: demos/s03/template_lambdas.cpp#template_lambda -->
```cpp
// C++14 generic lambda: `auto` gives you a type you cannot name inside the body.
auto sum14 = [](const auto& s) { double t = 0; for (auto x : s) t += x; return t; };

// C++20 template lambda: name the parameter types, constrain them, use them in the body.
auto sum20 = []<typename T>(std::span<const T> s) {
    T t{};                                          // T is nameable now
    for (auto x : s) t += x;
    return t;
};
std::vector<int> v{1, 2, 3};
std::println("{} {}", sum14(v), sum20(std::span<const int>{v}));
```

<!-- snippet: demos/s03/template_lambdas.cpp#unevaluated -->
```cpp
// C++20: a lambda may appear in decltype, so a stateless comparator needs no named type.
auto less_by_size = [](const std::string& a, const std::string& b) { return a.size() < b.size(); };
std::set<std::string, decltype(less_by_size)> by_size;   // default-constructible closure (C++20)
by_size.insert("ccc"); by_size.insert("a");
std::unique_ptr<FILE, decltype([](FILE* f) { if (f) std::fclose(f); })> file{std::fopen("/dev/null", "r")};
std::println("{} {}", *by_size.begin(), file != nullptr);
```

<!--
Notes: The unique_ptr-with-lambda-deleter line is the practical one: a RAII wrapper for any C
handle in one declaration, no named deleter struct, no std::function overhead. Demo file:
demos/s03/template_lambdas.cpp
-->

---

<!-- _class: feature -->

## Class types as template arguments <span class="badge cpp20">C++20</span>

<p class="problem">A compile-time string as a template parameter, and why the exercise's SensorConfig cannot be one.</p>

<!-- snippet: demos/s03/auto_nttp.cpp#fixed_string -->
```cpp
// C++20: a class type as a template argument, if it is "structural" (all public members, no
// private state, and every member is itself structural). A string_view is NOT (private pointer).
template <std::size_t N>
struct FixedString {
    char data[N]{};
    constexpr FixedString(const char (&s)[N]) { std::copy_n(s, N, data); }
    constexpr std::string_view view() const { return {data, N - 1}; }
};

template <FixedString Name>                    // a compile-time string as a template argument
struct Sensor {
    static constexpr std::string_view name = Name.view();
};
static_assert(Sensor<"rpm">::name == "rpm");
// Why the exercise's SensorConfig cannot be an NTTP: its string_view members are not structural.
```

<!--
Notes: "Structural type": all public members, each itself structural. std::string_view has a
private pointer, so a struct holding one cannot be an NTTP; FixedString holds a char array and
can. The FixedString idiom is in every modern reflection-ish library. Demo file:
demos/s03/auto_nttp.cpp
-->

---

<!-- _class: twocol -->

## Concepts vs SFINAE: the error messages <span class="badge cpp20">C++20</span>

<div class="cols">
<div>

#### C++11 (`enable_if`)

```
no matching function for call to
  'twice_sfinae(double)'
candidate: template<class T, typename
  std::enable_if<is_integral_v<T>,
  int>::type <anonymous> >
template argument deduction/
  substitution failed:
no type named 'type' in
  'struct std::enable_if<false, int>'
```

</div>
<div>

#### C++20 (concept)

```
no matching function for call to
  'twice_concept(double)'
candidate: 'auto twice_concept(auto:1)'
constraints not satisfied
note: 'double' does not satisfy
  'integral'
```

</div>
</div>

<!--
Notes: The same constraint, the same wrong call, both compilers. The right side says what was
required and what failed to meet it. Demo file: demos/s03/concepts_vs_sfinae.cpp (errors under
SHOW_ERRORS; GCC 14 output quoted)
-->

---

<!-- _class: takeaway -->

## Concepts takeaway

**Constrain every template parameter at a library boundary** with the weakest concept that makes the body compile. Build concepts from named concepts so they subsume.

**The error message is the feature.** Everything else concepts do, `enable_if` could do worse.

**Monday morning:** replace one `enable_if` with a concept and compare the two error messages.

<!--
Notes: 1:20. Deducing this: fifteen minutes.
-->

---

<!-- SEGMENT: Deducing this (1:20) -->

## Three problems, one feature <span class="badge cpp23">C++23</span>

1. **`const` duplication:** every accessor written twice, `const` and non-`const`, with identical bodies
2. **CRTP:** a base class that needs its derived type as a template parameter to call down
3. **Recursive lambdas:** a lambda cannot name itself, so recursion needed `std::function` or a Y-combinator

All three are "the member function does not know the type or value category of the object it was called on". C++23 lets you declare that object as a parameter and deduce it.

<!--
Notes: P0847, "deducing this". The most consequential C++23 language feature for class design.
-->

---

<!-- _class: twocol -->

## Explicit object parameter <span class="badge cpp23">C++23</span>

<div class="cols">
<div>

#### Before (C++11)

```cpp
struct SensorStats {
    SensorStats& add(double v, Status s) {
        // ...update...
        return *this;
    }
};
// SensorStats{}.add(1).add(2)
//   returns a reference to a temporary
//   and copying out of it copies
```

</div>
<div>

#### After (C++23)

```cpp
struct SensorStats {
    template <typename Self>
    constexpr Self&& add(this Self&& self,
                         double v, Status s) {
        // ...update self...
        return std::forward<Self>(self);
    }
};
// stats.add(1).add(2)        -> SensorStats&
// SensorStats{}.add(1).add(2) -> SensorStats&&
```

</div>
</div>

<!--
Notes: Exercise task 7. `this Self&& self` is a forwarding reference to the object; the return
type follows its value category, so chaining on a temporary moves through and chaining on an
lvalue returns a reference. Hand-typed from the solution.
-->

---

<!-- _class: feature -->

## Deduplicating `const` overloads <span class="badge cpp23">C++23</span>

<p class="problem">One accessor instead of two, and the return type follows the object's constness and value category.</p>

<!-- snippet: demos/s03/deducing_this.cpp#const_dedup -->
```cpp
struct Config {
    std::vector<std::string> names;

    // C++11: two copies of every accessor
    //   const std::string& at(std::size_t i) const { return names[i]; }
    //         std::string& at(std::size_t i)       { return names[i]; }

    // C++23: one. Self deduces as Config&, const Config&, or Config&&, and the return follows.
    template <typename Self>
    auto&& at(this Self&& self, std::size_t i) { return std::forward_like<Self>(self.names[i]); }
};
```

`std::forward_like<Self>(member)` (C++23) forwards a member with the object's category: `const&` object gives `const&` member, `&&` object gives `&&` member.

<!--
Notes: The "four overloads" problem (const/non-const times lvalue/rvalue) collapses to one.
forward_like is the companion utility that makes the member's category match. Demo file:
demos/s03/deducing_this.cpp
-->

---

<!-- _class: feature -->

## CRTP without the template parameter <span class="badge cpp23">C++23</span>

<p class="problem">The base calls down into the derived type without being told what it is.</p>

<!-- snippet: demos/s03/deducing_this.cpp#crtp -->
```cpp
// C++11 CRTP: the base needs the derived type as a template parameter.
//   template <typename Derived> struct Shape11 {
//       void draw() { static_cast<Derived*>(this)->draw_impl(); } };
//   struct Circle : Shape11<Circle> { void draw_impl() {...} };

// C++23: the base is a plain struct; self deduces to the derived type at the call site.
struct Shape {
    void draw(this auto&& self) { self.draw_impl(); }
};
struct Circle : Shape { void draw_impl() { std::println("circle"); } };
struct Square : Shape { void draw_impl() { std::println("square"); } };
```

<!--
Notes: `this auto&& self` deduces to the DERIVED type at the call site, because that is the type
of the object the member was called on. No template parameter on the base, no static_cast, and
the base is a plain struct you can put in a container. Demo file: demos/s03/deducing_this.cpp
-->

---

<!-- _class: feature -->

## Recursive lambdas <span class="badge cpp23">C++23</span>

<p class="problem">A lambda that can call itself, with no std::function and no helper.</p>

<!-- snippet: demos/s03/deducing_this.cpp#recursive -->
```cpp
// A recursive lambda without std::function or a Y-combinator:
auto fib = [](this auto self, int n) -> int { return n < 2 ? n : self(n - 1) + self(n - 2); };
```

<!-- snippet: demos/s03/deducing_this.cpp#chain -->
```cpp
struct Builder {
    std::string out;
    template <typename Self>
    Self&& add(this Self&& self, std::string_view s) { self.out += s; return std::forward<Self>(self); }
};
// Builder{}.add("a").add("b") moves the temporary through each call; Builder b; b.add("a") returns b&.
```

<!--
Notes: `this auto self` by value: the closure is empty, so copying it is free, and the
recursion has no indirection. The Builder shows the chaining idiom generalized. Demo file:
demos/s03/deducing_this.cpp
-->

---

<!-- _class: takeaway -->

## Deducing `this` takeaway

Declare the object as a parameter (`this Self&& self`) and the member function knows its type and value category. **One accessor**, **CRTP without the parameter**, **recursive lambdas**.

**The by-value trick:** `this auto self` for small types passed in registers.

**Monday morning:** find one `const`/non-`const` accessor pair and merge it.

<!--
Notes: 1:35. Exercise.
-->

---

<!-- SEGMENT: Exercise (1:35) -->

<!-- _class: demo -->

## Reading the diagnostics

Before the exercise, one live comparison:

```
serialize(ParseError::EmptyLine);
```

in the **starter** (seven overloads) and the **solution** (four constrained templates), on both compilers.

Then break the sensor table (`.min_valid = 500.0`) and rebuild.

<!--
Notes: Two minutes. This primes task 2 and task 8.
-->

---

## Exercise: compile time and concepts

Open `exercises/s03-compile-time/README.md`

**In class (20 minutes):**

1. `make_crc_table()` as `constexpr`, `inline constexpr auto kCrcTable`, `crc16` `constexpr`; then `static_assert(crc16("123456789") == 0x29B1)`. You will hit `std::as_bytes`.
2. `consteval std::string_view validate(std::span<const SensorConfig>)` and a `static_assert` on the table
3. `serialize(std::integral auto)`, `serialize(std::floating_point auto)`, a `StringLike` concept

```
cmake --build build && ctest --test-dir build -R s03 --output-on-failure
```

**At home:** tasks 4 to 8 (`Serializable`, folds, `constexpr` lookups, deducing `this`, the diagnostics). `solution/` is next session's starter.

<!--
Notes: Walk the room. Task 1's as_bytes snag is deliberate; let them find it, then point at the
"reinterpret_cast is never constexpr" slide. Task 2: the static_assert message plus the returned
string are both in the error.
-->

---

<!-- _class: takeaway -->

## Session 3 takeaway

Three functions ran at startup or per call and now run at build time. One class of bug (a bad configuration table) **cannot reach a running program**. The generic code got shorter, and its error messages got readable.

**`constexpr` by default, `consteval` when it must, concepts at every library boundary, deducing `this` for every accessor pair.**

**Next session:** ranges. `SerializableRange` becomes `std::ranges::input_range`, and the report loop becomes a pipeline.

<!--
Notes: Point at the timeline handout: the constexpr row is the longest on it.
-->
