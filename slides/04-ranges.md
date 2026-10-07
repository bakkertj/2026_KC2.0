---
marp: true
theme: course
paginate: true
footer: 'The Evolution of C++ | Session 4: Ranges and Algorithms'
---

<!-- _class: lead -->
<!-- _paginate: false -->

# The Evolution of C++
## Session 4: Ranges and Algorithms

A new model of iteration that composes

<!--
Notes: The biggest change to how loops are written since range-based for. C++20 gave the model,
C++23 the pieces that make it daily-usable. Warn up front that libc++ 18 is behind on several
C++23 views; the support matrix slide at the end lists them.
-->

---

## Agenda

1. Recap and framing (10 min)
2. The C++20 ranges foundation (25 min)
3. Views and pipelines (25 min)
4. C++23: ranges become practical (20 min)
5. Parallel algorithms (10 min)
6. Algorithm additions worth knowing (10 min)
7. Guided exercise (20 min)

Every demo opens in Compiler Explorer, preconfigured for GCC 14: `handouts/compiler-explorer-links.md`

<!--
Notes: Exercise README: exercises/s04-ranges/README.md. The starter is the Session 3 solution.
-->

---

## Session 3 recap

The solution: `constexpr` CRC table and `crc16`, a `consteval` validator on the sensor table, four constrained `serialize` templates with a `Serializable` concept, deducing `this` on `SensorStats::add`.

Where people got stuck:

- `std::as_bytes` in a `constexpr` function: it is a `reinterpret_cast`, and those are never constant expressions
- `transform(&SensorConfig::units)` on a temporary optional, still
- `SerializableRange` without `!StringLike`: a `std::string` matched both the string overload and the range overload, ambiguously

<!--
Notes: The third one is today's material: a std::string IS a range, and that fact will come up
again with views::split.
-->

---

## The iterator-pair problem

The Session 3 solution still contains:

```cpp
std::stable_sort(matching.begin(), matching.end(), [](const auto& a, const auto& b) { return a.value > b.value; });
std::lower_bound(records.begin(), records.end(), ts, [](const Record& r, Timestamp t) { return r.ts < t; });
std::copy_if(records.begin(), records.end(), std::back_inserter(matching), [sensor](const Record& r) { ... });
for (const auto& r : records) { lo = std::min(lo, r.value); hi = std::max(hi, r.value); }
```

Every one: **two iterators** that must match, a **comparator lambda** that says "by this member", and **no way to compose** the steps without a temporary vector.

<!--
Notes: The four lines are real, from stats.cpp. Count the lambdas: three. Two say "compare by a
member"; projections remove those two, and the min/max loop. The copy_if predicate captures
`sensor` and survives as the filter lambda on the pipe-syntax slide. Hand-typed excerpt.
-->

---

## Ranges in one sentence

**The standard algorithms with the iterator pairs removed**, plus **lazy views** that compose with `|`.

```cpp
std::ranges::sort(records, {}, &Record::value);                      // the range, and a projection

auto top = records | std::views::filter(is_rpm) | std::views::take(3);   // lazy: nothing runs yet
```

C++20 gave the model: concepts, constrained algorithms, projections, views, pipelines.
C++23 gave the pieces: `to`, `zip`, `enumerate`, `chunk_by`, `fold_left`, and twenty more.

<!--
Notes: Set expectations: the C++20 part is what you will use every day starting tomorrow; the
C++23 part is what makes views worth using in production.
-->

---

<!-- SEGMENT: Foundation (0:10) -->

<!-- _class: feature dense -->

## Range concepts <span class="badge cpp20">C++20</span>

<p class="problem">A range is anything with begin() and end(). The concepts say what kind.</p>

<!-- snippet: demos/s04/range_concepts.cpp#hierarchy -->
```cpp
namespace r = std::ranges;
using V = std::vector<int>;
static_assert(r::contiguous_range<V> && r::contiguous_range<std::span<int>> && r::contiguous_range<std::string_view>);
static_assert(r::random_access_range<std::deque<int>> && !r::contiguous_range<std::deque<int>>);
static_assert(r::bidirectional_range<std::list<int>> && !r::random_access_range<std::list<int>>);
static_assert(r::forward_range<std::forward_list<int>> && !r::bidirectional_range<std::forward_list<int>>);

// Every level includes the ones below it: a vector is also an input_range.
static_assert(r::input_range<V> && r::forward_range<V> && r::bidirectional_range<V>);

// Views are ranges too, at the level their source and adaptor allow.
static_assert(r::random_access_range<decltype(std::views::iota(0, 10))>);
static_assert(r::bidirectional_range<decltype(V{} | std::views::filter([](int) { return true; }))>);
static_assert(!r::random_access_range<decltype(V{} | std::views::filter([](int) { return true; }))>);   // filter loses it

static_assert(r::sized_range<V> && !r::sized_range<std::forward_list<int>>);
static_assert(r::view<std::span<int>> && r::view<std::string_view> && !r::view<V>);   // views: cheap to copy, non-owning
static_assert(r::borrowed_range<std::span<int>> && !r::borrowed_range<V>);            // iterators may outlive the object
```

<!--
Notes: input < forward < bidirectional < random_access < contiguous, each including the ones
below. The two orthogonal ones: sized_range (size() in O(1)), and view (cheap to copy, does
not own). The filter line is the one to point at: adapting a vector through filter drops it from
random-access to bidirectional, because you cannot jump ahead without testing the predicate.
Demo file: demos/s04/range_concepts.cpp (all static_asserts)
-->

---

<!-- _class: twocol -->

## Constrained algorithms <span class="badge cpp20">C++20</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s04/constrained_algorithms.cpp#before -->
```cpp
// C++11: iterator pairs and a comparator lambda, four times in the starter
void sort_by_value_cpp11(std::vector<Record>& v) {
    std::sort(v.begin(), v.end(),
              [](const Record& a, const Record& b) { return a.value < b.value; });
}
```

</div>
<div>

#### After (C++20)

<!-- snippet: demos/s04/constrained_algorithms.cpp#after -->
```cpp
// C++20: the range, and a projection instead of a comparator
void sort_by_value(std::vector<Record>& v) {
    std::ranges::sort(v, {}, &Record::value);          // {} = std::ranges::less
}
```

</div>
</div>

<!-- snippet: demos/s04/constrained_algorithms.cpp#why -->
```cpp
// Why std::ranges::sort exists beside std::sort:
//   - takes a range (or an iterator + sentinel), so begin/end mismatches cannot happen
//   - is constrained: sort(list) fails with "does not satisfy random_access_range", not 200 lines
//   - takes a projection, so "compare by member" needs no lambda
//   - returns more: sort returns the end iterator, copy returns both ends, minmax returns the elements
//   - is a function object, not a function: no ADL surprises, cannot be found by unqualified lookup
```

