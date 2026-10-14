# Session format from Session 2 on: four blocks of 20 + 10

Session 1 ran as 100 minutes of lecture and a 20-minute exercise at the end, and it was too much lecture. Sessions 2 to 5 use a block rhythm instead:

| Block | Lecture | Lab | What the lab is |
|---|---|---|---|
| 1 | 0:00 to 0:20 | 0:20 to 0:30 | `labs/s0N/lab1_*.cpp` |
| 2 | 0:30 to 0:50 | 0:50 to 1:00 | `labs/s0N/lab2_*.cpp` |
| 3 | 1:00 to 1:20 | 1:20 to 1:30 | `labs/s0N/lab3_*.cpp` |
| 4 | 1:30 to about 1:42 | ten minutes | `labs/s0N/lab4_*.cpp` (Session 5: the roadmap worksheet at 1:35) |
| close | by 1:55 | | at-home exercise launch and takeaway; the last five minutes are slack |

A lab is one `.cpp` file with three numbered TODOs that the segment just taught, sized for about seven minutes of work plus a two-minute debrief. Every lab opens on Compiler Explorer from the slide's link (most attendees had no local toolchain in Session 1), and the same file is in the repo under `labs/` for people who build locally (`ctest -R lab_s02`). A finished `*_solution.cpp` sits beside each starter; its output is the test oracle (`labs/s0N/*.expected`).

The telemetry exercise (`exercises/s0N-*`) is unchanged but is now entirely at-home work. Its in-class tasks became the first at-home tasks, and the exercise slide at the end of each deck launches it as homework.

## What moved out of the live decks

Lecture time per session drops from about 100 minutes to about 75. The slides that no longer fit were not deleted: each deck ends with a **Further reading** appendix, after the takeaway slide, that attendees get in the PDF and the speaker skips live. The scripts' `IF AHEAD` material points at them. The lists below are by the slide's title in the deck before the change.

### Session 2 (59 slides to 53 live, including 4 lab slides and three one-minute "at a glance" API tables, + 13 appendix)

- Block 1, Views (0:00): title, agenda, recap, what is a vocabulary type, ownership vocabulary, `string_view`, `string_view` at a glance, the dangling rule, when to still take `const std::string&`, `span`, `span` at a hardware boundary, views takeaway. **Lab 2.1** views.
- Block 2, Maybe, either, anything (0:30): `optional`, mechanics, `optional` at a glance, as return/member/parameter, monadic `optional`, `variant`, `variant` at a glance, `visit` and `overloaded`, variant in practice, `any` (one minute), choosing between them, takeaway. **Lab 2.2** optional and variant.
- Block 3, `expected` and formatting (1:00): the landscape, `expected`, mechanics, monadic pipeline, designing the error type, `expected` vs exceptions, takeaway, why `printf` and `iostream` both lost, `format`, the spec mini-language (one minute), `print`/`println`, `formatter` for your own types, formatting takeaway. **Lab 2.3** expected and format.
- Block 4, Library tour and close (1:30): the seven tour slides, deprecated and removed, support matrix, interface design checklist. **Lab 2.4** the checklist applied (1:42). Exercise (at home) and takeaway.
- Appendix (deck slides 55 to 67): `string_view` operations, not a C string, `span` mechanics, `mdspan` preview, transparent comparators, the pointer-to-member trap, `variant` details, where these came from, `expected<void, E>`, a formatter with its own spec, formatting ranges, formatting without allocating, migration.

### Session 3 (51 slides to 45 live, including 4 lab slides, + 10 appendix)

- Block 1, `constexpr` (0:00): title, agenda, recap, why compile time matters here (the two halves folded in), `constexpr` in C++11, relaxed `constexpr`, where `constexpr` values live, `constexpr` lambdas and `if constexpr`, `std::array` at compile time, dynamic allocation at compile time, `consteval`, takeaway. **Lab 3.1** constexpr.
- Block 2, C++17 templates (0:30): all eight slides unchanged (15 minutes). **Lab 3.2** templates at 0:45; the five minutes before block 3 are the buffer.
- Block 3, Concepts (1:00): the problem, a concept is a named predicate, four ways to constrain, abbreviated function templates, `requires` expressions, the standard concepts library, subsumption (both slides), concepts as questions about your API, design guidance, concepts vs SFINAE, takeaway. **Lab 3.3** concepts.
- Block 4, Deducing `this` and close (1:30): three problems, explicit object parameter, deduplicating `const` overloads, CRTP without the template parameter, recursive lambdas, takeaway, reading the diagnostics. **Lab 3.4** deducing this (1:45). Exercise (at home) and takeaway.
- Appendix (deck slides 47 to 56): the two halves of today, what a constant expression refuses, also refused: `reinterpret_cast`, the rest of C++20 `constexpr`, `consteval` with a message, `constinit`, almost nothing left (C++23), the full timeline, template lambdas and unevaluated lambdas, class types as template arguments.

