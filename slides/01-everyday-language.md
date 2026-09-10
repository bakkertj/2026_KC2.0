---
marp: true
theme: course
paginate: true
footer: 'The Evolution of C++ | Session 1: The Everyday Language'
---

<!-- _class: lead -->
<!-- _paginate: false -->

# The Evolution of C++
## Session 1: The Everyday Language

Small syntax changes that touch nearly every function you write

<!--
Notes: Set expectations for the whole course in the first two minutes: thematic, not chronological;
every feature carries a badge; the timeline handout is the map. Then the environment check.
-->

---

## Agenda

1. Why this course is organized by theme, not by standard (10 min)
2. C++11 calibration (20 min)
3. C++14: the polish release (25 min)
4. C++17: syntax you will use daily (25 min)
5. C++20 and C++23: small but valuable (15 min)
6. Guided exercise: modernize the syntax (20 min)

<!-- Notes: Point at the exercise README now so people can open it during the break. -->

---

<!-- SEGMENT: Course overview (0:00). Slides to write: thematic vs chronological, the size of each standard,
     how to read the cppreference support table, environment check. -->

## Why not one session per standard?

| Standard | Character | Share of "must know" |
|---|---|---|
| C++14 | Polish release for C++11 | ~5% |
| C++17 | The everyday features | ~25% |
| C++20 | Largest change since C++11 | ~50% |
| C++23 | Completes C++20, fills library gaps | ~20% |

<!-- Notes: constexpr, lambdas and ranges each span three or four standards. Chronological teaching means teaching each one four times. -->

---

<!-- SEGMENT: C++11 calibration (0:10). Slides to write: auto pitfalls, when std::move does nothing,
     lambdas capturing by reference that escape, unique_ptr vs shared_ptr, enum class, constexpr (C++11 form). -->

<!-- SEGMENT: C++14 (0:30). Slides to write: generic lambdas, init-capture, return type deduction,
     decltype(auto), make_unique, literals and separators, [[deprecated]], std::exchange, integer_sequence,
     relaxed constexpr (preview), gets removed. -->

<!-- _class: feature -->

## Generic lambdas <span class="badge cpp14">C++14</span>

<p class="problem">One comparison lambda per element type is boilerplate; auto parameters make one lambda a template.</p>

<!-- snippet: demos/s01/generic_lambda.cpp#generic -->
```cpp
// C++14: auto parameters make one lambda work for any comparable type
auto by_size = [](const auto& a, const auto& b) { return a.size() < b.size(); };
```

<!-- snippet: demos/s01/generic_lambda.cpp#init_capture -->
```cpp
// C++14: init-capture moves a unique_ptr into the closure, which C++11 could not express
auto make_printer(std::unique_ptr<std::string> owned) {
    return [s = std::move(owned)] { std::printf("%s\n", s->c_str()); };
}
```

<!--
Notes: Init-capture is the one people miss. Ask: how would you capture a unique_ptr in C++11? (You can't
without a shared_ptr or a wrapper.) Demo file: demos/s01/generic_lambda.cpp
-->

---

<!-- SEGMENT: C++17 (0:55). Slides to write: structured bindings, if/switch with initializer, inline variables,
     nested namespaces, attributes, guaranteed copy elision, CTAD, string_view preview, evaluation order,
     noexcept in the type, removals. -->

<!-- _class: twocol -->

## Structured bindings and if-with-initializer <span class="badge cpp17">C++17</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s01/structured_bindings.cpp#before -->
```cpp
// C++11: the return value of insert is a pair, and both halves need names
void record_cpp11(const std::string& key) {
    std::pair<std::map<std::string, int>::iterator, bool> r = counts.insert({key, 1});
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
// C++17: structured bindings name the halves; the initializer is scoped to the if
void record_cpp17(const std::string& key) {
    if (auto [it, inserted] = counts.insert({key, 1}); !inserted) {
        ++it->second;
    }
}
```

</div>
</div>

<!-- Notes: Emphasize the scoping: it and inserted die at the end of the if/else. Demo file: demos/s01/structured_bindings.cpp -->

---

<!-- SEGMENT: C++20/23 small features (1:20). Slides to write: designated initializers, range-for with init,
     <=> (full treatment), using enum, likely/unlikely, no_unique_address, char8_t, __VA_OPT__, auto(x),
     uz, #elifdef/#warning, to_underlying, unreachable, [[assume]]. -->

<!-- _class: twocol -->

## Three-way comparison <span class="badge cpp20">C++20</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s01/spaceship.cpp#before -->
```cpp
// C++11: six operators, all hand-written, all easy to get subtly wrong
struct Version11 {
    int major, minor, patch;
};
bool operator==(const Version11& a, const Version11& b) {
    return a.major == b.major && a.minor == b.minor && a.patch == b.patch;
}
bool operator<(const Version11& a, const Version11& b) {
    if (a.major != b.major) return a.major < b.major;
    if (a.minor != b.minor) return a.minor < b.minor;
    return a.patch < b.patch;
}
bool operator!=(const Version11& a, const Version11& b) { return !(a == b); }
bool operator>(const Version11& a, const Version11& b) { return b < a; }
bool operator<=(const Version11& a, const Version11& b) { return !(b < a); }
bool operator>=(const Version11& a, const Version11& b) { return !(a < b); }
```

</div>
<div>

#### After (C++20)

<!-- snippet: demos/s01/spaceship.cpp#after -->
```cpp
// C++20: one line; == and <=> are generated memberwise, the other four are rewritten
struct Version20 {
    int major, minor, patch;
    auto operator<=>(const Version20&) const = default;
};
```

</div>
</div>

<!-- Notes: Explain rewriting: a < b becomes (a <=> b) < 0; a != b becomes !(a == b). == is NOT derived from <=>
     (efficiency: string == can short-circuit on length). Demo file: demos/s01/spaceship.cpp -->

---

<!-- _class: demo -->

## Defaulted comparisons on a real struct

Compiler Explorer: <add link>

Watch for: the generated `operator==` short-circuiting on the first differing member, and what happens when a member type has no `<=>`.

---

<!-- SEGMENT: Guided exercise (1:35). One slide: point at exercises/s01-modernize-syntax/README.md. -->

<!-- _class: takeaway -->

## Takeaway

Almost every line of a modern function looks slightly different from its C++11 form, and **each difference removes a class of bug**: unused results, forgotten `break`, dangling globals in headers, six-way comparison boilerplate.

**Monday morning:** add `[[nodiscard]]` to one header and see what the compiler tells you.
