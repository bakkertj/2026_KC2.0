---
marp: true
theme: course
paginate: true
footer: 'The Evolution of C++ | Session 2: Vocabulary Types and the Standard Library'
---

<!-- _class: lead -->
<!-- _paginate: false -->

# The Evolution of C++
## Session 2: Vocabulary Types and the Standard Library

The types that replace raw pointers, sentinel values, out-parameters, and `printf`

<!--
Notes: The most immediately useful session for daily code. Every type today answers a question a
C++11 interface left to a comment: maybe? which one? did it fail, and why? who owns this?
-->

---

## Agenda

1. Recap and the vocabulary-type idea (10 min)
2. Views over data: `string_view`, `span` (25 min)
3. Maybe, either, anything: `optional`, `variant`, `any` (25 min)
4. Error handling: `std::expected` (15 min)
5. Formatting and output: `format`, `print`, `formatter` (15 min)
6. Library tour, fast (10 min)
7. Guided exercise (20 min)

<!--
Notes: Exercise README: exercises/s02-vocabulary-types/README.md. The starter is last session's
solution. Remind people to bring theirs or take ours.
-->

---

## Session 1 recap

The solution to last session's exercise: same program, six operators gone, `using` instead of `typedef`, structured bindings, `[[nodiscard]]` everywhere, `from_chars`, `inline constexpr` constants.

Where people got stuck:

- Leaving one old `operator<` declaration in the header next to the new `<=>`: ambiguous overload
- Trying to bind a structured binding into existing variables (it always declares new ones)
- `from_chars` for `double` missing on libc++ 18: the `__cpp_lib_to_chars` feature-test macro is the fix, and today's theme in miniature

<!--
Notes: The feature-test macro point is worth a minute: <version> (C++20) defines __cpp_lib_* for
every library feature, so code can adapt instead of guessing from compiler versions. It recurs
on the support matrix slide at the end.
-->

---

## What is a vocabulary type?

A type that appears **in interfaces** so that caller and callee agree on meaning without reading a comment.

The Session 1 solution still has five interface smells:

| Signature | What it really means |
|---|---|
| `bool parse(const std::string&, Record* out, ParseError* err)` | "a Record, or a reason" |
| `const SensorConfig* find_sensor(const std::string&)` | "maybe a config" |
| `std::pair<double,double> value_range(const std::vector<Record>&)` | "min and max, **if not empty**" (a comment) |
| `crc16(const unsigned char*, std::size_t)` | "some bytes" |
| `fprintf(out, "%-16s %lu", ...)` | "a formatted line", checked by nobody |

Today each one becomes a type that says it.

<!--
Notes: Have the starter's headers open. Each row is a task in today's exercise.
-->

---

<!-- _class: dense -->

## The ownership vocabulary

| Type | Owns? | Null? | What it says |
|---|---|---|---|
| `T&` | no | never | "I need one, it exists" |
| `T*` | no | maybe | "borrowed, may be absent" (C++98 spelling of optional-ref) |
| `std::unique_ptr<T>` | yes, alone | maybe | "I own this" |
| `std::shared_ptr<T>` | yes, shared | maybe | "lifetime is a runtime question" |
| `std::string_view` | no | no (empty) | "I will only read these characters" |
| `std::span<T>` | no | no (empty) | "a contiguous run I do not own" |
| `std::optional<T>` | yes, inline | maybe | "maybe a T" |
| `std::expected<T,E>` | yes, inline | no | "a T, or the reason there is none" |

<!--
Notes: The slide attendees screenshot. It returns at the end as the interface checklist. The
point of "owns" and "null" columns: a signature should answer both without a comment.
-->

---

<!-- SEGMENT: Views (0:10) -->

<!-- _class: twocol -->

## `std::string_view` <span class="badge cpp17">C++17</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s02/string_view_basics.cpp#before -->
```cpp
// C++11: three overloads, or one that forces a std::string
std::size_t count_commas_cpp11(const std::string& s) {
    std::size_t n = 0;
    for (char c : s) n += (c == ',');
    return n;
}
// count_commas_cpp11("a,b");         // constructs a std::string
// count_commas_cpp11(buf, len);      // no such overload
```

</div>
<div>

#### After (C++17)

<!-- snippet: demos/s02/string_view_basics.cpp#after -->
```cpp
// C++17: one function; literal, std::string, or slice, no copy
std::size_t count_commas(std::string_view s) {
    std::size_t n = 0;
    for (char c : s) n += (c == ',');
    return n;
}
```

</div>
</div>

<!--
Notes: A pointer and a length, 16 bytes, trivially copyable, pass by value. Accepts a literal, a
std::string, a substring, a char buffer, with no allocation in any case. This is `split` from the
exercise (task 5). Demo file: demos/s02/string_view_basics.cpp
-->

---

<!-- _class: feature -->

## `string_view` operations <span class="badge cpp17">C++17</span>

<p class="problem">Everything you can do without owning the characters, and all of it O(1) or a search.</p>

<!-- snippet: demos/s02/string_view_basics.cpp#ops -->
```cpp
void ops(std::string_view line) {
    auto head = line.substr(0, 5);          // O(1): a smaller view
    line.remove_prefix(2);                  // O(1): moves the start
    bool b = line.starts_with("25");        // C++20
    bool c = line.contains("temp");         // C++23
    auto pos = line.find(',');              // like std::string
    std::printf("%.*s %d %d %zu\n", static_cast<int>(head.size()), head.data(), b, c, pos);
}
```

`substr` and `remove_prefix` do not copy: they return or become a smaller view. `starts_with`/`ends_with` are C++20, `contains` is C++23, on `std::string` too.

<!--
Notes: Parsing a line into fields with substr and find is the exercise's split(): a vector of
views into the caller's buffer. Demo file: demos/s02/string_view_basics.cpp
-->

---

<!-- _class: feature dense -->

## The dangling rule <span class="badge cpp17">C++17</span>

<p class="problem">A view is a reference. It is valid exactly as long as the thing it views.</p>

<!-- snippet: demos/s02/string_view_dangling.cpp#dangling -->
```cpp
struct Config {
    std::string_view name;          // (3) stored view: safe only if the owner outlives Config
};

#ifdef SHOW_ERRORS                  // Clang 18 rejects all three below under -Werror:
std::string_view first_word() {     //   -Wreturn-stack-address, -Wdangling-gsl
    std::string s = "hello world";
    return std::string_view(s).substr(0, 5);   // (2) view of a local: dangles at return
}

void dangling() {
    std::string_view a = make();    // (1) view of a temporary: dead at the end of this line
    const char* p = std::string("x").c_str();   // (4) same bug, C++98 edition
    (void)a; (void)p;
}
#endif
```

<!-- snippet: demos/s02/string_view_dangling.cpp#safe -->
```cpp
std::string_view sensor_of(std::string_view line) {   // view in, view of the SAME buffer out: fine
    const auto comma = line.find(',');
    return line.substr(comma + 1);
}
```

<!--
Notes: Four ways to dangle: view of a temporary, returning a view of a local, storing a view
past its owner, and the C++98 version, c_str() of a temporary. Clang 18 rejects three of them
under -Werror (-Wdangling-gsl, -Wreturn-stack-address), which is why they are behind
SHOW_ERRORS. The safe pattern: view in, view of the same buffer out. Rule: a string_view
parameter is fine; a string_view member or return needs a lifetime argument written down.
Demo file: demos/s02/string_view_dangling.cpp
-->

---

<!-- _class: feature -->

## A `string_view` is not a C string <span class="badge cpp17">C++17</span>

<p class="problem">No NUL terminator. Anything that walks to a NUL reads past the end.</p>

