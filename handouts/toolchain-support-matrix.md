# Toolchain support matrix

Baseline: GCC 14 with libstdc++ 14, and Clang 18 with libc++ 18 (the repo's CMake adds `-stdlib=libc++` for Clang; see `cmake/warnings.cmake`). Verified by building this repo. Update as sessions add features.

| Feature | GCC 14 / libstdc++ | Clang 18 / libc++ | Notes and fallback |
|---|---|---|---|
| std::expected | OK | OK | Clang 18 with **libstdc++** has none: libstdc++ gates it on `__cpp_concepts >= 202002L`, which Clang reports only from 19. This is why Clang builds use libc++. |
| std::print / println | OK | OK | |
| std::format | OK | OK | |
| from_chars for double | OK | MISSING (libc++ 20) | `__cpp_lib_to_chars` is undefined on libc++ 18; the Session 1 solution and `demos/s01/from_chars.cpp` fall back to `strtod` under `#ifndef __cpp_lib_to_chars`. |
| Deducing this | OK | OK | |
| operator<=>, designated initializers, using enum, uz, auto(x), to_underlying, unreachable | OK | OK | Session 1 demos. |
| [[assume]] | OK (GCC 13+) | MISSING (Clang 19) | Clang 18 warns on the unknown attribute; kept out of compiled demos. |
| Range-for temporaries lifetime (P2718) | GCC 15 | Clang 19 | Slide shows the C++20 init-statement workaround. |
| std::generator | | | to verify in Session 5 |
| std::flat_map / flat_set | | | to verify in Session 2 |
| std::mdspan | | | to verify in Session 2 |
| std::stacktrace | | | to verify in Session 2 |
| std::optional monadic ops | | | to verify in Session 2 |
| std::string::contains | | | to verify in Session 2 |
| Named modules / import std; | | | to verify in Session 5; needs CMake 3.28+ / 3.30+ and Ninja |
| Parallel algorithms | | | to verify in Session 4; libstdc++ needs TBB |
| jthread / latch / barrier / semaphore | | | to verify in Session 5 |
| views::zip / enumerate / chunk_by / to | | | to verify in Session 4 |
