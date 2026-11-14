# Instructions for future contributors and AI sessions

Before planning, editing, or reporting on Pokémon Emerald: Wally's Journey, read:

**[docs/WALLY_JOURNEY_OWNER_REQUIREMENTS.md](docs/WALLY_JOURNEY_OWNER_REQUIREMENTS.md)**

It is the authoritative, versioned record of the owner's requests and corrections. Update it whenever the owner changes or adds a requirement. Do not treat static checker presence as a completed or playable feature. Validate changes by building and testing the Windows game. See "NO LIMITS" below.

In case of disagreement with historical design notes, the owner's most recent requirement (and the authoritative requirements document, once updated) wins.

## Project Deluxe (this branch)
Read `docs/PROJECT_PC_PORT.md` before doing anything on this branch: plan, owner requests, rules, build and test.

## Project ARIA (Kanto expansion)
If you are asked to **"continue Project ARIA"** (or anything about Kanto, Varnell or ARIA): read
`docs/KANTO_TASKS.md` first and follow its rules — work on branch `kanto-port`, take the first unchecked task,
and tick tasks off in `docs/KANTO_TASKS.md` in the same push as the work. Story and every owner decision are in
`docs/KANTO_PROJECT_ARIA.md`.

## No story spoilers in chat (owner rule, 2026-11-13)
One of the people working on this game does not want Project ARIA's story spoiled. In anything you write or
show to the person you are working for — replies, summaries, pictures, file captions — say nothing of the
Kanto plot: who anyone is, what happens, twists, endings, lines of dialogue. Talk about the work instead
("chapter 2's script is in", "the lab map loads"). The only exception is the story's writer: **the repo owner (Treston) wrote the story and may discuss it
freely** — the rule protects his friend, who works on the game but should not have the Kanto story spoiled. Story questions never go to the chat: write them into
`docs/KANTO_STORY_QUESTIONS.md` and push; the writer answers there.

## Who works on what — never cross work or overwrite each other (owner rule, 2026-11-13)
Two people, each with their own Claude, work on this repo at the same time:
- **The owner (Treston) → Project ARIA (Kanto).** Branch `kanto-port` only. Task list: `docs/KANTO_TASKS.md`.
- **His friend → polishing the game's existing (Hoenn) features.** His own branches, not `kanto-port`.

Rules for every session:
1. Find out which of the two you are working for (ask if it isn't clear) and stay in that lane and branch.
2. Before editing a file the other lane also uses (engine code in `src/` outside Kanto data, the 3D renderer
   `src/platform/voxel3d/`, save code, `include/`, `AGENTS.md`), check `docs/WORK_CLAIMS.md`. If the other
   lane has claimed it, don't edit it — write what you need in that file and tell the person you work for.
   If not, add your claim (file or area, who, date), push, then edit; remove the claim when you are done.
3. `git fetch` and rebase before every push; never force-push; never rewrite or revert the other lane's
   commits. If a rebase conflicts in the other lane's work, stop and ask the person you work for.
4. Small commits, pushed often, so the other lane sees your changes early.

## No guessing, ever (owner rule, 2026-11-13)
Port exactly how the reference does it (Emerald 3DS source for 3D, Gamma's files for lighting, FRLG/Crossroads code for Kanto). If the reference doesn't cover it, ask the owner. Never invent a fix, a value or a design.

## NO GBA ROM ANY MORE — the game is the .exe (owner rule, 2026-11-13)
"We are not making a GBA ROM anymore. The game is now a .exe game. ... Focus on the .exe and not work on the
GBA ROM anymore." So: do not build the ROM to test it, do not check that the ROM still works, do not send
`.gba` files, do not keep code or data small or GBA-shaped for the ROM's sake, and do not write `#ifdef`s to
keep the ROM unchanged. All testing is on `WallysJourney.exe` (and the Linux build of the same code).
`make release` is still run, only because it generates data files the PC build needs (map data, trainer
tables); if it stops linking as a ROM, fix only what the PC build needs from it. Old GBA saves must still
import (below).

## NO LIMITS — Windows game (owner rule, 2026-11-13, repeated 3 times; never forget)
Wally's Journey + Project ARIA is ONE game, and it is a native Windows game now. It started as a GBA ROM hack,
but there are **no GBA limits and no 3DS limits** of any kind: save size, flags, vars, SaveBlocks, flash
sectors, tiles, palettes, VRAM, sprites, vertex budgets, ROM size — none of them apply. Make anything as big
as it needs to be. The ONE hard rule: old GBA saves (.sav from the ROM hack) must still import into the PC
game. Saving back to GBA format is NOT needed. Never squeeze a feature to fit an old limit; if any doc says
otherwise, it is wrong — fix the doc. (Porting *from* the 3DS/Gamma/FRLG references still applies: copy how
they look and work, not their hardware limits.)
