# Instructions for future contributors and AI sessions

Before planning, editing, or reporting on Pokémon Emerald: Wally's Journey, read:

**[docs/WALLY_JOURNEY_OWNER_REQUIREMENTS.md](docs/WALLY_JOURNEY_OWNER_REQUIREMENTS.md)**

It is the authoritative, versioned record of the owner's requests and corrections. Update it whenever the owner changes or adds a requirement. Do not treat static checker presence as a completed or playable feature. Keep the project a GBA ROM hack and validate changes with compilation and emulator tests.

In case of disagreement with historical design notes, the owner's most recent requirement (and the authoritative requirements document, once updated) wins.

## Project Deluxe (this branch)
Read `docs/PROJECT_PC_PORT.md` before doing anything on this branch: plan, owner requests, rules, build and test.

## Project ARIA (Kanto expansion)
If you are asked to **"continue Project ARIA"** (or anything about Kanto, Varnell or ARIA): read
`docs/KANTO_TASKS.md` first and follow its rules — work on branch `kanto-port`, take the first unchecked task,
and tick tasks off in `docs/KANTO_TASKS.md` in the same push as the work. Story and every owner decision are in
`docs/KANTO_PROJECT_ARIA.md`.

## No guessing, ever (owner rule, 2026-11-13)
Port exactly how the reference does it (Emerald 3DS source for 3D, Gamma's files for lighting, FRLG/Crossroads code for Kanto). If the reference doesn't cover it, ask the owner. Never invent a fix, a value or a design.
