#!/usr/bin/env bash
# Print the path of LibreOffice's soffice, looking beyond PATH on macOS. Exit 1 if absent.
if command -v soffice >/dev/null 2>&1; then command -v soffice; exit 0; fi
for p in "/Applications/LibreOffice.app/Contents/MacOS/soffice" \
         "$HOME/Applications/LibreOffice.app/Contents/MacOS/soffice" \
         "/opt/homebrew/bin/soffice" "/usr/local/bin/soffice"; do
  if [ -x "$p" ]; then echo "$p"; exit 0; fi
done
echo "LibreOffice (soffice) not found: DOCX files were written, PDFs skipped." >&2
echo "Install it with: brew install --cask libreoffice   (or from libreoffice.org)" >&2
exit 1
