#!/usr/bin/env bash
# Regenerate labs/<session>/<name>.expected from the built solutions (build/ by default).
# Run after changing a lab's solution on purpose; review the diff before committing.
set -euo pipefail
build="${1:-build}"
cd "$(dirname "$0")/.."
for src in labs/s0*/*_solution.cpp; do
  name="$(basename "${src%_solution.cpp}")"; session="$(basename "$(dirname "$src")")"
  exe="$build/labs/lab_${session}_${name}_solution"
  [ -x "$exe" ] || { echo "missing $exe (build first)" >&2; exit 1; }
  "$exe" > "labs/$session/$name.expected"
  echo "wrote labs/$session/$name.expected"
done