<!-- snippet: demos/s02/string_view_not_cstring.cpp#nul -->
```cpp
double bad(std::string_view field) {
    return std::strtod(field.data(), nullptr);   // WRONG: reads past field.size() to find a NUL
}

double ok(std::string_view field) {
    return std::strtod(std::string(field).c_str(), nullptr);   // copy when a C API needs a NUL
}
```

The exercise hits this exactly once: the `strtod` fallback under `#ifndef __cpp_lib_to_chars` must copy the field.

<!--
Notes: The bad() version "works" in the demo by luck (the next char is a comma). It is a
read past the end of the field, and with a field at the end of a buffer it is a read past the
end of the buffer. Any C API: copy into a std::string first. Demo file:
demos/s02/string_view_not_cstring.cpp
-->

---

## When to still take `const std::string&` <span class="badge cpp17">C++17</span>

Take **`std::string_view`** when you only read: parsers, lookups, comparisons, logging.

Take **`const std::string&`** when you will call `c_str()`, or pass it to something that wants a `std::string` (avoids a copy-then-copy).

Take **`std::string` by value** when you will **keep** it: a sink parameter you `std::move` into a member. The caller decides whether to copy or move.

Never take `const std::string_view&`: it is already two words; pass by value.

<!--
Notes: The third rule is the one that changed with move semantics and people still miss it:
by-value-then-move for constructors and setters. One copy at most, zero if the caller moves.
-->

---

<!-- _class: twocol -->

## `std::span` <span class="badge cpp20">C++20</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s02/span_basics.cpp#before -->
```cpp
// C++11: says "a vector" when it means "some doubles"
double mean_cpp11(const std::vector<double>& v) {
    double s = 0;
    for (double x : v) s += x;
    return s / static_cast<double>(v.size());
}
// mean_cpp11(arr);            // error: a C array is not a vector
// mean_cpp11({v.begin()+1, v.end()});   // copies
```

</div>
<div>

#### After (C++20)

<!-- snippet: demos/s02/span_basics.cpp#after -->
```cpp
// C++20: any contiguous run of doubles, no copy, no ownership
double mean(std::span<const double> v) {
    double s = 0;
    for (double x : v) s += x;
    return s / static_cast<double>(v.size());
}
```

</div>
</div>

<!-- snippet: demos/s02/span_basics.cpp#calls -->
```cpp
mean(v);                       // vector
mean(arr);                     // C array: size deduced
mean(a);                       // std::array
mean(std::span{v}.subspan(1, 2));   // a slice: {2, 3}
mean({v.data() + 2, 2});       // pointer + count, the C interface
```

<!--
Notes: The vector& version said "a vector" when it meant "some doubles". A C array, a std::array,
a slice, or a pointer+count all become a span with no copy. This is value_range,
compute_stats and top_n_by_value in the exercise (task 6). Demo file: demos/s02/span_basics.cpp
-->

---

<!-- _class: dense -->

## `span` mechanics <span class="badge cpp20">C++20</span>

- `std::span<T>`: dynamic extent, pointer + size (16 bytes). `std::span<T, 8>`: static extent, pointer only, size checked at compile time when constructed from an array
- `span<const T>` for read-only; `span<T>` lets the callee write through (mutating a buffer in place)
- `first(n)`, `last(n)`, `subspan(off, n)`: slicing without copying
- `std::as_bytes(s)` / `as_writable_bytes(s)`: the same memory as `span<const std::byte>`
- No bounds checking on `operator[]` (like a pointer); no `at()` until C++26
- Same lifetime rule as `string_view`: a parameter, not a member, unless you know the owner

<!--
Notes: Static extent is the embedded case: a function that takes exactly a 12-byte header, and
the compiler rejects an 8-byte array. Show the demo's `std::span<std::byte, 8>` parameter.
-->

---

<!-- _class: feature -->

## `span` at a hardware boundary <span class="badge cpp20">C++20</span>

<p class="problem">Buffers, register windows, DMA descriptors: a pointer and a length, made into a type.</p>

<!-- snippet: demos/s02/span_bytes.cpp#bytes -->
```cpp
std::uint8_t checksum(std::span<const std::byte> data) {     // any object's bytes, any buffer
    std::uint8_t sum = 0;
    for (std::byte b : data) sum = static_cast<std::uint8_t>(sum + std::to_integer<std::uint8_t>(b));
    return sum;
}

struct Packet { std::uint16_t id; std::uint16_t len; std::uint32_t crc; };

void demo(std::string_view text, const Packet& p, std::span<std::byte, 8> fixed) {
    checksum(std::as_bytes(std::span{text}));                 // a string's bytes
    checksum(std::as_bytes(std::span{&p, 1}));                // a struct's bytes
    checksum(fixed);                                          // static extent: exactly 8, checked at compile time
}
```

`std::byte` (C++17) is "raw memory": no arithmetic, no implicit conversion to a number. Bit operations only.

<!--
Notes: crc16 in the exercise becomes crc16(std::span<const std::byte>) with a string_view
overload via as_bytes. Point out that std::byte is an enum class, so `b + 1` does not compile;
that is the feature. Demo file: demos/s02/span_bytes.cpp
-->

---

## `std::mdspan`: a preview <span class="badge cpp23">C++23</span>

```cpp
std::array<float, 12> buf{};
std::mdspan<float, std::extents<std::size_t, 3, 4>> m{buf.data()};   // 3x4 view over flat storage
m[1, 2] = 7.0f;                            // C++23 multidimensional subscript
std::mdspan<float, std::dextents<std::size_t, 2>> d{buf.data(), 3, 4};   // dynamic extents
```

- A view, like `span`, but with a shape and a **layout policy** (`layout_right`, `layout_left`, `layout_stride`): the same buffer as row-major or column-major with no copy
- Available: libc++ 17+; libstdc++ from GCC 15. Compiler Explorer for today.

<!--
Notes: One slide. The audience for it is anyone with image or matrix buffers. Hand-typed:
GCC 14 lacks the header.
-->

---

<!-- _class: feature -->

## Transparent comparators <span class="badge cpp14">C++14</span>

<p class="problem">Looking up a string_view key in a map<string, T> constructed a std::string per lookup.</p>

<!-- snippet: demos/s02/transparent_compare.cpp#transparent -->
```cpp
std::map<std::string, int> plain;
std::map<std::string, int, std::less<>> transparent;   // std::less<> compares any two comparable types

void lookups(std::string_view key) {
    plain.find(std::string(key));      // must build a std::string to search: an allocation per lookup (past SSO)
    transparent.find(key);             // compares string_view to string directly: no allocation
}
```

`std::less<>` (no template argument) compares any two comparable types. C++20 adds the same for `unordered_map` with a transparent hash **and** a transparent key-equal (`std::equal_to<>`).

<!--
Notes: The exercise's StatsBySensor gains std::less<> so find("rpm") works from a literal (task
9). Small feature, big effect in hot lookup paths. Demo file: demos/s02/transparent_compare.cpp
-->

---

<!-- _class: takeaway -->

## Views takeaway

`string_view` and `span` are the same idea: **a non-owning view of contiguous memory, passed by value**.

Their whole cost is one rule: **a view lives no longer than what it views.** As parameters, that is free. As members or return values, write the lifetime down.

**Monday morning:** change one `const std::string&` parameter that only reads to `std::string_view`.

<!--
Notes: 0:35. The next segment is the one with the most new types; keep moving.
-->

---

<!-- SEGMENT: Maybe, either, anything (0:35) -->

<!-- _class: twocol -->

## `std::optional` <span class="badge cpp17">C++17</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s02/optional_basics.cpp#before -->
```cpp
// C++11: a pointer that might be null, and "look elsewhere for the object"
const SensorConfig* find_cpp11(std::string_view name) {
    for (const auto& s : table) if (s.name == name) return &s;
    return nullptr;
}
```

