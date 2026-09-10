# Course Material Production Plan

## The Evolution of C++: C++14 through C++23

This plan covers how the slides, demos, exercises, and handouts for the five-session course will be built, in what order, and how each piece is checked before it is used in front of a class. It assumes the material is produced mostly with Claude in a series of working sessions, with Trevor reviewing and editing for voice and accuracy, and that everything lives in a single git repository on Trevor's Mac.

---

## 1. Tooling decisions

### Slides: Marp

Marp (Markdown Presentation Ecosystem) is the recommended choice over reveal.js for this course. One Markdown file per session, slides separated by `---`, speaker notes in HTML comments, a custom CSS theme, and a single `marp-cli` command that exports HTML and PDF. Code blocks get syntax highlighting with no extra setup. VS Code previews Marp decks natively with the Marp extension. reveal.js offers fragments and embedded live editors, but at the cost of an HTML build and a more complex repo; those features are not needed when every demo has a Compiler Explorer link.

The one thing Marp does not do well is side-by-side "before/after" columns out of the box. The custom theme adds a two-column layout class, since "C++11 on the left, C++20 on the right" is the dominant slide pattern in this course.

Install: `npm install -g @marp-team/marp-cli` and the "Marp for VS Code" extension.

### Code: CMake, doctest, Compiler Explorer

Every demo and exercise is a CMake target so the whole repository can be built and tested with one command, which is the only reliable way to guarantee that nothing on a slide has bit-rotted. Tests use doctest (single header, vendored into the repo, no package manager). Every demo also carries a Compiler Explorer short link in a comment at the top of the file and on its slide, so attendees can experiment without a local toolchain.

### Toolchain baseline

The course targets GCC 14 and Clang 18 with `-std=c++23`. Not every C++23 library feature is available on both, and the plan calls for a support matrix (Section 7) to be verified in the first working session so that no slide promises something the class toolchain cannot compile. A Dockerfile pinning `gcc:14` is included so attendees and CI share one environment; Compiler Explorer is the fallback for anything that needs a newer compiler than the baseline.

### Repository conventions

Plain Markdown, no em dashes in any prose, ASCII-only file names, one feature per demo file, and every code file compiling warning-free under `-Wall -Wextra -Wpedantic -Werror`. Commit history is kept clean enough to use `git log` as a change log for the course.

---

## 2. Repository layout

```
cpp-evolution/
  README.md                     how to build everything; links to decks and exercises
  LICENSE
  CMakeLists.txt                top level; adds demos/ and exercises/
  cmake/
    warnings.cmake              shared warning flags and standard settings
  Dockerfile                    gcc:14 image with cmake, ninja, marp-cli
  .github/workflows/ci.yml     build + test on gcc-14 and clang-18; build slides
  .clang-format
  .clang-tidy                   modernize-* checks used in Session 5
  syllabus/
    cpp-evolution-syllabus.md
    cpp-evolution-syllabus.docx
  slides/
    theme/course.css            Marp theme: two-column layout, standard badges
    build.sh                    marp --theme ... --pdf --html for every deck
    01-everyday-language.md
    02-vocabulary-types.md
    03-compile-time-and-generic.md
    04-ranges.md
    05-concurrency-coroutines-modules.md
    out/                        generated HTML and PDF (gitignored)
  demos/
    s01/ ... s05/               one small file per feature, each a CMake target
    CMakeLists.txt
  exercises/
    common/doctest.h
    s01-modernize-syntax/
      README.md                 the task, in attendee-facing language
      starter/                  C++11 code the attendee begins from
      solution/                 reference solution
      tests/                    doctest cases that pass on the solution
      CMakeLists.txt
    s02-vocabulary-types/
    s03-compile-time/
    s04-ranges/
    s05-concurrency/
    capstone/                   README only; attendees bring their own code
  handouts/
    feature-timeline.md         Appendix A of the syllabus as a printable sheet
    cheat-sheet-vocabulary-types.md
    cheat-sheet-ranges.md
    adoption-roadmap-template.md   the Session 5 workshop worksheet
    toolchain-support-matrix.md
  tools/
    check-standard-tags.py      finds features on slides missing a standard badge
    new-demo.sh                 scaffolds a demo file with header and CMake entry
```

