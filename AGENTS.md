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
("chapter 2's script is in", "the lab map loads"). The only exception is the story's writer: **Treston (the repo owner) wrote the story and may discuss it
freely. Brennan must never be told any of it** (no plot, twists, character reveals, dialogue — in chat,
pictures, file captions or commit messages to him). Story questions never go to the chat: write them into
`docs/KANTO_STORY_QUESTIONS.md` and push; the writer answers there.

## Who works on what — two projects, one game; never cross work or overwrite each other (owner rule, 2026-11-13)
Two people, each with their own Claude, work on this repo at the same time:
- **Project ARIA — Treston.** Kanto: maps, story, scripts, dialogue, trainers, Kanto 3D. Branch `kanto-port`.
  Task list: `docs/KANTO_TASKS.md`.
- **Project Polish — Brennan.** The existing game: Hoenn 3D, lighting, and bugs he finds. His own branches
  (not `kanto-port`). He must not be told the Kanto story (above).
Both projects feed the one game.

Rules for every session:
1. Find out which of the two you are working for (ask if it isn't clear) and stay in that project and branch.
2. **Check the shared task list before you start any task: `docs/TASKS.md`** (both projects, both branches —
   `git fetch` first), plus `docs/WORK_CLAIMS.md`. Read the other project's finished tasks' "Affects / don't
   revert" notes before touching those areas (e.g. Polish changes lighting → it affects Kanto too; keep it).
   Add your task there when you start it and check it off with its "Affects" note in the same push as the work.
3. Before editing a file both projects use (engine code in `src/` outside Kanto data, the 3D renderer
   `src/platform/voxel3d/`, `pc_data/voxel/`, save code, `include/`, `AGENTS.md`), check `docs/WORK_CLAIMS.md`.
   If the other project has claimed it, don't edit it — write what you need there and tell the person you work
   for. If not, add your claim (file or area, who, date), push, then edit; remove the claim when done.
4. `git fetch` and rebase before every push; never force-push; never rewrite or revert the other project's
   commits. If a rebase conflicts in the other project's work, stop and ask the person you work for.
5. Small commits, pushed often, so the other project sees your changes early.

## Builds you hand over: always the WHOLE game folder (owner rule, 2026-11-13)
Never send a `.gba` file, and never send the `.exe` alone. Every build for Treston or Brennan is the complete
Windows folder, zipped: `make -f Makefile.pc WIN=1`, then `tools/pc/make_package.sh NAME` (exe, SDL2 DLLs,
`voxel/` 3D data from `pc_data/voxel/`, README, launcher). They should only have to drop in their own settings
file (`WallysJourney.cfg`) and save (`WallysJourney.sav`). If you regenerate the 3D data, copy it into
`pc_data/voxel/` and commit it, so every build carries the current one.

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