</div>
<div>

#### After (C++17)

<!-- snippet: demos/s02/optional_basics.cpp#after -->
```cpp
// C++17: "maybe a SensorConfig", held inline, no pointer, no allocation
std::optional<SensorConfig> find(std::string_view name) {
    for (const auto& s : table) if (s.name == name) return s;
    return std::nullopt;
}
```

</div>
</div>

<!--
Notes: A nullable pointer says two things: "maybe" and "the object lives somewhere else". optional
says only "maybe" and holds the object inline. find_sensor in the exercise (task 2). Demo file:
demos/s02/optional_basics.cpp
-->

---

<!-- _class: feature -->

## `optional` mechanics <span class="badge cpp17">C++17</span>

<p class="problem">A T plus a bool, with pointer-like syntax and no allocation.</p>

<!-- snippet: demos/s02/optional_basics.cpp#use -->
```cpp
if (auto cfg = find("rpm")) {                  // contextual bool
    std::printf("%f\n", cfg->max);              // -> and * : unchecked, like a pointer
}
double m = find("nope").value_or(SensorConfig{"none", 0.0}).max;   // default when empty
auto c = find("temp");
c->max = 1.0;                                   // mutable: optional holds the object itself
c.reset();                                      // now empty
// find("nope").value();                        // throws std::bad_optional_access
```

<!--
Notes: `*` and `->` are unchecked (UB when empty), value() throws bad_optional_access. Prefer
the if-with-initializer form. sizeof(optional<double>) is 16: the bool plus padding. An
optional<T&> does not exist (C++26 adds it); use a pointer or std::reference_wrapper.
-->

---

## `optional` as return, member, parameter <span class="badge cpp17">C++17</span>

- **Return type:** yes. "Maybe a result" is what it is for. Replaces `-1`, `nullptr`, `""`, `bool` + out-param.
- **Member:** sometimes. A lazily computed field, or a genuinely absent configuration value. Not as a substitute for a proper default.
- **Parameter:** rarely. `f(std::optional<int> timeout)` usually reads better as two overloads or a default argument. Exception: forwarding a maybe-value through a layer.
- **Never** `std::optional<bool>`: three states with two names is a bug factory. Use an enum.

<!--
Notes: Opinionated slide; say so. The optional<bool> rule gets nods from anyone who has
debugged one.
-->

---

<!-- _class: twocol -->

## Monadic `optional` <span class="badge cpp23">C++23</span>

<div class="cols">
<div>

#### Before (C++17)

<!-- snippet: demos/s02/optional_monadic.cpp#before -->
```cpp
std::string units_cpp17(std::string_view name) {
    auto cfg = find_sensor(name);
    if (!cfg) return "";
    return std::string(cfg->units);
}
```

</div>
<div>

#### After (C++23)

<!-- snippet: demos/s02/optional_monadic.cpp#after -->
```cpp
std::string units(std::string_view name) {
    return std::string(find_sensor(name)
        .transform([](const SensorConfig& c) { return c.units; })   // optional<T> -> optional<U>
        .value_or(""));
}

std::optional<int> doubled(std::string_view s) {
    return to_int(s)
        .and_then([](int n) -> std::optional<int> { return n > 100 ? std::nullopt : std::optional{n}; })  // may fail
        .transform([](int n) { return n * 2; })                                                          // cannot fail
        .or_else([] { return std::optional{0}; });                                                       // recover
}
```

</div>
</div>

<!--
Notes: transform: apply a function that cannot fail, stay wrapped. and_then: apply one that
returns another optional (may fail). or_else: recover. The units lookup in the exercise's report
(task 8). Demo file: demos/s02/optional_monadic.cpp
-->

---

<!-- _class: feature -->

## The pointer-to-member trap <span class="badge cpp23">C++23</span>

<p class="problem">transform(&Struct::member) does not compile, on a temporary or a named optional, and the error is unhelpful.</p>

<!-- snippet: demos/s02/optional_monadic.cpp#trap -->
```cpp
// find_sensor(name).transform(&SensorConfig::units)   // does not compile: invoking the pointer-to-member
//                                                     // yields a reference to the member (string_view&
//                                                     // or &&), and optional<T&> is ill-formed. Use a lambda.
```

Invoking a pointer-to-member yields a reference to the member (an lvalue reference on a named optional, an rvalue reference on a temporary); `optional<T&>` and `optional<T&&>` are both ill-formed in C++23. A lambda that returns by value is the fix.

<!--
Notes: Both compilers produce a wall of text for this; the exercise README asks attendees to
read it once. It is a value-category bug (see the supplemental deck).
-->

---

<!-- _class: twocol -->

## `std::variant` <span class="badge cpp17">C++17</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s02/variant_visit.cpp#before -->
```cpp
// C++11: a tag plus a union; the compiler enforces nothing
struct ValueCpp11 {
    enum Kind { Int, Double, Text } kind;
    union { int i; double d; };
    std::string text;      // cannot live in the union, so it sits beside it
};
```

</div>
<div>

#### After (C++17)

<!-- snippet: demos/s02/variant_visit.cpp#after -->
```cpp
// C++17: exactly one of these, and the compiler knows which
using Value = std::variant<int, double, std::string>;

void inspect(const Value& v) {
    if (std::holds_alternative<int>(v)) std::printf("int %d\n", std::get<int>(v));
    if (auto* d = std::get_if<double>(&v)) std::printf("double %f\n", *d);   // nullptr if not a double
    std::printf("index %zu\n", v.index());                                   // 0, 1, or 2
    // std::get<int>(v) on a string throws std::bad_variant_access
}
```

</div>
</div>

<!--
Notes: A type-safe tagged union. Exactly one alternative is alive; the variant knows which.
get<T> throws on the wrong type; get_if returns nullptr; index() gives the position. Demo file:
demos/s02/variant_visit.cpp
-->

---

<!-- _class: feature -->

## `std::visit` and the overload-set visitor <span class="badge cpp17">C++17</span>

<p class="problem">A chain of holds_alternative checks is a switch the compiler cannot verify. visit is a switch it can.</p>

<!-- snippet: demos/s02/variant_visit.cpp#visitor -->
```cpp
template <class... Fs> struct overloaded : Fs... { using Fs::operator()...; };   // one object, all the overloads
template <class... Fs> overloaded(Fs...) -> overloaded<Fs...>;                // C++17 needs this guide; C++20 CTAD does not

std::string describe(const Value& v) {
    return std::visit(overloaded{
        [](int i)                { return "int " + std::to_string(i); },
        [](double d)             { return "double " + std::to_string(d); },
        [](const std::string& s) { return "text " + s; },
    }, v);
}
// Add a fourth alternative to Value and this stops compiling until some lambda accepts it.
// (One that converts implicitly, float to double, const char* to std::string, is accepted silently.)
```

<!--
Notes: The `overloaded` struct is two lines that appear in every codebase that uses variant:
inherit from all the lambdas, pull in all their operator()s, and CTAD (Session 1) deduces the
types; in C++17 the deduction guide is required, C++20 aggregate CTAD makes it optional.
Exhaustive: add an alternative and visit stops compiling until some lambda accepts it (an
implicit conversion to an existing parameter type counts, so float to double passes silently). Demo file:
demos/s02/variant_visit.cpp
-->

---

<!-- _class: feature -->

## Variant in practice: a message set <span class="badge cpp17">C++17</span>

<p class="problem">A protocol with a fixed set of message types is a variant, not a class hierarchy.</p>