<!--
Notes: std::ranges::sort is a different function from std::sort, in a different namespace, on
purpose: it is constrained (sort(list) gives a one-line concept error), takes a range or an
iterator+sentinel, takes a projection, and is a function object so ADL cannot hijack it.
Exercise task 1. Demo file: demos/s04/constrained_algorithms.cpp
-->

---

<!-- _class: feature -->

## Projections <span class="badge cpp20">C++20</span>

<p class="problem">"Do this by member X" without writing a lambda. The feature people miss most.</p>

<!-- snippet: demos/s04/projections.cpp#projections -->
```cpp
// Applied to each element BEFORE the algorithm compares it. Any invocable works.
void examples(std::vector<Record>& v, std::string_view name, long long ts) {
    auto [lo, hi] = std::ranges::minmax(v, {}, &Record::value);          // min and max RECORDS, by value
    auto it = std::ranges::lower_bound(v, ts, {}, &Record::ts);          // search timestamps
    auto cfg = std::ranges::find(kSensors, name, &SensorConfig::name);   // search names, get the config
    auto n = std::ranges::count(v, "rpm", &Record::sensor);              // count by sensor
    std::ranges::sort(v, std::ranges::greater{}, &Record::value);       // descending by value
    auto longest = std::ranges::max(v, {}, [](const Record& r) { return r.sensor.size(); });
    std::println("{} {} {} {} {} {}", lo.value, hi.value, it - v.begin(), cfg->max, n, longest.sensor);
}
```

<!--
Notes: The projection is applied to each element before the algorithm looks at it. A
pointer-to-member is the common case; any invocable works. Every comparator lambda in the
starter disappears in the solution. Note minmax returns the RECORDS, not the values. Demo file:
demos/s04/projections.cpp
-->

---

<!-- _class: feature -->

## What the algorithms return <span class="badge cpp20">C++20</span>

<p class="problem">More than the classic algorithms did, and one thing that refuses to be returned.</p>

<!-- snippet: demos/s04/algorithm_results.cpp#results -->
```cpp
auto [mn, mx] = std::ranges::minmax(v);                 // the elements, not iterators
auto end = std::ranges::sort(v);                        // the end iterator (rarely needed)
auto [in, o] = std::ranges::copy(v, out.begin());       // in_out_result: where both stopped
auto [last, fn] = std::ranges::for_each(v, [s = 0](int x) mutable { s += x; return s; });
                                                        // in_fun_result: the end iterator AND the functor, with its state

auto d = std::ranges::find(make(), 2);                  // on a temporary: std::ranges::dangling
// *d;                                                  // error: dangling has no operator*
// The algorithm refuses to hand back an iterator into an object that no longer exists.
```

<!--
Notes: The result structs (in_out_result, in_fun_result, min_max_result) are designed for
structured bindings. std::ranges::dangling is the important one: an algorithm called on a
temporary non-borrowed range returns a type you cannot dereference, because the iterator would
point into a dead object. Demo file: demos/s04/algorithm_results.cpp
-->

---

<!-- _class: feature -->

## Sentinels <span class="badge cpp20">C++20</span>

<p class="problem">The end of a range no longer has to be an iterator. It can be a condition.</p>

<!-- snippet: demos/s04/sentinels.cpp#sentinel -->
```cpp
// An end that is a predicate, not an iterator: "stop when *it == 0".
struct NulSentinel {
    friend bool operator==(const char* p, NulSentinel) { return *p == '\0'; }
};

int main() {
    const char* s = "hello, world";
    auto comma = std::ranges::find(s, NulSentinel{}, ',');           // no strlen first
    std::println("{}", comma - s);

    // Infinite ranges are fine when the end is a sentinel that never matches:
    auto squares = std::views::iota(1) | std::views::transform([](int x) { return x * x; });
    for (int sq : squares | std::views::take_while([](int x) { return x < 50; })) std::print("{} ", sq);
    std::println("");

    // std::unreachable_sentinel: "there is an end, trust me" (the search must succeed)
    auto it = std::ranges::find(s, std::unreachable_sentinel, 'w');   // no bounds check in the loop
    std::println("{}", it - s);
}
```

<!--
Notes: Three uses: a NUL-terminated string without a strlen pass first; infinite ranges (iota
with no bound) cut by take_while; unreachable_sentinel to tell the algorithm "this will be
found, skip the bounds check". Sentinels are why views::take on an infinite iota costs nothing.
Demo file: demos/s04/sentinels.cpp
-->

---

## Customization point objects <span class="badge cpp20">C++20</span>

`std::ranges::begin`, `end`, `size`, `data`, `swap`, and every `std::ranges::` algorithm are **objects**, not functions.

- They cannot be found by argument-dependent lookup, so `sort(v)` with `using namespace std::ranges` cannot be hijacked by a `sort` in `v`'s namespace
- They dispatch correctly to arrays, member `begin()`, or ADL free `begin()`, in that order, reject anything else with a concept error, and refuse an rvalue of a non-borrowed range
- `std::ranges::begin(r)` is the spelling to use in generic code; `r.begin()` only works for members
- Consequence: you can pass `std::ranges::sort` to another function without a wrapper lambda

<!--
Notes: Optional slide if behind. The practical takeaway is the last bullet: algorithm objects
are first-class values.
-->

---

<!-- _class: feature -->

## Range-for and ranges <span class="badge cpp20">C++20</span>

<p class="problem">Every view is a range, so it works in range-for. The temporary-lifetime rule from Session 1 still applies.</p>

```cpp
for (const auto& r : records | std::views::filter(is_rpm)) { ... }     // fine: records is an lvalue

for (const auto& r : load().records() | std::views::take(3)) { ... }   // BUG before C++23: records() is a reference into load(), which dies before the body
for (auto loaded = load(); const auto& r : loaded.records() | std::views::take(3)) { ... }   // C++20 fix

for (auto&& x : some_view) { ... }        // auto&& because some views yield prvalues (transform) and some references
```

<!--
Notes: The init-statement form from Session 1 is the C++20 fix; C++23's P2718 makes the bug
line legal (GCC 15, Clang 19). The bug needs a REFERENCE into the temporary: an accessor
returning const vector&. Piping a data member of a temporary (`load().records | take(3)`) is
already safe, because the rvalue vector is moved into an owning_view (P2415, a C++20 DR) that
lives for the loop. The auto&& advice: transform yields values, filter yields references;
auto&& binds either without a copy. Hand-typed.
-->

---

## `std::ranges::` or `std::`?

| Use `std::ranges::` when | Use `std::` when |
|---|---|
| writing new code (concepts, projections, sentinels) | you need an execution policy (`par`): no `ranges::` version until C++26 |
| calling with a whole container | the algorithm has no `ranges::` counterpart (`std::accumulate`, `std::reduce`, `inclusive_scan`, the `<numeric>` family) |
| you want a concept error instead of a template-instantiation error | interoperating with old code that hands you iterator pairs (`std::ranges::subrange(first, last)` bridges the gap) |