---

## 3. Slide conventions

### Deck size and pacing

Each deck targets 45 to 60 content slides for 90 minutes of lecture, which is roughly 90 seconds per slide with time reserved for live demos. Decks longer than 65 slides are cut, not sped up.

### Slide types

Most slides fall into one of five templates, defined as classes in the theme:

**Feature slide.** Title, a standard badge (C++14, C++17, C++20, C++23) in the top right, a one-sentence statement of the problem it solves, and a code sample. This is the workhorse.

**Before/after slide.** Two columns: the C++98/11 way on the left, the modern way on the right, with the same behavior in both. Used whenever a feature replaces an existing idiom, which is most of them.

**Evolution slide.** A single feature traced across standards (the `constexpr` slide, the lambda slide, the ranges slide). A vertical timeline with one code fragment per standard.

**Demo slide.** A title, the Compiler Explorer link, and what to watch for. The demo itself is done live in the browser or in the terminal, never read from the slide.

**Takeaway slide.** Closes each major segment with one to three sentences and, where applicable, the "what to change Monday morning" item.

### Speaker notes

Every slide has notes in an HTML comment: the talking points, the common question that comes up, and the one thing not to forget. Notes are written so that someone other than the author could deliver the deck.

### Code on slides

Code samples on slides are excerpts of files in `demos/`, never typed directly into the deck, and the demo file name appears in the slide's notes. A small script (`tools/extract-snippets.py`, added in the first working session) pulls marked regions from demo files into the Markdown so the two cannot drift. Samples are kept under 14 lines; anything longer becomes a demo slide with a link.

### Accuracy

Every feature slide is checked against cppreference for the standard tag and against the toolchain support matrix for availability. `tools/check-standard-tags.py` fails the build if a feature slide has no badge.

---

## 4. Exercise design

### The cumulative program

All five exercises operate on one program that attendees modernize step by step, so that by Session 5 they have carried a single C++11 codebase to C++23 and can see the whole arc in one diff. The program is a **telemetry record processor**: it reads a text file of timestamped sensor records, validates them, computes per-sensor statistics, and writes a report. The domain is close enough to real embedded and defense work to feel relevant, small enough to fit in about 400 lines, and free of anything proprietary.

The starter code is deliberately written in careful, idiomatic C++11 rather than bad C++. The point is to show that good C++11 still has strictly better modern replacements, not to fix sloppy code.

### Per-session exercises

**Session 1, modernize the syntax.** Starter: the C++11 processor. Tasks: replace `typedef` with `using`; replace `std::pair` returns with structured bindings; add `[[nodiscard]]` to every value-returning function and fix the warnings that appear; replace the six hand-written comparison operators on `Record` with `operator<=>`; convert `std::map::insert` return checks to `if`-with-initializer; replace `make_shared`/`new` for unique ownership with `make_unique`. Test: the doctest suite is unchanged and still passes, which proves behavior was preserved.

**Session 2, vocabulary types.** Tasks: change the parser signature from `bool parse(const char*, size_t, Record*, int* err)` to `std::expected<Record, ParseError> parse(std::string_view)`; replace the sentinel `-1` in `find_sensor` with `std::optional`; replace the report's `printf` calls with `std::println` and add a `std::formatter<Record>`; take `std::span<const Record>` in the statistics functions. Tests: new cases for each `ParseError` variant.

**Session 3, compile time and concepts.** Tasks: replace the runtime-built CRC lookup table with a `constexpr` one and add a `static_assert` on a known checksum; make the `serialize()` overload set a single constrained template using concepts and `if constexpr`; add a `consteval` validator for the sensor configuration table. Tests: compile-time assertions plus existing runtime cases.