<!-- snippet: demos/s02/variant_messages.cpp#messages -->
```cpp
struct Reading   { std::uint16_t sensor; double value; };
struct Heartbeat { std::uint32_t uptime_s; };
struct Fault     { std::uint16_t code; std::string detail; };

using Message = std::variant<Reading, Heartbeat, Fault>;   // the whole protocol, in one line

template <class... Fs> struct overloaded : Fs... { using Fs::operator()...; };
template <class... Fs> overloaded(Fs...) -> overloaded<Fs...>;

void handle(const Message& m) {
    std::visit(overloaded{
        [](const Reading& r)   { std::printf("reading %u = %.2f\n", r.sensor, r.value); },
        [](const Heartbeat& h) { std::printf("alive %us\n", h.uptime_s); },
        [](const Fault& f)     { std::printf("FAULT %u: %s\n", f.code, f.detail.c_str()); },
    }, m);
}
```

<!-- snippet: demos/s02/variant_messages.cpp#vs_virtual -->
```cpp
// variant: closed set of types, open set of operations (add a visitor anywhere)
// virtual: open set of types, closed set of operations (add a method to the base)
// Protocols and ASTs are closed sets of types. Plugins are open sets. Choose accordingly.
```

<!--
Notes: The closed-vs-open distinction is the whole decision. A telemetry protocol, an AST, a
state machine's states: closed set of types, many operations, variant. A plugin interface: open
set of types, fixed operations, virtual. No heap allocation, no vtable, value semantics, and
sizeof is the largest alternative plus an index. Demo file: demos/s02/variant_messages.cpp
-->

---

## `variant` details worth knowing <span class="badge cpp17">C++17</span>

- Default-constructs the **first** alternative; use `std::monostate` as the first if "empty" is a state
- `valueless_by_exception()`: true only if a conversion threw mid-assignment; rare, but `visit` throws on it
- `==`, `<`, `std::hash` work if every alternative supports them
- Alternatives may repeat (`variant<int, int>`); then use `get<0>`, not `get<int>`
- `std::visit` with two variants dispatches on both (double dispatch for free)
- C++26 adds member `visit` so `msg.visit(...)` reads naturally

<!--
Notes: Fast slide. monostate is the one that comes up in practice.
-->

---

<!-- _class: feature -->

## `std::any` <span class="badge cpp17">C++17</span>

<p class="problem">Sometimes the set of types is genuinely open and only the caller knows what it put in.</p>

<!-- snippet: demos/s02/any.cpp#any -->
```cpp
std::map<std::string, std::any> properties;   // a bag of values whose types are known only to the caller

void use() {
    properties["retries"] = 3;
    properties["name"] = std::string("gimbal");

    int r = std::any_cast<int>(properties["retries"]);            // throws std::bad_any_cast if wrong
    if (auto* s = std::any_cast<std::string>(&properties["name"])) {   // nullptr if wrong
        std::printf("%d %s\n", r, s->c_str());
    }
    // properties["retries"].type() == typeid(int)
}
```

Heap-allocates for anything larger than a pointer or two. Honest advice: plugin boundaries, scripting bridges, property bags. If you can name the alternatives, use `variant`.

<!--
Notes: any is the type people reach for and regret. It is the right tool about once per codebase.
Demo file: demos/s02/any.cpp
-->

---

## Choosing between them

| You need | Use | Not |
|---|---|---|
| Maybe one `T` | `optional<T>` | `T*`, sentinel values, `pair<T, bool>` |
| Exactly one of a **fixed** set | `variant<A, B, C>` | tag + union, `void*`, a base class with `dynamic_cast` |
| One of an **open** set, fixed operations | virtual base class | `variant` |
| Anything, checked at runtime | `any` | `void*` |
| Someone else's `T`, maybe absent | `T*` (non-owning) | `optional<T&>` (does not exist yet), `shared_ptr` |
| A `T` or a reason there is none | `expected<T, E>` (next) | `optional` (loses the reason), exceptions for expected failures |

<!--
Notes: Same table as the ownership one, from the other direction: start from the need.
-->

---

## Where these came from

<div class="evo">
<div class="step"><b>2003–2004</b> Boost.Optional (1.30) and Boost.Variant (1.31) ship; a decade of production use follows</div>
<div class="step"><b>2013</b> Library Fundamentals TS proposes optional; variant debated (what happens on a throwing assignment?)</div>
<div class="step"><b>2016</b> The "never valueless" vs "valueless by exception" argument settles; all three land in C++17</div>
<div class="step"><b>2022</b> std::expected adopted for C++23, from the same lineage (Boost.Outcome, tl::expected)</div>
<div class="step"><b>2023–</b> Monadic operations on optional/expected; optional&lt;T&amp;&gt; and member visit for C++26</div>
</div>

<!--
Notes: Optional history slide; skip if behind. The point: these types were proven in Boost for
fifteen years before standardization, so their designs are not experimental.
-->

---

<!-- _class: takeaway -->

## Maybe, either, anything: takeaway

**`optional`** replaces every sentinel value and every `bool` + out-parameter in your codebase.
**`variant`** replaces every tag-plus-union and most small class hierarchies over a fixed set.
**`any`** replaces `void*`, and almost nothing else.

**Monday morning:** find one function returning `-1` or `nullptr` for "not found" and give it an `optional` return.

<!--
Notes: 1:00. On to expected.
-->

---

<!-- SEGMENT: expected (1:00) -->

<!-- _class: dense -->

## The error-handling landscape

| Mechanism | Carries a reason? | Ignorable? | Composes? | Cost on the happy path |
|---|---|---|---|---|
| Exceptions | yes | no | yes (propagate) | ~zero; expensive to throw |
| Error codes (`int`, `errno`) | yes | **yes** | no | a compare |
| `bool` + out-params | yes, via another param | **yes** | no | a compare |
| `std::error_code` (C++11) | yes, typed | yes | no | a compare |
| `optional<T>` | **no** | yes, unless `[[nodiscard]]` | yes (monadic) | a compare |
| `expected<T, E>` (C++23) | yes | yes, unless `[[nodiscard]]` | yes (monadic) | a compare |

<!--
Notes: The gap in the table is the row optional cannot fill: it says "no" without saying why.
expected is optional with a reason. Two minutes; the next slide is the payoff.
-->

---

<!-- _class: twocol -->

## `std::expected` <span class="badge cpp23">C++23</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s02/expected_basics.cpp#before -->
```cpp
// C++11: a flag and two out-parameters; the caller can ignore all three
bool parse_cpp11(std::string_view s, int* out, ParseError* err) {
    if (s.empty()) { *err = ParseError::Empty; return false; }
    auto r = std::from_chars(s.data(), s.data() + s.size(), *out);
    if (r.ec != std::errc{}) { *err = ParseError::NotANumber; return false; }
    if (r.ptr != s.data() + s.size()) { *err = ParseError::Trailing; return false; }
    return true;
}
```

</div>
<div>

#### After (C++23)

<!-- snippet: demos/s02/expected_basics.cpp#after -->
```cpp
// C++23: the value, or the reason. One return; add [[nodiscard]] so the caller must look at it.
std::expected<int, ParseError> parse(std::string_view s) {
    if (s.empty()) return std::unexpected(ParseError::Empty);
    int v = 0;
    auto r = std::from_chars(s.data(), s.data() + s.size(), v);
    if (r.ec != std::errc{}) return std::unexpected(ParseError::NotANumber);
    if (r.ptr != s.data() + s.size()) return std::unexpected(ParseError::Trailing);
    return v;
}
```

</div>
</div>

<!--
Notes: This is parse_record from the exercise (task 1). One return type carries the Record or the
ParseError; the caller cannot forget to check because there is nothing to use until they do.
Note ParseError::None disappears: an error enum no longer needs a "not an error" value. Demo
file: demos/s02/expected_basics.cpp
-->

