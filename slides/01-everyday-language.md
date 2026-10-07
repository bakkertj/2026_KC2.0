---
marp: true
theme: course
paginate: true
footer: 'Modern C++ | Session 1: The Everyday Language'
---

<!-- _class: lead -->
<!-- _paginate: false -->

# The Evolution of C++
## Session 1: The Everyday Language

Small syntax changes that touch nearly every function you write

<!--
Notes: Two minutes. Who you are, who they are (quick show of hands: last standard you shipped
with?). Set the frame for the whole course: five sessions, thematic not chronological, every
feature carries a badge saying which standard it came from, the timeline handout is the map.
Then straight into the agenda.
-->

---

## Agenda

1. Why this course is organized by theme, not by standard (10 min)
2. C++11 calibration (20 min)
3. C++14: the polish release (25 min)
4. C++17: syntax you will use daily (25 min)
5. C++20 and C++23: small but valuable (15 min)
6. Guided exercise: modernize the syntax (20 min), then wrap-up (5 min)

Every demo opens in Compiler Explorer, preconfigured for GCC 14: `handouts/compiler-explorer-links.pdf`

<!--
Notes: Point at the exercise README now (exercises/s01-modernize-syntax/README.md) so people can
open it; the two-minute toolchain check on slide 5 builds the starter. The exercise is cumulative:
the program you modernize today is the one you carry through all five sessions.
-->

---

## Why not one session per standard?

| Standard | Character | Share of "must know" |
|---|---|---|
| C++14 | Polish release for C++11 | ~5% |
| C++17 | The everyday features | ~25% |
| C++20 | Largest change since C++11 | ~50% |
| C++23 | Completes C++20, fills library gaps | ~20% |

The important features cross standards: `constexpr` changed in **every one** of these.

<!--
Notes: constexpr was relaxed in 14, got if-constexpr and lambdas in 17, got allocation and
virtual in 20, and lost most remaining restrictions in 23. Lambdas: generic in 14, constexpr in 17,
template in 20. Ranges: arrived in 20, became usable in 23. Teaching by year means teaching each of
those three or four times. So: themes, with badges.
-->

---

## How to read a feature slide

<span class="badge cpp17">C++17</span>

Every feature slide has three parts:

- **The badge** (top right): the standard that introduced it. 
- **The problem line** (italic, under the title): what was wrong before
- **The code**: an excerpt from a file in `demos/` that compiles under `-Wall -Wextra -Wpedantic -Werror` on GCC 14 and Clang 18

"Before / After" slides show the C++11 way on the left and the modern way on the right, doing the same thing.

<!--
Notes: Make the point that every code block on a feature slide that names a demo file is pulled
from that compiled file by a script, so it is not pseudo-code. Short hand-typed blocks (C++11-only
code, one-liners, previews) say so in their notes.
-->

---

## The toolchain

- `-std=c++23` with GCC 14 or Clang 18. Either. Both, ideally.
- `-Wall -Wextra -Wpedantic -Werror` throughout
- Compiler Explorer (godbolt.org) links in every `demos/` file header: no local setup needed to follow along

**Two-minute check now:** `cmake -S . -B build && cmake --build build && ctest --test-dir build`

<!--
Notes: Actually wait the two minutes. Anyone who cannot build should pair up. Point out that some
C++23 *library* features are uneven between libstdc++ and libc++; the support matrix handout says
which. Today's session uses nothing that is missing on either.
-->

---

<!-- _class: demo -->

## The program we will modernize all course

`exercises/s01-modernize-syntax/starter/`: a telemetry record processor in careful C++11

```
1725000001000,temp_ambient,22.150,suspect
```

Reads lines like that, validates them against a sensor table, computes per-sensor statistics, prints a report.

Run it: `build/exercises/s01-modernize-syntax/s01_starter exercises/s01-modernize-syntax/data/sample.csv`

<!--
Notes: Run it live on sample.csv. Show the report: seven rejections, one per reason, then stats per
sensor. Then open record.h and scroll past the six comparison operators without comment. They will
be gone by the end of the session. Emphasize: this is GOOD C++11. Nothing in it is wrong. Every
change we make today removes something anyway.
-->

---

<!-- SEGMENT: C++11 calibration (0:10) -->

## What this course assumes you know

C++11, specifically:

`auto`, range-based `for`, lambdas, rvalue references and `std::move`, `std::unique_ptr` and `std::shared_ptr`, `nullptr`, `enum class`, `override`, `constexpr` (the original form), uniform initialization, `static_assert`, `<thread>` and `<atomic>` basics

The next eight slides are the parts people **think** they know.

<!--
Notes: Ask for a show of hands per item if the room is small. If most hands stay down for move
semantics, slow down on slides 9 and 10; they matter for everything in Session 2.
-->

---

<!-- _class: feature -->

## `auto` drops references and const <span class="badge cpp11">C++11</span>

<p class="problem">auto deduces like a template parameter: the top-level const and the reference are stripped.</p>

<!-- snippet: demos/s01/auto_pitfalls.cpp#drops -->
```cpp
auto a = c.get();         // std::string: a COPY. auto drops the & and the const
const auto& b = c.get();  // const std::string&: what you meant
auto& d = c.name;         // std::string&: a mutable alias into c
```

<!-- snippet: demos/s01/auto_pitfalls.cpp#braces -->
```cpp
auto x{42};        // int (N3922; GCC and Clang apply it back to C++11)
auto y = {1, 2};   // initializer_list<int>, still
std::vector<int> v(3, 7);   // three sevens
std::vector<int> w{3, 7};   // the values 3 and 7
```

<!--
Notes: The first line copies a 100-byte string silently; in a loop over a map that is a copy of
every pair. Rule: `const auto&` by default when reading, `auto&` when mutating, plain `auto` when
you want a copy. The braces rule changed with N3922, adopted for C++17 but treated as a defect
report: GCC and Clang give int even under -std=c++11. Demo file: demos/s01/auto_pitfalls.cpp
-->

---

<!-- _class: feature -->

## When `std::move` does nothing <span class="badge cpp11">C++11</span>

