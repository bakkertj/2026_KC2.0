# Toolchain support matrix

Baseline: GCC 14 (libstdc++) and Clang 18 (libc++). Fill in during working session 0 by compiling each feature's demo on both. Status: OK, PARTIAL (note), MISSING (fallback).

| Feature | GCC 14 | Clang 18 | Notes / fallback |
|---|---|---|---|
| std::print / println | | | |
| std::generator | | | |
| std::expected | | | |
| std::flat_map / flat_set | | | |
| std::mdspan | | | |
| std::stacktrace | | | |
| Named modules | | | needs CMake 3.28+, Ninja |
| import std; | | | needs CMake 3.30+ |
| Parallel algorithms (execution policies) | | | libstdc++ needs TBB |
| jthread / latch / barrier / semaphore | | | |
| views::zip / enumerate / chunk_by / to | | | |
| Deducing this | | | |
| constexpr std::unique_ptr | | | |
| from_chars for double | | | |
