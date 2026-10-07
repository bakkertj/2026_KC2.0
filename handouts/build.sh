#!/usr/bin/env bash
# Export every handout to Word and PDF in handouts/out/ (links stay clickable in both).
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p out
for md in *.md; do
  pandoc "$md" --from markdown-smart --to docx -o "out/${md%.md}.docx"
done
soffice --headless --convert-to pdf --outdir out out/*.docx >/dev/null
echo "Built: $(ls out)"
