#!/usr/bin/env bash
# Package what attendees get for a session into dist/session-N.zip:
#   the deck (PDF), the demo sources, the exercise (starter, tests, data, README; the solution
#   only with --with-solution), and every handout as PDF and DOCX.
# Never included: the speaking scripts, the outlines, the syllabus, the plan.
#
#   tools/package.sh 3                  # dist/session-3.zip
#   tools/package.sh all                # one zip per session
#   tools/package.sh 3 --with-solution  # add exercises/s03-*/solution
#
# Builds the deck PDF and the handouts first if they are missing (needs marp, pandoc, LibreOffice).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

which="${1:-}"; shift || true
with_solution=0
for arg in "$@"; do [ "$arg" = "--with-solution" ] && with_solution=1; done
[ -n "$which" ] || { echo "usage: tools/package.sh <1-5|all> [--with-solution]" >&2; exit 2; }

package_one() {
  local n="$1" s="s0$1"
  local deck; deck="$(ls slides/0"$n"-*.md | grep -v outline)"
  local pdf="slides/out/$(basename "${deck%.md}").pdf"
  if [ ! -f "$pdf" ]; then
    python3 tools/extract-snippets.py "$deck"
    marp --theme-set slides/theme/course.css --html --allow-local-files --pdf -o "$pdf" "$deck"
  fi
  [ -n "$(ls handouts/out/*.pdf 2>/dev/null)" ] || bash handouts/build.sh >/dev/null

  local stage; stage="$(mktemp -d)"
  local dir="$stage/session-$n"
  mkdir -p "$dir/slides" "$dir/demos" "$dir/exercise" "$dir/handouts"

  cp "$pdf" "$dir/slides/"
  [ "$n" = 1 ] && [ -f slides/out/supplemental-value-categories.pdf ] && cp slides/out/supplemental-value-categories.pdf "$dir/slides/"

  cp demos/"$s"/*.cpp "$dir/demos/"
  [ -d demos/"$s"/modules ] && cp -r demos/"$s"/modules "$dir/demos/"

  local ex; ex="$(ls -d exercises/"$s"-*)"
  cp "$ex"/README.md "$dir/exercise/"
  cp -r "$ex"/starter "$dir/exercise/"
  [ -d "$ex"/tests ] && cp -r "$ex"/tests "$dir/exercise/"
  [ -d "$ex"/data ] && cp -r "$ex"/data "$dir/exercise/"
  [ "$with_solution" = 1 ] && cp -r "$ex"/solution "$dir/exercise/"
  cp exercises/common/doctest.h "$dir/exercise/"

  cp handouts/out/*.pdf handouts/out/*.docx "$dir/handouts/"

  cat > "$dir/README.md" <<README
# Session $n materials

- slides/: the deck as PDF
- demos/: the source of every code example on the slides; each file's header has a Compiler
  Explorer link, and handouts/compiler-explorer-links.pdf lists them all (GCC 14, -std=c++23)
- exercise/: the in-class exercise (README.md, starter/, tests/, data/)$( [ "$with_solution" = 1 ] && echo ", with solution/" )
- handouts/: cheat sheets, the feature timeline, the toolchain support matrix, the adoption
  roadmap worksheet, and the Compiler Explorer links index, as PDF and DOCX

The demos and the exercise build with the course repository's CMake setup
(GCC 14 or Clang 18, CMake 3.28+); the demos also run as-is on Compiler Explorer.
README

  mkdir -p dist
  rm -f "dist/session-$n.zip"
  (cd "$stage" && zip -qr "$ROOT/dist/session-$n.zip" "session-$n")
  rm -rf "$stage"
  echo "wrote dist/session-$n.zip ($(unzip -l "dist/session-$n.zip" | tail -1 | awk '{print $2}') files)"
}

if [ "$which" = all ]; then for n in 1 2 3 4 5; do package_one "$n"; done; else package_one "$which"; fi