Both live in the same headers. Mixing them in one file is fine.

<!--
Notes: The accumulate gap is filled by ranges::fold_left in C++23; the parallel gap by C++26.
-->

---

<!-- _class: feature -->

## Borrowed ranges and `dangling` <span class="badge cpp20">C++20</span>

<p class="problem">An iterator into a temporary is a bug. The library refuses to hand you one.</p>

<!-- snippet: demos/s04/dangling.cpp#borrowed -->
```cpp
std::vector<int> v{1, 2, 3};
auto a = std::ranges::find(v, 2);                 // v is an lvalue: iterator is safe
auto b = std::ranges::find(std::span{v}, 2);      // span is a borrowed_range: safe even as a temporary
auto c = std::ranges::find(make(), 2);            // temporary vector: std::ranges::dangling, cannot deref
static_assert(std::same_as<decltype(c), std::ranges::dangling>);
```

A **borrowed range** is one whose iterators can outlive it: lvalues, `span`, `string_view`, `subrange`, `iota`. Everything else, as a temporary, gives `dangling`.

<!--
Notes: Exercise task 8. The mechanism: enable_borrowed_range is specialized for view types that
do not own; the algorithms check it and substitute std::ranges::dangling for the return type.
Demo file: demos/s04/dangling.cpp
-->

---

<!-- _class: dense -->

## The concepts, with one type each

| Concept | Means | Example |
|---|---|---|
| `range` | `begin()` and `end()` | anything below |
| `input_range` | single pass forward | `std::istream_iterator` range, `views::istream` |
| `forward_range` | multi-pass forward | `std::forward_list`, `views::split` result |
| `bidirectional_range` | also backward | `std::list`, `std::map`, `views::filter` over a vector |
| `random_access_range` | `it + n` in O(1) | `std::deque`, `views::iota`, `views::transform` over a vector |
| `contiguous_range` | `data()` is a pointer | `std::vector`, `std::array`, `std::span`, `std::string_view` |
| `sized_range` | `size()` in O(1) | all of the above except `forward_list`, `filter`, and the `istream` and `split` views |
| `view` | cheap copy, non-owning | `span`, `string_view`, every `views::` result |
| `borrowed_range` | iterators may outlive it | `span`, `string_view`, `subrange`, any lvalue |
| `common_range` | `begin()` and `end()` same type | containers; `views::common` makes any range one |

<!--
Notes: Handout slide. The "views::filter over a vector is bidirectional, not random access" row
is the one to say aloud.
-->

---

<!-- _class: dense -->

## Constrained algorithms with new behavior

| Algorithm | What is new in `std::ranges::` |
|---|---|
| `for_each` | returns `{end, functor}`: the functor's state comes back |
| `copy`, `transform`, `move` | return `in_out_result` with both end positions |
| `minmax`, `min`, `max` | return the elements (with projection applied for comparison, not to the result) |
| `find`, `find_if`, `lower_bound` | return `dangling` on a temporary non-borrowed range |
| `sort`, `unique`, `remove` | return the end (or the subrange to erase, for `unique`/`remove`) |
| `contains`, `starts_with`, `ends_with` [23] | new; `contains` takes a projection |
| `find_last` [23] | returns a subrange from the last match to the end |
| `fold_left`, `fold_right`, `fold_left_first` [23] | the missing `accumulate`, range-taking |
| `iota`, `shift_left` [23] | ranges versions of the C++20 iterator-pair algorithms |

<!--
Notes: Handout slide. Availability: the [23] rows are uneven on libc++ 18; the support matrix
slide has the details.
-->

---

<!-- _class: takeaway -->

## Foundation takeaway

**Constrained algorithms and projections are strictly better than iterator pairs.** Adopt them in every new line: `std::ranges::sort(v, {}, &T::member)`.

The library refuses to hand you an iterator into a temporary (`dangling`). Read that as a feature.

**Monday morning:** find one `std::sort` with a comparator lambda and replace it with a projection.

<!--
Notes: 0:35. Views next.
-->

---

<!-- SEGMENT: Views (0:35) -->

<!-- _class: feature -->

## Views are lazy <span class="badge cpp20">C++20</span>

<p class="problem">A view is a recipe over another range. Nothing runs until you iterate, and iteration stops as early as it can.</p>

<!-- snippet: demos/s04/lazy.cpp#lazy -->
```cpp
int calls = 0;
std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
auto pipeline = v
    | std::views::filter([&](int x) { ++calls; return x % 2 == 0; })
    | std::views::transform([](int x) { return x * x; })
    | std::views::take(2);
std::println("after construction: {} predicate calls", calls);   // 0: nothing has run

for (int x : pipeline) std::print("{} ", x);                      // 4 16
std::println("\nafter iteration: {} predicate calls", calls);     // 6, not 10: it stopped early.
// Why 6 and not 4: after yielding 4, ++ advances filter to the NEXT match (tests 5, 6)
// before take's counter says stop. Lazy means "no more than needed", not "exactly".
```

<!--
Notes: Run it: zero predicate calls after construction, six after iterating two results (not
four: take advances the underlying filter to the next match before checking its own count).
"Lazy" means no more work than needed, with a small lookahead. Demo file: demos/s04/lazy.cpp
-->

---

<!-- _class: twocol -->

## The pipe syntax <span class="badge cpp20">C++20</span>

<div class="cols">
<div>

#### Before (C++17)

```cpp
std::vector<Record> matching;
std::copy_if(records.begin(), records.end(),
    std::back_inserter(matching),
    [sensor](const Record& r) {
        return r.sensor == sensor; });
std::stable_sort(matching.begin(),
    matching.end(),
    [](const auto& a, const auto& b) {
        return a.value > b.value; });
if (matching.size() > n) matching.resize(n);
```

</div>
<div>

#### After (C++23)

```cpp
auto matching = records
    | std::views::filter([sensor](const Record& r) {
          return r.sensor == sensor; })
    | std::ranges::to<std::vector>();
std::ranges::stable_sort(matching,
    std::ranges::greater{}, &Record::value);
if (matching.size() > n) matching.resize(n);
```

</div>
</div>

<!--
Notes: top_n_by_value from the exercise (task 2). `range | adaptor` is `adaptor(range)`;
adaptors compose left to right. The pipeline reads as the steps. Hand-typed from the solution.
-->

---

<!-- _class: feature dense -->

## The core adaptors <span class="badge cpp20">C++20</span>

<p class="problem">Twelve adaptors cover most pipelines.</p>

