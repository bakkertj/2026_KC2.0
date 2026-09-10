#!/usr/bin/env bash
# Build every deck to HTML and PDF in slides/out/.
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p out
for deck in 0*.md; do
  python3 ../tools/extract-snippets.py "$deck"
  marp --theme-set theme/course.css --html --allow-local-files -o "out/${deck%.md}.html" "$deck"
  marp --theme-set theme/course.css --html --allow-local-files --pdf -o "out/${deck%.md}.pdf" "$deck"
done
echo "Built: $(ls out)"
