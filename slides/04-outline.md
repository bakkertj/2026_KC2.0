# Session 4 deck outline: Ranges and Algorithms

One line per slide. Badge in brackets; "2col" is before/after; "demo" is live. Target: 49 slides, 90 minutes of content, then the exercise. "From the exercise" slides pull code from `exercises/s04-ranges/` starter and solution.

## 0. Opening (0:00, 5 slides)

1. Title
2. Agenda with minutes
3. Session 3 recap: the solution, and the three stumbles (`as_bytes` is a `reinterpret_cast`, `transform(&T::m)` still, the ambiguous overload when `SerializableRange` did not exclude strings)
4. The iterator-pair problem (feature): `std::sort(v.begin(), v.end(), cmp)` four times in the starter; verbosity, mismatched pairs, no composition, no laziness, a comparator lambda per call
5. Ranges in one sentence: the standard algorithms with the iterator pairs removed, plus lazy views that compose with `|`; C++20 gave the model, C++23 the pieces that make it daily-usable

## 1. The C++20 ranges foundation (0:10, 12 slides)

6. Range concepts [20] (feature): `range` is anything with `begin()`/`end()`; the hierarchy `input_range` < `forward_range` < `bidirectional_range` < `random_access_range` < `contiguous_range`; `sized_range`, `common_range`, `borrowed_range`, `view`; where `vector`, `list`, `span`, `string_view`, and `filter_view` sit
7. Constrained algorithms [20] (2col, from the exercise): `std::sort(v.begin(), v.end(), cmp)` vs `std::ranges::sort(v, {}, &Record::value)`; why `std::ranges::` exists beside `std::` (concept-checked, range-taking, projection-supporting, and they return more)
8. Projections [20] (feature, from the exercise): the feature people miss most; `minmax(records, {}, &Record::value)`, `lower_bound(records, ts, {}, &Record::ts)`, `find(kSensors, name, &SensorConfig::name)`; "compare by X" without a lambda; a pointer-to-member, a lambda, or any invocable
9. What the constrained algorithms return [20] (feature): `ranges::minmax` returns the elements; `ranges::sort` returns the end iterator; `ranges::copy` returns `in_out_result` with both; `ranges::find` on a temporary returns `std::ranges::dangling`
10. Sentinels [20] (feature): an end that is a predicate, not an iterator; `std::unreachable_sentinel`, `std::default_sentinel`; a null-terminated string as a range; why `views::take` can be O(1) to construct on an infinite `iota`
11. Customization point objects [20] (feature): `std::ranges::begin(r)` and friends are objects, not functions: ADL-safe, and the reason `ranges::sort` cannot be called with a wrong iterator pair
12. Range-for and ranges (feature): `for (const auto& r : records | views::filter(...))`; the temporary-lifetime rule from Session 1 applies; C++23 P2718 extends temporaries in the range expression
13. `std::ranges::` vs `std::` decision (feature): use `ranges::` for new code (concepts, projections); `std::` remains for execution policies (parallel algorithms have no `ranges::` version until C++26) and for a few algorithms without a ranges version
14. Borrowed ranges and dangling [20] (feature): `borrowed_range` (lvalue ranges, `span`, `string_view`) vs owning temporaries; `std::ranges::dangling` as the compile-time trap; exercise task 8
15. Concept table (dense): the range concepts with one example type each, for the handout
16. Constrained algorithm tour (dense): the algorithms with meaningful new behavior in `ranges::` (`for_each` returns the functor; `copy` returns both ends; `find_last` [23]; `contains` [23]; `starts_with`/`ends_with` [23]; `fold_left` [23])
17. Segment takeaway: adopt constrained algorithms and projections immediately; they are strictly better than iterator pairs

## 2. Views and pipelines (0:35, 12 slides)

18. Lazy evaluation (feature): a view is a range adaptor over another range, computed on iteration; `filter(pred) | transform(f) | take(3)` visits at most a few elements; nothing runs at construction
19. The pipe syntax [20] (2col, from the exercise): `top_n_by_value` before (`copy_if`, `back_inserter`, sort, resize) and after (`filter | to<vector>`, `stable_sort` with projection, resize)
20. The core adaptors [20] (feature): `filter`, `transform`, `take`, `drop`, `take_while`, `drop_while`, `reverse`, `iota`, `keys`/`values`, `split`/`join`, plus a chain; one line each with output (`elements`, `common`, `all`, `counted` are mentioned, not shown)
21. `views::split` and the empty-string edge [20] (feature, from the exercise): `text | split(',') | transform(to string_view) | to<vector>`; the test that failed; views have their own edge semantics and the tests define the contract
22. View semantics: cheap to copy, non-owning, O(1) [20] (feature): a view is a `string_view` over a computation; `views::all`, `ref_view`, `owning_view` (C++20 DR); `std::ranges::view` requirements
23. The `const` view trap [20] (feature, from the exercise): `filter_view::begin()` caches; `const filter_view` is not a range; `serialize(const R&)` failed and `serialize(R&&)` fixed it; which views are const-iterable (transform, take, iota) and which are not (filter, drop_while, split, reverse of non-const, join)
24. Dangling with views (feature): `auto v = split(std::string("a,b"), ',')` compiles and is UB; the borrowed-range rule again; `std::ranges::to` as the fix when the result must outlive the source
25. Cost model (feature): `filter | transform` inlines to a loop with a branch; `views::join` and `split` can be slow (no random access); `reverse | filter` evaluates the predicate twice per element in some cases; when a plain loop is still right (measure)
26. Composability: writing a range adaptor closure (feature): `auto top_n = views::filter(pred) | views::take(n);` stored and reused; `std::ranges::range_adaptor_closure` [23] for your own adaptors
27. Materialization: `std::ranges::to` [23] (2col): C++20's `std::vector(v.begin(), v.end())` on a non-common view vs `v | to<std::vector>()`; `to<std::map>`, nested `to`, with allocator
28. `views::zip`, `views::enumerate`, and `iota` for indexing [23] (feature, from the exercise): the report's rank column; `zip(iota(1uz), top | take(N))`; `enumerate` as the intended spelling (libc++ 20)
29. Segment takeaway: views for transformations that are consumed once, lazily; `to` when the result must outlive the source; take ranges by `R&&`

