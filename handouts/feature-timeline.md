# Feature timeline: C++14 through C++23

The features taught in this course, by standard. Session numbers in parentheses.

### C++14

Generic lambdas (1); lambda init-capture (1); function return type deduction and `decltype(auto)` (1); relaxed `constexpr` (3); variable templates (3); `std::make_unique` (1); binary literals and digit separators (1); `[[deprecated]]` (1); standard literals `s`, `ms`, etc. (1); `std::exchange` (1); `std::integer_sequence` (1); `std::shared_timed_mutex` (5); `gets` removed (1).

### C++17

Structured bindings (1); `if`/`switch` with initializer (1); `if constexpr` (3); `inline` variables (1); fold expressions (3); CTAD (1, 3); guaranteed copy elision (1); nested namespaces (1); `[[nodiscard]]`, `[[maybe_unused]]`, `[[fallthrough]]` (1); `constexpr` lambdas (3); `auto` non-type template parameters (3); evaluation order guarantees (1); `noexcept` in the type system (1); `std::optional`, `std::variant`, `std::any` (2); `std::string_view` (2); `std::filesystem` (2); parallel algorithms (4); `std::byte` (2); `to_chars`/`from_chars` (2); `std::invoke`, `std::apply` (2); `std::scoped_lock`, `std::shared_mutex` (5); map `try_emplace`, `extract`, `merge` (2); `std::clamp`, `std::gcd`, `std::sample` (2, 4); removals of `auto_ptr`, `register`, trigraphs, dynamic exception specifications, `random_shuffle` (1, 5).

### C++20

Concepts and `requires` (3); ranges (4); coroutines (5); modules (5); three-way comparison `<=>` (1); designated initializers (1); `consteval`, `constinit`, `constexpr` allocation/virtual/`try` (3); abbreviated function templates (3); template lambdas (3); class-type non-type template parameters (3); `using enum` (1); `[[likely]]`, `[[unlikely]]`, `[[no_unique_address]]` (1); range-for with initializer (1); `char8_t` (1); `__VA_OPT__` (1); `std::span` (2); `std::format` (2); `std::jthread`, `stop_token` (5); `std::latch`, `std::barrier`, `std::counting_semaphore` (5); atomic `wait`/`notify`, `std::atomic_ref` (5); `std::osyncstream` (5); `std::bit_cast`, `<bit>` (2); `std::source_location` (2); `<chrono>` calendars and time zones (2); `std::erase_if`, `starts_with`/`ends_with`, `contains` (2); `std::numbers`, `std::midpoint`, `std::lerp`, `std::to_array` (2); `std::shift_left`/`shift_right` (4); `std::is_constant_evaluated` (3).

### C++23

Deducing `this` (3); `if consteval` (3); `static operator()` and `operator[]`, multidimensional subscript (3); `auto(x)` (1); `uz` literal, `#elifdef`, `#warning` (1); `[[assume]]` (1); relaxed `constexpr` restrictions, `constexpr std::unique_ptr` (3); `std::expected` (2); `std::print`/`std::println`, range formatting (2); `std::optional` monadic operations (2); `std::flat_map`/`std::flat_set` (2); `std::mdspan` (2); `std::generator` (5); `std::stacktrace` (2); `std::move_only_function` (2, 5); `std::string::contains` (2); `std::to_underlying`, `std::unreachable` (1); `std::ranges::to` (4); `views::zip`, `enumerate`, `chunk`, `chunk_by`, `slide`, `stride`, `adjacent`, `join_with`, `cartesian_product`, `repeat`, `as_rvalue` (4); `ranges::fold_left` and friends, `ranges::contains`, `starts_with`, `ends_with`, `find_last` (4); `import std;` (5); `std::byteswap`, `std::out_ptr`, `std::forward_like`, `resize_and_overwrite` (2); `std::aligned_storage` deprecated, garbage-collection API removed (5).

---

