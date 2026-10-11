# Session 4 speaking script: Ranges and Algorithms

## How to use this script

- **Bold lines** are the must-say sentences. If you say nothing else on a slide, say those.
- Plain paragraphs are the talk track, written to be read aloud as is. Paraphrase freely once you know them.
- `(pause)` and `(beat)` are deliberate stops. A pause is two full seconds of silence; a beat is one. Do not skip them; on Teams, silence is when people unmute.
- `>> DO:` lines are actions (terminal, editor, Compiler Explorer). `>> ASK:` lines are questions to the room, with the answer you expect and what to say next.
- `>> IF AHEAD:` is 60 to 120 seconds of real extra depth for when you are early. `>> IF BEHIND:` is the one sentence that replaces the slide's talk track.
- Slide numbers count the title slide as 1. The deck has 54 slides: 43 presented (four of them lab launches), a Further reading divider, and 10 appendix slides that are in the PDF and not presented.

## Pace plan for a fast speaker

Four blocks: about twenty minutes of slides, then a ten-minute lab, four times. The lab is the checkpoint; if the lecture runs long, the lab still starts on time and the slides you did not reach are covered in the lab debrief or skipped (they are all in the deck's Further reading appendix for the PDF). Block 4 is twelve minutes of slides, so its lab starts at 1:42, and the at-home exercise launch and the takeaway fill the last three minutes.

| Checkpoint | Target clock |
|---|---|
| 1. Title | 0:00 |
| 6. Range concepts (block 1, Foundation) | 0:07 |
| 11. Borrowed ranges and `dangling` | 0:17 |
| 13. Lab 4.1 launched | 0:20 |
| 14. Views are lazy (block 2, Views) | 0:30 |
| 17. The `const` view trap | 0:37 |
| 21. `std::ranges::to` | 0:46 |
| 24. Lab 4.2 launched | 0:50 |
| 25. What C++20 was missing (block 3, C++23 and parallel) | 1:00 |
| 30. Execution policies | 1:13 |
| 34. Lab 4.3 launched | 1:20 |
| 35. C++17 numerics (block 4, Algorithms by standard) | 1:30 |
| 39. Guidance for production code | 1:37 |
| 41. Lab 4.4 launched | 1:42 |
| 42. Exercise (at home), 43. Takeaway | 1:52 |
| Done | 1:55 |

If you hit a lab launch more than 2 minutes early, use the IF AHEAD material in the slide before it rather than starting the lab early; people need the clock to be predictable. If you are late, cut the slide before the takeaway, never the lab. Block 3 has no slack of its own: `chunk_by` is four minutes and the parallel slides are tight, so if block 2 ran over, trim the IF AHEADs, not the lab.

Slow down deliberately on these four:

- **Slide 11, Borrowed ranges and `dangling`.** It is the first time the library changes a return *type* based on value category. People nod and do not get it. Say it twice; lab 4.1's TODO 3 then makes them type it.
- **Slide 14, Views are lazy.** The "6, not 4" result is subtle and is the mental model for everything after it. If they do not get lookahead here, slide 19 makes no sense.
- **Slide 17, The `const` view trap.** Counterintuitive for C++ veterans who were taught "take by `const&`". It is lab 4.2's TODO 3, the at-home task 4, and it will be a code-review argument back at work.
- **Slide 18, Dangling with views.** It compiles, it runs, it is UB. This is the one that ships to the field. Give it room.

## Before class checklist

- Build once on the compiler you present with, labs included. GCC 14 is the better presenting toolchain today: it has every C++23 view in the deck (libc++ 18 prints "not available" for windows, the zip family, and the folds).
  - `cmake -S . -B build -G Ninja && cmake --build build`
  - `ctest --test-dir build -R "s04|lab_s04" --output-on-failure`
  - Binaries are `./build/demos/s04/demo_s04_<stem>`, for example `./build/demos/s04/demo_s04_lazy`.
- Open the four lab starters on Compiler Explorer in four tabs, from the links on slides 13, 24, 34 and 41, and do each lab yourself once: `lab1_constrained_algorithms`, `lab2_views`, `lab3_cpp23_ranges`, `lab4_three_ways`. Keep the four solution links ready to paste at the seven-minute mark of each lab. For lab 4.1, uncomment the `dangling` line once and read the error so you can quote it; for lab 4.2, the same with the two `const` lines. Lab 4.3's TODO 3 needs GCC 14 on Compiler Explorer (libc++ 18 has no `views::enumerate`); know what the Clang error looks like.
- Check TBB: `ldd build/demos/s04/demo_s04_parallel | grep tbb`. If nothing prints, the `par` numbers will be sequential. Know which you have before slide 30.
- Demo files to open on the right, in order: `range_concepts.cpp`, `constrained_algorithms.cpp`, `projections.cpp`, `algorithm_results.cpp`, `sentinels.cpp`, `dangling.cpp`, `lazy.cpp`, `adaptors_tour.cpp`, `const_view_trap.cpp`, `cost_model.cpp`, `adaptor_closure.cpp`, `ranges_to.cpp`, `zip_enumerate.cpp`, `chunk_by.cpp`, `windows.cpp`, `fold.cpp`, `cpp23_algorithms.cpp`, `parallel.cpp`, `numeric17.cpp`, `same_task_three_ways.cpp`. Pin them as tabs in that order so Ctrl+Tab walks forward. (The appendix slides' demo, `zip_family.cpp`, only if you expect to use the IF AHEAD material on slide 27.)
- Compiler Explorer tabs to preload besides the labs (x86-64 GCC 14, `-std=c++23 -O2`). The demo headers still say `<add short link>`, so paste the files in by hand:
  1. `cost_model.cpp`, with a second compiler pane on Clang 18 `-stdlib=libc++`.
  2. `const_view_trap.cpp` with `-DSHOW_ERRORS` already in the flags (or be ready to type it).
  3. `same_task_three_ways.cpp`, assembly pane open, "Filter: library functions" on.
- Exercise: confirm `exercises/s04-ranges/starter/` and `solution/` are present in your checkout (the staged copy has only the README). On the starter, do task 3 yourself once and confirm the `split("", ',')` test fails exactly as the README promises, and that `s04_report_identical` passes on the untouched starter. You launch it as homework in one minute at slide 42; have the README open in a background tab.
- `handouts/cheat-sheet-ranges.md` is a stub (one line). Either finish it or do not point at it on slide 43.

## The script

### 1. The Evolution of C++: Session 4, Ranges and Algorithms · target 0:00, ~1 min

Welcome back. Session 4. **Today is the biggest change to how we write loops since range-based for.** (beat)

Here's the shape of it. C++20 gave us the model: concepts for ranges, algorithms that take a whole container, projections, and lazy views you compose with a pipe. C++23 filled in the pieces that make it something you'd actually use every day.

One warning up front. If you're on Clang with libc++ 18, you're going to see several of today's C++23 views print "not available". That's not you, that's the library. The support matrix in block four lists every gap, and the labs and the exercise avoid all of them but one, which I'll flag when we get there. (pause)

Let's look at where the two hours go.

### 2. Agenda · target 0:01, ~1 min

Four blocks. **Twenty minutes of me, ten minutes of you, four times.** Block one is the C++20 foundation: range concepts, constrained algorithms, projections, and the library refusing to hand you a dangling iterator. Block two is views: lazy pipelines, the two traps, and `ranges::to`. Block three is what C++23 added to make views practical, and the parallel algorithms. Block four is the smaller algorithm additions from 17 through 23, and then one task written three ways, which is also its lab.

(beat)

Each lab is one file on Compiler Explorer with the TODOs that block just taught. The slide that launches it has the link; you need nothing installed. If you built the repo, the same files are under `labs/`.

The telemetry exercise is at-home work this time. Its first three tasks are today's labs applied to the real program, so you will have done them once already. **The exercise starter is the Session 3 solution, same telemetry program, same tests.** So everything you see on a slide today with the words "from the exercise" is code you've already touched.

(pause)

Quick look back at Session 3 first.

### 3. Session 3 recap · target 0:02, ~2 min

The Session 3 solution: a `constexpr` CRC table and `crc16`, a `consteval` validator that checks the sensor table at compile time, four constrained `serialize` templates behind a `Serializable` concept, and deducing `this` on `SensorStats::add`.

Three places people got stuck. First: `std::as_bytes` in a `constexpr` function. It's a `reinterpret_cast` underneath, and a `reinterpret_cast` is never a constant expression. No compiler flag fixes that. (beat)

Second: `transform(&SensorConfig::units)` on a temporary optional. Still. This is the second session in a row. The member pointer yields a reference into an rvalue, and the monadic `transform` won't build an optional of a reference. Use a lambda that returns by value.

Third, and this is the one that matters for today. `SerializableRange` without the `!StringLike` exclusion. A `std::string` matched both the string overload and the range overload, and the call was ambiguous.

>> ASK: "Why was a `std::string` a match for the range overload at all?" Expected: because a string has `begin()` and `end()`, so it is a range. Say: "Right. **A string is a range of characters, and that fact comes back today with `views::split`.**"

(pause)

Keep that in your head. Now, what's wrong with the code we've been writing for twenty years?

>> IF BEHIND: "Session 3's three stumbles: `as_bytes` is a `reinterpret_cast`, member-pointer `transform` on a temporary optional, and a string is a range. That last one matters today."

### 4. The iterator-pair problem · target 0:04, ~2 min

These four lines are in the Session 3 solution today, in `stats.cpp`. Look at them as a reviewer would.

Line 1, the `stable_sort`: `matching.begin()`, `matching.end()`, and a comparator lambda whose whole job is to say "compare by `value`, descending". Line 2, the `lower_bound`: same pair, and a lambda that says "compare by `ts`". Line 3, the `copy_if` into a `back_inserter`: a pair, an output iterator, and a lambda that says "the sensor matches". Line 4 isn't even an algorithm. It's a hand-written loop tracking a running min and max.

>> ASK: "Count the lambdas. How many, and what do they say?" Expected: three. Two say "compare by this member", one is a filter predicate. Say: "Two of the three exist only to name a member. That's the part ranges deletes outright."

(pause)

Three problems. **Every one of these takes two iterators that must match, and nothing checks that they do.** A classic review finding is `std::sort(a.begin(), b.end())` after a copy-paste. It compiles. It's undefined behavior. It often appears to work in a unit test with small inputs.

Second, the comparator lambdas. Each one is four lines of ceremony around one member name, and every one is a chance to write `<` where you meant `>`, or `<=`, which isn't a strict weak ordering and corrupts the sort.

Third, no composition. Filter, then sort, then truncate means a temporary vector between each step, because the algorithms can't hand their output to each other.

In a long-lived codebase, this is the code people copy. So it multiplies. (beat)

Here's the fix in one sentence.

>> IF AHEAD: The `<=` comparator is worth thirty seconds. `std::sort` requires a strict weak ordering; `a <= a` is true, which violates irreflexivity. libstdc++'s introsort can then walk off the end of the buffer in its unguarded insertion sort when many elements compare equal. Building with `-D_GLIBCXX_DEBUG` adds irreflexivity checks to the sorting algorithms and catches some of these at run time; it's worth turning on in a CI test configuration. Projections reduce the risk because the default comparator is `ranges::less`, which you didn't write.

### 5. Ranges in one sentence · target 0:06, ~1 min

**Ranges are the standard algorithms with the iterator pairs removed, plus lazy views that compose with a pipe.** (pause)

Two lines on the slide. The first line: `std::ranges::sort(records, {}, &Record::value)`. The container goes in whole. The empty braces are the comparator, defaulted to "less". And `&Record::value` is a projection: "sort these by their value". No lambda.

The second line: `records | filter(is_rpm) | take(3)`. That builds a view. A recipe. Nothing runs on that line. It runs when you iterate it, and it stops as soon as it has three.

So set expectations. The C++20 half, the algorithms and projections, is what you can use tomorrow morning in any code built as C++20. It's strictly better than what you have. The C++23 half, `to`, `zip`, `enumerate`, `chunk_by`, `fold_left`, is what makes views worth using in production, because before 23 you couldn't even turn a view back into a vector cleanly.

That's the arc of today's four blocks: foundation, views, the C++23 completion with parallel alongside it, then the smaller algorithms by standard. (beat)

Let's start with what a range actually is.

>> IF BEHIND: "Ranges: algorithms that take the container, projections instead of comparators, and lazy pipelines. C++20 is the model, C++23 makes it usable."

### 6. Range concepts · C++20 · target 0:07, ~2 min

**A range is anything with `begin()` and `end()`. The concepts tell you what kind.**

This whole slide is `static_assert`s. It's `demos/s04/range_concepts.cpp`, and if it compiles, every line is true.

There's a ladder. `input_range` at the bottom: you can walk it once. `forward_range`: you can walk it more than once. `bidirectional_range`: you can go backward. `random_access_range`: you can jump, `it + n`, in constant time. `contiguous_range`: the elements sit in memory one after another, so `data()` gives you a pointer.

Each rung includes the ones below it. Line 6 on the slide says it: a vector is also an input range, a forward range, a bidirectional range.

Now walk the first block. Line 3: vector, span, and `string_view` are contiguous. Line 4: `deque` is random access but not contiguous; it's chunks. Line 5: `list` is bidirectional, not random access. Line 6: `forward_list` only goes forward.

Now the line to look at. Line 11 and 12, the `filter` lines. **Put a vector through `views::filter` and it drops from contiguous all the way down to bidirectional.** (pause) Why? Because to jump ahead five matches, you have to test the predicate on every element in between. There's no O(1) `it + 5` anymore. The concepts carry that information through the pipeline, so an algorithm that needs random access will refuse a filtered range at compile time.

Then two concepts that sit off to the side of the ladder. `sized_range`: `size()` in O(1). `forward_list` isn't. And `view`: cheap to copy, doesn't own. Span and `string_view` are views; vector isn't. And `borrowed_range`, last line: iterators that can outlive the range object. Span yes, vector no. Hold that one; it comes back on slide 11, five slides from now.

>> DO: Run `./build/demos/s04/demo_s04_range_concepts`. Expected output: `all range concept checks passed at compile time`. Point out that the run is a formality: the proof happened in the compiler. Recovery: if it fails to build, read the failing `static_assert` aloud; it is the lesson.

Why does any of this matter to you? Because these concepts are what turn a 200-line template error into one line.

>> IF AHEAD: A fun check for the room: `std::views::iota(0, 10)` is random access, line 10, even though it doesn't exist in memory. Random access is about iterator arithmetic, not storage. And there's a seventh property not on this slide: `common_range`, where `begin()` and `end()` have the same type. Many views aren't common, because their end is a sentinel. That breaks old code that does `std::vector<int>(v.begin(), v.end())` on a view. Slide 21 is about exactly that, and appendix slide 48 is the handout table with every concept and one type each.

>> IF BEHIND: "Input, forward, bidirectional, random access, contiguous; filter drops you to bidirectional; and `view` and `borrowed_range` are the two that matter for lifetimes."

### 7. Constrained algorithms · C++20 · target 0:09, ~2 min

Left side is what we have: `std::sort`, two iterators, a comparator lambda that says "by value". Right side: `std::ranges::sort(v, {}, &Record::value)`. The braces mean "default comparator", which is `std::ranges::less`. **This is lab 4.1's TODO 1, the exercise's task 1 at home, and the change you can make in any C++20 file tomorrow.**

Why a whole new namespace instead of new overloads of `std::sort`? Look at the comment block at the bottom. Five reasons, and I'll pick the three that matter.

One. It takes a range, or an iterator and a sentinel. You can't pass `a.begin()` and `b.end()` by accident when you only pass `a`.

Two. It's constrained. Call `std::ranges::sort` on a `std::list` and GCC says "no match for call to `ranges::__sort_fn`" and then "constraints not satisfied", pointing at `random_access_range`. Call plain `std::sort` on a list and you get a page of errors from deep inside the implementation about `operator-` on list iterators. (beat)

Three. It returns more. Next slide.

And notice where the braces go. The projection is the third argument. **You must write the `{}` to get to the projection position.** A common first mistake is `std::ranges::sort(v, &Record::value)`: that passes the member pointer as the comparator, and you get "no match for call". When you see that error in review, it's almost always the missing braces. You will probably see it once in the lab.

>> IF AHEAD: Why couldn't the committee just add these to `std::sort`? Because existing code calls `std::sort(first, last)` and expects unconstrained templates, and ADL-found `sort` overloads in user namespaces already exist in the wild. Adding constrained overloads to `std::` would have changed overload resolution in existing code. A parallel namespace was the compatible choice. That's also why both live in `<algorithm>` and you can mix them in one file.

>> IF BEHIND: "`std::ranges::sort(v, {}, &Record::value)`: the range, a default comparator, a projection. Concept errors instead of template spew."

Projections are the feature I want you to leave with, so let's look at them properly.

### 8. Projections · C++20 · target 0:11, ~2 min

**A projection is applied to each element before the algorithm looks at it.** (pause) That's the whole definition. The algorithm compares projected values but hands you back the original elements.

Walk the function on the right with me, `projections.cpp`, line by line.

Line 3: `minmax(v, {}, &Record::value)`. Min and max by value. But look what comes back: `lo` and `hi` are **records**, whole `Record` objects, not doubles. That's why the print line says `lo.value`. People expect a double and get a struct.

Line 4: `lower_bound(v, ts, {}, &Record::ts)`. The old version needed a heterogeneous comparator lambda, `Record` on one side and a timestamp on the other. Now you search for a timestamp in a range of records, and the projection bridges the types. Same precondition as always: the range must be sorted by `ts`.

Line 5: `find(kSensors, name, &SensorConfig::name)`. Search the config table by name; get back an iterator to the whole config. That's `find_sensor` in the exercise, in one call.

Line 6: `count` by sensor. Line 7: sort descending, with `ranges::greater{}` in the comparator slot and the projection after it. Line 8: the projection doesn't have to be a member pointer. Any invocable works. Here it's a lambda returning the sensor name's length, so `max` finds the record with the longest name.

>> DO: Run `./build/demos/s04/demo_s04_projections`. Expected: `1 3 1 125 2 temperature`. Point at each: min value 1, max value 3, the `lower_bound` landed at index 1, the `temp` config's max is 125, two `rpm` records, and the longest sensor name is `temperature`. Recovery: if the build is missing, read the expected output off this script and keep moving.

**Every comparator lambda in the starter that just named a member disappears in the solution.** That's the exercise's task 1 at home, and lab 4.1 is the same change on a file small enough to finish in ten minutes.

>> ASK: "If `minmax` hands back records, what does it cost when `Record` holds a `std::string`?" Expected: it copies two records, strings and all. Say: "Right, `ranges::minmax` returns by value. If that matters, use `ranges::minmax_element`, which takes the same projection and gives you iterators."

>> IF AHEAD: Projections compose with the comparator: the comparator receives projected values. So `sort(v, std::ranges::greater{}, &Record::value)` compares doubles with `greater`. A code-review heuristic: if a lambda passed to an algorithm has the shape `[](const T& a, const T& b) { return a.m < b.m; }`, it's a projection waiting to happen. clang-tidy 19's `modernize-use-ranges` will even do the iterator-pair half of the rewrite for you. One edge: a projection to a member of a type with no `operator<`, like a struct, still needs a comparator for that type. The projection doesn't invent ordering.

Next: what these algorithms give back.

### 9. What the algorithms return · C++20 · target 0:13, ~2 min

The classic algorithms threw information away. `std::copy` gave you the output iterator but not where the input stopped. `std::for_each` gave you the functor but not the end. The `ranges::` versions return structs designed for structured bindings.

Line 1: `minmax` gives the elements. Line 2: `sort` returns the end iterator. Rarely useful, but it's there. Line 3: `copy` returns an `in_out_result`, where both the input and the output stopped. Line 4, the interesting one: `for_each` with a stateful lambda, a mutable `s` that accumulates. You get back the end iterator **and the functor, with its state.**

>> DO: Run `./build/demos/s04/demo_s04_algorithm_results`. Expected: `1 5 5 14`. Min 1, max 5, the copy wrote 5 elements, and `fn(0)` returns 14: the sum of 1, 1, 3, 4, 5 that the lambda accumulated inside `for_each`. Recovery: say the numbers from here.

Now line 6. `std::ranges::find(make(), 2)`. `make()` returns a temporary vector. When `find` returns, that vector is gone. So what does `find` return? Not an iterator. **It returns `std::ranges::dangling`, a type with no `operator*`.** (pause) Uncomment the `*d` and it doesn't compile. The algorithm refuses to hand you an iterator into an object that no longer exists.

That's a compile-time fix for a whole category of use-after-free. We'll spend a full slide on it two slides from now.

>> IF BEHIND: "The ranges algorithms return more: both ends from `copy`, the functor from `for_each`, and `dangling` instead of an iterator into a temporary."

### 10. Sentinels · C++20 · target 0:15, ~2 min

**The end of a range no longer has to be an iterator. It can be a condition.** (beat)

Old rule: begin and end are the same type, and you walk until they're equal. New rule: the end can be a different type, called a sentinel, and all it needs is an `==` against the iterator.

Look at the top of `sentinels.cpp`. `NulSentinel` is an empty struct with one friend `operator==` that takes a `const char*` and returns `*p == '\0'`. That's it. Line 8: `std::ranges::find(s, NulSentinel{}, ',')`. Find a comma in a C string without calling `strlen` first. One pass instead of two. For anyone who's parsed NUL-terminated buffers off a serial line or out of a legacy API, that's the pattern you write by hand today.

Second use. Line 12: `views::iota(1)` with no upper bound. An infinite range of integers. Transform to squares, then `take_while` less than 50. That's fine, because nothing is computed until you iterate, and the end is a predicate that eventually matches.

Third. Line 17: `std::unreachable_sentinel`. That's a sentinel that never compares equal. You're telling the algorithm "there is a match, trust me, skip the bounds check". The loop drops the end comparison. **If you're wrong, you read off the end of the buffer.** (pause) So it's a tool for a hot loop where you've put a guard value at the end yourself, not a default.

>> DO: Run `./build/demos/s04/demo_s04_sentinels`. Expected three lines: `5`, then `1 4 9 16 25 36 49`, then `7`. The comma is at index 5, the squares stop before 64, and the `w` is at index 7. Recovery: read them out.

>> ASK: "Would you allow `unreachable_sentinel` through code review?" Expected: mixed, probably "only with a comment proving the match exists". Say: "That's the right answer. It's the same review standard as any unchecked index: show me the invariant."

>> IF AHEAD: The `NulSentinel` works with only `operator==` defined because C++20 synthesizes the reversed `==` and the `!=` from it. That's the Session 1 spaceship material doing quiet work here. And there's a standard spelling for counted ranges too: `std::views::counted(p, n)` gives a span or a subrange of n elements from a pointer, which is the safe way to adapt a pointer-and-length C API.

>> IF BEHIND: "A sentinel is an end that's a condition: NUL-terminated strings without `strlen`, infinite `iota`, and `unreachable_sentinel` when you can prove the match."

>> IF AHEAD: Three short slides from this block are in the appendix, 45 to 47. Customization point objects: `std::ranges::begin` and every `ranges::` algorithm are objects, not functions, so ADL cannot hijack them and you can pass `std::ranges::sort` to another function as a value. Range-for and ranges: a pipeline over an accessor on a temporary, `load().records() | take(3)`, iterates a dead vector before C++23; name the temporary with the init-statement. And the decision table, `std::ranges::` or `std::`: new code uses `ranges::`; `std::` stays for execution policies, the `<numeric>` family, and legacy iterator pairs, which `std::ranges::subrange(first, last)` bridges. One sentence each; the PDF has the rest.

Bridge: now the slow slide.

### 11. Borrowed ranges and `dangling` · C++20 · target 0:17, ~2 min

Slow down here. This is the first time you'll see the library change a return type based on whether you passed it a temporary.

**An iterator into a temporary is a bug, and the ranges library refuses to hand you one.** (pause)

Three lines in `dangling.cpp`. Line 2: `find(v, 2)` where `v` is a named vector. You get a normal iterator. Safe, because `v` outlives the call.

Line 3: `find(std::span{v}, 2)`. The span is a temporary. It dies at the semicolon. But you still get a real iterator. Why? Because a span doesn't own anything. Its iterators point into `v`, not into the span. When the span dies, the iterators are still good. That property has a name: **borrowed range.** A range whose iterators can outlive the range object.

Line 4: `find(make(), 2)`. `make()` returns a temporary vector. The vector owns its elements. When it dies, the elements die. So any iterator into it would dangle. The algorithm sees that: rvalue, not borrowed. And instead of an iterator it returns `std::ranges::dangling`. The `static_assert` on line 5 proves it: the type of `c` is exactly `dangling`.

(pause)

Let me say that again, because it's the idea. **Same function, same arguments, different return type, depending on whether the thing you passed will survive the call.**

Which types are borrowed? Any lvalue, because you named it, so you're responsible for it. `span`, `string_view`, `subrange`, `iota`. Types opt in by specializing `enable_borrowed_range`. Everything else, passed as a temporary, gives you `dangling`.

>> DO: Point at line 5 of the snippet in `dangling.cpp` on the right. No run needed; the `static_assert` is the demo. If someone asks to see the error, add `*c;` and build: GCC reports no match for `operator*` on `std::ranges::dangling`.

>> ASK: "`std::ranges::find(std::string("abc"), 'b')`: iterator or dangling? And with `std::string_view("abc")`?" Expected: dangling for the string (owns its characters), a real iterator for the `string_view` (borrowed; it points at the literal, which lives forever). Say: "Exactly. And the exercise's task 8 is this, deliberately."

This is exercise task 8 at home, and lab 4.1's TODO 3 makes you type it in a minute. It's also the rule that views make easy to break, which is slide 18.

>> IF AHEAD: If you write your own non-owning range type, a buffer view over DMA memory, say, you opt in with `template<> inline constexpr bool std::ranges::enable_borrowed_range<MyView> = true;`. Only do it if your iterators truly don't point into the object. Getting it wrong silently turns off the `dangling` protection for your type. That's a good thing to put on a review checklist for anyone writing range types.

### 12. Foundation takeaway · target 0:19, ~1 min

**Constrained algorithms and projections are strictly better than iterator pairs. Adopt them in every new line.** `std::ranges::sort(v, {}, &T::member)`.

There's no performance cost; they compile to the same code. There's no new lifetime risk; if anything there's less, because of `dangling`. The only requirement is C++20.

And read `dangling` as a feature. When you hit it, the library just caught a use-after-free at compile time.

The two handout tables for this block, the concepts with one type each and the algorithms with new behavior, are appendix slides 48 and 49. Screenshot them from the PDF; the one row to know is that `views::filter` over a vector is bidirectional, not random access, and not sized.

**Monday morning: find one `std::sort` with a comparator lambda and replace it with a projection.** (pause)

>> ASK: "Anyone have a codebase where you can't turn on C++20 yet?" Expected: some hands, citing a certified or vendor toolchain. Say: "Then this is your argument for the upgrade request: a whole category of comparator and iterator-pair bugs goes away, with no runtime cost. Put it in the roadmap worksheet at the end of Session 5."

Time check: 0:20, and lab 4.1 starts now. It is this block in one file: a sort, a find, a max, each with a projection, and the `dangling` error read once on purpose.

>> IF BEHIND: Read the bold lines and the Monday morning line. Start the lab on time.

After the lab: the part that's actually new, views.

### 13. Lab 4.1: constrained algorithms and projections · target 0:20, ~10 min

**Ten minutes. Three TODOs, in order. Build and run after each one; the three output lines must not change. The link is on the slide; the same file is `labs/s04/lab1_constrained_algorithms.cpp` if you built the repo.**

>> DO: Put the lab slide up and paste the starter link into chat. Say nothing for the first minute while people get the file open. Then: "Every TODO is one call: the range, the braces, the member pointer. The replacement is written in the comment above it; the thinking is in why the braces are there."

What to watch for:

- TODO 1: people drop the `{}` and write `std::ranges::sort(rs, &Reading::value)`. That passes the member pointer as the comparator, and GCC says "no match for call to `(const std::ranges::__sort_fn) (std::vector<Reading>&, double Reading::*)`", then "constraints not satisfied". The projection is the third argument; you have to write the default comparator to reach it. That is slide 7's warning, in their own file.
- TODO 2: `std::ranges::find(rs, "temp")` with no projection fails the same way: "no match for call to `__find_fn`", because `ranges::equal_to` cannot compare a `Reading` with a `char` array. The projection bridges the two types: compare `r.sensor`, get back the `Reading`. `it->value` still works because `find` returns an iterator, as before.
- TODO 2, the shortcut: someone keeps the lambda and writes `std::ranges::find_if(rs, lambda)`. It compiles and passes. Say the point is the lambda disappearing, not the namespace changing.
- TODO 3, the swap: `std::ranges::max_element(ok, {}, &Reading::value)` on the named vector `ok` is the whole change; `best->value` still works.
- TODO 3, the error: the uncommented line passes `non_fault(rs)`, a temporary vector, and the error is "base operand of `->` has non-pointer type `std::ranges::dangling`". Nothing was dereferenced; the compiler stopped at the arrow. The C++11 `max_element` would have returned an iterator into freed memory and the program would have printed something. Have them read the type name out loud, then comment the line again.
- The `copy_if` inside `non_fault` is not a TODO. People will convert it anyway to `std::ranges::copy_if(v, std::back_inserter(out), ...)`, as the solution does; fine, and note that its lambda stays, because it is a real predicate and not a member name.

At seven minutes, paste the solution link. Walk the three diffs in two minutes: three calls, each with the range in the first slot and the member in the last, and the comment at the bottom of `main` that says why the `dangling` line can never compile.

>> ASK: "How many lambdas are left in the file?" Expect: "one". Then: "The `copy_if` predicate. It says something, `not a fault`, so it earns its keep. The three that disappeared only named a member. That is what projections are for, and it is Monday morning's one-line change."

At ten minutes, move on whether or not people are done. Nothing in block 2 depends on this lab.

### 14. Views are lazy · C++20 · target 0:30, ~3 min

Slow down. This is the mental model for the rest of the block.

**A view is a recipe over another range. Nothing runs until you iterate, and iteration stops as early as it can.** (pause)

`lazy.cpp` on the right. There's a counter, `calls`, and the filter predicate increments it every time it runs. Ten elements, one through ten. The pipeline is: filter evens, square them, take two.

Line 7 prints the counter right after building the pipeline. Before you see the output, what do you expect?

>> ASK: "After construction, how many predicate calls? And after iterating, how many?" Expected: zero after construction; most people say four, some say ten, after iteration. Say: "Let's see."

>> DO: Run `./build/demos/s04/demo_s04_lazy`. Expected output:
>> `after construction: 0 predicate calls`
>> `4 16`
>> `after iteration: 6 predicate calls`
>> Point at the 0 first, then the 6. Recovery: if it won't run, read the three lines from here; the explanation is the point.

Zero after construction. Nothing ran. Building a view is just storing a reference and a lambda.

After iterating: six. (pause) Not ten. It didn't walk the whole vector. But not four either. Here's why. To get the first result, filter tests 1, 2: two calls, yields 4. To get the second, it tests 3, 4: four calls, yields 16. Now `take` has its two. But the loop's increment runs first: `++` advances the filter to the *next* match, so it tests 5 and 6, before `take` checks its counter and says "stop". Six.

**Lazy means "no more than needed", plus a small lookahead. It doesn't mean "exactly once per element you see."** That matters the moment your predicate has a side effect, or is expensive, or reads hardware. So rule of thumb: keep predicates pure and cheap. If it logs, counts, or touches a register, a view will surprise you.

>> IF AHEAD: The same lookahead happens with `take_while`, and it's worse with `istream` views: one extra element gets consumed from the stream. If you're reading a framed protocol and stop with `take(n)` on an `istream_view`, the next frame's first token may already be gone. That's a real bug shape, and it's why "consumed once" is part of the guidance on slide 39.

### 15. The pipe syntax · C++20 · target 0:33, ~2 min

This is `top_n_by_value` from the exercise, at-home task 2, before and after. Lab 4.4's `top3_ranges` has the same three steps.

Left: declare `matching`, `copy_if` with a `back_inserter` and a lambda, `stable_sort` with a comparator lambda, `resize`. Ten lines, and you have to read all of them to see that it's three steps.

Right: `records | filter(same sensor) | to<std::vector>()`. Then `stable_sort(matching, greater{}, &Record::value)`. Then the same `resize`. **The pipeline reads top to bottom as the steps.**

The mechanics are simple: `range | adaptor` is just `adaptor(range)`. The pipe is function application written left to right.

Notice the filter lambda is still there. That's not a member comparison; it captures `sensor`. Projections removed the comparator lambdas, not the predicates. (beat)

And notice we didn't sort the view. We materialized into a vector first. Why?

>> ASK: "Why `to<vector>` before the sort instead of sorting the filtered view directly?" Expected: filter is only bidirectional, sort needs random access; and sorting through a view would reorder the original records. Say: "Both. The concepts would reject it anyway."

>> IF BEHIND: "`copy_if` plus `back_inserter` becomes `filter | to<vector>`. Pipe is just function application, left to right."

### 16. The core adaptors · C++20 · target 0:35, ~2 min

The tour. Twelve adaptors on screen plus a chain at the bottom. Two it doesn't show: `elements`, which is `keys` and `values` generalized to any tuple index, and `common`, which we'll meet on slide 21.

>> DO: Run `./build/demos/s04/demo_s04_adaptors_tour`. Each line prints its label and result. Point at three: `drop_while 4 5 6`, `join a b c d` (a vector of strings joined into one range of characters), and `chain 6 5`. Recovery: the expected output is in the comments on each line of the file; scroll and read them.

**`filter` and `transform` are eighty percent of real use. `take`, `drop`, and their `_while` forms are most of the rest.** (beat)

Look at the last line, `chain`. Reverse, then filter greater than two, then take two: `6 5`. Read it like a sentence. That's the payoff.

And look at `split` on line 11. Split the string on commas, then transform each piece into a `string_view`. That's exactly the shape of the exercise's `split` function, at-home task 3, and appendix slide 50 shows it with the edge case the tests caught.

>> IF AHEAD: `views::keys` and `values` work on anything whose elements are pair-like, not only maps. A vector of `std::pair` or `std::tuple` works too, and `views::elements<2>` pulls the third field. For a vector of plain structs, though, you want `transform(&Record::sensor)`: structs aren't tuple-like unless you specialize `tuple_size`.

>> IF BEHIND: "Filter and transform are most of it. Take, drop, and the `_while` forms are the rest."

>> IF AHEAD: Two slides from this block are in the appendix. Slide 50, `views::split` and the edge case: when the exercise's loop became `text | split(',') | transform(to string_view) | to<vector>`, one test failed, because `views::split` of an empty string yields zero pieces and the loop yielded one. Either contract is defensible; changing it silently is not, so the solution keeps the old one with an early return. The tests define the contract. Slide 51, view semantics: a view is a `string_view` over a computation, it holds a `ref_view` (a pointer) for an lvalue source and an `owning_view` for an rvalue one, and it must not be stored past its source. Both in the PDF.

Bridge: now the one that goes against twenty years of training.

### 17. The `const` view trap · C++20 · target 0:37, ~3 min

Slow down. This one goes against twenty years of training.

**A `const filter_view` is not a range. You can't iterate it.** (pause)

Here's the reason. The range concept requires `begin()` to be amortized O(1). But `filter_view::begin()` has to find the first element that matches, which might be a long scan. So it does the scan once and caches the result inside the view. Caching means writing to a member. Writing means `begin()` can't be `const`. So `const filter_view` has no `begin()`.

Look at the two function templates. `count_const` takes `const R&`, the way we've all been taught to take "a thing I only read". `count` takes `R&&`, a forwarding reference. Same body.

>> DO: In the Compiler Explorer tab with `const_view_trap.cpp`, make sure `-DSHOW_ERRORS` is in the flags (or locally: `g++ -std=c++23 -DSHOW_ERRORS -c demos/s04/const_view_trap.cpp`). Expected GCC error on line 14: `passing 'const std::ranges::filter_view<std::ranges::ref_view<std::vector<int> >, ...>' as 'this' argument discards qualifiers`. Point at `const` and `filter_view` in that message. Then remove the flag: it builds, and `./build/demos/s04/demo_s04_const_view_trap` prints `2` then `4`. Recovery: if Compiler Explorer is slow, read the error text from here.

So the exercise hit this in `serialize`. A `serialize(const R&)` overload, called with `v | filter(...)`, didn't compile. Changing it to `serialize(R&&)` fixed it. That's at-home task 4, and lab 4.2's TODO 3 has you hit the same error on purpose and read it.

**Take ranges by `R&&` in generic code, and iterate with `auto&&`.** That's exactly how every `std::ranges` algorithm takes its range.

Which views have the problem? Bottom of the slide. Const-iterable: `transform`, `take`, `drop` over random access, `iota`, `all`. Not const-iterable: `filter`, `drop_while`, `split`, `chunk_by`, and `join` in some cases. The pattern: anything whose `begin()` has to search.

>> ASK: "Does `R&&` mean the function can modify the caller's container?" Expected: it can, if the caller passes a non-const lvalue. Say: "Yes. `R&&` gives up the `const` promise. That's the trade. If you need the promise for a container, overload, or document it. For views, the promise was never available."

>> IF AHEAD: Two follow-ons. First, the cache is also a thread-safety issue: calling `begin()` on the same non-const `filter_view` from two threads is a data race, because both may write the cache. Share the source, not the view; give each thread its own view. Second, C++23 has `views::as_const`, which makes the *elements* const, not the view. It doesn't fix this. It's for "I want to iterate without being able to modify the source", the `std::as_const` idea for ranges.

### 18. Dangling with views · C++20 · target 0:40, ~2 min

Slow down. This one ships.

**Views didn't change the lifetime rules from Session 2. They made them easier to break.** (pause)

Line 1 in `dangling.cpp`: `auto fields = split(std::string("a,b"), ',');` The split function takes a `string_view`. A temporary `std::string` converts to a `string_view` happily. Split returns a vector of `string_view`s, each pointing into that temporary string's buffer. At the semicolon, the temporary dies.

It compiles. No warning. And `fields[0]` on the next line is undefined behavior. (pause)

**It compiles, it runs, and in a small test it might even print the right answer, because the freed memory hasn't been reused yet.** That's what makes it dangerous. It passes your unit test and fails in the field.

This is not the `dangling` type from slide 11. That protection only works when the algorithm can see the temporary. Here the temporary was consumed inside `split`; what comes out is a vector, which is a perfectly normal owned value. It just contains pointers to nowhere. Clang's `-Wdangling` doesn't catch this one either.

Line 3, `split(line, ',')`: `line` is a named string that outlives the fields. Fine.

Line 4, the fix when the result must outlive the source: materialize. `transform` each `string_view` into a `std::string`, then `to<std::vector>()`. Notice this is safe even though it starts from a temporary, because the whole chain runs before the semicolon. The temporary string is still alive while the copies are made.

>> ASK: "How would you catch line 1 in CI?" Expected: AddressSanitizer. Say: "Yes. ASan with a test that actually reads the field reports heap-use-after-free. That's the strongest argument for running your unit tests under ASan on every build. Static warnings won't find this."

At-home task 8 makes you write this bug on purpose.

>> IF AHEAD: An API design heuristic for review: a function that takes `std::string_view` and returns something containing views into it (a vector of `string_view`, a span, a view) should either be documented "result borrows from the argument" or take `const std::string&` and delete the rvalue overload: `split(std::string&&, char) = delete;`. That turns line 1 into a compile error. It's the same trick the standard uses for `std::ref` on temporaries.

### 19. The cost model · C++20 · target 0:42, ~2 min

The performance question. Is this slower than a loop?

**Most pipelines inline to the loop you would have written. Some don't.** (beat)

First function, `sum_even_squares`: filter evens, transform to squares, sum in a range-for. At `-O2`, on both compilers, that's one loop with a branch. Same as the hand-written loop.

>> DO: Switch to the Compiler Explorer tab with `cost_model.cpp`, GCC 14 `-O2 -std=c++23`. Point at the `sum_even_squares` body: one loop, a test of the low bit, a multiply, an add. No calls. If the Clang pane is open, point at it: same shape. Recovery: if the tab won't load, say "one loop, no calls, I'll leave the link in the chat" and move on.

Second function, `reverse_of_filter`. Filter, then reverse. Here's the cost: the filter predicate runs about twice per element. Put a counter in the predicate, like the lazy demo: six elements, a forward filter runs the predicate six times, reverse of filter runs it twelve.

Why? The slide says it's because reverse has to find the end. For a vector, that's not quite it: a filter over a vector already knows its end. The real reason is how `reverse` iterates. It uses `std::reverse_iterator`, and every time you dereference a reverse iterator, it copies the underlying iterator and steps it back one. Stepping a filter iterator back means scanning backward, running the predicate, to the previous match. Then `++` on the reverse iterator does that same scan again. **So every element pays for the backward search twice.** (pause) The slide's explanation is right for a different case: a filter over a range whose end is a sentinel, where reverse first has to walk the whole thing to find the end.

Third: `join` and `split` are forward-only. No `size()`, no random access, a branch per element. Fine for parsing a config line. Measure before you put them in a hot loop.

>> DO: Optionally run `./build/demos/s04/demo_s04_cost_model`. Expected: `56 12`. Same answers; the cost difference is invisible at this size, which is the point: you only see it with a profiler.

**The rule: views in code you own, and measure the hot paths.** A plain loop is still the right answer in an inner loop where the view would do extra work.

>> IF AHEAD: Debug builds are the other cost nobody mentions. At `-O0`, every adaptor layer is a real function call, and a four-stage pipeline can be several times slower than the loop. If your team ships or tests debug builds with timing requirements (hardware-in-the-loop rigs often run unoptimized for debuggability), that matters. `-Og` recovers most of it. And stepping through a pipeline in GDB is painful; `skip -rfu ^std::` in your `.gdbinit` helps.

### 20. Composable pieces · C++20, C++23 · target 0:44, ~2 min

**An adaptor closure is a value. Name it, store it, compose it.**

Top snippet, `adaptor_closure.cpp`. `inline constexpr auto evens = views::filter(...)`. No range attached. Same for `squared`. Then `top2_even_squares = evens | squared | take(2)`. You just composed three adaptors into one, still without any data. That's a reusable, testable pipeline fragment. You can unit-test it against a fixed vector and then use it everywhere.

Bottom snippet, C++23. Want your own function to work after a pipe? Derive from `std::ranges::range_adaptor_closure<sum_fn>`. That's CRTP: the base gets the derived type as a template argument. Now `v | evens | sum` works, and `sum` is your function object.

>> DO: Run `./build/demos/s04/demo_s04_adaptor_closure`. Expected: `4 16 | 12`. The first two are the stored pipeline; 12 is 2 plus 4 plus 6 through your own `sum`. On libc++ 18 the second half prints `(range_adaptor_closure not available)`; it arrives in libc++ 19.

Notice that storing a closure is safe, unlike storing a view, because a closure doesn't refer to any data. (beat) That's the line to remember: closures are fine to store, views aren't.

>> IF BEHIND: "Adaptor closures are values you can name and compose. C++23 lets you make your own pipeable."

### 21. `std::ranges::to` · C++23 · target 0:46, ~2 min

**The single most-missed piece of C++20 ranges: there was no clean way to turn a view back into a container.** (pause)

Left: C++20. `std::vector<int> a(evens.begin(), evens.end())`. That works only if the view is "common", meaning `begin` and `end` have the same type. A filter over a vector happens to be common, so line 2 compiles. Plenty of views aren't, `iota` without a bound or anything ending in `take_while`, and then you have to adapt with `views::common` first. Two extra lines, and a constructor that can't reserve.

Right: C++23. `| std::ranges::to<std::vector>()`. You don't even name the element type; it's deduced. Line 2: `to<std::set>` dedupes and sorts in one step. Line 3: a range of pairs goes straight into `to<std::map>`. Line 4: a vector of vectors of `char` becomes a vector of strings: `to` converts element-wise, recursively.

Don't forget the parentheses: `to<std::vector>()`. Without them it's a template, not an adaptor, and the error is not friendly.

This is the tool for the rule from slide 18: **when the result must outlive the source, materialize with `to`.** Lab 4.2's TODO 2 is exactly this: the same pipeline, with `to` on the end instead of a range-for.

>> DO: `./build/demos/s04/demo_s04_ranges_to`. Expected: `1 1 3 4 16 ab`. One even, one even, three greater than one, four distinct values in the set, `m[4]` is 16, and the first word is `ab`. Recovery: read it.

>> IF AHEAD: `to` reserves when it can. If the source is a `sized_range` and the container has `reserve`, it calls it. A filter isn't sized, so it can't; a `transform` over a vector is, so it does. And containers gained `from_range_t` constructors and `insert_range`/`append_range` members in C++23 (P1206), which `to` uses when present. Availability: `to` is GCC 14 and libc++ 17.

### 22. `zip`, `enumerate`, and indexing · C++23 · target 0:48, ~1 min

The report needs a rank column: one, two, three. Normally that's a counter variable next to the loop.

Line 2: `views::zip(names, values)` walks two ranges in lockstep and gives you tuples you can unpack with a structured binding. Line 6: zip `iota(1uz)` with `names`. `iota` with no end is infinite, but zip stops at the shortest range, so you get exactly as many indices as names, starting at 1. **That's the portable way to number things, and it's what the exercise uses.**

`views::enumerate` is the intended spelling. It starts at zero. libc++ 18 doesn't have it, hence the `#ifdef`; lab 4.3's TODO 3 uses it, so Clang users on Compiler Explorer will need the GCC 14 compiler for that one.

>> DO: `./build/demos/s04/demo_s04_zip_enumerate`. On GCC 14: four lines, ending with `rpm:4811 temp:42 pressure:101` from `zip_transform`. On libc++ 18 you'll see only the first two lines. Recovery: read the first two lines from the comments.

>> IF BEHIND: "`zip(iota(1uz), xs)` numbers anything, portably. `enumerate` is the C++23 spelling, missing on libc++ 18."

### 23. Views takeaway · target 0:49, ~1 min

**Views for transformations that are consumed once, lazily. `to` when the result must outlive the source. Take ranges by `R&&`. Never store a view past its source.**

And the two traps the exercise's tests caught: `views::split("")`, which is appendix slide 50, and the `const filter_view`, which you are about to meet in the lab. Views have their own semantics. The tests define the contract. (pause)

Monday morning: turn one `copy_if` plus `back_inserter` into `filter | to<vector>`.

Time check: 0:50, and lab 4.2 starts now. One lazy pipeline, the same pipeline materialized, and the `const` trap read once on purpose.

>> IF BEHIND: Read the bold lines. Start the lab on time.

After the lab, at 1:00: what C++23 added.

### 24. Lab 4.2: views · target 0:50, ~10 min

**Ten minutes, three TODOs. One lazy pipeline, the same pipeline materialized with `ranges::to`, and the `const` view trap. Build and run after each; the first two output lines must not change, and TODO 3 adds a third.**

>> DO: Lab slide up, starter link in chat. One quiet minute. Then: "TODO 1 and TODO 2 are the same three adaptors; the only difference is what consumes them. TODO 3 is a compile error you are meant to read, not avoid."

What to watch for:

- TODO 1, adaptor order: `take(3)` goes after the filter. Put it first and the first line becomes `3200.0 88.5`, because the fault reading was among the first three and the filter then removed it. The expected line is `3200.0 88.5 3350.0`. Order in a pipeline is evaluation order; that is the whole reason it reads top to bottom.
- TODO 1, the projection in `transform`: `std::views::transform(&Reading::value)` works, no lambda needed; a lambda returning `r.value` also works. Both yield `double` prvalues, so `for (double v : ...)` is the natural loop variable.
- TODO 2: `std::ranges::to<std::vector>` without the trailing `()` gives "no match for `operator|` (operand types are `transform_view<...>` and `<unresolved overloaded function type>`)". Point at the parentheses. `to<std::vector<double>>()` also works; the deduction is a convenience.
- TODO 2, the shape: people rebuild the pipeline inline, or name `not_fault` once and reuse it, as the solution does. Either is fine. The point to say: TODO 1 never built a container, TODO 2 built exactly one.
- TODO 3: the error is "passing `const std::ranges::filter_view<std::ranges::ref_view<std::vector<Reading>>, ...>` as `this` argument discards qualifiers". Read the first three words: `passing const filter_view`. `begin()` finds and caches the first match, so it is not `const`, so a `const filter_view` has no `begin()` and the range-for cannot start. The fix is one word: `auto hot`.
- TODO 3, the question they will ask: "but I wanted the elements const". They still are. The loop binds `const auto& r`, and the lambda takes `const Reading&`. `const` on the view was never `const` on the elements; a view is a handle, like a `string_view`.

At seven minutes: solution link. Two minutes: the one range-for with three adaptors, the same three adaptors ending in `to<std::vector>()`, and `auto hot` with the comment above it.

>> ASK: "`const std::vector<Reading>&` as a parameter is fine. Why is `const filter_view` not?" Expect: "the vector's `begin()` is `const`, the filter's is not". Then: "Right. The vector knows where it starts; the filter has to search for it, and it remembers the answer. That is why the standard algorithms take ranges by `R&&`, and why your generic code should too."

At ten minutes, move on. Block 3 starts at 1:00 on the clock; nothing in it depends on this lab.

### 25. What C++20 was missing · target 1:00, ~2 min

Here's the honest history. C++20 ranges shipped incomplete. The committee ran out of time. Look at the left column: no way to materialize a view, no zip, no enumerate, no group-by, no windows, no fold, no `contains`, no join with a separator.

**The plan for C++23 was written before C++20 was even published.** P2214, "A Plan for C++23 Ranges", listed every one of these gaps, prioritized them, and C++23 delivered them.

So if you looked at ranges in 2020 and decided they were a toy, you weren't wrong for 2020. You'd be wrong now. **C++23 is the first standard where views are practical for daily code.** (beat)

>> ASK: "Which of these rows would you have used last week?" Expected: `to`, `contains`, and `zip` usually come up; telemetry people say `slide` or `adjacent`. Say: "Good, the next two slides and the folds after them are those."

>> IF AHEAD: P2214 tiered the work: tier 1 for C++23, tier 2 and 3 deferred. Tier 2 items like `views::concat` and `views::cache_latest` landed in C++26, which is slide 37's preview. The paper is very readable and is the best single source for "why does ranges look like this".

>> IF BEHIND: "C++20 ranges shipped without `to`, `zip`, `enumerate`, `chunk_by`, or `fold`. C++23 filled them in."

### 26. `views::chunk_by`: the group-by · C++23 · target 1:02, ~4 min

This is the report's group-by-sensor, at-home task 5, and lab 4.3's TODO 1.

Left, the C++11 way (strictly it's C++17, with that structured binding, but it's the shape you know). Build a `std::map` from sensor name to a vector of records. Every record gets copied into a map node's vector. Then iterate the map, sort each vector by value descending with a comparator lambda, print the front.

That's one heap allocation for each map node, more for each vector's growth, and a copy of every record.

Right, C++23. Two steps. Sort the records by sensor, with a projection. Then `views::chunk_by` with a predicate "same sensor as the previous one". **`chunk_by` splits a range into a subrange for every run of consecutive elements where the predicate holds.** (pause) Because we sorted first, every run is one sensor.

Each `group` is itself a range, a subrange of the original vector. No copies, no allocation per group. So inside the loop, the per-group code is just the algorithms you already know: `ranges::max(group, {}, &Record::value)` gets the best reading.

The important word is *consecutive*. **`chunk_by` only groups adjacent elements.** If you forget the sort, you get one group per run, so rpm, temp, rpm, temp would print four groups, not two.

>> DO: `./build/demos/s04/demo_s04_chunk_by`. Expected four lines: `rpm: 4830`, `temp: 42.5`, then the same two again. The first pair is the map version, the second is `chunk_by`. Same answer. Then the experiment: comment out the `ranges::sort` line in `top_per_sensor`, rebuild (`cmake --build build --target demo_s04_chunk_by`), and run: the `chunk_by` half now prints four groups, `rpm: 4800`, `temp: 41`, `rpm: 4830`, `temp: 42.5`. Put the sort back. Recovery: if the rebuild drags, just say what it prints.

>> ASK: "The map version's output is sorted by sensor name. Is the `chunk_by` version guaranteed to match?" Expected: yes, because we sorted by sensor; `std::map` orders by `std::string`'s `<` and so does the projection. Say: "Exactly the kind of thing the report-diff test checks. If the map had a custom comparator, you'd need the same one in the sort."

>> IF AHEAD: The predicate gets two adjacent elements, so `chunk_by` can do more than equal keys. `chunk_by([](auto a, auto b) { return b.ts - a.ts < gap; })` splits a telemetry stream into bursts wherever the timestamp gap exceeds a threshold. That's a session detector in one line. Also, `chunk_by` caches its first chunk like `filter` does, so it's on the not-const-iterable list from slide 17.

### 27. Windows: `chunk`, `slide`, `stride`, `adjacent` · C++23 · target 1:06, ~2 min

Four views for signals. This is the telemetry slide.

`signal` is 1, 2, 4, 8, 16, 32, 64: powers of two, so the outputs are easy to check by eye.

`chunk(3)`: non-overlapping blocks of three. The last block gets what's left: `[64 .. 64]`.

`slide(3)`: overlapping windows of three. One, two, four; then two, four, eight; and so on. **`slide(3)` plus an average is a moving average in three lines.** No index arithmetic, no off-by-one at the end.

`stride(2)`: every second element. That's decimation.

`adjacent<2>`: pairs of neighbors, as tuples you can unpack. `b - a` gives deltas. `pairwise` is just the name for `adjacent<2>`; the last line uses it to print ratios.

>> DO: `./build/demos/s04/demo_s04_windows`. Expected on GCC 14:
>> `[1 .. 4] [8 .. 32] [64 .. 64]`
>> `2.3 4.7 9.3 18.7 37.3`
>> `1 4 16 64`
>> `1 2 4 8 16 32`
>> `2 2 2 2 2 2`
>> Point at the moving average line, then the deltas. On libc++ 18 it prints `window views not available in this standard library`; none of these are in libc++ until 20. Recovery: read the five lines from here.

>> ASK: "Seven samples, window of three. How many moving-average outputs?" Expected: five. Say: "N minus W plus one. And `slide` gets that right for you; the hand-written version is where the off-by-one lives."

These aren't exotic. Deltas, decimation, and moving averages are what every telemetry pipeline does first. (beat)

>> IF AHEAD: `chunk` and `slide` yield subranges, so the window is a view into the original data, no copy. `adjacent<N>` yields tuples of references, which is why it takes N as a compile-time template argument and `slide` takes a run-time size. Pick `adjacent` when N is small and fixed, because structured bindings work; pick `slide` when N comes from configuration.

>> IF BEHIND: "Chunk is blocks, slide is overlapping windows, stride decimates, adjacent gives neighbors. A moving average is `slide` plus a mean."

>> IF AHEAD: The rest of the C++23 view family is appendix slide 52: `cartesian_product` for every combination (a test matrix in one loop), `repeat`, `join_with` for string joining with a separator (pass a `string_view` or a `char`, not a bare literal), and `as_rvalue` to move elements out of a range into a container. All on GCC 14, none on libc++ 18; the demo is `zip_family.cpp`.

Bridge: the missing `accumulate`.

### 28. `ranges::fold_left` and friends · C++23 · target 1:08, ~3 min

**C++20 ranges had no `accumulate`. C++23 has folds.** (beat)

Line 2 of `fold.cpp`: `fold_left(v | transform(&Record::value), 0.0, std::plus{})`. A range, an initial value, a binary operation. Notice the `transform` with a member pointer: that's how you get a projection into a fold, because the folds don't take a projection argument.

Line 5: `fold_left_first`. No initial value; it uses the first element. So what does it return on an empty range? (pause) There is no first element. So it returns `std::optional`. Empty range, `nullopt`. The type system makes you handle it.

Line 9: `fold_right`. It associates from the right, which matters when the operation isn't commutative. Look at what it does to string concatenation: the records are rpm, rpm, temp, and the output is `temp;rpm;rpm;`. Reversed. Fold right processes the last element first.

>> DO: `./build/demos/s04/demo_s04_fold`. Expected on GCC 14: `8 4 temp;rpm;rpm;`. Total 8, max 4, and the reversed names. On libc++ 18 the last field prints `(fold_right not available)`. Recovery: read it.

Now the bug at the bottom. `std::accumulate(v.begin(), v.end(), 0)` on doubles. The type of the init decides the accumulator type, so `0`, an int, truncates at every step. 1.5 plus 2.5 becomes 1 plus 2. **That's a classic bug and it compiles silently, maybe with a conversion warning if you have `-Wconversion` on.**

Here's the part the slide doesn't say, and lab 4.3's TODO 2 lets you check it. `fold_left` fixes it even if you pass `0`. Its accumulator type is whatever the operation returns, and `std::plus{}` with an int and a double returns a double. So `fold_left` of 1.5 and 2.5 with init `0` returns 4.0, a double. `fold_left_first` sidesteps it entirely because there's no init.

>> ASK: "Can you pass a `std::execution::par` to `fold_left`?" Expected: no. Say: "Right. Folds are sequential by definition: left fold has an order. For parallel, it's `std::reduce` with a policy, which is slide 35."

>> IF AHEAD: The other two folds: `fold_left_with_iter` and `fold_left_first_with_iter` return both the result and the end iterator, for when the range is input-only and you need to know where you stopped. And note `std::plus{}` with empty angle brackets: that's the transparent version from C++14. `std::plus<int>{}` would bring the truncation right back.

### 29. `contains`, `starts_with`, `find_last`, `iota` · C++23 · target 1:11, ~2 min

Small algorithms that each delete an idiom.

Line 1: `ranges::contains(v, 3)`. **No more `find(...) != end()`.** It reads like what it means.

Line 2: `contains` with a projection: does the sensor table contain a config named "temp"? That's the exercise's `validate()` duplicate check, at-home task 6. And `contains` is `constexpr`, so the `static_assert` that validates the table at compile time still works. That connects straight back to Session 3.

Lines 4 and 5: `starts_with` and `ends_with` on any range, not just strings. Line 9: `find_last` returns a subrange from the last match to the end. Line 10: `ranges::iota` fills a range with 10, 11, 12, and so on.

>> DO: `./build/demos/s04/demo_s04_cpp23_algorithms`. Expected on GCC 14: `true true false false 3 14`. Before anyone asks: the two `false`s are not the answers. `starts_with` and `ends_with` aren't in libstdc++ until GCC 15, so that block is compiled out and the fallback sets them to `false`. On libc++ 18 those two print `true`. The `3` is the index of the last 2, and `14` is the last value `iota` wrote. Recovery: say exactly that.

**This is what the support matrix looks like in real life: the same file, two compilers, different holes.** (pause) Feature-test macros like `__cpp_lib_ranges_starts_ends_with` are how you write code that builds on both.

One correction on availability: the slide notes say `find_last` and `ranges::iota` arrived in GCC 14. They're actually in libstdc++ 13; the feature-test macros are defined there. On our GCC 14 baseline it makes no difference.

`ranges::shift_left` and `shift_right` are the last ones; they're only in the newest libraries. The C++20 `std::shift_left` with iterators is everywhere.

>> IF AHEAD: Prefer feature-test macros over version checks when you can. This file uses both: `__cpp_lib_ranges_starts_ends_with` for one block, and a `_LIBCPP_VERSION >= 190000` check for `find_last`, because the version check is clearer when the gap is a whole library release. Under `-Werror`, an unused variable in the fallback branch will break the build; that's why the demo sets `pre` and `suf` in both branches.

>> IF BEHIND: "`contains` replaces `find != end`, and works with projections and in `constexpr`. The rest are small; check the matrix."

>> IF AHEAD: Appendix slide 53 closes the C++23 ranges story: range formatting from Session 2 applies to views, `println("{}", v | take(3))` prints `[1, 2, 3]` (libc++ 17, GCC 15), and `std::generator<Record>` is a coroutine whose result is an input range, so the exercise's `load_stream` could feed a pipeline one record at a time. That is Session 5.

Bridge: that's the C++23 ranges half of the block. Now let's make things go faster with one extra argument.

### 30. Execution policies · C++17 · target 1:13, ~3 min

C++17 added a first argument to most of the algorithms: an execution policy. **Same algorithm, one extra argument, and the library is permitted to run it in parallel.** (beat) "Permitted", not "required".

Four policies. `seq`: as if sequential. `par`: may split the work across threads. `par_unseq`: may also interleave iterations within one thread, which means vectorize. And C++20 added `unseq`: vectorize only, one thread.

Look at `parallel.cpp`. Five million doubles, shuffled. Sort with `seq`, reshuffle, sort with `par`, then `reduce` with `par_unseq`.

>> DO: `./build/demos/s04/demo_s04_parallel`. Expected shape: `seq NNNms  par NNNms  par_unseq reduce NNms  sum 1.250e+13`. Before you read the numbers aloud, look at lines 27 to 29 of the file: `t1` is captured after the `seq` sort, then the vector is reshuffled, then `t1` is captured *again*. So the printed `seq` time includes the second shuffle, roughly 70 ms for five million doubles. Say so: "The seq number is inflated by about the cost of a shuffle; that's a bug in the demo, and it makes `par` look better than it is." If `ldd` showed no TBB, `par` will be about `seq` minus that shuffle. Recovery: if the binary prints `parallel algorithms not available in this standard library build`, you're on libc++ without `-fexperimental-library`; talk through the code.

And there's the honest lesson in that demo bug: **benchmark code gets less review than production code, and that's where wrong conclusions come from.** (pause)

The last comment on the slide: none of this works with `std::ranges::` algorithms. Execution policies are iterator-pair only until C++26.

>> ASK: "If you parallelize a `std::reduce` over doubles, will you get the same sum every run?" Expected: not necessarily. Say: "Right. Floating-point addition isn't associative, and `reduce` may add in any order. That's slide 32."

>> IF AHEAD: `std::reduce` with `par_unseq` over a sorted sequence of doubles is the fastest line in the demo because summation is trivially vectorizable. The interesting comparison would be `std::accumulate` over the same data with `-O2`: GCC won't vectorize the accumulate loop by default, because reordering floating-point adds changes results. `-ffast-math` would let it, which is exactly the permission `reduce` gives you explicitly, for one call, without a global flag. That's a much better thing to approve in review than `-ffast-math`.

### 31. What actually runs in parallel · target 1:16, ~1 min

Here's the table that saves you a week.

libstdc++, GCC: the parallel backend is Intel TBB. You need the TBB headers when you compile and `-ltbb` when you link. **Without TBB, `par` compiles, runs, and is sequential.** No warning.

libc++, Clang: its own backend, experimental, behind `-fexperimental-library` on 17 and 18. Without that flag, the feature-test macro `__cpp_lib_parallel_algorithm` isn't defined at all, which is why the demo checks it. And depending on how your libc++ was built, the default backend may itself be serial; check before you trust a number.

MSVC: Windows thread pool, works out of the box.

The repo's Dockerfile installs TBB, and CMake links it when `find_package(TBB)` succeeds.

**The trap: a team turns on `par` everywhere, sees no speedup, and concludes parallel algorithms are useless, when the binary was never linked against TBB.** (pause)

>> IF AHEAD: For a certified or air-gapped toolchain, TBB is a third-party dependency with its own version, license (Apache 2.0), and supply-chain review. That's often the real blocker, not the C++ standard. If TBB isn't approvable, `par` on GCC is a no-op, and you should say so in the design rather than leave `par` in the code implying parallelism that never happens.

>> IF BEHIND: "On GCC, no TBB means no parallelism, silently. Check `ldd`."

### 32. Your responsibilities with `par` · target 1:17, ~2 min

The library gives you permission to run in parallel. You give it a promise in return.

**No data races between elements.** The body may not touch shared state without synchronization. With `par_unseq` it's stronger: not even with a mutex. Iterations can interleave on one thread, so if one iteration takes a lock and another on the same thread tries to take it, you deadlock. No locks, no allocation (the allocator may lock), no I/O.

**Exceptions terminate.** An exception escaping the body under any parallel policy calls `std::terminate`. No error propagates. In a codebase that uses exceptions for error handling, that alone can rule `par` out of a function.

Order is unspecified. `for_each(par)` visits in any order. `reduce` adds in any order, so a floating-point sum can differ in the last bits from run to run. If you have regression tests that compare floating-point output exactly, a `par` reduce will make them flaky.

When is it worth it? Large N, tens of thousands or more, or expensive per-element work. Starting work on a thread pool costs microseconds. **A `par` sort of a thousand ints is slower than `seq`.**

And measure. On a two-core box, `par` sort is barely faster than `seq`.

>> ASK: "You have `for_each(par, ...)` and the body does `results.push_back(x)` on a shared vector. What's wrong?" Expected: data race on the vector. Say: "Right. Fix it by sizing the output first and writing by index with `transform(par, ...)`, so each element owns its own slot."

>> IF BEHIND: "No shared state, exceptions terminate, order is unspecified, and only worth it for big N. Measure."

### 33. Parallel takeaway · target 1:19, ~1 min

**Execution policies are the cheapest parallelism in C++: one argument, if the body is independent per element.**

They're also the easiest to misuse. No TBB means no parallelism on GCC. `par_unseq` forbids locks. (beat)

Monday morning: check `__cpp_lib_parallel_algorithm` and `ldd` before you trust a `par` benchmark. And read the timing code as carefully as the code being timed.

Anything more complicated than "do this to every element independently" is Session 5's concurrency material.

Time check: 1:20, and lab 4.3 starts now. The C++23 half of this block in one file: `chunk_by`, `fold_left`, `enumerate`. Clang users on Compiler Explorer: pick GCC 14 for TODO 3.

>> IF BEHIND: Read the bold lines. Start the lab on time.

After the lab, at 1:30: the algorithm additions by standard that you may have missed, and the same task written three ways.

### 34. Lab 4.3: C++23 ranges · target 1:20, ~10 min

**Ten minutes, three TODOs, all C++23. A hand-written group-by becomes `chunk_by`, an `accumulate` becomes `fold_left`, and an index loop becomes `enumerate`. Build and run after each; the seven output lines must not change.**

>> DO: Lab slide up, starter link in chat. One quiet minute. Then: "The data is already sorted by sensor; that is not an accident, and TODO 1 depends on it. If you are on Clang with libc++ on Compiler Explorer, TODO 3 will not compile; switch the compiler to GCC 14 or keep the loop, the solution shows the fallback."

What to watch for:

- TODO 1: `same_sensor` takes two `Reading`s and compares `.sensor`; a lambda before the loop, or inline in the `chunk_by` call. `group.front().sensor.c_str()` and `group.size()` do the printing. `group.size()` compiles as is on libstdc++ (the subrange over a vector is sized and the type is `size_t`); the solution casts for `printf` so that it is portable.
- TODO 1, the word that matters: *adjacent*. `chunk_by` groups consecutive runs. If someone shuffles `load()` to test it, they get one group per run, not per sensor. That is why the slide sorted first.
- TODO 2: the line is in the comment. The thing to say in the debrief: the starter's comment warns that `0` as the init would truncate with `accumulate`. With `fold_left` the result type comes from the operation, `std::plus<>` on an `int` and a `double` is a `double`, so even `0` gives `9889.5`. Try it if someone asks. `std::plus<>{}` and `std::plus{}` both work; `std::plus<int>{}` would bring the truncation back.
- TODO 2, the include: `std::plus` lives in `<functional>`. The starter compiles without it on GCC 14 because `<algorithm>` pulls it in; the solution includes it explicitly. Say so if someone is on a stricter toolchain.
- TODO 3: `idx` is a signed `difference_type`; `%zu` wants `size_t`. GCC does not warn about the signedness by default, so the starter's `%zu` passes; the solution casts. On libc++ 18 `std::views::enumerate` does not exist, and the solution keeps the index loop behind `#if defined(__cpp_lib_ranges_enumerate)`. That is the support matrix from slide 38 in one `#if`.
- TODO 3, the binding: `for (auto [idx, r] : ...)`; `auto&` does not bind, because `enumerate` yields a tuple prvalue (of an index and a reference). `auto` or `auto&&` both work.

At seven minutes: solution link. Two minutes: the `chunk_by` loop with `same_sensor` above it, the one-line `fold_left`, and the `#if` around `enumerate`. Point at the `#else` branch: that is how the exercise's solution stays buildable on both compilers.

>> ASK: "Which of the three could you have written in C++20?" Expect: "none", or "the fold, with `accumulate`". Then: "None of the three. `chunk_by`, `fold_left` and `enumerate` all arrived in C++23, and they are the reason the agenda says views became practical in 23, not 20. The C++20 version of this file is the starter."

At ten minutes, move on. Block 4 starts at 1:30; nothing in it depends on this lab.

### 35. C++17 numerics · C++17 · target 1:30, ~2 min

The numeric algorithms that came with the execution policies.

Line 1: `std::reduce`. Like `accumulate`, with two differences. It may reorder the additions, which is what lets it parallelize. And its init defaults to the value type, `double{}` here, so it can't truncate the way `accumulate(..., 0)` does. **For a plain sum of doubles in new code, `reduce` is the better default, unless you need a specific order.**

Line 3: `transform_reduce` with two ranges: the sum of products. Dot product, weighted sum, one pass, parallelizable.

Lines 5 and 6: scans, prefix sums. `inclusive_scan` gives 1.5, 4.0, 7.0: each element includes itself. `exclusive_scan` starts from the init and excludes the current element: 0, 1.5, 4.0. Cumulative distance from speed samples, cumulative counts for a histogram: that's a scan.

Then `std::sample`: five elements without replacement, with an explicit engine so it's reproducible. And `std::clamp`.

>> DO: `./build/demos/s04/demo_s04_numeric17`. Expected: `7 17.5 4 5 100`. Sum 7, dot product 17.5, then `4`, which is `running[2]` after the exclusive scan overwrote the inclusive one, then five samples, and the clamp. Recovery: read it.

>> IF AHEAD: `std::clamp` returns a reference to one of its arguments. `const int& c = std::clamp(x, 0, 100);` with literals binds to a dead temporary when the result is a bound. It's in the same family as the dangling bugs today. Take the result by value. Also, `clamp` requires `lo <= hi`; violating it is undefined, not "swapped".

>> IF BEHIND: "`reduce` is the reorderable `accumulate` with a safe init; `transform_reduce` is a dot product; scans are prefix sums."

>> IF AHEAD: Appendix slide 54 is the other C++17 pair: `std::search` with a `boyer_moore_searcher` built once and reused across many buffers (log scanning, a sync word in a protocol stream), and `for_each_n` for the first n elements without computing an end iterator. Thirty seconds in the PDF.

Bridge: C++20's additions.

### 36. C++20 algorithm additions · C++20 · target 1:32, ~2 min

Fast list.

`shift_left` and `shift_right`: move elements by n within a range. **`shift_left` is the one people didn't know they needed: "drop the oldest n" on a fixed buffer, without `rotate`'s extra work.**

`lexicographical_compare_three_way`: the spaceship for sequences.

`std::ranges::` versions of the `<algorithm>` functions. Not quite every one: `shift_left`, `shift_right`, and `iota` only got `ranges::` versions in C++23, as the next slide shows, and `lexicographical_compare_three_way` has none. And nothing from `<numeric>`.

`midpoint` and `lerp` from Session 2. `midpoint` is the overflow-safe average; it's the fix for the binary-search bug where `(lo + hi) / 2` overflows. `ssize` for a signed size. `std::erase` and `erase_if` for every container: the end of erase-remove. And the `unseq` policy.

>> IF AHEAD: `std::midpoint` on integers rounds toward the first argument, not toward zero. `midpoint(0, 3)` is 1, `midpoint(3, 0)` is 2. That asymmetry is deliberate and documented, and it surprises people in review.

>> IF BEHIND: "Shift, three-way lexicographic compare, `midpoint`, `ssize`, and `erase_if`."

### 37. C++23 algorithms and what comes next · target 1:34, ~2 min

The availability table for the C++23 algorithms. I won't read it. Two corrections and one preview.

The corrections: the `find_last` and `ranges::iota` rows say GCC 14. They're in libstdc++ 13. Not that it matters on our GCC 14 baseline, but if you're on 13, you have them.

The preview is the last row. **C++26 is where ranges and parallelism meet: `std::ranges::sort(par, v)`, a policy as the first argument of the range algorithms.** That answers the question someone always asks: "why can't I use `par` with ranges?" Answer: you will, in C++26.

Also in C++26: `views::concat`, to chain ranges end to end; `views::cache_latest`, which fixes a cousin of the reverse-of-filter cost (a `transform` followed by a `filter` calls the transform twice per passing element); and `ranges::generate_random`.

>> ASK: "Given your toolchain upgrade cadence, when will you see C++26 library features?" Expected: years. Say: "Which is why everything in today's exercise uses only what GCC 14 and Clang 18 already have."

>> IF BEHIND: "Read the availability column for your compiler. C++26 brings `par` to the ranges algorithms."

### 38. Support matrix for this session · target 1:36, ~1 min

**Every row on this slide was found by building the demos on both compilers.** (beat)

Row 1: everything the exercise uses works on both. Constrained algorithms, projections, the core views, `to`, `zip`, `chunk_by`, `split`, `fold_left`, `contains`.

Row 2: `views::enumerate` is missing on libc++ 18. The exercise uses `zip(iota(1uz), ...)` instead.

Rows 3 and 4: the windows, the zip family, the other folds, `find_last`, `range_adaptor_closure`. All on GCC, none on libc++ 18.

Row 5 goes the other way: libc++ 18 has `starts_with`, `ends_with`, and range formatting; GCC 14 doesn't.

So neither compiler is a superset. **libc++ 18 is the laggard on C++23 views; GCC 14 lags on range formatting and `starts_with`.** If your project builds on two toolchains, you're programming to the intersection, and this table is that intersection for today.

>> IF BEHIND: "The exercise uses only row 1. Neither compiler is a superset; program to the intersection."

It's 1:37. The guidance, then the three functions you are about to write yourself.

### 39. Guidance for production code · target 1:37, ~2 min

This is the slide to screenshot.

**Constrained algorithms and projections: now, everywhere. There's no downside.**

Views in code you own, consumed once. A `filter | transform | take` inside a function body: yes. A view stored in a class member: no.

`ranges::to` at boundaries. **Return containers from functions, not views**, unless the whole point of the function is to be a view. A view in a public signature exposes a lambda type and a lifetime contract nobody will read.

Take ranges by `R&&` in generic code. Iterate with `auto&&`.

Measure `join`, `split`, and reverse-of-filter before they go in hot paths.

Test the edges when you convert a loop to a pipeline: empty input, one element, all keys equal. The `split("")` test is the example.

And clang-tidy's `modernize-use-ranges`, in clang-tidy 19 and later, converts iterator-pair calls to `ranges::` calls automatically. On a large codebase, that's the first mechanical pass. (beat)

Every bullet corresponds to something in today's labs or the at-home exercise.

### 40. The same task, three ways · Demo · target 1:39, ~3 min

One task: for each sensor, print the top two readings by value, highest first. Three versions in one file, `same_task_three_ways.cpp`. **You are about to write these yourself: lab 4.4 is the same shape on a smaller task, the top three non-fault readings, with the C++11 version given and the other two yours.** So read this one as the finished answer.

>> DO: Run `./build/demos/s04/demo_s04_same_task_three_ways`. Expected: the same four lines printed three times: `rpm #1 4830`, `rpm #2 4800`, `temp #1 42.5`, `temp #2 41`. Say "same output, three times."

>> DO: Scroll the file on the right, top to bottom. `cpp11`: spelled-out iterator types, a map of vectors, an allocation per group, an index loop with `i < rs.size() && i < 2`. `cpp20`: sort by sensor, then find each run by hand with `find_if` and a `while` loop, `subrange` plus `take(2)`, and a manual counter. `cpp23`: sort by sensor, `chunk_by`, sort each group descending, `zip(iota(1), group | take(2))`.

**The C++23 version has no index variable, no counter, and no allocation per group.** And every step is a named algorithm a reviewer recognizes.

One thing to notice in `cpp23`: it sorts each group in place while `chunk_by` is iterating. That's safe here because the sort only permutes records within the group and doesn't change the sensor, so the group boundaries don't move. If the inner step changed the key, you'd corrupt the grouping. Worth a review comment if you see it.

>> DO: Switch to the Compiler Explorer tab with the same file, GCC 14 `-O2 -std=c++23`. Collapse to the `cpp20` and `cpp23` functions in the assembly pane and compare their size and shape. Say: "Comparable. This isn't a benchmark, it's a sanity check that `chunk_by` didn't bring a runtime tax." Recovery: if Compiler Explorer is down, skip the assembly; the three-way source comparison is the point.

>> IF BEHIND: Run the binary, show only the `cpp23` function, skip Compiler Explorer. Say: "Same output, no counters, no map."

Time check: 1:42. Lab 4.4 starts now: your turn.

### 41. Lab 4.4: the same task, three ways · target 1:42, ~10 min

**Ten minutes, two TODOs, each a whole function. The task is on the slide: the three highest non-fault readings, highest first, as `"sensor=value"` strings. `top3_cpp11` is written. You write the C++17 version with `<algorithm>` and lambdas, then the ranges version, and `main` tells you whether all three agree.**

>> DO: Lab slide up, starter link in chat. One quiet minute. Then, before anyone builds: "The starter already prints `all three agree`. That is because both stubs call `top3_cpp11`. The check means something only after you replace the bodies, so do not take the first green run as a pass."

What to watch for:

- TODO 1, `std::min`: `std::min(3, ok.size())` fails with "no matching function for call to `min(int, std::vector<Reading>::size_type)`", deduced conflicting types. Write `std::min<std::size_t>(3, ok.size())`, or `std::min(ok.size(), std::size_t{3})`. This is the C++17 version's tax: the count bound is index arithmetic, and the types fight you.
- TODO 1, the transform: `std::transform(ok.begin(), ok.begin() + n, std::back_inserter(out), label)`; `label` is a plain function, so it passes as is. People who write `ok.begin() + 3` without the bound get the right answer on this data (six non-fault readings) and a latent out-of-range on any input with fewer. Say that in the debrief; it is the kind of bug the ranges version cannot have, because `take(3)` on two elements yields two.
- TODO 2, sorting the view: `std::ranges::sort` on the `filter` view directly fails: "no match for call to `__sort_fn` (`filter_view<...>`, `ranges::greater`, `double Reading::*`)", constraints not satisfied, `random_access_range` down the list. A filter is bidirectional at best. That is why the pipeline materializes with `to<std::vector>()` before the sort, exactly as slide 15 did.
- TODO 2, the direction: `std::ranges::sort(ok, {}, &Reading::value)` is ascending, so `take(3)` yields the three lowest and `main` prints `MISMATCH`. `std::ranges::greater{}` in the comparator slot, projection after it.
- TODO 2, the last line: `ok | take(3) | transform(label) | to<std::vector>()` deduces `std::vector<std::string>` because `label` returns a `std::string`. `to<std::vector<std::string>>()` is the explicit spelling and also fine.
- Both: anyone who finishes early can count lambdas and iterator pairs per version. C++11: zero lambdas, a named comparator struct, three index loops. C++17: three lambdas, three iterator pairs. Ranges: one lambda (the predicate), no pairs, no index.

At seven minutes: solution link. Two minutes, the three functions one above the other: the loops, then the same three steps as named algorithms with lambdas and iterator pairs, then the same three steps as a pipeline, a projected sort, and a pipeline. Same output, three times.

>> ASK: "Which version do you want to review, and what did C++17 buy you over C++11?" Expect: "the ranges one"; "named algorithms". Then: "Named algorithms, yes, and that is real. But the lambdas and the iterator pairs stayed, and so did the count arithmetic. Ranges removed those three things, and the pipeline reads as the sentence that describes the task. That is the whole course in one file: each standard removed a reason to write the loop by hand."

At ten minutes, move on: one minute for the at-home exercise, then the takeaway.

### 42. Exercise: ranges · target 1:52, ~1 min to launch

One minute. Open `exercises/s04-ranges/README.md` at home.

Tasks one to three are today's labs applied to the real program: projections in `stats.cpp` and `config.h`, `top_n_by_value` as `filter | to<vector>` with a projected `stable_sort`, and `split` as a pipeline. Then tasks four to eight. **Run the tests after task 3. One will fail. That's the task: decide which contract you want, and make the tests say so.** `s04_report_identical` still diffs your report against the starter's, byte for byte.

(beat)

No toolchain at home? The README has Compiler Explorer links for the whole program, starter and solution. The solution is next week's starter, so finishing is optional, but reading it is not.

>> DO: Paste into chat: `exercises/s04-ranges/README.md` and `cmake --build build && ctest --test-dir build -R s04 --output-on-failure`.

### 43. Session 4 takeaway · target 1:53, ~2 min

**Ranges are the standard algorithms with the iterator pairs removed, plus lazy views that compose.** Projections end comparator lambdas. Pipelines read top to bottom. `to` materializes.

The lifetime rules are the same as every other view: `string_view`, `span`, now pipelines. Nothing new to learn, just more places to apply it. And when a loop becomes a pipeline, **the tests define the contract.** (pause)

At home: the exercise, all eight tasks; the first three are today's labs on the real program. The `const` view and the dangling `split`, tasks 4 and 8, are the two worth doing even if you skip the rest. The `solution/` folder is next session's starter, so take a look before we meet.

Next session: concurrency with `jthread` and `stop_token`, `latch` and `barrier`; coroutines and `std::generator`, which is a range, so today plugs straight in; modules; and the adoption roadmap for your own codebase.

The support matrix is on slide 38 and in `handouts/toolchain-support-matrix.md`. Thanks, everyone.

## Appendix slides (in the PDF, not presented)

These sections are kept for the `IF AHEAD` moments and for answering questions; the slides sit after the Further reading divider (deck slide 44).

### A1. Customization point objects · C++20 (deck slide 45)

Quick one. `std::ranges::begin`, `end`, `size`, and every `ranges::` algorithm are objects, not functions. Two consequences.

First, ADL can't hijack them. With `using namespace std::ranges`, an unqualified `sort(v)` finds the object, and a `sort` lurking in `v`'s namespace doesn't get a vote.

Second, **you can pass `std::ranges::sort` to another function as a value, no wrapper lambda.** (beat)

One correction to the slide's second bullet: `ranges::begin` tries arrays first, then a member `begin()`, then a free `begin()` found by ADL. And it refuses an rvalue that isn't a borrowed range. That refusal is where `dangling` comes from.

>> IF AHEAD: Formally, C++20 only required the algorithms to behave this way (not found by ADL, and suppress ADL when found); every implementation made them objects. C++26 (P3136) makes "they are objects" official, so passing them as values is guaranteed rather than just universal practice.

>> IF BEHIND: Skip it entirely (the outline lists it on the cut list). Say: "The algorithms are objects, so ADL can't hijack them and you can pass them around."

### A2. Range-for and ranges · C++20 (deck slide 46)

Every view is a range, so it works in range-for. Line 1 is the everyday case: `records | filter(is_rpm)` where `records` is a named variable. Fine.

Now line 2, and I need to be precise here, because the slide is subtler than it looks. **The danger in range-for is a temporary that dies before the loop body runs, while the view still points into it.** (pause)

As written, `load().records | take(3)` where `records` is a plain data member is actually safe in C++20. `load().records` is an rvalue vector, and piping an rvalue container moves it into the view, into something called `owning_view`. The view owns the data, and the view lives for the whole loop. Put a print in the destructor and you'll see the struct die before the loop starts, with the records still fine.

The version that bites is an accessor. `load().records()` returning a `const&`. Now the pipe sees an lvalue, wraps it in a `ref_view`, which is just a pointer, and the temporary from `load()` dies at the end of the range expression. **Before C++23, that loop iterates a dead vector.** Try it with a destructor print and the loop prints garbage.

The C++20 fix is line 3: the init-statement from Session 1. `for (auto loaded = load(); ...)`. Name the thing. C++23's P2718 extends every temporary in the range expression to the end of the loop, so the bad line becomes legal. That's GCC 15 and Clang 19. We're on 14 and 18, so you write the init-statement.

Last line: iterate views with `auto&&`. `transform` yields values, `filter` yields references. `auto&&` binds either without a copy.

>> IF AHEAD: Two review heuristics. One: in a range-for over a pipeline, if the leftmost thing is a function call followed by `.something()`, check whether `something()` returns a reference; that's the dangling shape. Two: `-Wdangling` in Clang and `-Wdangling-reference` in GCC 13+ catch some of the simpler accessor cases on plain range-for, but not through a view pipeline. Don't count on the warning.

>> IF BEHIND: "Views work in range-for. If the range expression goes through an accessor on a temporary, use the init-statement; C++23 fixes it, our compilers don't yet."

### A3. `std::ranges::` or `std::`? (deck slide 47)

The decision table. **Use `std::ranges::` for new code.** Concepts, projections, sentinels, better return values.

Use `std::` in three cases. One, you need an execution policy, `par`: there's no `ranges::` parallel version until C++26. Two, the `<numeric>` family: `accumulate`, `reduce`, `inclusive_scan` have no `ranges::` version. C++23's `fold_left` fills the `accumulate` gap. Three, legacy code that hands you two iterators. And even there, `std::ranges::subrange(first, last)` turns the pair into a range so you can still use the new algorithms.

Same headers. Mixing them in one file is fine. (beat)

>> IF BEHIND: "New code: `ranges::`. Old style only for `par`, `<numeric>`, and legacy pairs, and `subrange` bridges those."

### A4. The concepts, with one type each (deck slide 48)

This is the handout slide. I won't read the table. Screenshot it.

**The one row to say out loud: `views::filter` over a vector is bidirectional, not random access, and it isn't sized.** (beat) If you filter, you lose `size()` and you lose `it + n`. Anything downstream that needs those won't compile, which is what you want.

One precision on the `sized_range` row: "all of the above except `forward_list` and `filter`" is a little generous. The `istream` view and `split` results aren't sized either. Neither is anything that can't know its length without walking.

>> IF BEHIND: "Screenshot it. Filter costs you random access and `size()`."

### A5. Constrained algorithms with new behavior (deck slide 49)

Second handout table: the algorithms where the `ranges::` version does something genuinely new.

Rows one and two you've seen: `for_each` gives back the functor, `copy` and `transform` give back both ends. Row three: `minmax`, `min`, `max` return the elements, and the projection is used for comparing, not applied to what you get back. Row four: `dangling` on temporaries.

Row five deserves a sentence. `ranges::remove` and `ranges::unique` return a subrange, the tail you should erase. So the erase-remove idiom becomes `v.erase(ret.begin(), ret.end())`. Or, simpler, use `std::erase_if(v, pred)` from Session 2.

The rest are C++23: `contains`, `starts_with`, `ends_with`, `find_last`, `fold_left`, `iota`, `shift_left`. **Those rows are uneven on libc++ 18, so check the support matrix on slide 38 before you rely on one.** Most of them are in block 3.

>> IF AHEAD: `ranges::find_last` returning a subrange rather than an iterator is deliberate: when nothing matches, the subrange is empty and starts at end, so "found nothing" and "found it at the end" are distinguishable without a second comparison. `std::find_end` on reverse iterators was the old way, and people got the base-iterator off-by-one wrong constantly.

>> IF BEHIND: "Second handout table. Note `remove` and `unique` return the subrange to erase."

### A6. `views::split` and the edge case the tests caught · C++20 (deck slide 50)

This is the exercise's `split`, task 3, as a pipeline. `text | split(delimiter) | transform(to string_view) | to<std::vector>()`.

What does `split` yield? Subranges of the original characters. The `transform` turns each one into a `string_view` over the same buffer. **No copies. Every field is a window into the caller's string.** Remember that sentence on slide 18, dangling with views.

Now the first line of the function. `if (text.empty()) return {std::string_view{}};` Why is that there?

Because when the exercise's loop became this pipeline, one test failed. `split("", ',').size() == 1`. The old loop returned one empty field for an empty string. **`views::split` of an empty string yields zero pieces.** (pause)

Which is right? Honestly, either. One empty field is how CSV readers usually behave. Zero fields is how `views::split` behaves. Both are defensible.

What isn't defensible is changing it silently. **The tests define the contract, not the pipeline.** The solution kept the old contract with that early return.

You'll hit this at home, task 3. Let it fail. That's the point of the task.

>> IF AHEAD: Trailing delimiters are another edge: `"a,b,"` gives three pieces with `views::split` in C++23 and in the C++20 DR (P2210), the last one empty. But the pre-DR implementations in GCC 10 and 11 differed in details. If you have a test suite from an older toolchain and it suddenly diverges on an upgrade, check whether your split behavior was what you thought. Also, `views::lazy_split` exists; it's the original C++20 design that works on input ranges but yields awkward inner ranges you can't turn into a `string_view`. For strings, use `split`.

>> IF BEHIND: "`views::split("")` gives zero pieces; the loop gave one. The test caught it. Tests define the contract."

### A7. View semantics · C++20 (deck slide 51)

**A view is a `string_view` over a computation.** Non-owning, O(1) to copy, holds a reference to its source plus the adaptor's state.

First bullet: `v | filter(p)` holds a `ref_view` of the vector, which is a pointer, plus `p`. Copying the view copies a pointer and a lambda.

Second bullet, the exception: `std::move(v) | filter(p)`. Now the source is an rvalue, so the vector is moved into an `owning_view`, inside the view. That was a C++20 defect report, P2415, and all our compilers have it. It's what made line 2 on appendix slide 46 safe: an rvalue container piped into a view gets owned by the view.

Third bullet. **A view must not be stored past the range it refers to.** Same rule as `string_view` and `span`. If you wouldn't store a `string_view` to it, don't store a view to it.

Fourth bullet: the formal `view` concept. Movable, O(1) destruction, and the "cheap to copy if copyable" rule. It used to require default construction too; that was dropped in another DR, which is what the slide's awkward wording means. `views::all` is what the pipe applies to your container to turn it into a view: `ref_view` for lvalues, `owning_view` for rvalues.

>> ASK: "Can you put a filter view in a class member?" Expected: technically yes, but its type includes a lambda type, and it holds a pointer to the source, so the class must not outlive or move away from the source. Say: "Right. And if the class is copied, the copy's view still points at the original's vector. That's the bug. Store the container, or store a span, and build the view in the function that uses it."

>> IF BEHIND: "A view is a `string_view` over a computation: cheap, non-owning, don't store it past its source. Rvalue containers get moved into the view."

### A8. `cartesian_product`, `repeat`, `join_with`, `as_rvalue` · C++23 (deck slide 52)

The rest of the C++23 view family. Four lines in `zip_family.cpp`.

`cartesian_product(sensors, channels)`: every combination. For a test matrix, sensor by channel by mode, that's nested loops collapsed into one. Output: `rpm/1 rpm/2 temp/1 temp/2`.

`repeat(7, 3)`: seven, three times. With no count, it's infinite, which pairs nicely with `zip` for "a constant alongside every element".

`join_with`: the one people will actually use. **String joining with a separator, finally standard.** `sensors | join_with(std::string_view{", "})` gives `rpm, temp`. Watch the separator: pass a `string_view`, not a bare string literal. A literal decays to `const char*`, which isn't a range, and the error is long. A single `char` also works: `join_with(',')`.

`as_rvalue`: turns each element into an rvalue so the consumer moves instead of copying. Here `to<vector>` moves the strings out of `src`.

>> DO: `./build/demos/s04/demo_s04_zip_family`. Expected on GCC 14: `rpm/1 rpm/2 temp/1 temp/2`, then `7 7 7`, then `rpm, temp`, then `2 true`. On libc++ 18 it prints `C++23 view family not available in this standard library`. Recovery: read them.

About that `true` at the end: it says the source string is empty after the move. On our libraries it is, for short strings. **The standard only says moved-from is valid but unspecified.** So the demo is showing you what happens, not a guarantee. Don't write code that depends on it.

>> IF AHEAD: `as_const` is the sibling the outline mentions: `src | views::as_const` makes the elements const references so a consumer can't modify the source. Remember from slide 17 it does not make the view const-iterable. And `cartesian_product`'s size is the product of the sizes, which overflows faster than you'd think for test matrices: four ranges of a thousand is 10 to the 12th.

>> IF BEHIND: "Cartesian product for every combination, `join_with` for joining strings with a separator, `as_rvalue` to move out."

### A9. Formatting and generating ranges · C++23 (deck slide 53)

Two things to close the C++23 block.

Line 1: range formatting from Session 2 applies to views. `println("{}", v | take(3))` prints `[1, 2, 3]`, brackets and commas included. Line 2: `{::.1f}` applies a format spec to each element: the first colon starts the range spec, the second the element spec. Range formatting is libc++ 17 and GCC 15, so on GCC 14 today, it's a loop.

Then the preview. `std::generator<Record>`. A coroutine: it reads lines, parses each, and `co_yield`s the good ones. **A generator is an input range, so it plugs straight into a pipeline.** Line 7 filters it like any other range.

That means the exercise's `load_stream`, which reads the whole file into a vector, could become a generator that feeds the pipeline one record at a time, without holding the file in memory. That's Session 5. GCC 14 has `std::generator`; libc++ doesn't yet.

>> IF BEHIND: "Views format with `println`. `std::generator` is a coroutine that's also a range; that's next session."

In the live deck, execution policies (slide 30) follow the C++23 algorithms directly.

### A10. C++17 searchers and `for_each_n` · C++17 (deck slide 54)

Thirty seconds. `std::search` with a searcher object. **Build a `boyer_moore_searcher` once for a pattern, and search many buffers with it.** That's log scanning, or finding a sync word in a protocol stream. The preprocessing cost is paid once.

Three flavors: `default_searcher`, `boyer_moore_searcher`, and `boyer_moore_horspool_searcher`, which uses less memory.

And `for_each_n`: the first n elements, without computing an end iterator. (beat)

>> IF BEHIND: Fold into slide 35 (cut list). Say: "C++17 also has Boyer-Moore searchers for repeated pattern search, and `for_each_n`."
## Running the labs

The per-lab coaching is in the script at slides 13, 24, 34 and 41. The rhythm is the same every time:

1. Lab slide up, starter link in chat, one quiet minute while people open the file. Then stay quiet for five more; answer chat, do not lecture.
2. At seven minutes, paste the solution link and walk the changes in two minutes. The debrief is the last word on the block's topic, so it is scripted.
3. One question to the room, then move on at ten minutes whether or not people are done. Nothing later depends on finishing a lab.

If the lecture runs late, the lab still starts within two minutes of its target; cut the slide before the takeaway instead. Block 4 is twelve minutes of slides on purpose, so that lab 4.4, the exercise launch and the takeaway fit before 1:55; it has no slack beyond that, so if lab 4.3 runs over, the C++20 additions slide (36) is the one to read as its bold lines only. Clang users on Compiler Explorer: labs 4.1, 4.2 and 4.4 build on libc++ 18; lab 4.3's TODO 3 (`views::enumerate`) does not, and the solution's `#if` shows the fallback. Say so when you launch lab 4.3.

## The at-home exercise

Launched at slide 42 in one minute; nothing is done in class. What to say when it comes up in chat during the week, and in next session's recap:

- Task 1, `minmax`: people write `auto [lo, hi] = std::ranges::minmax(records, {}, &Record::value);` and then use `lo` as a double. It's a `Record`. Use `lo.value`.
- Task 1, the missing `{}`: `std::ranges::lower_bound(records, ts, &Record::ts)` passes the projection as the comparator. Error says "no match for call". Add `{}`. (Lab 4.1's TODO 1 was the rehearsal.)
- Task 1, `lower_bound` precondition: records must be sorted by `ts`. If someone's test fails here, ask whether the data is sorted.
- Task 1, `find_sensor`: `ranges::find` returns an iterator. If `find_sensor` returns a pointer or an optional in the starter, they must still compare against `kSensors.end()` before converting. Watch for a dereference of the end iterator.
- Task 2: `std::ranges::to<std::vector>` without the trailing `()`. The error is long and mentions templates. Point at the parentheses. (Lab 4.2's TODO 2.)
- Task 2: trying to `stable_sort` the filter view directly. Concept error about `random_access_range`. Materialize first; that's why `to` is in the pipeline. (Lab 4.4's TODO 2 showed the same error.)
- Task 3: the transform lambda must take `auto&&` (or `auto`) and construct `std::string_view(part)` explicitly. A lambda taking `std::string_view` by parameter won't convert implicitly from the split subrange.
- Task 3, the failing test: `split("", ',').size() == 1`. Someone will "fix" it by editing the test's expected value without thinking. That's a legitimate decision only if they can say why the new contract is better and who depends on the old one. The solution keeps the loop's contract, one empty field, with an explicit early return, so the decision is visible in code and pinned by a test. **What's not acceptable is a refactor that changes behavior and nobody notices.** Appendix slide 50 is the answer.
- Task 3 on libc++: no differences; `views::split` and `to` are both there.
- Task 4, the `const` view: `serialize(const R&)` fails with "discards qualifiers" on a `filter_view`; `R&&` plus `auto&&` fixes it. Lab 4.2's TODO 3 was the rehearsal; slide 17 is the explanation.
- Task 5, `chunk_by` without the sort first: one group per run, so the report's per-sensor section repeats sensors and the diff test fails. Sort by `&Record::sensor`, then group. Lab 4.3's TODO 1 depended on pre-sorted data for the same reason.
- Task 5, numbering: `std::views::zip(std::views::iota(1uz), ...)`, not `enumerate`, so the solution builds on libc++.
- Task 8, dangling: `auto v = split(std::string("a,b"), ',');` compiles and is UB. ASan with a test that reads a field reports heap-use-after-free; static warnings do not find it. Slide 18.
- Task 7 is a written answer, not code; the criteria are consumed-once, no side effects, and more readable than the loop. `compute_stats` and `load_stream` should probably stay loops; the Q&A below has the reasoning.

Next session's recap slide should pick up whichever of these the chat shows most.

## Likely questions and answers

**Q: Our certified toolchain is GCC 11 (or older). How much of this can we use?**
A: GCC 10 first shipped `<ranges>`, but several C++20 defect reports that changed views, like `owning_view` and the reworked `views::split`, arrived around GCC 12, so on 11 some of today's slides behave differently. Constrained algorithms and projections are the safe subset on older C++20 toolchains. Everything badged C++23 needs GCC 13 or 14 at least. Check the specific feature against your toolchain rather than trusting a version number from me.

**Q: Do views cost anything at runtime?**
A: At `-O2`, `filter | transform | take` compiles to the loop you'd write; the cost model slide shows it. The exceptions are reverse over filter, which runs the predicate twice per element, and `join`/`split`, which are forward-only and branchy. At `-O0`, every adaptor layer is a call, so debug builds of heavy pipelines are noticeably slower.

**Q: Does MISRA or AUTOSAR allow ranges?**
A: MISRA C++:2023 is written against C++17, so C++20 ranges are simply outside its scope; it neither permits nor forbids them, and your deviation or extension process decides. AUTOSAR C++14 predates them entirely. Constrained algorithms with projections are the easiest sell, since they reduce hand-written comparators and iterator mismatches; views need a written lifetime policy.

**Q: Can I put views in a public API or across a library boundary?**
A: Avoid it. A view's type includes lambda types and adaptor internals, so it's unnameable, ties callers to your implementation, and carries a lifetime contract that's easy to violate. Return a container (`to` at the boundary) or a `std::span`; take ranges as `R&&` templates or as `std::span` for non-template interfaces.

**Q: What about ABI? Is it safe to mix object files built with different compiler versions in C++20 mode?**
A: Views are header-only templates, so they're instantiated into your objects; the risk is passing them between separately compiled components whose library versions differ. GCC treated its C++20 library support as experimental for several releases, and layouts of some views changed with the DRs. Keep views internal and don't let them cross a component boundary.

**Q: Why can't I use `std::execution::par` with `std::ranges::sort`?**
A: C++20 and C++23 don't have parallel overloads of the range algorithms. C++26 adds them, with the policy as the first argument. Until then, use `std::sort(par, v.begin(), v.end())`.

**Q: Is it safe for two threads to iterate the same view?**
A: Not for views whose `begin()` caches, like `filter`, `drop_while`, `split`, and `chunk_by`: the first `begin()` call writes the cache, so two threads calling it is a data race. Share the underlying container and give each thread its own view; views are cheap to create.

**Q: Will ranges hurt our compile times?**
A: `<ranges>` is a heavy header, and deep pipelines create long template instantiation chains, so yes, measurably in big translation units. Precompiled headers help today; `import std;` in Session 5 is the longer-term fix, toolchain permitting.

**Q: Is `std::ranges::sort` faster than `std::sort`?**
A: Same algorithm, same codegen in practice; on libstdc++ the ranges version forwards to the same implementation. The win is correctness and readability, not speed.

**Q: How do I debug a pipeline when the result is wrong?**
A: Break it into named stages (`auto s1 = v | filter(...);`) and print each with a loop, or with `println("{}", s1)` where range formatting exists. Keep predicates pure so that inspecting a stage doesn't change the result. In GDB, `skip -rfu ^std::` stops you stepping into library internals.

**Q: `views::split` or `views::lazy_split`?**
A: For strings, `split`: it yields contiguous subranges you can turn into `string_view`s. `lazy_split` is the original C++20 design kept for input ranges; its inner ranges aren't contiguous, so you can't make a `string_view` from them.

**Q: Should `compute_stats` and `load_stream` become pipelines too (task 7)?**
A: Probably not. `compute_stats` mutates a map with `try_emplace` per record, which is a fold into a map, clearer as a loop. `load_stream` reads from a stream with side effects, where view lookahead would consume extra input. The criteria are consumed-once, no side effects, and more readable than the loop.

## Deck issues found

Slide numbers in this list are the deck's numbering before the block restructure (see `docs/session-format.md`); the items are kept for the record. The timing item about slides 46 to 49 is resolved by the restructure: the exercise is now at-home work and block 4 holds the guidance slide, the three-ways demo and lab 4.4.

Status: fixed in the deck, demos, outline and README on 2026-10-02 (see scripts/README.md). Items kept for the record. Still open: the Compiler Explorer `<add short link>` placeholders in every demo file header.

- Slide 4 notes: "Projections remove all three" lambdas. Only two are compare-by-member (the `stable_sort` and `lower_bound` comparators); the `copy_if` predicate captures `sensor` and survives as the `filter` lambda on slide 19. Projections also replace the min/max loop.
- Slide 11, second bullet: `ranges::begin` dispatch order is arrays, then member `begin()`, then ADL free `begin()`, not "member, free, arrays". It also rejects rvalues of non-borrowed ranges.
- Slide 12, line 2 and slide 22 notes: `load().records | views::take(3)` with `records` a data member is safe in C++20, because the rvalue vector is moved into an `owning_view` (P2415) that lives for the loop (verified with a destructor print on GCC without P2718). The dangling case is an accessor returning a reference, e.g. `load().records()`. Slide 22's note that `load() | views::take(3)` is unsafe to iterate in range-for before C++23 is wrong for the same reason.
- Slide 15, `sized_range` row: "all of the above except `forward_list` and `filter`" also wrongly includes the `istream` view and `views::split` results, which are not sized.
- Slide 20: says "Fourteen adaptors"; the slide and `adaptors_tour.cpp` show twelve (plus a chain). `elements` and `common` from the outline are missing.
- Slide 25 and `cost_model.cpp` comment: the explanation for reverse-of-filter's double predicate evaluation ("reverse needs the end") doesn't apply to a filter over a vector, which is common. The cause is `std::reverse_iterator::operator*` decrementing a copy of the base iterator, so each element's backward scan runs twice (measured: 12 predicate calls vs 6 for six elements).
- Slide 31 and `chunk_by.cpp`: the "Before (C++11)" code uses structured bindings (C++17) and `std::println` (C++23).
- Slide 33 / `zip_family.cpp`: `src[0].empty()` after `as_rvalue` prints `true` in practice, but a moved-from `std::string` is only valid-but-unspecified. Also, the note "the pattern must be a range" is incomplete: `join_with` also accepts a single element (`join_with(',')`). `as_const` is named in the outline and the demo header but never shown.
- Slides 35 and 44: `ranges::find_last` and `ranges::iota` are listed as GCC 14; libstdc++ 13 already defines `__cpp_lib_ranges_find_last` and `__cpp_lib_ranges_iota` (verified on GCC 13.3).
- Slide 37 and `parallel.cpp` lines 27 to 29: `t1` is overwritten after the reshuffle, so the printed `seq` time includes the second shuffle (about 70 ms for 5M doubles on a 2-core box), inflating `seq` relative to `par`. Fix: take `t1` once before the shuffle, and a separate timestamp after it.
- Slide 43: "`std::ranges::` versions of every `<algorithm>` function" overstates it; `shift_left`/`shift_right` got `ranges::` versions only in C++23 (as slide 44 says), and `lexicographical_compare_three_way` has none.
- Timing, slides 46 to 49: the Exercise segment starts at 1:40 and must hold the guidance slide, the three-ways demo, a "20 minute" in-class exercise, and the takeaway in 20 minutes. Realistically about 10 minutes of hands-on time remain; the syllabus itself puts wrap-up at 1:55. Either trim a content segment by 8 to 10 minutes or relabel the exercise as 10 minutes.
- Outline vs deck: the outline promises 50 slides; the deck has 49 (outline items 33 and 34 merged into deck slide 33). Outline also lists demo files `split_edge.cpp`, `join_with.cpp`, and `searchers.cpp` that don't exist; those slides are hand-typed or use other files.
- Slide 49 notes point at `handouts/cheat-sheet-ranges.md`, which is a one-line stub.
- All `demos/s04/*.cpp` headers still read `Compiler Explorer: <add short link>`.