<p class="problem">std::move is a cast. It does not move anything; it only permits a move that may not happen.</p>

<!-- snippet: demos/s01/move_does_nothing.cpp#cases -->
```cpp
std::string make() {
    std::string s(100, 'x');
#ifdef SHOW_ERRORS
    return std::move(s);   // WRONG: "prevents copy elision", say both compilers
#else
    return s;              // RIGHT: locals are moved anyway, usually elided
#endif
}

void consume(std::string s) { std::printf("%zu\n", s.size()); }

void cases() {
    const std::string c(100, 'c');
    consume(std::move(c));   // COPIES: cannot move from const. No warning.

    std::string s(100, 's');
    consume(std::move(s));   // moves; s is now valid-but-unspecified
    std::printf("%zu\n", s.size());   // legal; never rely on the value
}
```

<!--
Notes: Three cases. (1) return std::move(local): pessimizing, both compilers now warn, and this
repo's -Werror rejects it, which is why the wrong line is behind SHOW_ERRORS. (2) moving from
const: silently copies; the const overload of the copy constructor wins. (3) moved-from is
valid-but-unspecified: you may assign to it or destroy it, nothing else. Demo file:
demos/s01/move_does_nothing.cpp
-->

---

<!-- _class: feature -->

## Lambdas that outlive their captures <span class="badge cpp11">C++11</span>

<p class="problem">Capture by reference is a pointer to a stack frame. If the lambda escapes the frame, it dangles.</p>

<!-- snippet: demos/s01/escaping_lambda.cpp#escape -->
```cpp
std::function<void()> make_logger_bad(const std::string& prefix) {
    std::string tag = "[" + prefix + "] ";
    return [&] { std::printf("%s\n", tag.c_str()); };   // by reference: dangles on return
}

std::function<void()> make_logger(const std::string& prefix) {
    std::string tag = "[" + prefix + "] ";
    return [tag] { std::printf("%s\n", tag.c_str()); };   // a copy: safe
}
```

<!--
Notes: `[&]` is fine inside std::for_each or an algorithm that finishes before the function
returns. It is wrong the moment the closure is stored: std::function, a thread, a callback
registry. Rule: default to `[=]` or explicit captures for anything stored. The C++14 init-capture
slide later shows how to move into a closure. AddressSanitizer catches the bad version instantly.
Demo file: demos/s01/escaping_lambda.cpp
-->

---

## `unique_ptr` vs `shared_ptr`: ownership, not "modern pointer"

- `std::unique_ptr<T>`: exactly one owner. Zero overhead over a raw pointer. **The default.**
- `std::shared_ptr<T>`: reference counted. An atomic increment per copy (and a decrement per destruction), a control block per object, and it makes lifetime a runtime question. **A design decision, not a convenience.**
- Raw `T*`: non-owning, may be null. Still fine as a parameter. Never for ownership.
- `T&`: non-owning, never null. The best parameter when it fits.

<!--
Notes: The mistake to name: reaching for shared_ptr because it "just works". It works until two
subsystems disagree about who releases what. The pattern from C++14 on is: factories return
unique_ptr, containers hold unique_ptr, functions take T& or T*. shared_ptr where ownership is
genuinely shared (caches, graphs with cycles broken by weak_ptr).
-->

---

## The C++11 features everyone actually uses

- `enum class`: scoped, no implicit conversion to int, forward-declarable
- `nullptr`: a real null pointer type, overloads correctly (`f(0)` vs `f(nullptr)`)
- `override` and `final`: the compiler checks you are actually overriding
- `static_assert`: compile-time checks with a message
- `= default` and `= delete`: say what you mean about special members

<!--
Notes: Quick slide. If anyone is still writing `virtual void f()` in derived classes without
`override`, this is the moment to say the word. `= delete` returns in the copy-elision slide.
-->

---

<!-- _class: feature -->

## `constexpr` in C++11: the original form <span class="badge cpp11">C++11</span>

<p class="problem">One return statement. No loops, no locals, no if. Recursion or the ternary operator, and that is all.</p>

```cpp
// C++11: legal
constexpr int factorial(int n) { return n <= 1 ? 1 : n * factorial(n - 1); }

// C++11: NOT legal (a loop and a local). Legal from C++14. Full story in Session 3.
constexpr int factorial14(int n) {
    int r = 1;
    for (int i = 2; i <= n; ++i) r *= i;
    return r;
}
```

<!--
Notes: This code block is hand-typed, because the C++11 restriction cannot be demonstrated with a
file compiled as C++23. Just plant the flag: constexpr in 11 was a toy; the
Session 3 timeline shows it becoming a second language.
-->

---

## Uniform initialization and its one trap

```cpp
std::vector<int> a(3, 7);   // three sevens
std::vector<int> b{3, 7};   // two elements: 3 and 7
std::vector<int> c{};       // empty
Record r{1, "rpm", 4800.0, Status::Ok};   // aggregate init, no constructor needed
```

Braces prevent narrowing (`int x{3.5}` is an error). Braces prefer `initializer_list` constructors, which is the trap.

<!--
Notes: The rule most teams adopt: braces everywhere except when the type has an initializer_list
constructor and you mean the other one (vector, string). CTAD (later today) makes the trap
slightly worse: `std::vector v{3, 7}` is two ints. Hand-typed block; the same lines live in
demos/s01/ctad.cpp.
-->

---

<!-- _class: takeaway -->

## Calibration takeaway

Good C++11 is good code. Nothing in the starter program is wrong.

**Every slide from here replaces some of it anyway**: not because it was broken, but because something shorter, safer, or faster became possible.

<!--
Notes: Transition. If you are ahead of time here, ask what people found ugly in the starter when
they built it. Someone will name the six comparison operators.
-->

---

<!-- SEGMENT: C++14 (0:30) -->

## C++14 in one slide

<span class="badge cpp14">C++14</span>

- Shipped three years after C++11, mostly fixing what C++11 got almost right
- No large features. About ten small ones, and you will use half of them every day
- The biggest: generic lambdas, init-capture, relaxed `constexpr`, `make_unique`
- Removed exactly one thing: `gets`

