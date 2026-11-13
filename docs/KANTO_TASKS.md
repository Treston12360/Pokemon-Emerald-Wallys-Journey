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

- [x] **K1. Compile Kanto maps.** Done 2026-11-13 (Claude session 019khs4X). mapjson filters and the `.if IS_FRLG`
      in `event_scripts.s` removed (as Crossroads); FRLG tilesets (`src/data/tilesets/*.h`) and FRLG object-event
      graphics (`src/data/object_events/*.h`, palettes in `event_object_movement.c`) always built; FRLG flags given
      real ids at `KANTO_FLAGS_START` (0x1000) kept in the new PC-only save block `SaveBlockPC` (no size limit,
      stored after the 128 KB GBA image in the .sav — old GBA saves load with Kanto clear); Celadon Game Corner
      coin checks written without spaces for the PC assembler. GBA data build and Windows/Linux PC build link.
- [ ] **K2. Tilesets load.** WIP with K3 (Claude session 014PwRDj, 2026-11-13). FRLG primary tilesets are 640 tiles / 640 metatiles / 7 palettes; Emerald's are
      512 / 512 / 6. The engine must handle both in one game (`include/fieldmap.h` has both sizes; see how
      Crossroads loads them).
- [ ] **K3. Warp into Pallet Town works** — WIP: Pallet Town (37 0) and Viridian City (37 1) load and draw in 2D
      with NPCs (2026-11-13). Still to check: the rest of the list below and 3D. (test script: `warp 37 0 10 10`; today it crashes because
      `gMapGroups` has no group 37). Then check Viridian, Vermilion, Mt. Moon, Pokémon Tower, Silph Co.,
      a route with grass, a building — 2D and 3D. Verify only; no need to show the owner.
- [ ] **K4. Flags, vars and save space.** Kanto story flags must not collide with Emerald's (Crossroads moved
      FRLG flags out of the trainer-flag range). NO save-size limits (Windows game, see AGENTS.md "NO LIMITS"):
      give Kanto its own save section, as big as needed. **Old GBA saves must still import.**
- [ ] **K5. Kanto scripts, specials, object events, heal locations, wild encounters, trainers work.**
- [ ] **K6. Region-aware Fly and region map** (Fly map can switch Hoenn/Kanto; normal Fly animation; no
      key item — owner decision).

- [ ] **K7. Kanto in 3D.** Kanto must work in the 3D view too (owner 2026-11-13). The 3D data is made by Emerald 3DS's
      generators (`tools/voxel3ds/README.md`); they skip every FRLG layout today ("WJTEST skipped layout ..._FRLG:
      IndexError" — their reader expects Emerald's 512/512 tileset split, see K2). Fix the reader for FRLG tilesets,
      then model Kanto's buildings/rooms the 3DS way (`voxel_building_specs.py`), and check Pallet, Viridian etc.
      in 3D. Do K2 first.

## Also open on the 3D side (branch `deluxe-3d`, see docs/DELUXE_WORK_QUEUE.md)
- **DLX-28 furniture in every house 3D** (owner 2026-11-13: "all houses furniture should be 3D"). 3DS left ~30
  rooms "walls only" (`PLAIN_ROOMS` in `voxel_building_specs.py`). Started: `dewford_hall()` in
  `tools/voxel3ds/our_changes.patch` — not generated or checked yet. Model each room's furniture with the
  reference's `piece()` / `house_chair()` / partition pieces (copy House1/House2/Briney rooms), measure pixels
  from the room's art, run `--only <room> --preview`, then regenerate and test in game.
- **DLX-22 black lines in Granite Cave** — still there on the owner's GPU (not visible in Mesa software GL).
  Full-width straight rows across the floor. Need a picture from the newest build.

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
