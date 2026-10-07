#!/usr/bin/env bash
# Regenerate the syllabus DOCX and PDF from cpp-evolution-syllabus.md (the markdown is the source).
set -euo pipefail
cd "$(dirname "$0")"
pandoc cpp-evolution-syllabus.md --from markdown-smart --to docx -o cpp-evolution-syllabus.docx
if SOFFICE="$(../tools/soffice.sh)"; then
  "$SOFFICE" --headless --convert-to pdf --outdir . cpp-evolution-syllabus.docx >/dev/null
fi
ls -1 cpp-evolution-syllabus.*
