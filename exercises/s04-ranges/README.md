# Session 4 exercise: ranges

## The program

`starter/` is the Session 3 solution. Count the hand-written loops in it: `split` walks characters with `find` and `remove_prefix`; `value_range` tracks a running min and max; `top_n_by_value` copies with `copy_if`, sorts with a comparator lambda, and truncates; `first_at_or_after` calls `lower_bound` with a comparator; `find_sensor` loops over the table; the report iterates the stats map and, per sensor, calls `top_n_by_value`. Every one of them is a standard algorithm or a pipeline in disguise.

This session replaces them with constrained algorithms (with projections) and views. The report must stay byte-identical (`ctest -R s04_report_identical`).

    cmake -S ../.. -B ../../build && cmake --build ../../build
    ctest --test-dir ../../build -R s04 --output-on-failure

Target tests: `solution/tests/`. Clang users are on libc++, where a few C++23 views are missing (`views::enumerate` among them); the solution avoids those, and `handouts/toolchain-support-matrix.md` lists the rest.

## In class (about 10 minutes; finish at home if needed)

1. **Projections.** In `stats.cpp`, replace the min/max loop in `value_range` with one call: `std::ranges::minmax(records, {}, &Record::value)` returns the min and max **records**, compared by value. Replace the comparator lambda in `first_at_or_after` with `std::ranges::lower_bound(records, ts, {}, &Record::ts)`. In `config.h`, `find_sensor` becomes `std::ranges::find(kSensors, name, &SensorConfig::name)`. Note what disappeared: every comparator lambda.

2. **A pipeline.** `top_n_by_value` becomes: `records | std::views::filter(...) | std::ranges::to<std::vector>()`, then `std::ranges::stable_sort(matching, std::ranges::greater{}, &Record::value)`, then resize. Three lines, each saying one thing.

3. **`views::split`.** `split` becomes `text | std::views::split(delimiter) | std::views::transform([](auto&& part) { return std::string_view(part); }) | std::ranges::to<std::vector>()`. Run the tests. One fails: `views::split` of an empty string yields zero pieces, the loop yielded one empty field. Decide which contract you want, and make the tests say so. (The solution keeps the loop's contract with an early return; the point is that the tests, not the pipeline, define the behavior.)

## At home

4. **`std::ranges::input_range`.** Replace the hand-written `SerializableRange` concept's `begin()`/`end()` requirement with `std::ranges::input_range<T>`. Then try `serialize(v | std::views::filter(...))`. It fails to compile: a `const filter_view` cannot be iterated, because `filter` caches its first element in `begin()`. Change the overload to take `R&&` and iterate with `auto&&`. That is how the standard algorithms take ranges, and why.

5. **Group the report with `chunk_by`.** The report currently drives off the stats map. Rewrite the per-sensor section as: copy the records, `std::ranges::sort(by_sensor, {}, &Record::sensor)`, then `for (auto group : by_sensor | std::views::chunk_by(same_sensor))`. Inside each group: `stable_sort` by value descending, `views::take(kTopReadings)`, and number the lines with `std::views::zip(std::views::iota(1uz), ...)`. (`views::enumerate` is the C++23 spelling; libc++ 18 does not have it.) The diff test tells you whether the output is still identical.

6. **`ranges::contains` (C++23).** The duplicate-name check in `validate` becomes `std::ranges::contains(table.first(i), s.name, &SensorConfig::name)`. It is `constexpr`, so the `static_assert` still holds.

7. **Look at what you did not change.** `compute_stats` still uses a `for` loop with `try_emplace`, and `load_stream` still uses `while (std::getline(...))`. Both could be pipelines. Should they be? Write down why or why not; the criteria are on the Session 4 slides (dangling, laziness, readability, whether the result is consumed once).

8. **Dangling.** Write `auto v = split(std::string("a,b"), ',');` and use `v`. It compiles, and it is undefined behavior: the views point into a temporary that died at the `;`. Then look at what `std::ranges::dangling` does when you call `std::ranges::find` on a temporary vector. Views did not change the lifetime rule from Session 2; they made it easier to hit.

## Checking your work

`solution/` is our version: 20 tests plus the report-diff test, on both compilers. It is the Session 5 starter.

## What you should notice

No comparator lambdas remain; projections replaced them. Multi-step transformations read top to bottom as pipelines. And two things the tests caught that the pipelines would have silently changed: the empty-string split, and the `const` view. Ranges are not a different language; they are the standard algorithms with the iterator pairs removed, and the same lifetime rules as every other view.
