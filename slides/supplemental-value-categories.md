---
marp: true
theme: course
paginate: true
footer: 'The Evolution of C++ | Supplemental: Value Categories'
---

<!-- _class: lead -->
<!-- _paginate: false -->

# Value Categories
## lvalue, prvalue, xvalue, glvalue, rvalue

Supplemental material for The Evolution of C++

Every classification in this deck is a `static_assert` in `demos/supplemental/value_categories.cpp`.

<!--
Notes: Twenty minutes of self-study or a short live segment. The goal is not the standardese; it
is being able to answer "will this copy or move?" and "why does std::move do nothing here?" by
looking at an expression.
-->

---

## Why this matters

Value categories decide, for every expression:

- Whether a **move** or a **copy** happens
- Which **overload** is chosen (`T&`, `const T&`, `T&&`)
- Whether a **temporary** is created, and how long it lives
- Whether **copy elision** is guaranteed (C++17)
- What `decltype`, `auto&&`, and `std::forward` produce

Most "why did that copy?" bugs are value-category bugs.

<!--
Notes: Tie back to Session 1: the "std::move does nothing" slide and the guaranteed copy elision
slide were both value-category facts in disguise.
-->

---

## A little history

- **C:** an *lvalue* is something that can appear on the **l**eft of `=`. Everything else is an rvalue.
- **C++98:** the same two, but references complicated it: a function returning `T&` is an lvalue, `const T&` binds to rvalues.
- **C++11:** rvalue references and move semantics needed a third kind: something with an identity that you are *allowed to steal from*. The taxonomy was redrawn.
- **C++17:** prvalues stopped being objects at all ("temporary materialization"), which is what made copy elision guaranteed.

<!--
Notes: The "left of assignment" story is why people find the names confusing: the names survived
but the definitions changed. Tell them to forget "left" and "right".
-->

---

## The taxonomy: two questions

Every expression is classified by two yes/no questions:

|  | **Can be moved from?** No | **Can be moved from?** Yes |
|---|---|---|
| **Has identity?** Yes | **lvalue** | **xvalue** |
| **Has identity?** No | (does not exist) | **prvalue** |

- **Has identity:** you can take its address, or compare it with another for sameness
- **Can be moved from:** binding it to `T&&` is allowed

<!--
Notes: This is Stroustrup's formulation (N3055). The empty cell is why there are exactly three
leaf categories. Everything else in the deck is corollaries of this table.
-->

---

## The tree

```
                    expression
                   /          \
             glvalue          rvalue
            /       \        /      \
       lvalue      xvalue        prvalue
```

- **glvalue** ("generalized lvalue") = has identity = **lvalue or xvalue**
- **rvalue** = can be moved from = **xvalue or prvalue**
- **xvalue** ("expiring value") is in both: it has identity **and** may be moved from

Five names, three leaves. `glvalue` and `rvalue` are the two unions.

<!--
Notes: If they remember one picture, this one. The two unions are what the standard's rules are
mostly written in terms of: "if the operand is a glvalue..." "an rvalue of class type..."
-->

---

## The probe used in this deck

`decltype((expr))`, with the double parentheses, reports the category:

- `T` for a **prvalue**
- `T&` for an **lvalue**
- `T&&` for an **xvalue**

<!-- snippet: demos/supplemental/value_categories.cpp#probe -->
```cpp
#define IS_LVALUE(e)  std::is_lvalue_reference_v<decltype((e))>
#define IS_XVALUE(e)  std::is_rvalue_reference_v<decltype((e))>
#define IS_PRVALUE(e) (!std::is_reference_v<decltype((e))>)
```

<!--
Notes: Single parentheses give the declared type of a name; double parentheses treat it as an
expression. This is the one legitimate everyday use of decltype's second rule. Everything on the
following slides is checked with these three macros.
-->

---

## lvalues: things with a name or a home

<!-- snippet: demos/supplemental/value_categories.cpp#lvalues -->
```cpp
static_assert(IS_LVALUE(i));          // a name
static_assert(IS_LVALUE(*p));         // dereference
static_assert(IS_LVALUE(arr[1]));     // subscript on an lvalue array
static_assert(IS_LVALUE(w.n));        // member of an lvalue
static_assert(IS_LVALUE(++i));        // pre-increment returns the object
static_assert(IS_LVALUE(i = 5));      // assignment returns the left operand
static_assert(IS_LVALUE("hello"));    // string literal: an lvalue array of const char
static_assert(IS_LVALUE(global()));   // a call returning T&
```

<!--
Notes: The surprises: a string literal is an lvalue (it is an array with static storage, you can
take its address); pre-increment and assignment return the object itself; a function returning T&
is an lvalue no matter how it is called.
-->

