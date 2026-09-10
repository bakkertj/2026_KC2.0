# Toolchain support matrix

Baseline: GCC 14 with libstdc++ 14, and Clang 18 with libc++ 18 (the repo's CMake adds `-stdlib=libc++` for Clang; see `cmake/warnings.cmake`). Verified by building this repo. Update as sessions add features.

| Feature | GCC 14 / libstdc++ | Clang 18 / libc++ | Notes and fallback |
|---|---|---|---|
| std::expected | OK | OK | Clang 18 with **libstdc++** has none: libstdc++ gates it on `__cpp_concepts >= 202002L`, which Clang reports only from 19. This is why Clang builds use libc++. |
| std::print / println | OK | OK | |
| Range formatting (`std::println("{}", vec)`) | MISSING (GCC 15) | OK | `demos/s02/format_ranges.cpp` gated on `__cpp_lib_format_ranges`. |
| std::move_only_function, std::out_ptr | OK | MISSING (libc++ 19+) | `demos/s02/tour_cpp23.cpp` gated on the feature-test macros. |
| std::make_format_args | takes lvalues | takes lvalues | C++23 DR P2905: pass a named variable, not a literal. |
| std::format, std::formatter specializations | OK | OK | libc++ requires the formatter's `format()` member to be a template on the context type (it checks against a compile-time context). |
| from_chars for double | OK | MISSING (libc++ 20) | `__cpp_lib_to_chars` is undefined on libc++ 18; the Session 1 solution and `demos/s01/from_chars.cpp` fall back to `strtod` under `#ifndef __cpp_lib_to_chars`. |
| Deducing this | OK | OK | Session 3 solution (`SensorStats::add`), constexpr, chained on lvalues and rvalues. |
| operator<=>, designated initializers, using enum, uz, auto(x), to_underlying, unreachable | OK | OK | Session 1 demos. |
| consteval, constexpr std::array/optional/string_view/span in constant expressions | OK | OK | Session 3 solution. `std::as_bytes` is not constexpr on either (reinterpret_cast). |
| Concepts, abbreviated function templates, requires-clauses, fold expressions | OK | OK | Session 3 solution. |
| [[assume]] | OK (GCC 13+) | MISSING (Clang 19) | Clang 18 warns on the unknown attribute; kept out of compiled demos. |
| Range-for temporaries lifetime (P2718) | GCC 15 | Clang 19 | Slide shows the C++20 init-statement workaround. |
| std::generator | | | to verify in Session 5 |
| std::flat_map / flat_set | MISSING (GCC 15) | MISSING (libc++ 20 has flat_set only) | `demos/s02/tour_flat_map.cpp` gated on `__cpp_lib_flat_map`; slide uses Compiler Explorer with a trunk compiler. |
| std::mdspan | MISSING (GCC 15) | OK (libc++ 17+) | Slide only; Compiler Explorer. |
| std::stacktrace | OK, link `-lstdc++exp` | MISSING | `demos/s02/tour_stacktrace.cpp` gated on `__cpp_lib_stacktrace`; CMake adds the link library for GCC. |
| std::optional monadic ops, std::expected value_or/error_or | OK | OK | Session 2 solution. `transform(&T::member)` on a temporary optional does not compile on either (rvalue-reference result); use a lambda. |
| std::string::contains / ends_with | OK | OK | Session 2 solution and tests. |
| Named modules / import std; | | | to verify in Session 5; needs CMake 3.28+ / 3.30+ and Ninja |
| Parallel algorithms | | | to verify in Session 4; libstdc++ needs TBB |
| jthread / latch / barrier / semaphore | | | to verify in Session 5 |
| views::zip / enumerate / chunk_by / to | | | to verify in Session 4 |
