#!/usr/bin/env bash
# Scaffold a demo file and register it with CMake.
# Usage: tools/new-demo.sh s02 string_view "C++17"
set -euo pipefail
session=$1; name=$2; std=${3:-C++20}
root="$(cd "$(dirname "$0")/.." && pwd)"
f="$root/demos/$session/$name.cpp"
[ -e "$f" ] && { echo "exists: $f"; exit 1; }
cat > "$f" <<EOF
// Demo: $name ($std)
// Session: $session
// Compiler Explorer: <add short link>
// Slide: slides/0${session#s0}-*.md
#include <print>

// [snippet: main]
int main() {
    std::println("$name");
}
// [/snippet]
EOF
echo "add_demo($session $name)" >> "$root/demos/$session/CMakeLists.txt"
echo "created $f"