---

## prvalues: pure values, no identity

<!-- snippet: demos/supplemental/value_categories.cpp#prvalues -->
```cpp
static_assert(IS_PRVALUE(42));           // literal
static_assert(IS_PRVALUE(i + 1));        // arithmetic
static_assert(IS_PRVALUE(i++));          // post-increment returns a copy
static_assert(IS_PRVALUE(make()));       // a call returning by value
static_assert(IS_PRVALUE(Widget{}));     // a temporary
static_assert(IS_PRVALUE(&i));           // address-of
static_assert(IS_PRVALUE([] { return 1; }));   // a lambda expression
static_assert(IS_PRVALUE(i < 3 ? 1 : 2));      // both arms prvalues
```

Since C++17 a prvalue is **not an object**; it is a recipe for initializing one.

<!--
Notes: Post-increment returns a copy, so prvalue; contrast with pre-increment on the previous
slide. The lambda expression itself is a prvalue (the closure object is what gets created). The
"recipe" framing pays off two slides later.
-->

---

## xvalues: identity, but expiring

<!-- snippet: demos/supplemental/value_categories.cpp#xvalues -->
```cpp
static_assert(IS_XVALUE(std::move(i)));        // the whole point of std::move
static_assert(IS_XVALUE(static_cast<int&&>(i)));
static_assert(IS_XVALUE(stolen()));            // a call returning T&&
static_assert(IS_XVALUE(make().name));         // member of an rvalue object
static_assert(IS_XVALUE(std::move(arr)[0]));   // subscript on an rvalue array
```

An xvalue is an existing object that the code has **promised** it no longer needs intact.

<!--
Notes: std::move is nothing but static_cast<T&&>, which produces an xvalue. A member of an
rvalue object is an xvalue: make().name can be moved out of the temporary. Same for elements of
an rvalue array.
-->

---

## The rule everyone trips on

<!-- snippet: demos/supplemental/value_categories.cpp#named_rref -->
```cpp
void take(Widget&& x) {
    static_assert(IS_LVALUE(x));               // x has a name: it is an lvalue
    Widget a = x;                               // ...so this COPIES
    Widget b = std::move(x);                    // ...and this moves
    static_assert(IS_XVALUE(std::move(x)));
    (void)a; (void)b;
}
```

**Type** and **value category** are different things. `x` has type `Widget&&`. The expression `x` is an lvalue, because it has a name. So inside the function, `x` copies unless you `std::move` it again.

<!--
Notes: This is the single most important slide. "If it has a name, it is an lvalue" covers 90% of
real-world confusion, including why move constructors are written with std::move on every member.
-->

---

## Overload resolution

<!-- snippet: demos/supplemental/value_categories.cpp#overloads -->
```cpp
const char* f(Widget&)        { return "Widget&"; }
const char* f(const Widget&)  { return "const Widget&"; }
const char* f(Widget&&)       { return "Widget&&"; }
// f(w)            -> Widget&         (lvalue)
// f(make())       -> Widget&&        (prvalue, materialized into a temporary)
// f(std::move(w)) -> Widget&&        (xvalue)
// f(cw)           -> const Widget&   (const lvalue)
```

Rules of thumb: `T&&` wins for rvalues; `T&` wins for non-const lvalues; `const T&` catches everything else and is the fallback when `T&&` does not exist.

<!--
Notes: Run the demo: it prints "Widget& Widget&& Widget&& const Widget&". Point out that a
prvalue and an xvalue pick the same overload; the distinction between them matters for elision
and lifetime, not for overloading.
-->

---

## Temporary materialization

When a prvalue is used where an object is needed (bind a reference, access a member, call a member function), the compiler **materializes** a temporary: the prvalue is converted to an **xvalue** that names it.

```cpp
make().name                 // prvalue materialized so .name exists; result: xvalue
const Widget& r = make();   // materialized; r extends its lifetime
Widget w = make();          // NOT materialized: the prvalue initializes w directly
```

That last line is guaranteed copy elision: there is nothing to elide, because there was never a second object.

<!--
Notes: This is the C++17 change (P0135). Before it, make() produced a temporary object that was
then copied or moved into w and the compiler was merely permitted to skip that. Now the standard
says w IS the result. Consequence: non-movable types can be returned by value (Session 1 demo
demos/s01/copy_elision.cpp).
-->

---

## Lifetime: what binds and what dangles

```cpp
const std::string& a = std::string("temp");   // OK: extended to a's scope
std::string&& b = std::string("temp");        // OK: same
const std::string& c = std::move(s);          // NO extension: s already exists
const char* d = std::string("temp").c_str();  // DANGLES after this line
auto&& e = make();                            // OK: auto&& binds anything
```

