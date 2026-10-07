#!/usr/bin/env bash
# Export the speaking scripts to Word and PDF: scripts/out/sNN-script.{docx,pdf}
# Needs pandoc (markdown -> docx) and LibreOffice (docx -> pdf, so both look the same).
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p out
for md in s0?-script.md; do
  base="${md%.md}"
  pandoc "$md" --from gfm --to docx -o "out/$base.docx"
  echo "wrote out/$base.docx"
done
soffice --headless --convert-to pdf --outdir out out/s0?-script.docx >/dev/null
ls -1 out/*.pdf | sed 's/^/wrote /'
