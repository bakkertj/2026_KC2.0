#!/usr/bin/env bash
# Build every deck to HTML and PDF in slides/out/, and every outline to a paged document
# (DOCX and PDF). Outlines are review documents, not decks, so Marp must not touch them.
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p out
for deck in 0*.md supplemental-*.md; do
  case "$deck" in *-outline.md) continue ;; esac
  python3 ../tools/extract-snippets.py "$deck"
  marp --theme-set theme/course.css --html --allow-local-files -o "out/${deck%.md}.html" "$deck"
  marp --theme-set theme/course.css --html --allow-local-files --pdf -o "out/${deck%.md}.pdf" "$deck"
done
for outline in 0*-outline.md; do
  pandoc "$outline" --from gfm --to docx -o "out/${outline%.md}.docx"
done
if SOFFICE="$(../tools/soffice.sh)"; then
  "$SOFFICE" --headless --convert-to pdf --outdir out out/0*-outline.docx >/dev/null
fi
rm -f out/0*-outline.html
echo "Built: $(ls out)"