### Session 4 (49 slides to 43 live, including 4 lab slides, + 10 appendix)

- Block 1, Foundation (0:00): title, agenda, recap, the iterator-pair problem, ranges in one sentence, range concepts, constrained algorithms, projections, what the algorithms return, sentinels, borrowed ranges and `dangling`, takeaway. **Lab 4.1** constrained algorithms.
- Block 2, Views (0:30): views are lazy, the pipe syntax, the core adaptors, the `const` view trap, dangling with views, the cost model, composable pieces, `ranges::to`, `zip`/`enumerate`, takeaway. **Lab 4.2** views.
- Block 3, C++23 and parallel (1:00): what C++20 was missing, `chunk_by`, windows, `fold_left`, `contains`/`starts_with`/`find_last`/`iota`, execution policies, what actually runs in parallel, your responsibilities with `par`, parallel takeaway. **Lab 4.3** C++23 ranges.
- Block 4, Algorithms and close (1:30): C++17 numerics, C++20 algorithm additions, C++23 algorithms and what comes next, support matrix, guidance for production code, the same task three ways. **Lab 4.4** three ways (1:42). Exercise (at home) and takeaway.
- Appendix (deck slides 45 to 54): customization point objects, range-for and ranges, `std::ranges::` or `std::`, the concepts with one type each, constrained algorithms with new behavior, `views::split` and the edge case, view semantics, `cartesian_product`/`repeat`/`join_with`/`as_rvalue`, formatting and generating ranges, C++17 searchers and `for_each_n`.

### Session 5 (53 slides to 45 live, including 3 lab slides and the worksheet, + 11 appendix)

- Block 1, Concurrency (0:00): title, agenda, recap, why these three are last, the course so far in one program, what C++11 gave, reader/writer locks, `scoped_lock`, `jthread`, cooperative cancellation, semaphores, the bounded queue, ThreadSanitizer as a test, takeaway. **Lab 5.1** jthread and stop_token.
- Block 2, Coroutines (0:30): what a coroutine is, why they matter here, `std::generator`, generator mechanics, what the compiler generates, awaitables, a coroutine as a state machine, where coroutines pay off, takeaway. **Lab 5.2** generator.
- Block 3, Modules and deprecations (1:00): what modules fix, `export module`/`export`/`import`, the global module fragment, `import std;`, build systems (two minutes), compiler status, an adoption posture, modules takeaway, deprecated and removed, finding them, deprecations takeaway. **Lab 5.3** deprecations. (Modules cannot run as a single Compiler Explorer file, so the lab is the deprecations one.)
- Block 4, Roadmap and close (1:30): three tiers, tooling as a force multiplier, writing the coding-standard update, the worksheet (**the block's lab**, 10 minutes at 1:35), the capstone (which launches the at-home tasks), what is coming in C++26, where to follow the language, course takeaway.
- Appendix (deck slides 47 to 57): false sharing, `latch` and `barrier`, `atomic::wait`/`notify` and `atomic_ref`, `atomic<shared_ptr>`, `osyncstream`, `move_only_function`, what is still missing, coroutines and `expected`, allocation and performance, availability, partitions and header units.

## Running a lab

1. Show the lab slide. It has the Compiler Explorer link, the three TODOs in one line each, and the time. Say "ten minutes, three TODOs, do them in order, build after each one".
2. Give people one minute to get the file open before you say anything else. Then stay quiet for five.
3. At seven minutes, show the solution link and walk the three changes in two minutes: the debrief is the lecture's last word on the topic.
4. Whoever is not finished keeps the solution link; nothing in the next block depends on having finished the lab.

The scripts carry the per-lab coaching: the stumble to expect on each TODO, what to say in the debrief, and the one question to ask the room.
