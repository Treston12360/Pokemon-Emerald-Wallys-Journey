# One game, two projects — shared task list

**Read this before you start any task, and update it in the same push as the work.** (Owner rule, 2026-11-13.)
Both projects check it so neither undoes the other: if the other project changed something that affects the
whole game (lighting, the 3D renderer, save code, the engine), read what it says under **Affects / don't revert**
before you touch that area.

- **Project ARIA — Treston** (Kanto). Branch `kanto-port`. Detailed list: `docs/KANTO_TASKS.md`.
- **Project Polish — Brennan** (existing game: Hoenn 3D, lighting, bugs). His branches.
  Detailed list: `docs/DELUXE_WORK_QUEUE.md` (DLX items).
- Brennan must not be told the Kanto story (see AGENTS.md). Keep story out of this file.

How to use:
1. Before starting: `git fetch`, read this file on **both** projects' branches, and `docs/WORK_CLAIMS.md`.
2. Starting a task: add it under your project as `[ ] WIP (who, date)`, push.
3. Finished: `[x]`, date, commit, and **Affects / don't revert** — what other parts of the game it changed
   and what the other project must keep. Push with the work.

---

## Project ARIA (Treston)

| Status | Task | Date / commit | Affects / don't revert |
|---|---|---|---|
| [x] | K1 Kanto maps compile into the game | 2026-11-13 | New PC-only save block `SaveBlockPC` (`include/global.h`, `src/event_data.c`, `src/save.c`, `src/platform/sdl2.c` extra save): **keep it** — Kanto flags/vars live there; old GBA saves still import |
| [x] | K2 Kanto tilesets (also in 3D) | 2026-11-13 | 3D renderer reads FireRed tile sizes (`voxel_world.c`, `voxel_atlas.c`, `voxel_entities.c`: `Voxel_IsFrlg` etc.) — keep when changing the renderer |
| [x] | K3 Kanto maps load (2D + flat 3D) | 2026-11-13 | — |
| [x] | K4 Kanto flags, vars, trainer ids in the PC save block | 2026-11-13 | `GetVarPointer`, trainer flags (`battle_setup.c`), script macros (`asm/macros/event.inc`) know the Kanto ranges |
| [x] | K5 Kanto scripts/doors/PCs/music/Safari/day care work | 2026-11-13 | `IN_KANTO_MAP` (`global.fieldmap.h`) replaces `IS_FRLG` in field code; both games' doors built (`field_door.c`); wild encounter tool builds FireRed's tables |
| [x] | K6 Fly map flips Hoenn/Kanto with L/R | 2026-11-13 | `src/region_map.c` (forced flight region); Fly map now resets sprites on exit |
| [x] | DLX-28 3D furniture in ~25 Hoenn houses | 2026-11-13 | `pc_data/voxel/buildings.bin` regenerated from `tools/voxel3ds/our_changes.patch` — **regenerate from that patch, don't replace with an older buildings.bin** |
| [x] | Haze weaker (0.5 → 0.35), belt sea all flat (owner) | 2026-11-13 | `voxel_gl.c` haze value, `voxel_mesh_builder.c` `BorderWaterLevel` |
| [x] | Indoor bloom off, heal balls on the machine, Dewford border fix (DLX-23/24/25) | 2026-11-13 | `voxel_gl.c`, `field_effect.c`, `wide_view.c`, `sdl2.c`, `voxel_world.c` |
| [x] | Full-folder builds: `tools/pc/make_package.sh`, `pc_data/voxel/` in git | 2026-11-13 | Every build uses this |
| [ ] | S0 strip FireRed's own story from Kanto maps — WIP (Claude 019khs4X, 2026-11-13). steps 1–3 done (Pallet→Pewter, all Rockets/Blue, Bill, Rocket-era blocks); log in `docs/KANTO_S0_STRIPPED.md` | 2026-11-13 | Kanto map scripts; new `SaveBlockPC.kantoMapFlagsSet` + `SaveBlockPC_InitKanto()` (`event_data.c`, `new_game.c`) runs once per save — keep |
| [ ] | S1+ story chapters (script approved for ch. 0–1) | | Ch. 0 touches Littleroot/Birch's lab/Slateport harbour scripts (Hoenn maps) — will claim first |
| [ ] | K7 Kanto in 3D — paused (story first) | | Will regenerate `pc_data/voxel/*` — claim first |

## Project Polish (Brennan)

| Status | Task | Date / commit | Affects / don't revert |
|---|---|---|---|
| [ ] | (Brennan's Claude: add your tasks here) | | |

Open items handed to Polish (Hoenn): DLX-22 Granite Cave black lines; Hoenn trainers' repetitive lines
(owner: every trainer needs believable, unique dialogue — `WALLY_JOURNEY_OWNER_REQUIREMENTS.md` 2026-11-13);
Fortree tree-trunk furniture and the Fortree decoration shop benches (DLX-28 leftovers).