---

<!-- _class: feature -->

## `expected` mechanics <span class="badge cpp23">C++23</span>

<p class="problem">Same shape as optional, plus an error() side.</p>

<!-- snippet: demos/s02/expected_basics.cpp#use -->
```cpp
if (auto n = parse("42")) std::printf("%d\n", *n);               // bool, *, -> like optional
auto e = parse("4x");
if (!e) std::printf("error %d\n", static_cast<int>(e.error()));   // the reason
int v = parse("").value_or(-1);                                    // default
ParseError why = parse("7").error_or(ParseError::Empty);           // C++23: the error, or a default
// parse("").value();   // throws std::bad_expected_access<ParseError>, carrying the error
```

- `std::unexpected(e)` constructs the error state; a plain `return value;` constructs the success state
- `value()` throws `bad_expected_access<E>`, which carries the `E`
- `error_or(default)` is the C++23 mirror of `value_or`

<!--
Notes: The unexpected() wrapper is what makes `return v;` and `return unexpected(e);` unambiguous
even when T and E are the same type. Demo file: demos/s02/expected_basics.cpp
-->

---

<!-- _class: feature -->

## Monadic `expected`: a pipeline <span class="badge cpp23">C++23</span>

<p class="problem">Three stages that can each fail, without three ifs.</p>

<!-- snippet: demos/s02/expected_pipeline.cpp#chain -->
```cpp
// Each stage may fail; the first failure short-circuits the rest.
std::expected<double, Error> scaled(std::string_view line) {
    return read_field(line)
        .and_then(to_int)                              // expected<sv> -> expected<int>
        .and_then(check_range)                         // expected<int> -> expected<int>
        .transform([](int v) { return v * 0.5; });     // cannot fail: plain value in, wrapped out
}

// Map an inner error type to an outer one at a layer boundary.
enum class ApiError { Parse, Io };
std::expected<double, ApiError> api(std::string_view line) {
    return scaled(line).transform_error([](Error e) {
        return e == Error::Io ? ApiError::Io : ApiError::Parse;
    });
}
```

<!--
Notes: and_then for stages that can fail (they return an expected), transform for those that
cannot, transform_error to change the error type at a layer boundary (an inner ParseError
becomes an outer ApiError), or_else to recover. The first failure short-circuits everything
after it. Compare with the commented-out C++11 version in the demo file. Demo file:
demos/s02/expected_pipeline.cpp
-->

---

## Designing the error type <span class="badge cpp23">C++23</span>

- **`enum class`**: the exercise's `ParseError`. Cheap, comparable, switchable. Right for one subsystem.
- **A struct**: `{ ParseError kind; std::size_t line; std::string detail; }` when the caller needs context. Keep it small: `expected` is `max(sizeof T, sizeof E)` plus a flag.
- **`std::error_code`**: interoperates with `<system_error>` and OS errors; heavier to define.
- **A `variant` of error kinds**: when layers have different errors and you do not want to flatten them.
- Mark every `expected`-returning function `[[nodiscard]]`. The type says "check me"; the attribute enforces it.

<!--
Notes: The size point matters for hot paths: expected<Record, ParseError> is sizeof(Record) + 8.
Do not put a std::string in E unless you need it.
-->

---

<!-- _class: feature -->

## `expected` vs exceptions <span class="badge cpp23">C++23</span>

<p class="problem">Not a replacement. A second tool, for a different kind of failure.</p>

**Exceptions still win** for: constructors (no return value), failures that are genuinely exceptional (out of memory, broken invariant), deep call stacks where every layer would just forward the error.

**`expected` wins** for: failures that are **expected** (bad input, not found, timeout), hot paths (no unwinding machinery), `-fno-exceptions` environments, and code where "what can fail here" should be readable in the signature.

**What `expected` costs:** a branch at every call, a bigger return type, and the discipline to not `.value()` blindly.

<!--
Notes: Take a position: in an embedded or safety-critical codebase that already bans exceptions,
expected is what makes that policy defensible rather than a workaround. In application code, use
both, by the rule above. The audience will have opinions; let them.
-->

---

<!-- _class: feature -->

## `expected<void, E>` <span class="badge cpp23">C++23</span>

<p class="problem">"Did it work, and if not, why": no value to return, but a reason to report.</p>

<!-- snippet: demos/s02/expected_void.cpp#void -->
```cpp
// "Did it work, and if not, why": no value to return, but a reason to report.
std::expected<void, IoError> write(std::string_view data, bool open) {
    if (!open) return std::unexpected(IoError::NotOpen);
    if (data.size() > 64) return std::unexpected(IoError::Full);
    return {};                                    // success: an empty expected
}

void use() {
    if (auto r = write("hello", true); !r) {
        std::printf("failed: %d\n", static_cast<int>(r.error()));
    }
}
```

Coroutines (Session 5) pair naturally with this: `co_await` an operation that yields an `expected`.

<!--
Notes: The void specialization is the replacement for `bool write(...)` plus a global error
state. `return {};` is success. Demo file: demos/s02/expected_void.cpp
-->

---

<!-- _class: takeaway -->

## `expected` takeaway

`expected<T, E>` is `optional<T>` **with a reason**, and it is the piece that makes "no exceptions" a policy rather than a workaround.

Use it for failures you **expect**. Keep exceptions for the ones you do not.

**Monday morning:** take one `bool f(..., Error* err)` function and give it an `expected` return.

<!--
Notes: 1:15. Formatting next; it is the most immediately gratifying segment.
-->

---

<!-- SEGMENT: Formatting (1:15) -->

## Why `printf` and `iostream` both lost

**`printf`:** the format string and the arguments are checked by nobody (`%s` with an `int`: UB). No user types. Casts everywhere (`%lu` with `size_t`).

**`iostream`:** stateful (`std::hex` sticks until you unset it), verbose (`std::setw(8) << std::setprecision(3) << std::fixed`), slow (locale and virtual dispatch per operation), and `<<` chains do not compose into a string easily.

**`std::format` (C++20) and `std::print` (C++23):** `printf`'s ergonomics, checked at compile time, extensible to your types, and fast.

<!--
Notes: If anyone asks: {fmt} is the library std::format was standardized from, and it is a drop-in
for pre-C++20 codebases.
-->

---

<!-- _class: twocol -->

## `std::format` <span class="badge cpp20">C++20</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s02/format_specs.cpp#before -->
```cpp
// C++11: snprintf. A buffer, a size, and a format string checked by nobody.
std::string fmt_cpp11(double v) {
    char buf[64];
    std::snprintf(buf, sizeof buf, "%.3f", v);
    return buf;
}
```

</div>
<div>

#### After (C++20)

<!-- snippet: demos/s02/format_specs.cpp#after -->
```cpp
// C++20: checked at compile time, no buffer, returns a std::string
std::string fmt(double v) { return std::format("{:.3f}", v); }
```

</div>
</div>

A mismatched format string is a **compile error**, because the format string is a `consteval` parameter.

<!--
Notes: serialize(double) from the exercise. Show the compile error live: std::format("{:d}",
3.14) does not build. Demo file: demos/s02/format_specs.cpp
-->

---

<!-- _class: feature -->

## The format spec mini-language <span class="badge cpp20">C++20</span>

<p class="problem">Everything printf's % could do, in a syntax you can read.</p>