## 3. C++23: ranges become practical (1:00, 7 slides)

30. What C++20 was missing (feature): no way to materialize, no `zip`, no `enumerate`, no `chunk`, no `fold`; the 2020 to 2023 catch-up (P2214 "A Plan for C++23 Ranges")
31. `views::chunk_by` [23] (2col, from the exercise): the report's group-by-sensor; before (iterate the stats map, call `top_n_by_value` per key) and after (sort by sensor, `chunk_by(same sensor)`, per-group pipeline)
32. `views::chunk`, `slide`, `stride`, `adjacent` [23] (feature): fixed-size windows over a signal; a moving average as `slide(3) | transform(mean)`; `adjacent<2>` for deltas
33. `cartesian_product`, `repeat`, `join_with`, `as_rvalue` [23] (feature): all sensor pairs; joining with a separator; moving out of a view
34. `ranges::fold_left`, `fold_right`, `fold_left_first` [23] (feature): the missing `accumulate`; a sum with a projection; why `fold_left_first` returns `optional`
35. `ranges::contains`, `starts_with`, `ends_with`, `find_last`, `iota`, `shift_left` [23] (feature, from the exercise): the `validate` duplicate check with `contains` and a projection
36. Formatting ranges again [23] (feature): `std::println("{}", v | views::take(3))` (libc++ / GCC 15); `std::generator` preview for Session 5

## 4. Parallel algorithms (1:20, 4 slides)

37. Execution policies [17] (feature): `std::execution::seq`, `par`, `par_unseq`, `unseq` [20]; `std::sort(std::execution::par, v.begin(), v.end())`; iterator pairs only (no `ranges::` version until C++26)
38. What actually runs in parallel (feature): libstdc++ requires TBB and links `-ltbb`; libc++ has partial support (`-fexperimental-library`); MSVC has a native backend; the repo's Docker image includes TBB
39. Your responsibilities (feature): no data races between elements, exceptions terminate, `par_unseq` forbids locks and allocation in the body; when it is worth it (large N, expensive per-element work) and when it is not
40. Segment takeaway

## 5. Algorithm additions worth knowing (1:30, 5 slides)

41. C++17 numerics [17] (feature): `std::reduce`, `transform_reduce`, `inclusive_scan`/`exclusive_scan`, `std::gcd`/`lcm`, `std::sample`, `std::clamp`; `reduce` vs `accumulate` (order and parallelizability)
42. C++17 searchers and `for_each_n` [17] (feature): `boyer_moore_searcher` with `std::search`
43. C++20 additions [20] (feature): `std::shift_left`/`shift_right`, `lexicographical_compare_three_way`, `std::midpoint`/`lerp`, `ranges::` versions of everything, `std::ssize`
44. C++23 additions [23] (dense): `ranges::iota`, `ranges::shift_left`, `ranges::fold_*`, `ranges::find_last`, `ranges::contains`, `ranges::to`; C++26 preview: `ranges::` parallel algorithms, `views::concat`
45. Support matrix for this session (dense): `views::enumerate` (libc++ 20), `views::chunk_by` (both), `ranges::to` (both), `fold_left` (both), parallel algorithms (TBB), range formatting (GCC 15)

## 6. Exercise and close (1:40, 4 slides)

46. Guidance for production (feature): constrained algorithms and projections now; views in code you own, consumed once; `to` at boundaries; never store a view; take ranges by `R&&`; measure `join`/`split`; the clang-tidy `modernize-use-ranges` check [libc++/GCC]
47. Demo: the same task in C++11, C++20, and C++23 (demo): the report's group-and-rank in three files side by side on Compiler Explorer, with the generated code
48. The exercise: `exercises/s04-ranges/README.md`, in-class tasks 1 to 3 (projections, the `top_n` pipeline, `views::split` and the test that fails), the report-diff test
49. Session takeaway and preview of Session 5 (concurrency, coroutines with `std::generator` as a range, modules, and the adoption roadmap)

## Demo files needed (`demos/s04/`)

- `range_concepts.cpp` (static_asserts placing types in the hierarchy), `constrained_algorithms.cpp`, `projections.cpp`, `algorithm_results.cpp`, `sentinels.cpp`, `dangling.cpp`
- `lazy.cpp` (with a counter proving laziness), `adaptors_tour.cpp`, `const_view_trap.cpp`, `cost_model.cpp`, `adaptor_closure.cpp`, `ranges_to.cpp`, `zip_enumerate.cpp` (the `views::split` slide is hand-typed from the solution)
- `chunk_by.cpp`, `windows.cpp` (chunk/slide/stride/adjacent), `zip_family.cpp` (also `join_with`, `as_rvalue`), `fold.cpp`, `cpp23_algorithms.cpp`
- `parallel.cpp` (gated on TBB availability via CMake `find_package(TBB)`), `numeric17.cpp` (the searchers slide is hand-typed)
- `same_task_three_ways.cpp` for the closing demo
- Exercise excerpts from `exercises/s04-ranges/{starter,solution}`

## Cut list

- Slide 42 (searchers) into 41
- Slide 33 (`cartesian_product`, `repeat`, `join_with`, `as_rvalue`) into 32
- Slide 11 (customization point objects) if the foundation segment runs long
