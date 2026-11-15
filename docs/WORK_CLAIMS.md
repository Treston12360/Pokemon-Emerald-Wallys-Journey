# Work claims — files and areas being changed right now

See AGENTS.md "Who works on what". Add a line before you edit a shared file or area; remove it when you are
done (in the same push as the work). Projects: **ARIA** (Treston, branch `kanto-port`) and **POLISH** (Brennan, his branches).

| Lane | Who / session | File or area | Since | What for |
|---|---|---|---|---|
| ARIA | Claude 019khs4X (Treston) | `tools/voxel3ds/` (3DS generator patch) and `pc_data/voxel/` | 2026-11-13 | K7 Kanto in 3D, DLX-28 furniture (paused for story) |
| ARIA | Claude 019khs4X (Treston) | L1 Hoenn post-game rules: `src/battle_script_commands.c` (EXP), `src/wally_journey.c` (Steven, E4 unlock), `data/scripts/wally_journey.inc` (rival final), Champion's Room defenses | 2026-11-14 | Owner's 2026-11-13 post-game rules before Kanto |
| ARIA | Claude 019khs4X (Treston) | Hoenn maps: Littleroot (player's houses 1F/2F, town), Birch's lab, Slateport harbour; new `data/scripts/aria_*.inc` | 2026-11-14 | S1 Project ARIA ch. 0 (post-Champion call to Kanto). Only runs after the first Champion win — earlier saves unchanged |
| POLISH | Claude 019khs4X (Treston, helping Polish) | branch `polish`: `src/platform/voxel3d/voxel_gl.c` + `voxel_world.c` (3D night, POL-26b) + one public wrapper in `src/overworld.c`/`include/overworld.h` (WJ_WindowsLit) + `voxel_entities.c` (night lamps; Treston OK'd taking it over 2026-11-15) + `voxel_sign.c` (lamp pool wrapper). Treston OK'd taking voxel_gl.c over (2026-11-14); 014PwRDj: re-check `git log` on voxel_gl.c before editing it | 2026-11-14 | Treston's POL-26b (3D night) |
| POLISH | Claude 014PwRDj (Brennan) | branch `polish`: `src/platform/voxel3d/voxel_gl.c`, `voxel_entities.c` (NOTE 2026-11-15: Treston OK'd Claude 019khs4X taking voxel_gl.c and voxel_entities.c over for POL-26b - check `git log` on them before editing) (freezes, crash, memory — POL-11/14); HM forget checks (`src/pokemon.c`/summary/move relearner — POL-25) | 2026-11-14 | Brennan's priority fixes, `docs/POLISH_TASKS.md` |