**Session 4, ranges.** Tasks: rewrite the report generator (filter valid records, group by sensor, take the top N by value, print) as a ranges pipeline using `views::filter`, `views::chunk_by`, `views::take`, `views::enumerate`, and `ranges::to`; replace three hand-written loops in statistics with `ranges::` algorithms and projections. Tests: report output is byte-identical to the Session 3 version.

**Session 5, concurrency and coroutines.** Tasks: split parsing and statistics into a producer and consumer on `std::jthread` with a `stop_token`, synchronized with a `std::counting_semaphore`; expose the parser as a `std::generator<Record>`; run the clang-tidy `modernize-*` checks on the finished program and fix what they flag. Tests: existing suite under ThreadSanitizer.

**Capstone (optional, take-home).** Attendees apply the tier-1 and tier-2 roadmap items to 300 to 500 lines of their own code and bring a before/after diff. The repo provides only a README and the roadmap worksheet.

### Exercise packaging

Each exercise directory has a `README.md` written for the attendee (goal, tasks in order, hints, how to run the tests), a `starter/` that builds and passes its tests as-is, a `solution/` that is the next session's starter, and `tests/`. The 20-minute in-class window is enough for the first two or three tasks; the rest are marked "finish at home" and the next session opens with the solution.

---

## 5. Working sessions with Claude

The material is built in an ordered sequence of working sessions. Each session produces something that builds and is reviewed before the next one begins. Estimated count: 14 sessions, typically one to two hours of Trevor's time each (mostly review).

| # | Working session | Produces | Trevor's review gate |
|---|---|---|---|
| 0 | Infrastructure | Repo scaffold, CMake, Marp theme with the five slide templates, Dockerfile, CI, doctest vendored, snippet extractor, toolchain support matrix verified against GCC 14 and Clang 18 | Repo builds cleanly on the Mac; theme looks right in a 10-slide sample deck; support matrix approved |
| 1 | Cumulative program | The C++11 telemetry processor (starter for Session 1) with full doctest suite | Code reads as good C++11; domain and size feel right for the audience |
| 2 | Session 1 exercise | Solution, tests, attendee README | Tasks fit in 20 minutes plus take-home; solution is the intended style |
| 3 | Session 1 slides | Deck outline for review first, then full deck with notes and demos | Outline approved before slides are written; every slide has a badge and notes |
| 4 | Session 2 exercise | As above | |
| 5 | Session 2 slides | As above | |
| 6 | Session 3 exercise | As above | |
| 7 | Session 3 slides | As above | |
| 8 | Session 4 exercise | As above | |
| 9 | Session 4 slides | As above | |
| 10 | Session 5 exercise | As above, plus clang-tidy configuration | |
| 11 | Session 5 slides | As above, plus the adoption roadmap worksheet | |
| 12 | Handouts | Feature timeline, two cheat sheets, toolchain matrix, capstone README | |
| 13 | Dry run and polish | Trevor delivers each deck to a timer; slides cut or reordered; notes updated with what actually got asked | Each deck fits 90 minutes with 10 minutes of slack |

Exercises are built before the slides for each session on purpose. Writing the exercise first forces the feature set for the session to be concrete and tested, and the slides then explain exactly what the exercise uses. It also means the demos on the slides can be excerpts of the exercise solution.

### The per-session pattern

Each slide-building session follows the same steps. First, Claude produces a slide-by-slide outline (title and one line per slide) from the syllabus segment table, and Trevor edits it. Second, Claude writes the deck with notes and creates or links the demo files, then builds it with `marp-cli` and the snippet extractor and confirms every demo compiles under both compilers. Third, Trevor does a voice pass: rewording notes, cutting slides, adding war stories. Fourth, the deck is exported to PDF and committed.

For exercise sessions the pattern is: Claude writes the solution first, then derives the starter by reverting the target features, then writes the README and tests; CI proves that starter and solution both build and that the solution passes the tests.

---

## 6. Quality gates

Before any deck is delivered to a class:

- Every code sample compiles under GCC 14 and Clang 18 with `-Werror` (enforced by CI on every commit)
- Every feature slide carries the correct standard badge (enforced by `check-standard-tags.py`; badges verified against cppreference during review)
- Every feature on a slide appears in the toolchain support matrix with a status for both compilers
- Every slide has speaker notes
- The deck has been delivered once against a timer
- Prose contains no em dashes and the deck passes a spell check

Before any exercise is delivered:

- Starter builds and passes its tests unchanged
- Solution builds and passes its tests, and is identical to the next session's starter
- The README's "in class" tasks have been timed at 20 minutes or less by someone other than the author
- Session 5's solution is clean under ThreadSanitizer

---

## 7. Toolchain support matrix (to verify in working session 0)

The features most likely to cause trouble on a baseline of GCC 14 / libstdc++ and Clang 18 / libc++. Verified status goes in `handouts/toolchain-support-matrix.md`; the plan below records what needs checking and the fallback if a feature is missing.

| Feature | Concern | Fallback |
|---|---|---|
| `std::print` / `std::println` | Present in GCC 14; libc++ support arrived in Clang 17 but check flags | Use `std::format` + `std::cout` on the slide, note `print` |
| `std::generator` | libstdc++ has it in GCC 14; libc++ did not as of Clang 18 | Demo on GCC only; note on slide |
| `std::expected` | GCC 12+, Clang 16+ | None needed |
| `std::flat_map` / `flat_set` | Landed in GCC 15; not in Clang 18 libc++ | Show on Compiler Explorer with a trunk compiler |
| `std::mdspan` | GCC 14 lacks it; libc++ has it from Clang 17 | Show on Clang or Compiler Explorer |
| `std::stacktrace` | GCC needs `-lstdc++exp` (or `-lstdc++_libbacktrace` on older); libc++ lacks it | GCC-only demo |
| Modules and `import std;` | `import std` needs GCC 15 or Clang 17+ with libc++ and CMake 3.30+; named modules need CMake 3.28+ and Ninja | Slides show syntax; live demo on Compiler Explorer; repo includes a minimal module example marked experimental |
| Parallel algorithms | libstdc++ requires TBB; libc++ support is partial | Note on slide; demo with TBB installed in the Docker image |
| `std::jthread`, `latch`, `barrier`, `semaphore` | Fine on both | None |
| Ranges C++23 views (`zip`, `chunk_by`, `enumerate`, `to`) | GCC 13/14 good; libc++ has gaps in Clang 18 | Demo on GCC; matrix notes which views libc++ lacks |
| Deducing `this` | GCC 14, Clang 18 | None |
| `constexpr std::unique_ptr` | GCC 13+, Clang 17+ | None |

The cloud workspace used to verify the scaffold currently has GCC 13 and Clang 18, so anything that needs GCC 14 is confirmed on Trevor's Mac (Homebrew `gcc@14`) or in the Docker image rather than here.

---

## 8. Risks and mitigations

**Slide count creep.** C++20 alone could fill three decks. Mitigation: outline review before writing each deck, a hard 65-slide cap, and a "cut list" section in each deck's notes for material that was removed and can be reinstated if time allows.

**Code and slides drifting apart.** Mitigation: the snippet extractor and CI; nothing on a slide is typed by hand.

**Toolchain surprises in the room.** Mitigation: the Docker image and Compiler Explorer links on every demo slide, and a five-minute environment check at the start of Session 1.

**Exercise too long for 20 minutes.** Mitigation: explicit "in class" vs. "at home" split in every README, timed by a second person during working session 13.

**Reuse of existing material.** Trevor's `~/code` already contains `c++23`, `Modern-C-Course-Exercises`, `Class-Slides`, and `Pandoc-Themes`. Working session 0 should begin by looking through these; an existing theme or a set of C++23 examples could save several hours.

---

## 9. Immediate next steps

1. Connect a folder on the Mac for the repo (and optionally the existing `c++23` and `Class-Slides` folders for reuse).
2. Working session 0: commit the scaffold produced alongside this plan, install `marp-cli` and `gcc@14`, run the CMake build and the slide build once, and fill in the support matrix.
3. Working session 1: review the cumulative program's domain and starter code.