<!-- snippet: demos/s02/format_specs.cpp#specs -->
```cpp
auto show = [](std::string_view r) { std::printf("[%.*s]\n", static_cast<int>(r.size()), r.data()); };
show(std::format("{:.3f}", 3.14159));        // [3.142]         precision
show(std::format("{:<16}|", "left"));        // [left            |]   width, left-align
show(std::format("{:>8}", 42));              // [      42]      right-align
show(std::format("{:^9}", "mid"));           // [   mid   ]     center
show(std::format("{:04X}", 0xBEEF));         // [BEEF]          zero-pad, hex upper
show(std::format("{:#010x}", 255));          // [0x000000ff]    alternate form
show(std::format("{:+d}", 5));               // [+5]            always sign
show(std::format("{:e}", 12345.678));        // [1.234568e+04]
show(std::format("{1} {0}", "a", "b"));      // [b a]           positional
show(std::format("{:*^11}", "x"));           // [*****x*****]   fill character
show(std::format("{{}}"));                   // [{}]            escaping
```

<!--
Notes: `{[index]:[fill][align][sign][#][0][width][.precision][type]}`. The translation table
for the exercise (task 3): %-16s is {:<16}, %.3f is {:.3f}, %04X is {:04X}, %lu is {}.
Demo file: demos/s02/format_specs.cpp
-->

---

<!-- _class: twocol -->

## `std::print` and `std::println` <span class="badge cpp23">C++23</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s02/print.cpp#before -->
```cpp
// C++11: printf casts and format letters; iostream manipulators that stick
void report_cpp11(std::FILE* out, const std::string& name, std::size_t n, double mean) {
    std::fprintf(out, "%-16s n=%lu mean=%.3f\n", name.c_str(), static_cast<unsigned long>(n), mean);
}
```

</div>
<div>

#### After (C++23)

<!-- snippet: demos/s02/print.cpp#after -->
```cpp
// C++23: format-string safety, printf ergonomics, and a FILE* overload
void report(std::FILE* out, const std::string& name, std::size_t n, double mean) {
    std::println(out, "{:<16} n={} mean={:.3f}", name, n, mean);
}
```

</div>
</div>

Overloads for `FILE*` and `std::ostream`. Unicode-correct on Windows consoles. `println()` with no arguments is C++26 (P3142).

<!--
Notes: The report in the exercise (task 3): every fprintf becomes a println and every cast to
unsigned long disappears. The FILE* overload is why write_report keeps its FILE* parameter and the
tests keep tmpfile(). Demo file: demos/s02/print.cpp
-->

---

<!-- _class: feature -->

## `std::formatter` for your own types <span class="badge cpp20">C++20</span>

<p class="problem">printf could not print a Record. format can, once you tell it how.</p>

<!-- snippet: demos/s02/formatter_custom.cpp#simple -->
```cpp
// An enum-like type: inherit parse() and the width/alignment handling from formatter<string_view>
template <>
struct std::formatter<Status> : std::formatter<std::string_view> {
    template <typename Ctx>
    auto format(Status s, Ctx& ctx) const {
        return std::formatter<std::string_view>::format(to_string(s), ctx);
    }
};
```

<!-- snippet: demos/s02/formatter_custom.cpp#full -->
```cpp
// A struct: write parse() (accept an empty spec) and format() (delegate to format_to)
template <>
struct std::formatter<Record> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    template <typename Ctx>
    auto format(const Record& r, Ctx& ctx) const {
        return std::format_to(ctx.out(), "{{{},\"{}\",{:.3f},{}}}", r.ts, r.sensor, r.value, r.status);
    }
};
// format() is a template on the context: the standard allows any basic_format_context,
// and libc++ checks at compile time that yours does.
```

<!--
Notes: Two patterns. Enum-like: inherit from formatter<string_view> and get width/alignment for
free. Struct: write parse (accept empty) and format (delegate to format_to). Make format a
template on the context type: the standard allows any basic_format_context and libc++ checks
it. Exercise task 4. Demo file: demos/s02/formatter_custom.cpp
-->

---

<!-- _class: feature -->

## A formatter with its own spec <span class="badge cpp20">C++20</span>

<p class="problem">parse() sees the text between ':' and '}'. It can mean whatever your type needs.</p>

<!-- snippet: demos/s02/formatter_spec.cpp#spec -->
```cpp
// Supports {} (one decimal) and {:f} (fahrenheit): parse() reads the spec once, format() uses it
template <>
struct std::formatter<Celsius> {
    bool fahrenheit = false;

    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it == 'f') { fahrenheit = true; ++it; }
        if (it != ctx.end() && *it != '}') throw std::format_error("Celsius: spec is '' or 'f'");
        return it;
    }
    template <typename Ctx>
    auto format(Celsius c, Ctx& ctx) const {
        return fahrenheit ? std::format_to(ctx.out(), "{:.1f}F", c.v * 9 / 5 + 32)
                          : std::format_to(ctx.out(), "{:.1f}C", c.v);
    }
};
```

<!--
Notes: parse() runs at compile time when the format string is a literal, so a bad spec is a
compile error too. Throwing format_error from parse is how you report one. Demo file:
demos/s02/formatter_spec.cpp
-->

---

<!-- _class: feature -->

## Formatting ranges <span class="badge cpp23">C++23</span>

<p class="problem">Printing a vector used to be a loop. Now it is "{}".</p>

<!-- snippet: demos/s02/format_ranges.cpp#ranges -->
```cpp
std::println("{}", v);           // [1.5, 2.25, 3]
std::println("{::.1f}", v);      // [1.5, 2.2, 3.0]   the spec after :: applies to each element
std::println("{:n}", v);         // 1.5, 2.25, 3      no brackets
std::println("{}", m);           // {"rpm": 2, "temp": 1}
std::println("{}", std::pair{1, "x"});   // (1, "x")
```

Available on libc++ 17+; libstdc++ gets it in GCC 15. Feature-test macro: `__cpp_lib_format_ranges`.

<!--
Notes: The `::` spec passes a spec to each element; `:n` drops the brackets. Maps print as
{k: v}; pairs and tuples as (a, b). Demo file: demos/s02/format_ranges.cpp (gated on the macro).
-->

---

<!-- _class: feature -->

## Formatting without allocating <span class="badge cpp20">C++20</span>

<p class="problem">Embedded and hot paths: format into a buffer you already own.</p>

<!-- snippet: demos/s02/format_to_buffer.cpp#buffer -->
```cpp
std::array<char, 32> buf;                                 // no heap: embedded-friendly
auto r = std::format_to_n(buf.data(), buf.size(), "{}:{:.2f}", "rpm", 4811.0);
std::size_t written = static_cast<std::size_t>(r.out - buf.data());   // chars actually in buf
std::size_t would_need = static_cast<std::size_t>(r.size);            // what it WOULD have needed
std::size_t needed = std::formatted_size("{}:{:.2f}", "rpm", 4811.0);

std::string s;
std::format_to(std::back_inserter(s), "{} ", 1);          // append to any output iterator
std::format_to(std::back_inserter(s), "{}", 2);

std::string user_fmt = "{:>6}";                            // not a literal: needs vformat
int answer = 42;                                           // make_format_args takes lvalues (C++23 DR)
std::string t = std::vformat(user_fmt, std::make_format_args(answer));
```

<!--
Notes: format_to_n never overruns and tells you the full size; formatted_size lets you size a
buffer first; format_to appends to any output iterator. vformat is for runtime format strings,
which lose the compile-time check; C++26 adds std::runtime_format to say so explicitly. Note
make_format_args wants lvalues (a C++23 defect fix). Demo file: demos/s02/format_to_buffer.cpp
-->

---

<!-- _class: dense -->

## Migration

