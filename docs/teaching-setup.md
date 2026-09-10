# Teaching from one window

Slides, code, and terminal in a single VS Code window, so a Teams share never needs alt-tab.

## One-time setup

1. Install `marp-cli` (`npm install -g @marp-team/marp-cli`) and the recommended extensions when VS Code offers them (Marp, C/C++, CMake Tools).
2. Open the repo folder in VS Code. `.vscode/settings.json` sets the teaching zoom, font sizes, and hides the sidebar and status bar. Undo any of it with the Command Palette if it gets in the way.
3. Build once so the demos and exercise exist: Terminal > Run Build Task (`build + test`).

## Before each class

1. Terminal > Run Task > `slides: serve (live reload)`. This runs `marp -s` on `slides/`, serving each deck as HTML at `http://localhost:8080/`. Edits to the Markdown reload the page.
2. Command Palette > `Simple Browser: Show` > `http://localhost:8080/01-everyday-language.md`. The deck renders full-frame in an editor tab. Click inside it once; arrow keys, space, and Page Up/Down move between slides.
3. Drag that tab to the left editor group (or `View: Move Editor into New Group` and arrange). Open the session's demo file on the right (`demos/s01/...`). Open the terminal (Ctrl+`) at the bottom.
4. Optional: a second `Simple Browser: Show` with `https://godbolt.org` in the same group as the slides, so Compiler Explorer demos are a tab switch, not a window switch.
5. In Teams, share the **VS Code window**, not the screen. The layout is restored the next time you open the folder.

## Speaker notes

The Simple Browser cannot open Marp's presenter view (it needs a popup window). Two options:

- Run Task > `slides: export notes` writes `slides/out/<deck>.notes.txt`, one block per slide, for a second screen, a tablet, or a phone.
- Or keep the deck's Markdown open in a narrow third editor group and scroll it alongside; the notes are the HTML comments under each slide.

## Layout at a glance

```
+----------------------------+--------------------------+
|  Simple Browser            |  demos/s01/spaceship.cpp |
|  (slides, arrow keys)      |  (editor)                |
|  [tab: godbolt.org]        |                          |
+----------------------------+--------------------------+
|  Terminal: ./build/demos/s01/demo_s01_spaceship       |
+-------------------------------------------------------+
```

## Fallbacks

- If the Simple Browser refuses localhost, install Microsoft's **Live Preview** extension, which has its own embedded browser, and point it at `slides/out/01-everyday-language.html` after `bash slides/build.sh`.
- For a terminal-only setup (tmux, no VS Code), `presenterm` renders Markdown slides in the terminal and can run code blocks, but it does not understand this deck's HTML two-column layout; it would need a plain-Markdown variant of each deck.