<!-- snippet: demos/s04/adaptors_tour.cpp#tour -->
```cpp
show("filter",     n | v::filter([](int x) { return x % 2 == 0; }));     // 2 4 6
show("transform",  n | v::transform([](int x) { return x * 10; }));      // 10 20 ... 60
show("take",       n | v::take(3));                                      // 1 2 3
show("drop",       n | v::drop(4));                                      // 5 6
show("take_while", n | v::take_while([](int x) { return x < 4; }));      // 1 2 3
show("drop_while", n | v::drop_while([](int x) { return x < 4; }));      // 4 5 6
show("reverse",    n | v::reverse);                                      // 6 5 ... 1
show("iota",       v::iota(10, 15));                                     // 10 11 12 13 14
show("keys",       m | v::keys);                                         // a b
show("values",     m | v::values);                                       // 1 2
show("split",      std::string_view{"a,b,c"} | v::split(',') | v::transform([](auto&& p) { return std::string_view(p); }));
show("join",       std::vector<std::string>{"ab", "cd"} | v::join);      // a b c d (chars)
show("chain",      n | v::reverse | v::filter([](int x) { return x > 2; }) | v::take(2));   // 6 5
```

<!--
Notes: Run the demo; each line prints its result. filter and transform are 80% of real use;
take/drop and their _while forms most of the rest. Demo file: demos/s04/adaptors_tour.cpp
-->

---

<!-- _class: feature -->

## `views::split` and the edge case the tests caught <span class="badge cpp20">C++20</span>

<p class="problem">The exercise's split() as a pipeline, and the one input on which it disagreed with the loop.</p>

```cpp
std::vector<std::string_view> split(std::string_view text, char delimiter) {
    if (text.empty()) return {std::string_view{}};       // views::split("") yields ZERO pieces; the loop yielded one
    return text | std::views::split(delimiter)
                | std::views::transform([](auto&& part) { return std::string_view(part); })
                | std::ranges::to<std::vector>();
}
```

`views::split` yields subranges of the original characters; `transform` turns each into a `string_view` over the same buffer. No copies.

<!--
Notes: Exercise task 3. The test `split("", ',').size() == 1` failed when the loop became a
pipeline. Views have their own edge semantics; the tests define the contract. Either answer
(zero pieces or one) is defensible; silently changing it is not. Hand-typed from the solution.
-->

---

## View semantics <span class="badge cpp20">C++20</span>

A view is a **`string_view` over a computation**: non-owning, O(1) to copy and move, holds a reference (or a copy of another view) plus the adaptor's state.

- `v | views::filter(p)` holds a `ref_view<vector>` (a pointer) and `p`. Copying it copies a pointer and a functor.
- `std::move(v) | views::filter(p)` holds an `owning_view<vector>`: the vector is inside the view (C++20 DR, all compilers)
- A view **must not be stored** past the range it refers to (same rule as `string_view` and `span`)
- `std::ranges::view` requires: movable, default-constructible-or-not, O(1) destroy; `views::all` is what the pipe applies to a container to make it one

<!--
Notes: The owning_view case is the exception to "non-owning": piping an rvalue container moves
it into the view. That is what makes `load() | views::take(3)` safe both to store and to
iterate in a range-for, even in C++20: the container now lives inside the view. The dangling
case is a REFERENCE into a temporary, `load().records() | views::take(3)` (the range-for slide).
-->

---

<!-- _class: feature -->

## The `const` view trap <span class="badge cpp20">C++20</span>

<p class="problem">A const filter_view is not a range. The exercise hit this in serialize().</p>

<!-- snippet: demos/s04/const_view_trap.cpp#trap -->
```cpp
// filter_view::begin() finds the first match and CACHES it (so begin() is amortized O(1)),
// so begin() is not const, so a const filter_view is not a range.
template <typename R>
std::size_t count_const(const R& r) {          // const&: fails for filter_view
    std::size_t n = 0;
    for ([[maybe_unused]] auto&& x : r) ++n;
    return n;
}

template <typename R>
std::size_t count(R&& r) {                     // R&&: how the standard algorithms take ranges
    std::size_t n = 0;
    for ([[maybe_unused]] auto&& x : r) ++n;
    return n;
}
// const-iterable:     transform, take, drop (of random-access), iota, all
// NOT const-iterable: filter, drop_while, split, chunk_by, join (sometimes)
```

<!--
Notes: Exercise task 4. filter's begin() finds the first match and caches it so that begin() is
amortized O(1) as the range concept requires; caching mutates, so begin() is non-const. Take
ranges by R&& (as every std::ranges algorithm does) and iterate with auto&&. Demo file:
demos/s04/const_view_trap.cpp
-->

---

<!-- _class: feature -->

## Dangling with views <span class="badge cpp20">C++20</span>

<p class="problem">Views did not change the lifetime rule from Session 2. They made it easier to hit.</p>

<!-- snippet: demos/s04/dangling.cpp#views_dangle -->
```cpp
auto fields = split(std::string("a,b"), ',');     // COMPILES. The views point into a temporary that
// std::println("{}", fields[0]);                 // died at the ';'. Undefined behavior.
std::string line = "a,b";
auto ok = split(line, ',');                       // line outlives the views: fine
auto owned = split(std::string("a,b"), ',')       // if it must outlive the source, materialize:
    | std::views::transform([](std::string_view s) { return std::string(s); })
    | std::ranges::to<std::vector>();
```

<!--
Notes: `split(std::string("a,b"), ',')` compiles because string_view converts from a temporary
string and the resulting views are just pointers. Clang's -Wdangling does not catch this one
(the temporary is consumed inside split). The fix when the result must outlive the source:
materialize with `to`. Demo file: demos/s04/dangling.cpp
-->

---

<!-- _class: feature -->

## The cost model <span class="badge cpp20">C++20</span>

<p class="problem">Most pipelines inline to the loop you would have written. Some do not.</p>

<!-- snippet: demos/s04/cost_model.cpp#cost -->
```cpp
// Inlines to one loop with a branch: as fast as the hand-written loop.
int sum_even_squares(const std::vector<int>& v) {
    int s = 0;
    for (int x : v | std::views::filter([](int x) { return x % 2 == 0; })
                   | std::views::transform([](int x) { return x * x; })) s += x;
    return s;
}

// Evaluates the filter predicate TWICE per element: reverse_iterator::operator* decrements a
// COPY of the filter iterator (running the predicate back to the previous match), then
// operator++ decrements the real one and runs it again. Measured: 12 calls for 6 elements.
int reverse_of_filter(const std::vector<int>& v) {
    int s = 0;
    for (int x : v | std::views::filter([](int x) { return x % 2 == 0; }) | std::views::reverse) s += x;
    return s;
}

// join and split are forward-only: no size(), no random access, and a per-element branch.
// Fine for parsing a line; measure before putting them in a hot loop.
```

