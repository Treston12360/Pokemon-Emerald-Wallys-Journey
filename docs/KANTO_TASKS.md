# Project ARIA — Kanto task list

Owner's handoff list for the Kanto expansion. To hand work to any AI, say:
**"Continue Project ARIA"** (repo `Treston12360/Pokemon-Emerald-Wallys-Journey`, file `docs/KANTO_TASKS.md`).

## Rules for every session (people and AIs)

1. Read first, in order: `docs/KANTO_PROJECT_ARIA.md` (story + design, every owner decision),
   `docs/kanto_mockups/README.md` (mockups, test harness, Crossroads references),
   `docs/PROJECT_PC_PORT.md` (build/test), `docs/WALLY_JOURNEY_OWNER_REQUIREMENTS.md` (owner rules).
2. Work on branch **`kanto-port`**. More than one person/AI works on this: `git fetch` and rebase before every
   push, and push small commits often.
3. Pick the **first unchecked task** below (or the one marked WIP). Mark it `WIP (who, date)` and push before
   starting, so nobody else starts the same task.
4. When a task is done and tested: change `[ ]` to `[x]`, add the date and commit hash, and push **in the
   same push** as the work. If you only got partway, leave it unchecked and write exactly where you stopped.
5. Owner rules: GBA saves must still import; show the owner pictures and ask before new wording or visual
   changes; no new hand-drawn art (recolours OK); the level curve must be checked with the simulator.
6. New decisions from the owner go into `docs/KANTO_PROJECT_ARIA.md` right away.
7. Credit: the Kanto-in-Emerald port follows Pokémon Crossroads (github.com/eonlynx/pokecrossroads,
   Beta 1.4, permission from eonlynx). Read their code for each step; don't copy files wholesale — we are
   on pokeemerald-expansion 1.17.2, they are on a newer snapshot.

## Phase 1 — Get Kanto into the Emerald build (port from Crossroads)

- [ ] **K1. Compile Kanto maps.** WIP (Claude session 019khs4X, 2026-11-13). Done so far on `kanto-port`:
      removed the "skip other game's maps" filters in `tools/mapjson/mapjson.cpp` (3 places, same as
      Crossroads) and the `.if IS_FRLG` block around the Kanto scripts in `data/event_scripts.s`.
      **Not yet built or tested.** Next: `make -C tools/mapjson`, `make release`, fix the build errors
      (expect FRLG-only flags/vars/specials/text used by Kanto scripts), until it builds.
- [ ] **K2. Tilesets load.** FRLG primary tilesets are 640 tiles / 640 metatiles / 7 palettes; Emerald's are
      512 / 512 / 6. The engine must handle both in one game (`include/fieldmap.h` has both sizes; see how
      Crossroads loads them).
- [ ] **K3. Warp into Pallet Town works** (test script: `warp 37 0 10 10`; today it crashes because
      `gMapGroups` has no group 37). Then check Viridian, Vermilion, Mt. Moon, Pokémon Tower, Silph Co.,
      a route with grass, a building — 2D and 3D. Verify only; no need to show the owner.
- [ ] **K4. Flags, vars and save space.** Kanto story flags must not collide with Emerald's (Crossroads moved
      FRLG flags out of the trainer-flag range). NO save-size limits (Windows game, see AGENTS.md "NO LIMITS"):
      give Kanto its own save section, as big as needed. **Old GBA saves must still import.**
- [ ] **K5. Kanto scripts, specials, object events, heal locations, wild encounters, trainers work.**
- [ ] **K6. Region-aware Fly and region map** (Fly map can switch Hoenn/Kanto; normal Fly animation; no
      key item — owner decision).

## Phase 2 — Story (see `docs/KANTO_PROJECT_ARIA.md` chapters)

- [ ] **S0. Remove FRLG's own story start** (Oak's parcel, rival intro etc.) where it conflicts.
- [ ] **S1. Chapter 0 — the call:** Mom → Birch → S.S. Anne ticket; Wally's send-off at Slateport.
- [ ] **S2. Chapter 0b — arrival:** S.S. Anne to Vermilion, fade to black, Oak's lab in Pallet.
- [ ] **S3. Chapter 1 — the investigation team:** Oak, Varnell, Blue; first assignment (Mt. Moon).
- [ ] S4+. Remaining chapters, one task each, added as they're reached.

## Phase 3 — Levels and balance

- [ ] **L1. Hoenn post-game rules:** remove May/Brendan lab battle (and from the E4-rematch unlock); lower
      Steven to Wally's strength; Gym Leader rematch EXP 5% (or none); repeat Elite Four runs 5% EXP;
      Champion defenses locked until Kanto is beaten.
- [ ] **L2. Re-run `tools/exp_curve_full.py`** with L1's rules; update the arrival level in the design doc.
- [ ] **L3. Kanto trainer and wild levels** per the design doc; check with `tools/exp_curve_kanto.py`.
- [ ] **L4. Kanto Gym Leaders / Elite Four / important teams** (Kanto families only, no regional forms) —
      show the owner before adding.

## Done

(nothing yet)