Everything here compiles on every compiler you will ever meet.

<!--
Notes: 25 minutes for this segment. C++14 is the release where "modern C++" became comfortable to
write. If your codebase is on 11, moving to 14 costs nothing and buys the next ten slides.
-->

---

<!-- _class: twocol -->

## Generic lambdas <span class="badge cpp14">C++14</span>

<div class="cols">
<div>

#### Before (C++11)

```cpp
struct ValueDescending {
    bool operator()(const Record& a,
                    const Record& b) const {
        return a.value > b.value;
    }
};
std::sort(v.begin(), v.end(),
          ValueDescending());
```

</div>
<div>

#### After (C++14)

<!-- snippet: demos/s01/generic_lambda.cpp#generic -->
```cpp
// auto parameters: one lambda, any type with .value
auto value_descending = [](const auto& a, const auto& b) { return a.value > b.value; };
```

```cpp
std::sort(v.begin(), v.end(), value_descending);
```

</div>
</div>

<!--
Notes: `auto` in a lambda parameter makes operator() a template. One comparator works for any
type with .value. The starter's stats.cpp has ValueDescending exactly as on the left; the
solution replaces it with a generic lambda (exercise task 6). The left block is hand-typed to
mirror the exercise starter. Demo file: demos/s01/generic_lambda.cpp
-->

---

<!-- _class: feature -->

## Lambda init-capture <span class="badge cpp14">C++14</span>

<p class="problem">C++11 lambdas could capture by copy or by reference. There was no way to move something into a closure.</p>

<!-- snippet: demos/s01/generic_lambda.cpp#init_capture -->
```cpp
// init-capture moves the unique_ptr into the closure
auto make_printer(std::unique_ptr<std::string> owned) {
    return [s = std::move(owned)] { std::printf("%s\n", s->c_str()); };
}
```

Also: `[n = compute()]` to capture an expression, `[&r = *ptr]` to capture a reference under a new name.

<!--
Notes: The question to ask: how would you capture a unique_ptr in C++11? You could not, short of
wrapping it in a shared_ptr. Init-capture is the fix for the escaping-lambda slide too: move the
state in, and there is nothing left to dangle. Demo file: demos/s01/generic_lambda.cpp
-->

---

## Return type deduction and `decltype(auto)` <span class="badge cpp14">C++14</span>

```cpp
auto make_printer(std::unique_ptr<std::string> owned) { return [s = std::move(owned)] { ... }; }
```

- `auto` return: the type is whatever the `return` statements say. Every `return` must agree.
- Needed for lambdas-returning-lambdas and for closures, whose types cannot be named
- `decltype(auto)`: deduce **exactly**, references included. For forwarding wrappers.
- **Do not** use `auto` returns in public headers: the signature is the documentation

<!--
Notes: The rule most teams settle on: auto return is for local helpers, generic code, and
anything returning a lambda; explicit types on anything a colleague calls. decltype(auto) is
rare; show it once and move on. Hand-typed block.
-->

---

<!-- _class: twocol -->

## `std::make_unique` <span class="badge cpp14">C++14</span>

<div class="cols">
<div>

#### Before (C++11)

```cpp
std::unique_ptr<Sensor> s(
    new Sensor("rpm", 12000));

// exception-safety hazard in C++11:
f(std::unique_ptr<A>(new A),
  std::unique_ptr<B>(new B));
```

</div>
<div>

#### After (C++14)

```cpp
auto s = std::make_unique<Sensor>(
    "rpm", 12000);

// safe, and no `new` anywhere:
f(std::make_unique<A>(),
  std::make_unique<B>());
```

</div>
</div>

<!--
Notes: make_shared shipped in C++11; make_unique was forgotten and added in 14. With it, "never
write new" became a coding standard you can actually enforce (clang-tidy:
modernize-make-unique). The exception-safety hazard on the left is real in C++11/14 and fixed by
C++17 evaluation-order rules, but the make_unique form is still clearer. Hand-typed to keep it
short.
-->

---

## Binary literals and digit separators <span class="badge cpp14">C++14</span>

```cpp
constexpr std::uint16_t kPolynomial = 0b0001'0000'0010'0001;   // 0x1021, and you can see the bits
constexpr double kMaxRpm = 12'000.0;
constexpr long long kMillisPerDay = 86'400'000;
constexpr unsigned kMask = 0xFF'FF'00'00;
```

The separator is `'` and it is ignored by the compiler. It works in any base.

