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
| [x] | S-LEAGUE (except Janine): Blue Viridian Gym, Koga E4 4th room, Lance Champion + Hall of Fame. Dialogue DRAFT (owner to approve), teams FireRed placeholders (L4). Janine's own overworld sprite added 2026-11-15 (owner-approved v2, `OBJ_EVENT_GFX_JANINE`) | 2026-11-14 | Kanto League maps + trainers_frlg.party (new TRAINER_LEADER_BLUE 624, TRAINER_ELITE_FOUR_KOGA 625) |
| [x] | Kanto badges + HM licences (S-HM) | 2026-11-14 | `FLAG_KANTO_BADGE0x_GET` (Kanto flag space) used by all FireRed gym scripts; `src/field_move.c` checks Kanto badges in Kanto maps only — Hoenn field moves unchanged. **Keep.** |
| [x] | Seasons message names the region (ARIA, 2026-11-14) | 2026-11-14 | `data/scripts/wally_journey.inc` season texts use `{REGION}` (Kanto in Kanto maps, Hoenn otherwise); 'grey' -> 'gray' |
| [ ] | Story chapters — 0, 0b, 1, 2, 3, 3b, 4 IN GAME (2026-11-14); next ch. 5 Celadon | 2026-11-14 | Ch. 0 touches Littleroot/Birch's lab/Slateport harbour scripts (Hoenn, post-game only) |
| [ ] | K7 Kanto in 3D — paused (story first) | | Will regenerate `pc_data/voxel/*` — claim first |

## Project Polish (Brennan)

| Status | Task | Date / commit | Affects / don't revert |
|---|---|---|---|
| [x] | POL-25 HM moves can be replaced/forgotten (summary screen ignored the setting) | 2026-11-14 | `src/bw_summary_screen.c` CanReplaceMove uses `CannotForgetMove()`; new test aids `state setmove` / `state learnscreen`, env `WJPC_TEST_NO_BATTLES` |
| [ ] | **Brennan's 2026-11-14 fix list (POL-1..29)**: `docs/POLISH_TASKS.md` on branch `polish`. WIP (Claude 014PwRDj): POL-11 freezes in 3D, POL-14 crash after ~10 min in 3D, POL-25 unlearn HM moves first | 2026-11-14 | Will touch `src/platform/voxel3d/` (renderer), `src/platform/sdl2.c`, HM-forget checks — see WORK_CLAIMS |

## Project Tandem (Brennan) — online multiplayer

