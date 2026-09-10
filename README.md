# The Evolution of C++: C++14 through C++23

Course material for a five-session, ten-hour course that brings C++98/C++11 engineers up to C++23. See `syllabus/` for the syllabus and `PLAN.md` for how the material is produced.

## Layout

- `slides/` Marp decks, one per session; `slides/theme/course.css` is the theme; `slides/build.sh` exports HTML and PDF to `slides/out/`
- `demos/` one small file per feature shown in class; each is a CMake target and carries a Compiler Explorer link
- `exercises/` per-session exercises with `starter/`, `solution/`, `tests/`, and an attendee README; the cumulative telemetry processor moves from session to session
- `handouts/` the feature timeline, cheat sheets, the adoption roadmap worksheet, and the toolchain support matrix
- `tools/` snippet extractor (keeps slide code in sync with demo files), badge checker, demo scaffolder

## Build

Requirements: CMake 3.28+, GCC 14 or Clang 18, Ninja (optional), Node 20+ with `npm install -g @marp-team/marp-cli` for slides. A `Dockerfile` pins all of this.

    cmake -S . -B build -G Ninja
    cmake --build build
    ctest --test-dir build --output-on-failure
    bash slides/build.sh

## Conventions

- Code on slides is an excerpt of a compiled file in `demos/`, pulled in by `tools/extract-snippets.py`, wherever the code can compile under C++23; the few hand-typed blocks (C++11-only forms, fragments) say so in their speaker notes
- Every feature slide carries a standard badge; `tools/check-standard-tags.py` enforces it
- All code builds warning-free under `-Wall -Wextra -Wpedantic -Werror` on both compilers
- No em dashes in prose
