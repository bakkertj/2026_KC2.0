# Speaking scripts

One spoken script per session, slide by slide, in deck order. They are written for rehearsal and for keeping open in a third editor group while presenting (see `docs/teaching-setup.md`).

| Session | Script | Slides |
|---|---|---|
| 1. The Everyday Language | `s01-script.md` | 55 |
| 2. Vocabulary Types | `s02-script.md` | 59 |
| 3. Compile Time and Generic | `s03-script.md` | 51 |
| 4. Ranges | `s04-script.md` | 49 |
| 5. Concurrency, Coroutines, Modules | `s05-script.md` | 53 |

Each script has: a pace plan with clock checkpoints and the slides to slow down on, a before-class checklist, the per-slide talk track (bold must-say lines, `(pause)` cues, `>> DO` demo steps, `>> ASK` questions, `>> IF AHEAD` / `>> IF BEHIND` flex material), exercise coaching, likely questions with answers, and a **Deck issues found** list at the end.

The talk track is sized at about 170 words per minute. Spoken faster, it runs short by design; the `IF AHEAD` blocks are the reserve.

## Deck fixes applied (2026-10-02)

Every item in the scripts' "Deck issues found" sections has been applied to the decks, demo files, outlines, exercise READMEs, handouts and syllabus, except where the issue itself turned out to be wrong (noted in the script). Slide numbering and the segment clock did not change, so the scripts' checkpoints still hold. Things that still need you:

- `handouts/cheat-sheet-ranges.md` and `cheat-sheet-vocabulary-types.md` were written on 2026-10-02 from the decks; read them once before Sessions 2 and 4
- Session 5 README: tasks 1 to 3 are now labeled at-home work (the deck has no slot); the in-class exercise is the roadmap worksheet, now 8 minutes
- Sessions 2 and 4: the in-class exercise is labeled 15 and 10 minutes respectively, to match the clock
- Rebuild and run the tests after pulling these edits; demos were syntax-checked here with GCC 13 and Clang 18, not the GCC 14 / libc++ pair the repo targets

Exercise `starter/` and `solution/` trees were read for the fixes, but the scripts' expected outputs ("seven rejections" and similar) still deserve a rehearsal check.