<!--
Notes: filter | transform | take: one loop with a branch, identical codegen at -O2 on both
compilers (show on Compiler Explorer). reverse of filter: the predicate runs twice per element,
because reverse_iterator dereferences through a decremented copy and then decrements again.
join and split are forward-only and branchy. Rule: views in code you own and measure; a plain
loop is still right in a hot inner loop when the adaptor stack repeats work like this.
Demo file: demos/s04/cost_model.cpp
-->

---

<!-- _class: feature -->

## Composable pieces <span class="badge cpp20">C++20</span> <span class="badge cpp23">C++23</span>

<p class="problem">An adaptor closure is a value. Name it, store it, compose it. C++23 lets you write your own.</p>

<!-- snippet: demos/s04/adaptor_closure.cpp#closure -->
```cpp
// A range adaptor closure is a value: store it, name it, reuse it, compose it.
inline constexpr auto evens = std::views::filter([](int x) { return x % 2 == 0; });
inline constexpr auto squared = std::views::transform([](int x) { return x * x; });
inline constexpr auto top2_even_squares = evens | squared | std::views::take(2);   // composed, no range yet
```

<!-- snippet: demos/s04/adaptor_closure.cpp#own -->
```cpp
// C++23: make your own function pipeable by deriving from range_adaptor_closure
struct sum_fn : std::ranges::range_adaptor_closure<sum_fn> {
    template <std::ranges::input_range R>
    int operator()(R&& r) const { int s = 0; for (auto&& x : r) s += x; return s; }
};
inline constexpr sum_fn sum;                    // v | evens | sum
```

<!--
Notes: `inline constexpr auto evens = views::filter(...)` is a reusable, testable pipeline
fragment with no range attached. range_adaptor_closure (C++23, P2387) is how you make your own
function pipeable; libc++ 18 lacks it. Demo file: demos/s04/adaptor_closure.cpp
-->

---

<!-- _class: twocol -->

## `std::ranges::to` <span class="badge cpp23">C++23</span>

<div class="cols">
<div>

#### Before (C++20)

<!-- snippet: demos/s04/ranges_to.cpp#before -->
```cpp
// C++20: a view is not a container. Materializing one was awkward:
std::vector<int> a(evens.begin(), evens.end());          // only if begin/end have the same type ("common")
auto common = evens | std::views::common;                // ...otherwise adapt first
std::vector<int> b(common.begin(), common.end());
```

</div>
<div>

#### After (C++23)

<!-- snippet: demos/s04/ranges_to.cpp#after -->
```cpp
// C++23: one adaptor, any container, nested if needed
auto c = v | std::views::filter([](int x) { return x > 1; }) | std::ranges::to<std::vector>();
auto s = v | std::ranges::to<std::set>();                                            // dedupe + sort
auto m = v | std::views::transform([](int x) { return std::pair{x, x * x}; })
           | std::ranges::to<std::map>();                                          // pairs -> map
auto words = std::vector<std::vector<char>>{{'a', 'b'}, {'c'}}
           | std::ranges::to<std::vector<std::string>>();                          // element-wise conversion
```

</div>
</div>

<!--
Notes: The single most-missed piece of C++20 ranges: there was no clean way to turn a view into
a container. `to<std::vector>()` deduces the element type; `to<std::set>` dedupes; a range of
pairs becomes a map; nested conversion works. Demo file: demos/s04/ranges_to.cpp
-->

---

<!-- _class: feature -->

## `zip`, `enumerate`, and indexing <span class="badge cpp23">C++23</span>

<p class="problem">The report's rank column: pair each record with its index, without a counter variable.</p>

<!-- snippet: demos/s04/zip_enumerate.cpp#zip -->
```cpp
    // zip: iterate two (or more) ranges in lockstep, as tuples
    for (const auto& [name, value] : std::views::zip(names, values)) std::print("{}={} ", name, value);
    std::println("");

    // an index alongside: zip with iota (works everywhere), or enumerate (libc++ 20+)
    for (const auto& [i, name] : std::views::zip(std::views::iota(1uz), names)) std::print("#{} {} ", i, name);
    std::println("");
#ifdef __cpp_lib_ranges_enumerate                   // enumerate: from 0 (libc++ 20+)
    for (const auto& [i, name] : names | std::views::enumerate) std::print("[{}] {} ", i, name);
    std::println("");
#endif
```

<!--
Notes: Exercise task 5. views::enumerate is the intended spelling and libc++ 18 lacks it; zip
with iota(1uz) is portable and starts at 1, which the report wants anyway. zip_transform is
libstdc++-only until libc++ 20. Demo file: demos/s04/zip_enumerate.cpp
-->

---

<!-- _class: takeaway -->

## Views takeaway

Views for transformations that are **consumed once, lazily**. `to` when the result must **outlive the source**. Take ranges by **`R&&`**. Never **store** a view past its source.

The two traps the exercise's tests caught: `views::split("")` and the `const filter_view`. Views have their own semantics; **the tests define the contract**.

**Monday morning:** turn one `copy_if` + `back_inserter` into `filter | to<vector>`.

<!--
Notes: 1:00. C++23 additions.
-->

---

<!-- SEGMENT: C++23 (1:00) -->

## What C++20 was missing

| Need | C++20 | C++23 |
|---|---|---|
| view to container | `vector(v.begin(), v.end())`, only if common | `ranges::to` |
| iterate two ranges together | hand-written index loop | `views::zip`, `zip_transform` |
| index alongside elements | counter variable | `views::enumerate` |
| group consecutive equal keys | hand-written run detection | `views::chunk_by` |
| fixed windows, sliding windows, every Nth | index arithmetic | `chunk`, `slide`, `stride`, `adjacent` |
| sum a range | `std::accumulate` on iterators | `ranges::fold_left` |
| "does it contain" | `find() != end()` | `ranges::contains` |
| join with a separator | a loop | `views::join_with` |

P2214, "A Plan for C++23 Ranges", delivered all of these. It is why C++23 is the first standard in which views are practical for daily code.

<!--
Notes: The honest history: C++20 ranges shipped incomplete because the committee ran out of
time, and the plan for 23 was written before 20 was even published.
-->

---

<!-- _class: twocol -->

## `views::chunk_by`: the group-by <span class="badge cpp23">C++23</span>

<div class="cols">
<div>

#### Before (C++11)

<!-- snippet: demos/s04/chunk_by.cpp#before -->
```cpp
// C++11: group by sensor with a map of vectors, then iterate the map
#include <map>
void top_per_sensor_cpp11(std::vector<Record> v) {
    std::map<std::string, std::vector<Record>> groups;
    for (const auto& r : v) groups[r.sensor].push_back(r);
    for (auto& g : groups) {                      // no structured bindings yet
        std::vector<Record>& rs = g.second;
        std::sort(rs.begin(), rs.end(), [](const Record& a, const Record& b) { return a.value > b.value; });
        std::println("{}: {}", g.first, rs.front().value);   // println for output only; the rest is C++11
    }
}
```