| Status | Task | Date / commit | Affects / don't revert |
|---|---|---|---|
| [ ] | **Project Tandem** planned (named 2026-11-26): friend-hosted multiplayer - see `docs/TANDEM_TASKS.md`. First T0, a read-only audit and plan for review (after Polish's Jagged Pass list); first milestone two players walking the same map | 2026-11-26 | Nothing changed yet. Will touch shared engine code (overworld, saves, battles, link code) on branch `tandem` - claims in `WORK_CLAIMS.md` first |

| [x] | **PC-SPECIALVAR (found by Tandem, 2026-11-28): `specialvar` read garbage from 8-bit specials on PC** - `scrcmd.c` ScrCmd_specialvar calls every special through a u16 pointer; a bool8/u8/s8 special sets only AL on x86, the rest of EAX is left over (seen: ShouldTryRematchBattle gave 0x..01 - Match Call rematches by talking never started; 101 specials return 8 bits). Now `#ifdef PORTABLE` it keeps the low 8 bits (sign-extends s8) for the specials listed in `build/.../gen/special_widths.c`, made every build by `tools/pc/gen_special_widths.py` from `data/specials.inc` + the sources (Makefile.pc SPECIAL_WIDTHS rule). | 2026-11-28 tandem | **Both projects - don't revert.** Every `specialvar` of an 8-bit special now gives the GBA's value (scripts that only worked by luck may now take their "true" branch, as on the GBA). New specials need nothing: the list is rebuilt from the source each build (a special declared `bool8 Name(void)` on one line in a .c file). |

## Whole-game engine tasks (either project — claim in `docs/WORK_CLAIMS.md` first; they touch shared code)

| Status | Task | Date / commit | Affects / don't revert |
|---|---|---|---|
| [ ] WIP (ChatGPT/Treston, 2026-11-15) | **PC-BUGS-1115: Six owner-reported PC rendering/input issues**, report and safe repro plan in `docs/PC_OWNER_ISSUES_2026-11-15.md`: house colors lost; black lines in Granite Cave/Pokémon Centers; **OWNER-CONFIRMED 2026-11-15:** skip the 3D intro and immediately access the initialized battle commands, including normal Run in wild encounters (no forced trainer-battle escape; exact input still TBD); broken 2D zoom; start menu clipping under certain screen settings; outer tree scenery changing across route seams. **Status: 2 scoped source patches (2D zoom quantization and undersized UI clipping) in `src/platform/sdl2.c` commits cd66f0f/e6ca9d7; math regression file aaa4d37; 180 geometry-model checks passed, no Windows exe build/testing, 0/6 fully verified. Four issues remain unmodified due claimed 3D files or battle-intro state complexity; **second source audit 120d64a:** confirmed zoom disabled when Widescreen OFF, normal-size start-menu clipping still unexplained, separate battle engine/controller intro, seasonal recolor applied to building textures, possible stale packaged voxel data, and diagnostic isolation of GPU-dependent seams. Details and exact tests in `docs/PC_OWNER_ISSUES_2026-11-15.md`.** Owner provided a screenshot in chat (not in repo). Existing Polish tasks POL-6, POL-8, POL-28, POL-30 and DLX-22 overlap; POL-30's unverified fix is already on `polish`. No renderer files edited — active claims prevent conflicting edits. | 2026-11-15, docs: 7314476 / 2af2839 | **Do not revert/duplicate work claimed by POLISH.** Check `WORK_CLAIMS.md`; verify 2D and 3D, all relevant screen modes, and Windows packaged executable; retain old-save import. |

| [x] | **ENG-MP3. Music tracks can be MP3 files** (owner, 2026-11-14). **Done 2026-11-14 (ARIA):** put `music/MUS_NAME.mp3` next to the exe (name = the song's `MUS_` constant); optional `MUS_NAME.txt` with `loop_start=`/`loop_end=` (seconds) and `volume=` (default 0.15 ≈ GBA music level). The GBA song keeps running muted underneath, so fades, fanfare pauses, song changes and volume dips work as before. Code: `src/platform/mp3_music.c` (dr_mp3 v0.7.4, public domain/MIT-0), `mp3_glue.c`, hooks in `sound_mixer.c`, `cgb_audio.c`, `music_player.c`; name table generated by `tools/pc/gen_mp3_song_names.py`; package copies `pc_data/music/` (README inside). Tested Linux + Windows exe (Wine): a test MP3 replaces Littleroot's music, GBA song fully silent, stops on the next map's song. Let any song play from an MP3 instead of the GBA music engine (e.g. new or remixed themes). Old tracks keep working; a track uses its MP3 only if one is provided. Must work in the Windows `.exe` package (MP3 files ship in the game folder). | | Sound code (`src/platform/sdl2.c` audio, `sound/`); `tools/pc/make_package.sh` must copy the music folder |
| [ ] | **ENG-NAMEBOX. Speaker name box for every important character, whole game** (owner, 2026-11-14): the green name box above the text, on for the whole time they speak — rivals (Wally, May/Brendan, Blue), professors (Birch, Oak), Mom, Norman, Gym Leaders, Elite Four, Champions, Wallace, Juan, Steven, Archie, Maxie, and anyone important to the story (Varnell, Bill...). How: add a name to `SP_NAME_*` (`include/constants/speaker_names.h` + `src/data/speaker_names.h`), then `setspeaker SP_NAME_X` before each of their `msgbox`es; drop any "Name:" prefix from the text. Ordinary trainers no longer get it (owner): `IsNameboxTrainer` in `src/battle_setup.c` limits the automatic trainer box to rivals, Leaders, Elite Four, Champions, team leaders/admins (both games' classes). **ARIA has done:** Mom, Birch, Wally, Oak, Varnell, Blue, Bill, Lance, Koga (+ Miguel, Kiyoshi names ready). **Hoenn pass done 2026-11-14 (ARIA):** (1) every Emerald-style "Name: " prefix of an important character at the start of a page became `{SPEAKER SPK_X}` (`tools/namebox_prefixes.py`, 690+ texts incl. `match_call.inc`; a second speaker on line 2 of a page now gets its own page). Names: Mom, Dad, Prof. Birch, May, Brendan, Wally, Steven, Wallace, Juan, all Gym Leaders, Elite Four, Archie, Maxie, Tabitha, Matt, Shelly, Courtney, Frontier Brains (owner: yes), Scott, Mr. Briney, Capt. Stern, Mr. Stone, Prof. Cozmo. Left as plain text prefix: ordinary NPCs (Uncle, Wanda, Gabby, fans...), and trainer DEFEAT texts (shown inside the battle, where there is no name box). (2) 108 unprefixed lines checked by hand got `setspeaker` (`tools/namebox_setspeaker.py` + reviewed list): Gym Leaders' badge/after/rematch lines, E4 rematch lines, rivals, Steven, Archie, admins' post-battle lines, Mom/Dad at home. Rules in code: a `{SPEAKER}` code names only its own message (the next message without one goes back to the script's `setspeaker`, or no box); "obtained/received ITEM" lines never show a box (`Speaker_HoldForSystemText` in the item scripts) and the character's box comes back after. `SP_NAME_RIVAL` shows May or Brendan for shared rival texts. Charmap `SPK_*` values must match the enum order (STATIC_ASSERT on the count). Kanto FireRed scripts not done (S-TALK rewrites them). Lines that never had a prefix and whose script does not name the character may still be missing a box: report any you see. | | Many map scripts (Hoenn + Kanto); speaker table |
| [ ] | **ENG-SPRITES. Remove the colour limits on sprites** (owner, 2026-11-14). **Engine done 2026-11-14 (ARIA):** list `FULL_COLOUR.png GAME.png [shared.pal]` in `graphics/hicolor/list.txt`, run `python3 tools/pc/hicolor.py` (rewrites GAME.png as the 16-colour stand-in + `src/platform/hicolor_tiles.h`). On PC, `src/platform/hicolor.c` matches sprite tiles in VRAM by content + palette and draws the full-colour pixels (2D renderer and wide view), fading them with the palette; anything that doesn't match exactly draws the normal way. Tested with a hue-shifted Mom (shared palette). Not yet: 3D sprite atlas (`voxel_entities.c`), 8bpp sprites. Original request: "I want them to be the same size so they don't stick out, but no limit on the colours." Overworld, battle and trainer sprites may use more than 16 colours (full colour PNGs). **Keep sizes and pixel scale the same as FireRed/Emerald** (e.g. 64×64 battle pictures, 16×32 / 32×32 overworld) so new art matches. Existing 16-colour sprites keep working unchanged. | | Graphics loading + drawing (sprite/OAM code, `src/platform/` renderer, 3D sprite path in `voxel3d`), `graphics/` build rules |

Open items handed to Polish (Hoenn): DLX-22 Granite Cave black lines; Hoenn trainers' repetitive lines
(owner: every trainer needs believable, unique dialogue — `WALLY_JOURNEY_OWNER_REQUIREMENTS.md` 2026-11-13);
Fortree tree-trunk furniture and the Fortree decoration shop benches (DLX-28 leftovers).