Lifetime extension applies to a **materialized prvalue** bound **directly** to a reference. Not through a member function, not through a cast, not to an xvalue of something that already exists.

<!--
Notes: Line 4 is the string_view/c_str dangling bug from Session 2 in its purest form. Line 3 is
the one people get wrong the other way: they think std::move creates a temporary. It does not.
-->

---

## `std::move` and `std::forward`

<!-- snippet: demos/supplemental/value_categories.cpp#forward -->
```cpp
template <typename T>
void relay(T&& arg) {                    // forwarding reference: T&& on a deduced T
    f(std::forward<T>(arg));             // lvalue in, lvalue out; rvalue in, xvalue out
}
```

- `std::move(x)` is `static_cast<T&&>(x)`: it **turns an lvalue into an xvalue**. No code runs.
- `std::forward<T>(arg)` casts to `T&&` and lets **reference collapsing** decide: `T` = `Widget&` gives `Widget& &&` = `Widget&` (still an lvalue); `T` = `Widget` gives `Widget&&` (an xvalue)
- A `T&&` parameter where `T` is deduced is a **forwarding reference**, not an rvalue reference

<!--
Notes: Reference collapsing: & & = &, & && = &, && & = &, && && = &&. Only rvalue-rvalue stays
rvalue. That table is all std::forward is. Emphasize: use forward only on forwarding references,
move only on things you own and are done with.
-->

---

## Common mistakes, classified

| Code | Category fact | What actually happens |
|---|---|---|
| `return std::move(local);` | local is already treated as an rvalue in a return | Blocks elision; both compilers warn |
| `consume(std::move(constObj));` | xvalue of a `const` object | Copies silently; `T&&` cannot bind to `const` |
| `void f(T&& x) { g(x); }` | `x` is an lvalue | Copies; needs `std::move(x)` or `std::forward` |
| `for (auto& e : get().items())` | `get()` is a prvalue, dies at `;` | Dangles (pre-C++23); Session 1 range-for slide |
| `auto p = std::string("x").c_str();` | pointer into a materialized temporary | Dangles immediately |

<!--
Notes: Each row is something that appeared in Session 1 or 2. The value-category column is the
"why"; if attendees can fill that column themselves, the deck has done its job.
-->

---

## Quiz: classify each expression

```cpp
int i = 0;  int arr[4]{};  std::string s;  std::vector<int> v;
Widget w;   Widget& ref = w;

1.  i + 1              6.  std::move(s)
2.  ++i                7.  ref
3.  i++                8.  Widget{}.n
4.  arr[2]             9.  v[0]
5.  "text"            10.  std::vector<int>{1, 2}[0]
```

<!--
Notes: Give them two minutes. The two that catch people: 5 (string literal is an lvalue) and 10
(subscript on an rvalue vector is an lvalue, because operator[] returns int&; contrast with the
built-in array case on the xvalue slide).
-->

---

## Answers

| # | Expression | Category | Why |
|---|---|---|---|
| 1 | `i + 1` | prvalue | computed value |
| 2 | `++i` | lvalue | returns the object |
| 3 | `i++` | prvalue | returns a copy |
| 4 | `arr[2]` | lvalue | subscript on lvalue array |
| 5 | `"text"` | lvalue | array with static storage |
| 6 | `std::move(s)` | xvalue | cast to `std::string&&` |
| 7 | `ref` | lvalue | a name |
| 8 | `Widget{}.n` | xvalue | member of a prvalue (materialized) |
| 9 | `v[0]` | lvalue | `operator[]` returns `int&` |
| 10 | `std::vector<int>{1, 2}[0]` | lvalue | still `int&`; the vector dies at `;` (dangling if bound) |

<!--
Notes: Number 10 is the sting in the tail: the standard's rule for rvalue arrays does not apply to
user-defined operator[], so you get an lvalue referring into a temporary. Exactly the bug on the
lifetime slide.
-->

---

<!-- _class: takeaway -->

## Cheat sheet

**Has a name or an address? lvalue.** Named `T&&` variables included.
**Computed on the spot, no identity? prvalue.** Literals, arithmetic, calls returning by value, temporaries.
**An existing object you have promised to give up? xvalue.** `std::move(x)`, calls returning `T&&`, members of rvalues.

**`std::move` is a cast to xvalue.** It moves nothing by itself.
**A prvalue is a recipe, not an object** (C++17), which is why returning by value is free.
**Lifetime extension** is only for prvalues bound directly to a reference.

<!--
Notes: Point at demos/supplemental/value_categories.cpp for the static_assert version of every
claim here, and at cppreference's "Value categories" page for the exhaustive list.
-->