</div>
<div>

#### After (C++23)

<!-- snippet: demos/s04/chunk_by.cpp#after -->
```cpp
// C++23: sort by key, then chunk_by splits into one subrange per run of equal keys. No map.
void top_per_sensor(std::vector<Record> v) {
    std::ranges::sort(v, {}, &Record::sensor);
    for (auto group : v | std::views::chunk_by([](const Record& a, const Record& b) { return a.sensor == b.sensor; })) {
        auto best = std::ranges::max(group, {}, &Record::value);
        std::println("{}: {}", group.front().sensor, best.value);
    }
}
```

</div>
</div>

<!--
Notes: Exercise task 5: the report's per-sensor section. Sort by key, chunk_by splits into a
subrange per run of equal keys; no map, no allocation per group. Each group is itself a range,
so the per-group pipeline (sort, take, zip) applies directly. Demo file: demos/s04/chunk_by.cpp
-->

---

<!-- _class: feature -->

## Windows: `chunk`, `slide`, `stride`, `adjacent` <span class="badge cpp23">C++23</span>

<p class="problem">Fixed-size blocks, sliding windows, every Nth element, and neighbors, over a signal.</p>

<!-- snippet: demos/s04/windows.cpp#windows -->
```cpp
for (auto c : signal | v::chunk(3)) std::print("[{} .. {}] ", c.front(), c.back());   // [1..4] [8..32] [64..64]
std::println("");

// a moving average: slide(3) yields overlapping windows of 3
for (auto w : signal | v::slide(3)) {
    double sum = 0; for (double x : w) sum += x;
    std::print("{:.1f} ", sum / 3);                                                   // 2.3 4.7 9.3 ...
}
std::println("");

for (double x : signal | v::stride(2)) std::print("{} ", x);                          // 1 4 16 64
std::println("");

for (auto [a, b] : signal | v::adjacent<2>) std::print("{} ", b - a);                 // deltas: 1 2 4 8 16 32
std::println("");
for (auto [a, b] : signal | v::pairwise) std::print("{} ", b / a);                    // ratios: pairwise = adjacent<2>
std::println("");
```

Availability: libstdc++ 13+; libc++ 20+ (none of these on libc++ 18).

<!--
Notes: The telemetry use: slide(3) | transform(mean) is a moving average in one line; adjacent<2>
gives deltas; stride(2) decimates. Demo file: demos/s04/windows.cpp (gated on feature-test macros; prints a notice on libc++)
-->

---

<!-- _class: feature -->

## `cartesian_product`, `repeat`, `join_with`, `as_rvalue` <span class="badge cpp23">C++23</span>

<p class="problem">The rest of the C++23 view family.</p>

<!-- snippet: demos/s04/zip_family.cpp#family -->
```cpp
for (auto [s, c] : v::cartesian_product(sensors, channels)) std::print("{}/{} ", s, c);   // every pair
std::println("");

for (int x : v::repeat(7, 3)) std::print("{} ", x);                                      // 7 7 7
std::println("");

for (char ch : sensors | v::join_with(std::string_view{", "})) std::print("{}", ch);    // rpm, temp
std::println("");                                                                        // (pattern: a range or a single element, e.g. ',')

std::vector<std::string> src{"a", "b"};
auto moved = src | v::as_rvalue | std::ranges::to<std::vector>();   // moves the strings out of src
std::println("{} {}", moved.size(), moved[0]);                      // src's strings are now moved-from: valid but unspecified
```

<!--
Notes: cartesian_product for "every combination" (test matrices, sensor x channel);
join_with for string joining (the pattern is a range such as a string_view, or a single
element such as ','; a const char* is neither); as_rvalue to move elements out of a range into
a container. The moved-from strings in src are valid but unspecified, so print moved, not src.
Demo file: demos/s04/zip_family.cpp
-->

---

<!-- _class: feature -->

## `ranges::fold_left` and friends <span class="badge cpp23">C++23</span>

<p class="problem">C++20 ranges had no accumulate. C++23 has four folds.</p>

<!-- snippet: demos/s04/fold.cpp#fold -->
```cpp
// C++20 ranges had no accumulate. C++23: fold_left, with a range and any binary op.
double total = std::ranges::fold_left(v | std::views::transform(&Record::value), 0.0, std::plus{});

// fold_left_first: no initial value, so the result is optional (empty range -> nullopt)
auto maxv = std::ranges::fold_left_first(v | std::views::transform(&Record::value),
                                         [](double a, double b) { return a > b ? a : b; });

// fold_right: associates from the right (matters for non-commutative ops)
auto names = std::ranges::fold_right(v | std::views::transform(&Record::sensor), std::string{},
                                     [](const std::string& s, std::string acc) { return acc + s + ";"; });

// vs std::accumulate(v.begin(), v.end(), 0): iterator pair, and the init's type decides the
// arithmetic, so 0 (an int) truncates doubles at every step. A classic bug.
```

<!--
Notes: fold_left(range, init, op). fold_left_first has no init, so it returns optional (empty
range gives nullopt) and cannot fall into the std::accumulate(..., 0) integer-truncation trap.
libc++ (Apple Clang 17 included) has fold_left only; the demo is gated on __cpp_lib_ranges_fold. Demo file: demos/s04/fold.cpp
-->

---

<!-- _class: feature -->

## `contains`, `starts_with`, `find_last`, `iota` <span class="badge cpp23">C++23</span>

<p class="problem">Small algorithms that each remove an idiom.</p>

<!-- snippet: demos/s04/cpp23_algorithms.cpp#algos -->
```cpp
    bool has = std::ranges::contains(v, 3);                                         // no more find() != end()
    bool dup = std::ranges::contains(table, "temp", &SensorConfig::name);            // with a projection: the exercise's validate()
#ifdef __cpp_lib_ranges_starts_ends_with                                          // GCC 15 / libc++ 17
    bool pre = std::ranges::starts_with(v, std::array{1, 2});
    bool suf = std::ranges::ends_with(v, std::array{2, 5});
#else
    bool pre = false, suf = false;
#endif
    std::vector<int> seq(5);
#ifdef __cpp_lib_ranges_find_last                                                 // GCC 13 / libc++ 19
    auto last2 = std::ranges::find_last(v, 2);                                       // a subrange from the last match to the end
#else
    auto last2 = std::ranges::subrange(v.begin() + 3, v.end());
#endif
#ifdef __cpp_lib_ranges_iota                                                      // GCC 13; not yet in libc++ (Apple Clang 17 included)
    std::ranges::iota(seq, 10);                                                      // 10 11 12 13 14 (ranges version)
#else
    std::iota(seq.begin(), seq.end(), 10);
#endif
    // std::ranges::shift_left / shift_right: GCC 15 / libc++ 20 (std::shift_left is C++20 and everywhere)
```