<!--
Notes: Embedded people love binary literals for register masks. The polynomial line is from the
Session 1 solution (crc.cpp). Digit separators are in the sensor table too (12'000.0). Hand-typed
block.
-->

---

## Standard literals <span class="badge cpp14">C++14</span>

```cpp
using namespace std::literals;         // or std::string_literals, std::chrono_literals

auto name = "temp_core"s;              // std::string, not const char*
auto timeout = 250ms;                  // std::chrono::milliseconds
auto period = 2h + 30min;              // std::chrono::minutes
std::this_thread::sleep_for(100us);
```

`auto x = "abc";` is still `const char*`. The `s` suffix is how you get a `std::string` with `auto`.

<!--
Notes: The chrono literals are the ones that change code: sleep_for(100) does not compile,
sleep_for(100ms) does, and the unit is in the source. Convention: `using namespace
std::literals;` at function scope or file scope in .cpp files, never in headers. Hand-typed block.
-->

---

## `[[deprecated]]` <span class="badge cpp14">C++14</span>

```cpp
[[deprecated("use parse_record(std::string_view) instead")]]
bool parse_record(const char* line, std::size_t len, Record* out);
```

- The first standard attribute most people used. Works on functions, types, variables; on enumerators and namespaces too from C++17 (N4266).
- Compiler warns at every call site, with your message. Under `-Werror`, that is a build break you schedule.
- The same `[[...]]` syntax carries `[[nodiscard]]`, `[[fallthrough]]`, `[[likely]]` later today.

<!--
Notes: The migration pattern: add [[deprecated]] on the old signature, add the new one, fix the
warnings over a sprint, delete the old one. Much better than a grep. Hand-typed block.
-->

---

## Relaxed `constexpr` <span class="badge cpp14">C++14</span>

<div class="evo">
<div class="step"><b>C++11</b> one return statement; recursion only</div>
<div class="step"><b>C++14</b> loops, locals, `if`, mutation of locals; multiple returns</div>
<div class="step"><b>C++17</b> `constexpr` lambdas; `if constexpr`</div>
<div class="step"><b>C++20</b> dynamic allocation, `std::vector`/`std::string`, virtual calls, `try`; `consteval`, `constinit`</div>
<div class="step"><b>C++23</b> almost no remaining restrictions: `static` locals, non-literal variables, `std::unique_ptr`</div>
</div>

Session 3 walks this whole timeline with the CRC table from the exercise.

<!--
Notes: Evolution slide format: one line per standard. Today, just the C++14 step: constexpr
functions can now look like normal functions. The factorial14 from the calibration slide is legal
from here on.
-->

---

## Small library additions <span class="badge cpp14">C++14</span>

- `std::exchange(obj, new_value)`: sets and returns the old value; the move-constructor idiom `ptr_ = std::exchange(other.ptr_, nullptr)`
- `std::integer_sequence` / `std::index_sequence`: the tool for unpacking tuples in variadic templates
- `std::cbegin` / `std::cend`, `std::rbegin` / `std::rend` as free functions
- `std::quoted` for streaming strings with quotes and escapes
- `std::shared_timed_mutex`: reader/writer lock (Session 5)
- Transparent comparators: `std::map<std::string, T, std::less<>>` finds by `const char*` without constructing a string

<!--
Notes: Name-drop slide; do not linger. exchange is the one worth a sentence. Transparent
comparators matter once string_view arrives (Session 2).
-->

---

<!-- _class: takeaway -->

## C++14 takeaway

Removed: `gets`. Added: nothing large.

**What changes for you:** lambdas become the default way to write a function object, `make_unique` makes `new` disappear, and `constexpr` becomes something you can actually write.

**Monday morning:** turn on clang-tidy `modernize-make-unique` and `modernize-use-auto`.

<!--
Notes: Time check: should be at 0:55. If behind, the C++17 segment has two slides that can be
skipped (the string_view preview, the two quiet fixes).
-->

---

<!-- SEGMENT: C++17 (0:55) -->

## C++17 in one slide

<span class="badge cpp17">C++17</span>

- The release that changed how ordinary functions look
- Language: structured bindings, `if` with initializer, `if constexpr`, `inline` variables, fold expressions, CTAD, guaranteed copy elision, the useful attributes
- Library: `optional`, `variant`, `string_view`, `filesystem`, parallel algorithms, `from_chars` (Session 2 covers the types; today covers the syntax)
- Removed: `auto_ptr`, `register`, trigraphs, dynamic exception specifications, `random_shuffle`

<!--
Notes: 25 minutes, 15 slides. This is the segment attendees will use most tomorrow. Structured
bindings and if-with-initializer are the two to make sure everyone gets.
-->

---

<!-- _class: twocol -->

## Structured bindings <span class="badge cpp17">C++17</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s01/structured_bindings.cpp#before -->
```cpp
// C++11: insert returns a pair; name both halves
void record_cpp11(const std::string& key) {
    std::pair<std::map<std::string, int>::iterator, bool> r =
        counts.insert({key, 1});
    if (!r.second) {
        ++r.first->second;
    }
}
```

</div>
<div>

#### After (C++17)

<!-- snippet: demos/s01/structured_bindings.cpp#after -->
```cpp
// C++17: bindings name the halves; scoped to the if
void record_cpp17(const std::string& key) {
    if (auto [it, inserted] = counts.insert({key, 1});
        !inserted) {
        ++it->second;
    }
}
```

</div>
</div>

<!--
Notes: This is compute_stats / load_stream from the starter (exercise task 3). `.first` and
`.second` are the smell; a structured binding gives both halves names. The right side also uses
if-with-initializer, next slide. Demo file: demos/s01/structured_bindings.cpp
-->

---

## Structured bindings: what they bind <span class="badge cpp17">C++17</span>

```cpp
auto [lo, hi] = value_range(records);          // std::pair
auto [ptr, ec] = std::from_chars(b, e, v);     // any struct with public members
auto [x, y, z] = point;                        // arrays too: double p[3]
for (const auto& [name, stats] : by_sensor) {  // the map loop, finally readable
    ...
}
```

- Always introduces **new** names; you cannot bind into existing variables
- `auto&` / `const auto&` avoid copying the whole object
- Names are bound to the members, so `[a, b]` on a struct with three members is an error

<!--
Notes: The map loop alone justifies the feature. Mention std::tie(a, b) = f() still exists for
assigning into existing variables. Hand-typed; each line mirrors a use in the solution.
-->

---

## `if` and `switch` with initializer <span class="badge cpp17">C++17</span>

```cpp
if (auto [it, inserted] = counts.try_emplace(key, 1); !inserted) {
    ++it->second;
}                                       // it and inserted are gone here

if (std::lock_guard lk{mu}; queue.empty()) { return; }   // the lock is scoped to the if

switch (auto status = poll(); status.kind) { ... }
```

<p class="problem">Same scoping as the init-statement in a for loop. The variable lives for the whole if/else chain, and no longer.</p>

<!--
Notes: The bug it prevents: a variable declared before the if leaks into the rest of the
function, gets reused, and now means something else. Also the lock_guard idiom on line 2 is worth
a second: the lock is held for exactly the if/else and released before the next statement.
Hand-typed block.
-->

---

## The map API C++11 should have had <span class="badge cpp17">C++17</span>

```cpp
auto [it, inserted] = stats.try_emplace(name);   // construct only if name is new
stats.insert_or_assign(name, SensorStats{});      // overwrite; reports whether it existed
b.insert(a.extract("rpm"));                       // move a node between maps, no realloc
a.merge(b);                                       // splice everything that does not collide
```

- `try_emplace` fixes `insert(make_pair(k, expensive()))` constructing the value you may discard
- `emplace` in C++11 could **also** construct and discard, and could move-from your argument even when the key existed. `try_emplace` never touches the arguments unless it inserts.

<!--
Notes: The starter's compute_stats uses insert(make_pair(...)) which constructs a SensorStats
every iteration. The solution uses try_emplace. This is exercise task 3's bonus. Hand-typed block.
-->

---

<!-- _class: twocol -->

## `inline` variables <span class="badge cpp17">C++17</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s01/inline_variable.cpp#before -->
```cpp
// header:   extern const std::size_t kMaxLineLength;
// one .cpp: const std::size_t kMaxLineLength = 256;
// (or a static per TU, or a function returning a static)
```

</div>
<div>

#### After (C++17)

<!-- snippet: demos/s01/inline_variable.cpp#after -->
```cpp
// header, and nowhere else:
inline constexpr std::size_t kMaxLineLength = 256;

struct Limits {
    static inline const std::string kUnits = "raw";  // members too
};
```

</div>
</div>

<!--
Notes: kMaxLineLength in the starter's config.h is the extern/definition pair on the left
(exercise task 7). inline variables mean header-only libraries can have globals, and static data
members no longer need an out-of-class definition. Also the cleanest singleton: `inline Registry
g_registry;` in a header. Demo file: demos/s01/inline_variable.cpp
-->

---

## Nested namespace definitions <span class="badge cpp17">C++17</span>

```cpp
// C++11
namespace telemetry { namespace detail { namespace crc {
    ...
}}}

// C++17
namespace telemetry::detail::crc {
    ...
}
```

C++20 adds `namespace a::inline b {}` for inline namespaces.

<!--
Notes: Thirty seconds. Purely cosmetic, universally adopted. Hand-typed block.
-->

---

<!-- _class: feature -->

## `[[nodiscard]]` <span class="badge cpp17">C++17</span>

<p class="problem">A function whose return value is the whole point of calling it should not compile silently when the value is dropped.</p>

<!-- snippet: demos/s01/nodiscard.cpp#nodiscard -->
```cpp
[[nodiscard]] bool parse(const std::string& s, int* out);   // dropping it is a bug
[[nodiscard("released immediately if dropped")]] int acquire();   // C++20: reason

struct [[nodiscard]] Error { int code; };   // applies to every function returning Error
Error try_write();
```

Put it on: parsers, lookups, anything returning an error or a handle, `empty()`, factories. Leave it off: functions whose side effect is the point.

<!--
Notes: The attribute that finds real bugs, today, in existing code. On a type ([[nodiscard]]
struct Error) it applies to every function returning that type. C++20 adds the reason string.
Exercise task 4 asks them to add it across the starter and see what the compiler says. Under
this repo's -Werror the SHOW_ERRORS block demonstrates the failure. Demo file:
demos/s01/nodiscard.cpp
-->

---

<!-- _class: feature -->

## `[[maybe_unused]]` and `[[fallthrough]]` <span class="badge cpp17">C++17</span>

<p class="problem">Two warnings you used to silence with casts to void and comments the compiler could not read.</p>

<!-- snippet: demos/s01/nodiscard.cpp#others -->
```cpp
enum class Level { Trace, Debug, Info };

int verbosity(Level lvl, [[maybe_unused]] bool color) {   // used only in some builds
    switch (lvl) {
    case Level::Trace:
        std::puts("trace on");
        [[fallthrough]];                                  // intentional, and checked
    case Level::Debug:
        return 2;
    case Level::Info:
        return 1;
    }
    return 0;
}
```

<!--
Notes: `-Wimplicit-fallthrough` is in -Wextra on GCC; Clang needs it spelled out (its -Wextra does
not enable it). [[fallthrough]] is how you tell either one the fall-through is deliberate.
[[maybe_unused]] replaces `(void)param;` and applies to variables, functions, and types too
(things only used under #ifdef). Demo file: demos/s01/nodiscard.cpp
-->

---

<!-- _class: feature -->

## Guaranteed copy elision <span class="badge cpp17">C++17</span>

<p class="problem">Before C++17, returning by value required a copy or move constructor to exist, even when the compiler elided the call.</p>

<!-- snippet: demos/s01/copy_elision.cpp#factory -->
```cpp
struct Pinned {                       // not copyable, not movable
    std::mutex m;
    int id;
    explicit Pinned(int i) : id(i) {}
    Pinned(const Pinned&) = delete;
    Pinned& operator=(const Pinned&) = delete;
};

Pinned make_pinned(int id) {
    return Pinned{id};                // C++17: legal. C++11/14: error.
}

int main() {
    Pinned p = make_pinned(7);        // no copy, no move: p is initialized in place
    std::printf("%d\n", p.id);
}
```

<!--
Notes: The formal change: a prvalue is now "a recipe for initializing an object", not a temporary
object. So `Pinned p = make_pinned(7)` initializes p directly; no object is ever copied or moved,
and no copy/move constructor need exist. Consequences: factories for mutexes, atomics, hardware
handles; `T x = T(...)` is free. NRVO (returning a named local) is still optional, which is why
move_does_nothing.cpp says "usually". Demo file: demos/s01/copy_elision.cpp
-->

---

<!-- _class: feature -->

## Class template argument deduction <span class="badge cpp17">C++17</span>

<p class="problem">You had std::make_pair because you could not write std::pair without the template arguments.</p>

<!-- snippet: demos/s01/ctad.cpp#ctad -->
```cpp
std::pair p{1, 2.5};              // pair<int, double>; no more make_pair
std::lock_guard lk{m};            // lock_guard<std::mutex>: the killer use
std::vector v{1, 2, 3};           // vector<int>, three elements

std::vector<int> a(3, 7);         // three sevens
std::vector b{3, 7};              // TRAP: two elements, not three sevens
```

<!--
Notes: The killer use is std::lock_guard lk{m}; nobody misses typing std::lock_guard<std::mutex>.
The trap is the same initializer_list trap as before, made easier to hit. Deduction guides
(writing your own rules) exist; most code never needs them. Demo file: demos/s01/ctad.cpp
-->

---

<!-- _class: twocol -->

## `std::from_chars` and `std::to_chars` <span class="badge cpp17">C++17</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s01/from_chars.cpp#before -->
```cpp
bool parse_value_cpp11(const std::string& text,
                       double* out) {
    if (text.empty()) return false;
    errno = 0;                    // global state
    char* end = nullptr;          // NUL-terminated; locale
    const double v = std::strtod(text.c_str(), &end);
    if (errno == ERANGE || *end != '\0') return false;
    *out = v;
    return true;
}
```

</div>
<div>

#### After (C++17)

<!-- snippet: demos/s01/from_chars.cpp#after -->
```cpp
bool parse_value(const std::string& text,
                 double* out) {
    const char* last = text.data() + text.size();
    double v = 0.0;               // parse into a local: *out untouched on failure
    auto [ptr, ec] = std::from_chars(text.data(), last, v);
    if (ec != std::errc{} || ptr != last) return false;
    *out = v;
    return true;
}
```

</div>
</div>

<!--
Notes: This is parse_value from the starter's parser.cpp (exercise task 5). What disappears:
errno, the NUL-terminator requirement, the locale dependency (in a German locale strtod reads
"41.25" as 41 and stops at the dot). from_chars is also several times faster. One difference:
from_chars rejects a leading '+' that strtod accepts. Caveat: floating-point
from_chars needs libstdc++ 11+ or libc++ 20+. Demo file: demos/s01/from_chars.cpp
-->

---

## `std::string_view`: a preview <span class="badge cpp17">C++17</span>

```cpp
bool parse_record(std::string_view line, Record* out, ParseError* err);   // no copy, any source

parse_record("1,rpm,4800", ...);          // from a literal: no std::string constructed
parse_record(std::string_view(buffer).substr(0, n));   // from a slice: no allocation
```

A pointer and a length. Non-owning. **Never store one** unless you own what it points at.

Session 2 covers it properly with `span`, `optional`, and friends.

<!--
Notes: One slide only. Plant the lifetime rule now because it is the same rule as span, and the
same rule as the escaping lambda: a view is a reference. Hand-typed preview.
-->

---

## Two quiet fixes <span class="badge cpp17">C++17</span>

**Evaluation order is now specified** for `a.b`, `a->b`, `a(b)` argument-vs-callee, `a = b`, `a << b`, and more.
`f(std::unique_ptr<A>(new A), g())` can no longer leak if `g` throws. `m[k] = m.size();` behaves.

**`noexcept` is part of the function type.**
`void (*p)() noexcept = f;` requires `f` to be `noexcept`. A `noexcept` function pointer cannot receive a throwing function.

<!--
Notes: Optional slide if behind. Both are things that made C++11 code subtly wrong and are now
simply correct. The leak fix: arguments are now indeterminately sequenced, so `new A` and the
unique_ptr constructor cannot interleave with `g()`. The order between arguments is still
unspecified, and the operands of `+` are still unordered; only the listed operators were fixed.
-->

---

<!-- _class: takeaway -->

## C++17 takeaway

Removed: `auto_ptr`, `register`, trigraphs, `throw(...)` specifications, `random_shuffle`, `bind1st`.

**What changes for you:** structured bindings and `if`-with-initializer reshape every function; `[[nodiscard]]` finds bugs the day you add it; `inline` variables end the extern/definition dance.

**Monday morning:** add `[[nodiscard]]` to one header and read what the compiler says.

<!--
Notes: Time check: 1:20. The next segment is 12 slides in 15 minutes; three of them are the
spaceship operator and deserve the time, the rest are fast.
-->

---

<!-- SEGMENT: C++20/23 small features (1:20) -->

<!-- _class: twocol -->

## Designated initializers <span class="badge cpp20">C++20</span>

<div class="cols">
<div>

#### Before (C++11)

```cpp
const SensorConfig kSensors[] = {
    {"rpm", "rpm", 0.0, 12000.0},
    // which double is which?
};
```

</div>
<div>

#### After (C++20)

<!-- snippet: demos/s01/designated_init.cpp#designated -->
```cpp
struct SensorConfig {
    const char* name;
    const char* units;
    double min_valid;
    double max_valid;
};

constexpr SensorConfig kRpm{
    .name = "rpm", .units = "rpm", .min_valid = 0.0, .max_valid = 12'000.0};
constexpr SensorConfig kTemp{
    .name = "temp_core", .units = "degC", .min_valid = -40.0, .max_valid = 125.0};
// Skipped members are value-initialized (but -Wextra warns).
// SensorConfig bad{.units = "V", .name = "x"};   // error: wrong order
```

</div>
</div>

<!--
Notes: Aggregates only, declaration order only (unlike C99, no out-of-order and no array
designators). Skipped trailing members are value-initialized but -Wextra warns, so under this
repo's flags you name them all. The sensor table in the exercise is the natural place for this.
The left block is hand-typed. Demo file: demos/s01/designated_init.cpp
-->

---

<!-- _class: twocol -->

## Three-way comparison, 1: the problem <span class="badge cpp20">C++20</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s01/spaceship.cpp#before -->
```cpp
// C++11: six operators, all hand-written
struct Version11 {
    int major, minor, patch;
};
using V = Version11;
bool operator==(const V& a, const V& b) {
    return a.major == b.major && a.minor == b.minor
        && a.patch == b.patch;
}
bool operator<(const V& a, const V& b) {
    if (a.major != b.major) return a.major < b.major;
    if (a.minor != b.minor) return a.minor < b.minor;
    return a.patch < b.patch;
}
bool operator!=(const V& a, const V& b) {return !(a == b);}
bool operator>(const V& a, const V& b)  {return b < a;}
bool operator<=(const V& a, const V& b) {return !(b < a);}
bool operator>=(const V& a, const V& b) {return !(a < b);}
```

</div>
<div>

#### After (C++20)

<!-- snippet: demos/s01/spaceship.cpp#after -->
```cpp
// C++20: one line; == and <=> generated memberwise
struct Version20 {
    int major, minor, patch;
    auto operator<=>(const Version20&) const = default;
};
```

</div>
</div>

<!--
Notes: The starter's Record has exactly the left side, six operators over four members (exercise
task 2). The left is correct as written; the question is how the next edit could break it: forget a member in ==, order members differently
in < and ==, write !(a < b) for <= when the type is only partially ordered. The right side cannot
be inconsistent. Demo file: demos/s01/spaceship.cpp
-->

---

<!-- _class: feature -->

## Three-way comparison, 2: rewriting and `==` <span class="badge cpp20">C++20</span>

<p class="problem">You declare one or two operators. The compiler rewrites the other four calls in terms of them.</p>

<!-- snippet: demos/s01/spaceship_details.cpp#rewriting -->
```cpp
// What the compiler does with a < b when only <=> is declared:
//     a < b      becomes   (a <=> b) < 0
//     a >= b     becomes   (a <=> b) >= 0
//     a > b      becomes   (a <=> b) > 0
//     42 > v     becomes   0 > (v <=> 42)    (reversed: only when the types differ)
// What it does NOT do: derive == from <=>. A defaulted <=> also defaults ==,
// but a hand-written <=> leaves == undeclared. Reason: == can be much faster
// (std::string compares lengths first), so the two are kept separate.
```

<!--
Notes: Two operators, not one: <=> gives the four relational operators; == gives == and !=. A
defaulted <=> defaults == for free. A hand-written <=> does not, and you must write == yourself
or you will get a compile error on a == b. The reason is performance: equality can bail out
early (string length), ordering cannot. Demo file: demos/s01/spaceship_details.cpp
-->

---

<!-- _class: feature -->

## Three-way comparison, 3: categories and member order <span class="badge cpp20">C++20</span>

<p class="problem">The return type says what kind of ordering you have. `auto` deduces the weakest category among the members.</p>

<!-- snippet: demos/s01/spaceship_details.cpp#categories -->
```cpp
struct Version {
    int major, minor;
    std::strong_ordering operator<=>(const Version&) const = default;
};

struct Reading {
    std::string sensor;
    double value;                                    // NaN exists, so...
    std::partial_ordering operator<=>(const Reading&) const = default;   // (auto deduces this)
};
```

<!-- snippet: demos/s01/spaceship_details.cpp#order -->
```cpp
struct Bad {
    double value;      // members compare in declaration order,
    long long ts;      // so this orders by value first. Oops.
    auto operator<=>(const Bad&) const = default;
};
```

<!--
Notes: strong_ordering: equal means substitutable (ints). weak_ordering: equivalent but
distinguishable (case-insensitive strings). partial_ordering: some pairs are unordered (doubles,
because of NaN). Record has a double, so its defaulted <=> is partial; that is fine for sorting
as long as there are no NaNs. Member order IS the comparison order; the Bad struct is the trap.
Demo: run demo_s01_spaceship_details and then swap two members on Compiler Explorer.
-->

---

<!-- _class: feature -->

## Range-for with initializer <span class="badge cpp20">C++20</span>

<p class="problem">The range expression's temporaries die before the loop body. Anything reached through a temporary is a dangling reference.</p>

<!-- snippet: demos/s01/range_for_init.cpp#bug -->
```cpp
// for (const auto& n : load().items()) {}   // BUG before C++23: the Batch dies first
```

<!-- snippet: demos/s01/range_for_init.cpp#fix -->
```cpp
for (auto batch = load(); const auto& n : batch.items()) {   // C++20: lives all loop
    std::printf("%s\n", n.c_str());
}
```

<!--
Notes: `for (auto& x : get_batch().items())` is a well-known dangling bug: get_batch() returns a
temporary, .items() returns a reference into it, the temporary is destroyed, the loop iterates
freed memory. C++20 lets you name the temporary in the init-statement. C++23 (P2718) finally
extends the lifetime of all temporaries in the range expression, so the bug line becomes legal on
a C++23 compiler; GCC 15 and Clang 19 implement it. Demo file: demos/s01/range_for_init.cpp
-->

---

<!-- _class: twocol -->

## `using enum` <span class="badge cpp20">C++20</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s01/using_enum.cpp#before -->
```cpp
const char* to_string_cpp11(Status s) {
    switch (s) {
    case Status::Ok:      return "ok";
    case Status::Suspect: return "suspect";
    case Status::Fault:   return "fault";
    }
    return "?";
}
```

</div>
<div>

#### After (C++20)

<!-- snippet: demos/s01/using_enum.cpp#after -->
```cpp
const char* to_string(Status s) {
    using enum Status;          // in scope for this block
    switch (s) {
    case Ok:      return "ok";
    case Suspect: return "suspect";
    case Fault:   return "fault";
    }
    return "?";
}
```

</div>
</div>

<!--
Notes: Scoped to the block it appears in. The right answer to "enum class is too verbose in
switches" that people used to solve with plain enums. Exercise task 8. Demo file:
demos/s01/using_enum.cpp
-->

---

## `[[likely]]`, `[[unlikely]]`, `[[no_unique_address]]` <span class="badge cpp20">C++20</span>

```cpp
if (err != ParseError::None) [[unlikely]] { return false; }

struct Handle {
    int fd;
    [[no_unique_address]] EmptyDeleter d;   // takes zero bytes; sizeof(Handle) == sizeof(int)
};
```

- `[[likely]]`/`[[unlikely]]` on a branch or `case`: a hint to code layout. Measure before using; PGO beats guesses.
- `[[no_unique_address]]`: lets an empty member occupy no storage (the empty-base-class trick without inheritance). MSVC needs `[[msvc::no_unique_address]]`.

<!--
Notes: Fast slide. The honest advice on likely/unlikely: use them on error paths in hot loops
and nowhere else. no_unique_address matters for allocator-aware containers and policy-based
designs; most application code never writes it. Hand-typed block.
-->

---

## Know they exist <span class="badge cpp20">C++20</span>

- `char8_t`: a distinct type for UTF-8; `u8"..."` literals are now `const char8_t*`, which **breaks** code that assigned them to `const char*`
- `__VA_OPT__(,)`: variadic macros that work with zero arguments
- `consteval` (must run at compile time) and `constinit` (must be statically initialized): Session 3
- `[=, this]` capture spelled explicitly; `[=]` still captures `this` implicitly, but that is now deprecated
- Aggregates can be initialized with parentheses: `Record(1, "rpm", 0.0, Status::Ok)`

<!--
Notes: Name-drop slide. char8_t is the one that bites during migration: -fno-char8_t exists on
both compilers as an escape hatch. Everything else here: recognize it in code review.
-->

---

<!-- _class: feature -->

## The small C++23 features <span class="badge cpp23">C++23</span>

<p class="problem">Three quality-of-life additions and one new way to say "this cannot happen".</p>

<!-- snippet: demos/s01/small_cpp23.cpp#small -->
```cpp
void small(std::vector<std::string>& v) {
    for (auto i = 0uz; i < v.size(); ++i) {}              // uz: a size_t literal

    std::erase(v, auto(v.front()));                         // auto(x): explicit decay copy.
    // Without it, erase takes v.front() by reference, and that reference is to an
    // element the erase is moving. The copy cannot alias.

    std::printf("%u\n", std::to_underlying(Level::High));   // no static_cast needed
}

int classify(int x) {
    if (x < 0) return -1;
    if (x == 0) return 0;
    if (x > 0) return 1;
    std::unreachable();                                     // a promise; UB if reached
}
```

<!--
Notes: uz ends the signed/unsigned comparison warning in index loops. auto(x) is the explicit
"give me a copy that does not alias" that was previously spelled `T(x)` or `std::decay_t`; without
it, `std::erase(v, v.front())` misses the second "first" (3 elements left instead of 2). to_underlying
replaces a static_cast to underlying_type_t. std::unreachable is UB if reached, which the
optimizer exploits; assert-then-unreachable is the pattern. Demo file: demos/s01/small_cpp23.cpp
-->

---

## Preprocessor and grammar tidying <span class="badge cpp23">C++23</span>

<!-- snippet: demos/s01/small_cpp23.cpp#preprocessor -->
```cpp
#ifdef NDEBUG
#elifdef TRACE          // C++23: #elifdef / #elifndef
#warning "tracing build"   // C++23: standard at last
#endif
```

Also in C++23: `[[assume(expr)]]` (a promise to the optimizer, UB if false), labels at the end of a compound statement, `static_assert` with no message required (that one was C++17), alias declarations in `if` initializers.

<!--
Notes: `#warning` was a universal extension that finally got standardized. [[assume]] is GCC 13+
and Clang 19+; Clang 18 ignores it with a warning, which under -Werror is a build break, so it
stays off the compiled demo. Under a minute for this slide.
-->

---

## Deprecations worth knowing today <span class="badge cpp23">C++20/23</span>

- **C++20 deprecated** compound assignment on `volatile` (`v += 1`, `v++`); C++23 un-deprecated `+=` and friends but `++`/`--` stay deprecated. Embedded code is full of these; `v = v + 1` is the safe spelling.
- **C++23 deprecated** `std::aligned_storage` and `std::aligned_union`: use `alignas` on a byte array.
- **C++20 deprecated** implicit capture of `this` via `[=]`; write `[=, this]`.
- **C++17 deprecated** (still present): `std::iterator`, `<codecvt>`. **C++17 deprecated, C++20 removed**: `std::result_of`, `std::uncaught_exception` (use the plural).

<!--
Notes: The volatile story is the one to tell in a defense/embedded room: P1152 deprecated the
compound ops in C++20 because they hide two accesses; in C++23, P2327 restored the bitwise ones
(`|=`, `&=`, `^=`) and CWG 2654 the rest, after pushback from the embedded community. Increment
is still deprecated. -Wdeprecated finds them.
-->

---

<!-- _class: takeaway -->

## C++20 and C++23 small features: takeaway

**Use now:** `operator<=>` on every value type; designated initializers on configuration tables; `using enum` in switches; `uz` in index loops.

**Know for review:** `[[likely]]`, `char8_t`, `[=, this]`, the volatile rules.

**Monday morning:** delete six operators from one struct.

<!--
Notes: 1:35. Transition to the exercise.
-->

---

<!-- SEGMENT: Guided exercise (1:35) -->

## Exercise: modernize the syntax

Open `exercises/s01-modernize-syntax/README.md`

**In class (20 minutes), in order, run the tests after each:**

1. `typedef` to `using`
2. Six comparison operators on `Record` to one defaulted `operator<=>`
3. Structured bindings and `if`-with-initializer in `load_stream` and `compute_stats`

```
cmake --build build && ctest --test-dir build -R s01 --output-on-failure
```

**At home:** tasks 4 to 9. `solution/` is next session's starter.

The starter builds as C++11. Before task 2, change `11` to `23` in `exercises/s01-modernize-syntax/CMakeLists.txt`.

<!--
Notes: Walk the room. The first blocker: the starter is compiled as C++11 (add_exercise_variant(s01
starter 11) in the exercise CMakeLists), so `operator<=>` will not compile until that is 23.
The common stumble on task 2: forgetting `#include <compare>` or leaving
one old operator declaration in the header (ambiguous overload). On task 3: trying to bind into
existing variables. Call time at 20 minutes and show the solution's record.h.
-->

---

<!-- _class: takeaway -->

## Session 1 takeaway

Almost every line of a modern function looks slightly different from its C++11 form, and **each difference removes a class of bug**: unused results, forgotten `break`, dangling globals in headers, six-way comparison boilerplate.

**Next session:** the types that replace raw pointers, sentinels, out-parameters, and `printf`. `optional`, `variant`, `expected`, `string_view`, `span`, `format`. Bring your solution.

<!--
Notes: Close by pointing at handouts/feature-timeline.md: every feature from today is on it,
tagged with session and standard.
-->