| From | To | Notes |
|---|---|---|
| `printf("%d %s %.2f", i, s.c_str(), d)` | `print("{} {} {:.2f}", i, s, d)` | drop `c_str()` and the casts; `%lu`/`%zu` are just `{}` |
| `printf("%-10s|%5d", ...)` | `print("{:<10}|{:>5}", ...)` | width and alignment move after the colon |
| `printf("%08.3f", d)` | `print("{:08.3f}", d)` | zero-pad, width, precision in the same order |
| `std::cout << std::hex << x` | `print("{:x}", x)` | no sticky state |
| `std::cout << std::setw(8) << x` | `print("{:>8}", x)` | |
| `std::ostringstream` | `std::format` | returns the `std::string` directly |
| `snprintf(buf, n, ...)` | `format_to_n(buf, n, ...)` | |

`clang-tidy modernize-use-std-print` does the `printf` rows automatically.

<!--
Notes: The exercise's task 3 is this table applied to report.cpp. The diff test catches a
single space of difference.
-->

---

<!-- _class: takeaway -->

## Formatting takeaway

`std::format` is `printf` with the format string checked by the compiler and user types allowed. `std::print` is `printf` ergonomics for it.

**Monday morning:** write one `std::formatter` for your most-logged type and delete the `to_string` helper it replaces.

<!--
Notes: 1:30. Ten fast slides of library tour, then the exercise.
-->

---

<!-- SEGMENT: Library tour (1:30) -->

<!-- _class: feature -->

## C++17 tour, 1: `std::filesystem` <span class="badge cpp17">C++17</span>

<p class="problem">Portable paths, directory iteration, and file queries without a platform layer.</p>

<!-- snippet: demos/s02/tour_filesystem.cpp#fs -->
```cpp
fs::path p = fs::temp_directory_path() / "telemetry" / "run1.csv";   // operator/ joins portably
std::println("{} {} {}", p.filename().string(), p.extension().string(), p.parent_path().string());

fs::create_directories(p.parent_path());
std::ofstream{p} << "ts,sensor,value\n";                 // make the file so the queries have something to find
std::println("exists: {}", fs::exists(p));            // no exception: a bool
for (const auto& entry : fs::directory_iterator(p.parent_path())) {
    std::println("{} {}", entry.path().string(), entry.is_regular_file() ? entry.file_size() : 0);
}
std::error_code ec;
fs::remove_all(p.parent_path(), ec);                   // error_code overload: never throws
```

<!--
Notes: `operator/` joins paths. Every operation has a throwing overload and an error_code
overload. The exercise's main.cpp could check fs::exists before opening. Demo file:
demos/s02/tour_filesystem.cpp
-->

---

<!-- _class: feature -->

## C++17 tour, 2: small library additions <span class="badge cpp17">C++17</span>

<p class="problem">The ones you will actually use.</p>

<!-- snippet: demos/s02/tour_cpp17.cpp#tour -->
```cpp
std::println("{}", std::clamp(150, 0, 100));                 // 100
std::println("{} {}", std::gcd(12, 18), std::lcm(4, 6));     // 6 12
std::println("{}", std::invoke(add, 2, 3));                  // call anything callable uniformly
std::println("{}", std::apply(add, std::tuple{2, 3}));      // unpack a tuple into arguments

std::map<std::string, int> a{{"x", 1}}, b{{"y", 2}, {"x", 9}};
a.merge(b);                                                  // splice "y" in; "x" stays in b
auto node = a.extract("x");                                  // take a node out without reallocating
node.key() = "z";                                            // change the key in place
a.insert(std::move(node));
std::println("{} {} {}", a.size(), a.count("z"), b.count("x"));   // 2 1 1

std::vector<int> v{1, 2, 3, 4, 5, 6}, sample;
std::sample(v.begin(), v.end(), std::back_inserter(sample), 2, std::mt19937{42});
std::println("{} {}", sample.size(), std::reduce(v.begin(), v.end()));   // reduce: order-agnostic sum
```

Also: `std::byte`, `from_chars`/`to_chars` (Session 1), `std::size`/`data`/`empty`, `std::as_const`, `std::not_fn`, `std::optional`/`variant`/`any` (today), `std::string_view` (today).

<!--
Notes: map::extract and merge are the sleepers: moving nodes between maps without reallocating,
and changing a key in place. Demo file: demos/s02/tour_cpp17.cpp
-->

---

<!-- _class: feature -->

## C++20 tour, 1: `<bit>` and `source_location` <span class="badge cpp20">C++20</span>

<p class="problem">Bit twiddling without compiler intrinsics, and logging without macros.</p>

<!-- snippet: demos/s02/tour_bit.cpp#bit -->
```cpp
float f = 1.0f;
auto bits = std::bit_cast<std::uint32_t>(f);          // the bytes, reinterpreted; constexpr; no UB
// (C++11 spelling: std::memcpy(&bits, &f, sizeof f), or a union, or a UB pointer cast)

std::println("{:08X}", bits);                          // 3F800000
std::println("{}", std::popcount(0xF0u));              // 4
std::println("{}", std::has_single_bit(64u));          // true: a power of two
std::println("{}", std::bit_width(255u));              // 8
std::println("{:08X}", std::rotl(0x80000001u, 1));     // 00000003
std::println("{}", std::endian::native == std::endian::little);
```

<!-- snippet: demos/s02/tour_bit.cpp#source_location -->
```cpp
log("no macros: the caller's file and line come from the default argument");
```

<!--
Notes: bit_cast is the correct replacement for the type-punning union and the memcpy idiom, and
it is constexpr. source_location as a default argument captures the CALLER's location: the end
of __FILE__/__LINE__ macros in logging APIs. Demo file: demos/s02/tour_bit.cpp
-->

---

<!-- _class: feature -->

## C++20 tour, 2: containers and numerics <span class="badge cpp20">C++20</span>

<p class="problem">Small things that remove a line each.</p>

<!-- snippet: demos/s02/tour_cpp20.cpp#tour -->
```cpp
std::vector<int> v{1, 2, 3, 4, 5, 6};
std::erase_if(v, [](int x) { return x % 2 == 0; });        // finally: no erase(remove_if(...))
std::println("{} {} {}", v[0], v[1], v[2]);                 // 1 3 5 (range formatting: GCC 15 / libc++)

std::string s = "temp_core";
std::println("{} {}", s.starts_with("temp"), s.ends_with("_core"));
std::set<int> primes{2, 3, 5};
std::println("{}", primes.contains(3));                     // no more find() != end()

auto arr = std::to_array({1, 2, 3});                        // std::array<int, 3> from a literal
std::println("{} {}", std::ssize(v), arr.size());           // signed size: no -Wsign-compare
std::println("{} {}", std::midpoint(1, 4), std::lerp(0.0, 10.0, 0.25));   // overflow-safe midpoint
std::println("{:.5f} {:.5f}", std::numbers::pi, std::numbers::sqrt2);
```

<!--
Notes: erase_if ends the erase-remove idiom. contains() ends find() != end(). ssize() ends the
signed/unsigned warning in loops. midpoint is overflow-safe, which (a+b)/2 is not. Demo file:
demos/s02/tour_cpp20.cpp
-->

---

<!-- _class: feature -->

## C++20 tour, 3: `<chrono>` calendars and time zones <span class="badge cpp20">C++20</span>

<p class="problem">The exercise's timestamp is a long long. It could have a type, and print itself.</p>

<!-- snippet: demos/s02/tour_chrono.cpp#chrono -->
```cpp
// The exercise's `long long` timestamp, with a type:
sys_time<milliseconds> ts{milliseconds{1725000001000}};
std::println("{:%F %T} UTC", ts);                         // 2024-08-30 06:40:01.000 UTC

auto day = floor<days>(ts);                                // truncate to the day
year_month_day ymd{day};
std::println("{} {} {}", ymd.year(), ymd.month(), ymd.day());
std::println("{}", weekday{day});                          // Fri

auto next = ymd + months{1};                               // calendar arithmetic
std::println("{}", next);                                  // 2024-09-30

// Time zones (needs the tzdata database; libstdc++ 13+ ships one)
// zoned_time local{"America/Chicago", ts};
// std::println("{:%F %T %Z}", local);
```