<!--
Notes: contains with a projection is the exercise's validate() duplicate check (task 6), and it
is constexpr. starts_with/ends_with: GCC 15, libc++ 17. find_last and ranges::iota: GCC 13,
libc++ 19. Demo file: demos/s04/cpp23_algorithms.cpp (gated)
-->

---

## Formatting and generating ranges <span class="badge cpp23">C++23</span>

```cpp
std::println("{}", v | std::views::take(3));          // [1, 2, 3]       (libc++ 17+, GCC 15)
std::println("{::.1f}", readings | std::views::transform(&Record::value));   // element spec

std::generator<Record> records(std::istream& in) {    // Session 5: a coroutine that IS an input_range
    for (std::string line; std::getline(in, line);)
        if (auto r = parse_record(line)) co_yield *r;
}
for (const Record& r : records(file) | std::views::filter(is_rpm)) { ... }
```

<!--
Notes: Range formatting from Session 2 applies to views. std::generator (C++23) is the bridge
to Session 5: a coroutine whose result is a lazy input_range, so load_stream could become a
generator and feed pipelines directly. Hand-typed preview.
-->

---

<!-- SEGMENT: Parallel (1:20) -->

<!-- _class: feature -->

## Execution policies <span class="badge cpp17">C++17</span>

<p class="problem">The same algorithms, with a first argument that permits parallel or vectorized execution.</p>

<!-- snippet: demos/s04/parallel.cpp#parallel -->
```cpp
namespace ex = std::execution;
std::mt19937 rng{42};
std::shuffle(v.begin(), v.end(), rng);
auto t0 = std::chrono::steady_clock::now();
std::sort(ex::seq, v.begin(), v.end());                          // sequential: same as plain sort
auto t1 = std::chrono::steady_clock::now();
std::shuffle(v.begin(), v.end(), rng);                           // same input again, for a fair comparison
auto t2 = std::chrono::steady_clock::now();                      // a fresh timestamp: the reshuffle is not timed
std::sort(ex::par, v.begin(), v.end());                          // may use threads; elements must be independent
auto t3 = std::chrono::steady_clock::now();
double s = std::reduce(ex::par_unseq, v.begin(), v.end());       // may also vectorize: no locks, no allocation in the body
auto t4 = std::chrono::steady_clock::now();
// Not available for std::ranges:: algorithms until C++26. Iterator pairs only.
```

<!--
Notes: seq: as if sequential. par: may split across threads. par_unseq: may also interleave
within a thread (vectorize), so the body may not lock or allocate. unseq (C++20): vectorize
only. Iterator pairs only: no std::ranges:: version until C++26. Demo file:
demos/s04/parallel.cpp
-->

---

## What actually runs in parallel

| Library | Backend | Notes |
|---|---|---|
| libstdc++ (GCC) | Intel TBB | needs `<tbb/...>` headers at compile time and `-ltbb`; **without TBB, `par` compiles and runs sequentially** |
| libc++ (Clang) | own backend, experimental | `-fexperimental-library` on 17/18; `__cpp_lib_parallel_algorithm` undefined otherwise |
| MSVC STL | Windows thread pool | works out of the box |

The repo's `Dockerfile` installs TBB. CMake links it when `find_package(TBB)` succeeds; otherwise the demo prints honest sequential timings.

<!--
Notes: The trap: a team enables par everywhere, sees no speedup, and concludes parallel
algorithms are useless, when the binary was never linked against TBB. Check
__cpp_lib_parallel_algorithm and ldd.
-->

---

## Your responsibilities with `par`

- **No data races between elements.** The body may not touch shared state without synchronization; with `par_unseq`, not even with a mutex (no locks, no allocation, no I/O)
- **Exceptions terminate.** An exception escaping the body under any parallel policy calls `std::terminate`
- **Order is unspecified.** `for_each(par, ...)` visits in any order; `reduce` sums in any order (floating point results can differ run to run)
- **Worth it when:** N is large (tens of thousands or more) or the per-element work is expensive. Thread startup costs microseconds; a `par` sort of 1,000 ints is slower than `seq`.
- **Measure.** The demo prints timings; on a two-core box `par` sort is barely faster than `seq`.

<!--
Notes: Take a position: par is for embarrassingly parallel bulk work. Everything else belongs
in Session 5's concurrency material.
-->

---

<!-- _class: takeaway -->

## Parallel takeaway

Execution policies are the cheapest parallelism in C++: **one argument**, if the body is independent per element.

They are also the easiest to misuse: **no TBB means no parallelism** on GCC, and **`par_unseq` forbids locks**.

**Monday morning:** check `__cpp_lib_parallel_algorithm` and `ldd` before trusting a `par` benchmark.

<!--
Notes: 1:30. Ten minutes of algorithm additions, then the exercise.
-->

---

<!-- SEGMENT: Algorithms (1:30) -->

<!-- _class: feature -->

## C++17 numerics <span class="badge cpp17">C++17</span>

<p class="problem">reduce, scans, sampling, and clamp: the numeric algorithms that came with the execution policies.</p>

<!-- snippet: demos/s04/numeric17.cpp#numeric -->
```cpp
double sum = std::reduce(v.begin(), v.end());                        // like accumulate, but order-agnostic:
                                                                     // parallelizable, and init defaults to T{}
double dot = std::transform_reduce(v.begin(), v.end(), v.begin(), 0.0);   // sum of products, one pass
std::vector<double> running(v.size());
std::inclusive_scan(v.begin(), v.end(), running.begin());            // prefix sums: 1.5 4.0 7.0
std::exclusive_scan(v.begin(), v.end(), running.begin(), 0.0);       // 0 1.5 4.0

std::vector<int> pool(100), sample;
std::iota(pool.begin(), pool.end(), 0);
std::sample(pool.begin(), pool.end(), std::back_inserter(sample), 5, std::mt19937{42});   // 5 without replacement
int c = std::clamp(150, 0, 100);
```

<!--
Notes: reduce vs accumulate: reduce may reorder (so it can parallelize) and defaults the init
to T{} (so it does not truncate doubles to int). transform_reduce is the dot product / weighted
sum. Scans are prefix sums. Demo file: demos/s04/numeric17.cpp
-->

---

## C++17 searchers and `for_each_n` <span class="badge cpp17">C++17</span>

```cpp
std::string haystack = ..., needle = "temp_core";
auto it = std::search(haystack.begin(), haystack.end(),
                      std::boyer_moore_searcher(needle.begin(), needle.end()));   // preprocessed needle, reused across searches

std::for_each_n(v.begin(), 3, [](auto& x) { x *= 2; });   // the first n, without computing an end iterator
```

