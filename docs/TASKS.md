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
| [x] | S0 strip FireRed's own story from Kanto maps (steps 1–5) | 2026-11-14 | Kanto map scripts; `SaveBlockPC.kantoMapFlagsSet` = start steps applied (`SaveBlockPC_InitKanto`) — never edit a released step, add one |
| [x] | S-LEAGUE (except Janine): Blue Viridian Gym, Koga E4 4th room, Lance Champion + Hall of Fame. Dialogue DRAFT (owner to approve), teams FireRed placeholders (L4). Janine waits for a sprite the owner approves | 2026-11-14 | Kanto League maps + trainers_frlg.party (new TRAINER_LEADER_BLUE 624, TRAINER_ELITE_FOUR_KOGA 625) |
| [x] | Kanto badges + HM licences (S-HM) | 2026-11-14 | `FLAG_KANTO_BADGE0x_GET` (Kanto flag space) used by all FireRed gym scripts; `src/field_move.c` checks Kanto badges in Kanto maps only — Hoenn field moves unchanged. **Keep.** |
| [x] | Seasons message names the region (ARIA, 2026-11-14) | 2026-11-14 | `data/scripts/wally_journey.inc` season texts use `{REGION}` (Kanto in Kanto maps, Hoenn otherwise); 'grey' -> 'gray' |
| [ ] | Story chapters — 0, 0b, 1, 2, 3, 3b, 4 IN GAME (2026-11-14); next ch. 5 Celadon | 2026-11-14 | Ch. 0 touches Littleroot/Birch's lab/Slateport harbour scripts (Hoenn, post-game only) |
| [ ] | K7 Kanto in 3D — paused (story first) | | Will regenerate `pc_data/voxel/*` — claim first |

## Project Polish (Brennan)

| Status | Task | Date / commit | Affects / don't revert |
|---|---|---|---|
| [ ] | (Brennan's Claude: add your tasks here) | | |

## Whole-game engine tasks (either project — claim in `docs/WORK_CLAIMS.md` first; they touch shared code)

| Status | Task | Date / commit | Affects / don't revert |
|---|---|---|---|
| [ ] | **ENG-MP3. Music tracks can be MP3 files** (owner, 2026-11-14). Let any song play from an MP3 instead of the GBA music engine (e.g. new or remixed themes). Old tracks keep working; a track uses its MP3 only if one is provided. Must work in the Windows `.exe` package (MP3 files ship in the game folder). | | Sound code (`src/platform/sdl2.c` audio, `sound/`); `tools/pc/make_package.sh` must copy the music folder |
| [ ] | **ENG-NAMEBOX. Speaker name box for every important character, whole game** (owner, 2026-11-14): the green name box above the text, on for the whole time they speak — rivals (Wally, May/Brendan, Blue), professors (Birch, Oak), Mom, Norman, Gym Leaders, Elite Four, Champions, Wallace, Juan, Steven, Archie, Maxie, and anyone important to the story (Varnell, Bill...). How: add a name to `SP_NAME_*` (`include/constants/speaker_names.h` + `src/data/speaker_names.h`), then `setspeaker SP_NAME_X` before each of their `msgbox`es; drop any "Name:" prefix from the text. Ordinary trainers no longer get it (owner): `IsNameboxTrainer` in `src/battle_setup.c` limits the automatic trainer box to rivals, Leaders, Elite Four, Champions, team leaders/admins (both games' classes). **ARIA has done:** Mom, Birch, Wally, Oak, Varnell, Blue, Bill, Lance, Koga (+ Miguel, Kiyoshi names ready). **Hoenn pass done 2026-11-14 (ARIA):** (1) every Emerald-style "Name: " prefix of an important character at the start of a page became `{SPEAKER SPK_X}` (`tools/namebox_prefixes.py`, 690+ texts incl. `match_call.inc`; a second speaker on line 2 of a page now gets its own page). Names: Mom, Dad, Prof. Birch, May, Brendan, Wally, Steven, Wallace, Juan, all Gym Leaders, Elite Four, Archie, Maxie, Tabitha, Matt, Shelly, Courtney, Frontier Brains (owner: yes), Scott, Mr. Briney, Capt. Stern, Mr. Stone, Prof. Cozmo. Left as plain text prefix: ordinary NPCs (Uncle, Wanda, Gabby, fans...), and trainer DEFEAT texts (shown inside the battle, where there is no name box). (2) 108 unprefixed lines checked by hand got `setspeaker` (`tools/namebox_setspeaker.py` + reviewed list): Gym Leaders' badge/after/rematch lines, E4 rematch lines, rivals, Steven, Archie, admins' post-battle lines, Mom/Dad at home. Rules in code: a `{SPEAKER}` code names only its own message (the next message without one goes back to the script's `setspeaker`, or no box); "obtained/received ITEM" lines never show a box (`Speaker_HoldForSystemText` in the item scripts) and the character's box comes back after. `SP_NAME_RIVAL` shows May or Brendan for shared rival texts. Charmap `SPK_*` values must match the enum order (STATIC_ASSERT on the count). Kanto FireRed scripts not done (S-TALK rewrites them). Lines that never had a prefix and whose script does not name the character may still be missing a box: report any you see. | | Many map scripts (Hoenn + Kanto); speaker table |
| [ ] | **ENG-SPRITES. Remove the colour limits on sprites** (owner, 2026-11-14): "I want them to be the same size so they don't stick out, but no limit on the colours." Overworld, battle and trainer sprites may use more than 16 colours (full colour PNGs). **Keep sizes and pixel scale the same as FireRed/Emerald** (e.g. 64×64 battle pictures, 16×32 / 32×32 overworld) so new art matches. Existing 16-colour sprites keep working unchanged. | | Graphics loading + drawing (sprite/OAM code, `src/platform/` renderer, 3D sprite path in `voxel3d`), `graphics/` build rules |

Open items handed to Polish (Hoenn): DLX-22 Granite Cave black lines; Hoenn trainers' repetitive lines
(owner: every trainer needs believable, unique dialogue — `WALLY_JOURNEY_OWNER_REQUIREMENTS.md` 2026-11-13);
Fortree tree-trunk furniture and the Fortree decoration shop benches (DLX-28 leftovers).