<!--
Notes: sys_time<milliseconds> is "milliseconds since the epoch" as a type; formatting with
{:%F %T} needs no strftime. year_month_day arithmetic handles month lengths. Time zones need
the tzdata database (libstdc++ 13+ ships one; libc++ 19+). Demo file: demos/s02/tour_chrono.cpp
-->

---

<!-- _class: feature dense -->

## C++23 tour, 1: `flat_map`, `stacktrace`, `move_only_function` <span class="badge cpp23">C++23</span>

<p class="problem">Three additions with uneven availability today (GCC 15 / libc++ 20 for flat_map; libstdc++ only for stacktrace, link -lstdc++exp; libstdc++ only for move_only_function, libc++ has none yet).</p>

```cpp
std::flat_map<std::string_view, double> limits{{"rpm", 12000.0}};   // sorted vectors, map API
auto trace = std::stacktrace::current();       // where am I, without a debugger
std::println("{}", trace);
std::move_only_function<int()> task =          // a std::function that can hold a move-only closure
    [p = std::make_unique<int>(5)] { return *p; };
```

<!--
Notes: flat_map is the cache-friendly map: contiguous, cheap to iterate, expensive to insert.
Right for a table built once (the sensor table). stacktrace is the "where am I" you used to
need a debugger for. move_only_function fixes std::function's copyability requirement for
task queues (Session 5). Hand-typed because of availability; demos are gated on the
feature-test macros: demos/s02/tour_flat_map.cpp, tour_stacktrace.cpp, tour_cpp23.cpp
-->

---

<!-- _class: feature dense -->

## C++23 tour, 2: strings, bytes, and C APIs <span class="badge cpp23">C++23</span>

<p class="problem">Odds and ends that each close a long-standing gap.</p>

<!-- snippet: demos/s02/tour_cpp23.cpp#tour -->
```cpp
std::string s = "temp_core";
std::println("{}", s.contains("_"));                          // finally

std::println("{:04X}", std::byteswap(std::uint16_t{0x1234}));   // 3412; constexpr

s.resize_and_overwrite(16, [](char* buf, std::size_t n) {     // write into uninitialized capacity
    return static_cast<std::size_t>(std::snprintf(buf, n, "rpm=%d", 4811));
});
std::println("{}", s);
```

Also: `std::to_underlying` and `std::unreachable` (Session 1), `std::forward_like`, `std::spanstream`, `std::out_ptr`/`inout_ptr` for C APIs that fill a `T**` (in the demo file, gated on `__cpp_lib_out_ptr`).

<!--
Notes: byteswap is the endian-conversion everyone had a macro for; constexpr. out_ptr adapts a
unique_ptr to a `T**` out-parameter, which every C library has. Demo file:
demos/s02/tour_cpp23.cpp
-->

---

<!-- _class: dense -->

## Deprecated and removed in the library

| Standard | Deprecated | Removed |
|---|---|---|
| C++17 | `std::iterator`, `std::result_of`, `<codecvt>`, `std::uncaught_exception` | `auto_ptr`, `random_shuffle`, `bind1st`/`bind2nd`, `std::function` allocator support |
| C++20 | `volatile` compound ops, `std::is_pod`, `atomic_init` | `std::result_of`, `raw_storage_iterator`, `uncaught_exception`, `<ciso646>` |
| C++23 | `std::aligned_storage`/`aligned_union`, `std::numeric_limits<T>::has_denorm` | garbage-collection support API |
| C++26 | | `<codecvt>` (P2871), `strstream` (P2867) |

`-Wdeprecated-declarations` (on by default) on both compilers reports uses of these; clang-tidy `modernize-*` fixes many.

<!--
Notes: Fast. The aligned_storage one bites anyone with a hand-written small-buffer optimization:
replace with alignas(T) std::byte buf[sizeof(T)]. std::iterator is still only deprecated in C++23.
-->

---

<!-- _class: dense -->

## Support matrix for this session

| Feature | GCC 14 / libstdc++ | Clang 18 / libc++ | Fallback |
|---|---|---|---|
| `std::expected` | OK | OK (**libstdc++ on Clang 18: missing**) | the repo builds Clang with libc++ |
| `std::print` / `format` | OK | OK | |
| `from_chars<double>` | OK | missing (libc++ 20) | `#ifndef __cpp_lib_to_chars` fallback |
| Range formatting | missing (GCC 15) | OK | `__cpp_lib_format_ranges` |
| `flat_map` | missing (GCC 15) | missing | Compiler Explorer |
| `mdspan` | missing (GCC 15) | OK | Compiler Explorer |
| `stacktrace` | OK, `-lstdc++exp` | missing | `__cpp_lib_stacktrace` |
| `move_only_function`, `out_ptr` | OK | missing (`out_ptr`: libc++ 19; `move_only_function`: not yet) | feature-test macros |

**The tool:** `<version>` (C++20) and `__cpp_lib_*` macros. Test the feature, not the compiler version.

<!--
Notes: Every row was discovered building this repo. The lesson is the last line; the details are
in handouts/toolchain-support-matrix.md.
-->

---

<!-- SEGMENT: Exercise and close (1:40) -->

## Interface design checklist

| The parameter or return is... | Use |
|---|---|
| read-only text | `std::string_view` by value |
| read-only contiguous elements | `std::span<const T>` by value |
| a buffer to write into | `std::span<T>` |
| a value you will keep | `T` by value, then `std::move` it |
| maybe a result | `std::optional<T>` |
| a result or a reason | `std::expected<T, E>`, `[[nodiscard]]` |
| one of a fixed set | `std::variant<...>` |
| owned, one owner | `std::unique_ptr<T>` |
| borrowed, non-null | `T&` |
| a line of text output | `std::format` / `std::print` with a `std::formatter` |

<!--
Notes: The screenshot slide. Everything on it is in today's solution.
-->

---

## Exercise: vocabulary types

Open `exercises/s02-vocabulary-types/README.md`

**In class (15 minutes):**

1. `parse_record` returns `std::expected<Record, ParseError>`; delete `ParseError::None`
2. `find_sensor` returns `std::optional<SensorConfig>`; `parse_status` returns `std::optional<Status>`
3. Every `fprintf` in the report becomes `std::println`

```
cmake --build build && ctest --test-dir build -R s02 --output-on-failure
```

The new test `s02_report_identical` diffs your report against the starter's. One space off and it fails: that is the point.

**At home:** tasks 4 to 9 (`formatter`, `string_view`, `span`, monadic ops). `solution/` is next session's starter.

<!--
Notes: Walk the room. The common stumble on task 1 is forgetting std::unexpected around the
error; on task 3 it is the %-16s to {:<16} translation. Call time at 15 minutes and show the
solution's parser.h.
-->

---

<!-- _class: takeaway -->

## Session 2 takeaway

A modern interface **says what it means**: `optional` means "maybe", `expected` means "or this error", `string_view` means "I will only look", `span` means "a contiguous run I do not own".

The information that used to live in comments, sentinels, and out-parameters now lives in the signature, where the compiler can see it.

**Next session:** moving work to compile time. The CRC table, the configuration table, and the `serialize` overload set are the targets. `constexpr`, `consteval`, concepts, and deducing `this`.

<!--
Notes: Point at the support matrix handout and the feature-timeline handout, both updated for
today's features.
-->