Searchers: `default_searcher`, `boyer_moore_searcher`, `boyer_moore_horspool_searcher`. Build once, search many buffers.

<!--
Notes: Thirty seconds. The searcher matters when the same pattern is searched in many buffers
(log scanning, protocol framing). Hand-typed.
-->

---

## C++20 algorithm additions <span class="badge cpp20">C++20</span>

- `std::shift_left` / `std::shift_right`: move elements by n within a range, the missing complement to `rotate`
- `std::lexicographical_compare_three_way`: `<=>` for sequences
- `std::ranges::` versions of nearly every `<algorithm>` function (not `<numeric>`; `shift_left`/`shift_right` wait for C++23, `lexicographical_compare_three_way` has none)
- `std::midpoint`, `std::lerp` (Session 2), `std::ssize`
- `std::erase` / `std::erase_if` free functions for every container (Session 2): the end of erase-remove
- `unseq` execution policy

<!--
Notes: Fast. shift_left is the one people did not know they needed (ring-buffer style
"drop the oldest n").
-->

---

<!-- _class: dense -->

## C++23 algorithms and what comes next

| C++23 | Availability |
|---|---|
| `ranges::contains`, `contains_subrange` | GCC 13+, libc++ 17+ |
| `ranges::starts_with`, `ends_with` | GCC 15, libc++ 17+ |
| `ranges::find_last`, `find_last_if` | GCC 13+, libc++ 19+ |
| `ranges::fold_left`, `fold_right`, `fold_left_first`, `fold_left_with_iter` | GCC 13+; libc++ 18 `fold_left` only |
| `ranges::iota`, `ranges::shift_left`/`shift_right` | GCC 13 / 15; libc++ 19 / 20 |
| `ranges::to` | GCC 14, libc++ 17+ |
| **C++26 preview:** `std::ranges::` parallel algorithms (`ranges::sort(par, ...)`), `views::concat`, `views::cache_latest`, `ranges::generate_random` | |

<!--
Notes: The C++26 row is the answer to "when do ranges and par meet": C++26, with the
parallel ranges algorithms taking a policy as their first argument.
-->

---

<!-- _class: dense -->

## Support matrix for this session

| Feature | GCC 14 / libstdc++ | Clang 18 / libc++ | In the exercise |
|---|---|---|---|
| Constrained algorithms, projections, core views, `ranges::to`, `zip`, `chunk_by`, `split`, `fold_left`, `contains` | OK | OK | yes |
| `views::enumerate` | OK | missing (libc++ 20) | avoided: `zip(iota(1uz), ...)` |
| `chunk`, `slide`, `stride`, `adjacent`, `cartesian_product`, `join_with`, `zip_transform` | OK | missing (also on Apple Clang 17 / Xcode 26; demos print a notice) | no |
| `fold_left_first`, `fold_right`; `find_last`, `ranges::iota`; `range_adaptor_closure` | OK | missing (libc++ 19/20) | no |
| `ranges::starts_with`/`ends_with`; range formatting | missing (GCC 15) | OK | no |
| Parallel algorithms | OK with TBB | needs `-fexperimental-library` | no |

<!--
Notes: Every row was found building demos/s04. libc++ 18 is the laggard on C++23 views; GCC 14
on range formatting and starts_with. The exercise uses only the rows marked yes.
-->

---

<!-- SEGMENT: Exercise (1:40) -->

## Guidance for production code

- **Constrained algorithms and projections: now, everywhere.** No downside.
- **Views in code you own, consumed once.** `filter | transform | take` in a function body: yes. A view stored in a member: no.
- **`ranges::to` at boundaries.** Return containers from functions, not views, unless the function's whole point is a view.
- **Take ranges by `R&&`** in generic code; iterate with `auto&&`.
- **Measure `join`, `split`, and `reverse`-of-`filter`** before putting them in hot paths.
- **Test edge cases** when converting loops to pipelines: empty input, single element, all-equal keys.
- **clang-tidy `modernize-use-ranges`** (Clang 19+) converts iterator-pair calls automatically.

<!--
Notes: The slide to screenshot. Every bullet corresponds to something in today's exercise.
-->

---

<!-- _class: demo dense -->

## The same task, three ways

For each sensor, print the top 2 readings by value, highest first.

<!-- snippet: demos/s04/same_task_three_ways.cpp#cpp23 -->
```cpp
void cpp23(std::vector<Record> records) {
    std::ranges::sort(records, {}, &Record::sensor);
    for (auto group : records | std::views::chunk_by([](const Record& a, const Record& b) { return a.sensor == b.sensor; })) {
        std::ranges::sort(group, std::ranges::greater{}, &Record::value);
        for (auto [i, r] : std::views::zip(std::views::iota(1), group | std::views::take(2)))
            std::println("{} #{} {}", r.sensor, i, r.value);
    }
}
```

Compiler Explorer: [`demos/s04/same_task_three_ways.cpp`](https://godbolt.org/z/b6YddecKn), three functions, same output. Compare the generated code for `cpp20` and `cpp23`: the `chunk_by` version is not slower.

<!--
Notes: Show all three functions in the demo file, then the assembly. The C++11 version has a map
of vectors (an allocation per group); the C++20 version does run detection by hand; the C++23
version is the chunk_by pipeline. Same output, and the C++23 version generates comparable code
to C++20. Demo file: demos/s04/same_task_three_ways.cpp
-->

---

## Exercise: ranges

Open `exercises/s04-ranges/README.md`

**In class (10 minutes; finish at home):**

1. Projections: `minmax`, `lower_bound`, `find` with `&Record::value`, `&Record::ts`, `&SensorConfig::name`. Every comparator lambda disappears.
2. `top_n_by_value` as `filter | to<vector>` plus a projected `stable_sort`
3. `split` as `views::split | transform | to`. Run the tests. **One fails.** Decide the contract.

```
cmake --build build && ctest --test-dir build -R s04 --output-on-failure
```

**At home:** tasks 4 to 8 (`input_range`, the `const` view, `chunk_by` in the report, `contains`, what not to convert, dangling). `solution/` is next session's starter.

<!--
Notes: Task 3's failing test is the moment of the session. Let them find it, then ask: which
contract do you want, and where should the decision live? (In the tests.)
-->

---

<!-- _class: takeaway -->

## Session 4 takeaway

Ranges are **the standard algorithms with the iterator pairs removed**, plus lazy views that compose. Projections end comparator lambdas; pipelines read top to bottom; `to` materializes.

The lifetime rules are the same as every other view. The tests define the contract when a loop becomes a pipeline.

**Next session:** concurrency (`jthread`, `stop_token`, `latch`, `barrier`), coroutines and `std::generator`, modules, and the adoption roadmap for your own codebase.

<!--
Notes: Point at handouts/cheat-sheet-ranges.md and the support
matrix.
-->
