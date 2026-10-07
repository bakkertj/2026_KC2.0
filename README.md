# The Evolution of C++: C++14 through C++23

Course material for a five-session, ten-hour course that brings C++98/C++11 engineers up to C++23. See `syllabus/` for the syllabus (markdown, DOCX and PDF) and `PLAN.md` for how the material is produced.

## Layout

- `slides/` Marp decks, one per session; `slides/theme/course.css` is the theme; `slides/build.sh` exports HTML and PDF to `slides/out/` (decks via Marp; the `*-outline.md` review documents via pandoc and LibreOffice as paged DOCX and PDF)
- `demos/` one small file per feature shown in class; each is a CMake target and carries a Compiler Explorer link
- `exercises/` per-session exercises with `starter/`, `solution/`, `tests/`, and an attendee README; the cumulative telemetry processor moves from session to session
- `handouts/` the feature timeline, cheat sheets, the adoption roadmap worksheet, the toolchain support matrix, and the Compiler Explorer links index; `handouts/build.sh` exports them all to DOCX and PDF (links stay clickable) in `handouts/out/`
- `tools/` snippet extractor (keeps slide code in sync with demo files), badge checker, demo scaffolder, Compiler Explorer link generator, and `package.sh` (zips what attendees get for a session: deck PDF, demo sources, exercise starter, handouts; never the scripts, outlines or syllabus)
- `syllabus/` the syllabus; `syllabus/build.sh` regenerates its DOCX and PDF from the markdown

## Build

Requirements: CMake 3.28+, GCC 14 or Clang 18, Ninja (optional), Node 20+ with `npm install -g @marp-team/marp-cli` for slides, and pandoc plus LibreOffice (`brew install pandoc && brew install --cask libreoffice` on macOS) for the DOCX/PDF exports of outlines, scripts and handouts. Without LibreOffice the build scripts still write the DOCX files and skip the PDFs. A `Dockerfile` pins all of this.

    cmake -S . -B build -G Ninja
    cmake --build build
    ctest --test-dir build --output-on-failure
    bash slides/build.sh

## Conventions

- Code on slides is an excerpt of a compiled file in `demos/`, pulled in by `tools/extract-snippets.py`, wherever the code can compile under C++23; the few hand-typed blocks (C++11-only forms, fragments) say so in their speaker notes
- Every feature slide carries a standard badge; `tools/check-standard-tags.py` enforces it
- All code builds warning-free under `-Wall -Wextra -Wpedantic -Werror` on both compilers
- No em dashes in prose

## Compiler Explorer links

`handouts/compiler-explorer-links.md` has one link per demo, preconfigured for x86-64 GCC 14.2, `-std=c++23`, and an execution pane. `tools/godbolt-links.py` regenerates it from the demo sources; the links it writes by default are self-contained (the source is encoded in the URL) and need no network to produce. Run `tools/godbolt-links.py --shorten` from a machine that can reach godbolt.org to turn them into `godbolt.org/z/...` short links; that also writes each short link into the demo's header comment, and `--check` fails CI while any header still says `<add short link>`. The header, snippet-marker and `// godbolt:` lines are blanked rather than dropped in the godbolt copy, so line numbers there match the files in the repo and the "line N" cues in the scripts. Per-demo flags go in a `// godbolt: <flags>` comment (`// godbolt: skip` for demos that need several files or TBB). The exercise programs get links too: `tools/amalgamate.py` flattens each starter and solution into one translation unit, and the link feeds `data/sample.csv` on stdin, so the whole telemetry processor runs on godbolt.
