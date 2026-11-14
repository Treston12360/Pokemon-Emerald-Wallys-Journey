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
5. Owner rules: the game is the Windows .exe — no GBA ROM is made or tested any more (AGENTS.md); old GBA saves
   must still import; show the owner pictures and ask before new wording or visual
   changes; no new hand-drawn art (recolours OK); the level curve must be checked with the simulator.
6. **No story spoilers in chat** (AGENTS.md): one of the owners does not want the story spoiled. Report work
   without plot details; put story questions in `docs/KANTO_STORY_QUESTIONS.md`, not in the chat.
   New decisions from the owner go into `docs/KANTO_PROJECT_ARIA.md` right away.
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
- [x] **K2. Tilesets load.** Done 2026-11-13 (Claude session 014PwRDj). 2D needed nothing: the engine already
      takes the split from `mapLayout->isFrlg` (`fieldmap.c`), and the zoomed-out view asks the same functions.
      The 3D view had Emerald's split (512 metatiles / 512 tiles / 6 palettes) and 16-bit attributes fixed in:
      now `Voxel_IsFrlg` / `Voxel_PrimaryMetatiles` / `Voxel_PrimaryTiles` / `Voxel_PrimaryPals` (`voxel_world.c`,
      a tile set is noted as Kanto's when a map using it is placed) are used by `voxel_world.c`, `voxel_atlas.c`
      (`AtlasSource.split*`) and the door picture in `voxel_entities.c`; Kanto's 32-bit attributes are handed on
      in Emerald's 16-bit form.
- [x] **K3. Warp into Pallet Town works.** Done 2026-11-13 (sessions 019khs4X + 014PwRDj). Checked by test script
      (`warp G N X Y`, pictures looked at), 2D: Pallet, Viridian, Pewter, Cerulean, Lavender, Vermilion, Celadon,
      Fuchsia, Cinnabar, Saffron, Indigo Plateau, Routes 1 and 22, Viridian Forest, Mt. Moon 1F, Rock Tunnel 1F
      (dark, needs Flash, as FRLG), Seafoam 1F, Pokémon Tower 1F, Silph Co. 1F, the player's house, Viridian
      Pokémon Center — all load with the right tiles and people, no crash. 3D: Pallet, Viridian, Vermilion,
      Celadon, Route 1, Mt. Moon, Seafoam, Pokémon Tower, Silph Co., the Pokémon Center draw with the right
      tiles, **flat** (no building, tree or height data for Kanto yet: that is K7).
      Seen on the way: FRLG's own story scripts still fire (Bill talks on arriving in Cerulean) — S0.
      **Build note:** after `make release` regenerates the map data, the PC build does not notice
      (`Makefile.pc` has `.SECONDARY`): delete `build/pc-linux32/data/*.o`, `build/pc-win32/data/*.o` and the
      exe, run `make -f Makefile.pc WIN=0 linux-data -j8` on its own first (run under `-j16` from the Windows
      build it races), then the Windows build. Symptom of stale data: crash in `Overworld_GetMapHeaderByGroupAndId`.
- [x] **K4. Flags, vars and save space.** Done 2026-11-13 (Claude session 014PwRDj; flags were K1's). Kanto has its
      own numbers and its own room in the PC save block (`struct SaveBlockPC`, `include/global.h`; grows at the end):
      - **Flags** `KANTO_FLAGS_START` 0x1000 (768) -> `kantoFlags` (K1).
      - **Vars**: FireRed's vars sat on Emerald's numbers (both from 0x4020). The 211 FireRed-only ones are now
        `KANTO_VAR(0x40NN)` = 0x5000 + NN (`include/constants/vars_frlg.h`) -> `kantoVars[256]`
        (`GetVarPointer`); names Emerald also has stay Emerald's var. The script macros that tell a var from a
        number (`setvar` warning, `copyvar`, `compare`, `asm/macros/event.inc`) know the Kanto range.
      - **Trainers**: FireRed's ids 1-623 were Hoenn trainers' ids. Now `KANTO_TRAINER(n)` = 2048 + n
        (`include/constants/opponents_frlg.h`), `TRAINERS_COUNT` 2816, both `.party` files in `gTrainers`
        (`src/data.c`), "defeated" flags -> `kantoTrainerFlags` (`battle_setup.c` TrainerFlagGet/Write).
      - These constants pass through the script assembler's macros: **no spaces inside them**, and script
        arguments need their commas (one FRLG script had none).
      - Checked with the new test-script command `state WHAT ID [VALUE]` (var / flag / trainer / save; answers
        in `state.log`): an old GBA-size save loads with Kanto all clear; writing Kanto var 0x5050, flag 0x1003
        and trainer 2362 leaves Emerald's var 0x4050 and trainer 314 alone; after saving (file grows by the PC
        block) and loading again all of them are still set. `trainerbattle 2362` is Leader Brock with his
        FireRed team.
- [x] **K5. Kanto scripts, specials, object events, heal locations, wild encounters, trainers work.** Done 2026-11-13
      (session 019khs4X). Checked in game: Route 1 wild battle (FireRed's table, Lv3 Rattata), Viridian Pokémon
      Center nurse (full heal script), Viridian Forest Bug Catcher Rick (sight, intro, battle), walking into the
      Viridian Pokémon Center door. Changes: FireRed's wild tables built in (`tools/wild_encounters/
      wild_encounters_to_header.py`, as Crossroads `FRLG_KANTO_MAP_WILD_PKMN = FIRE_RED`; LeafGreen's left out);
      both games' door animations (`field_door.c`, matched by tileset); new `IN_KANTO_MAP` (run-time "this map is
      FireRed's", `global.fieldmap.h`) instead of `IS_FRLG` for: Pokémon Center PCs, cycling/surf music, TV/food/
      machine signs, the after-white-out heal script, Safari Zone steps window and step count, the {REGION} text;
      new special `IsInKantoMap` used by `data/scripts/safari_zone.inc` (which Safari Zone to leave to); Route 5
      day care kept in `SaveBlockPC.route5DayCareMon`.
      **Left on purpose (later tasks):** `{RIVAL}` text still gives May/Brendan (rival name is story, S0); the PC is
      still "Lanette's PC" in Kanto (wording — ask owner); HM badge checks stay Hoenn's (owner: HMs carry over);
      Trainer Tower / Sevii code still `IS_FRLG` (Sevii decided later); cable club attendants Emerald's.
- [x] **K6. Region-aware Fly and region map.** Done 2026-11-13 (session 019khs4X). Owner's choice: "switch on the
      map" — the Fly map opens on the region you are in; **L or R flips to the other region** once one of that
      region's fly spots has been visited (until then the button does nothing). Ported from Crossroads' Flight Call
      (`sUseForcedFlightRegion`, `GetActiveRegionMapType`, `InitMapForForcedFlightRegion` in `src/region_map.c`),
      no key item; normal Fly. No player head is drawn on the other region's map. Checked in game: Dewford → Fly
      map → R shows Kanto (Pallet Town) → L back to Hoenn; R then A on Pallet Town flies there. Test-script command
      `flymap` opens the Fly map from the field. Not done: no on-screen hint that L/R switch (wording/visual — ask
      the owner first).
- [ ] **K7. Kanto in 3D.** Kanto must work in the 3D view too (owner 2026-11-13). The 3D data is made by Emerald 3DS's
      generators (`tools/voxel3ds/README.md`); they skip every FRLG layout today ("WJTEST skipped layout ..._FRLG:
      IndexError" — their reader expects Emerald's 512/512 tileset split, see K2). Fix the reader for FRLG tilesets,
      then model Kanto's buildings/rooms the 3DS way (`voxel_building_specs.py`), and check Pallet, Viridian etc.
      in 3D. Do K2 first.

## Also open on the 3D side (branch `deluxe-3d`, see docs/DELUXE_WORK_QUEUE.md)
- **DLX-28 furniture in every house 3D** — mostly done 2026-11-13 (session 019khs4X), checked in game: Dewford hall
  (table, partition, bath wall), House3/4 + Lilycove house + Tunneler's + Trick House entrance + Cove Lily Motel (low
  tables, cushions, jars), Pacifidlog 1/2 + Cutter's (table, chairs, kitchen), Steven's (table, cases), Rustboro
  houses/flats + Sootopolis houses (tables, a bed), Wanda's (partition, bed), Fossil Maniac (case, bookcase),
  Pokémon School (desks). In `tools/voxel3ds/our_changes.patch` (`PLAIN_FURNITURE` etc.); passes `--verify` except
  the 4 lint failures the reference already had (Briney's room etc.). **Left:** the Fortree houses' tree trunk
  (`fortree_room()` written but fails lint "under" — not hooked up), Fortree decoration shop benches, flower shop
  counter (3DS left it flat on purpose). The shipped `voxel/buildings.bin` is regenerated from this patch.
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
