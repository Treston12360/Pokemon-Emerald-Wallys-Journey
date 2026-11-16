# Project ARIA — Kanto task list

Owner's handoff list for the Kanto expansion. To hand work to any AI, say:
**"Continue Project ARIA"** (repo `Treston12360/Pokemon-Emerald-Wallys-Journey`, file `docs/KANTO_TASKS.md`).

> **Build gotcha (2026-11-14):** after changing map scripts/text, run `make release`, then `touch data/*.s` before
> `make -f Makefile.pc`. The PC build doesn't see changes in files that `data/*.s` include, so it can say
> "Nothing to be done" and keep the old text. Check the binary's time before testing. Same for graphics: after
> changing a PNG, delete its build/assets/... file and touch the .c file that includes it.

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
      coin checks written without spaces for the PC assembler. Data build and Windows/Linux build link.
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
      still "Lanette's PC" in Kanto (wording — ask owner); HM rule changed since (owner 2026-11-13): Kanto HMs come from Kanto Gym Leaders and need Kanto badges (Fly always works) — new task S-HM below;
      Trainer Tower / Sevii code still `IS_FRLG` (Sevii decided later); cable club attendants Emerald's.
- [x] **K6. Region-aware Fly and region map.** Done 2026-11-13 (session 019khs4X). Owner's choice: "switch on the
      map" — the Fly map opens on the region you are in; **L or R flips to the other region** once one of that
      region's fly spots has been visited (until then the button does nothing). Ported from Crossroads' Flight Call
      (`sUseForcedFlightRegion`, `GetActiveRegionMapType`, `InitMapForForcedFlightRegion` in `src/region_map.c`),
      no key item; normal Fly. No player head is drawn on the other region's map. Checked in game: Dewford → Fly
      map → R shows Kanto (Pallet Town) → L back to Hoenn; R then A on Pallet Town flies there. Test-script command
      `flymap` opens the Fly map from the field. Not done: no on-screen hint that L/R switch (wording/visual — ask
      the owner first).
- [ ] **K7. Kanto in 3D.** Seen 2026-11-14 (Windows build, 3D): Oak's lab shows the three starter Poké Balls on the
      table in 3D although their objects are hidden (2D is right) — check the 3D entity code honours hide flags.
       PAUSED 2026-11-13 (owner: story first, then 3D). Also done: `voxel_cells.py` reads FireRed's 32-bit attributes and 640 split (`split_for`, `read_attr16`), water lookups in relief fixes / buildings too. Done so far: the 3DS generators' map reader
      reads FireRed layouts (`layout_version: frlg` → 640/640 split, 7 palettes, `*_frlg` tileset folders;
      `voxel_building.py` LayoutArt, `dump_region_art.py` Tilesets — in `tools/voxel3ds/our_changes.patch`); Pallet
      Town composes correctly. **Left (big):** the rest of the generators still assume 512 (`NUM_PRIMARY` in
      `voxel_cells.py`, `voxel_terraces.py`, `gen_voxel_relief.py`, `voxel_bridge.py`, `gen_voxel_buildings.py`,
      `voxel_relief_fixes.py`) and their tree/cliff/rock tables are Emerald's metatiles; Kanto's buildings and rooms
      need modelling in `voxel_building_specs.py` the 3DS way (the 3DS port never did Kanto). Then regenerate
      relief/trees/buildings and check Pallet, Viridian etc. in 3D.

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

- [x] **S0. Remove FRLG's own story start** (done 2026-11-14, steps 1-5 in `data/scripts/aria_kanto_start.inc`, log `docs/KANTO_S0_STRIPPED.md`) (Oak's parcel, rival intro etc.) where it conflicts.
- [x] **S1. Chapter 0 — the call:** Mom → Birch → S.S. Anne ticket; Wally's send-off at Slateport.
- [x] **S2. Chapter 0b — arrival:** S.S. Anne to Vermilion, fade to black, Oak's lab in Pallet.
- [x] **S3. Chapter 1 — the investigation team:** Oak, Varnell, Blue; first assignment (Mt. Moon).
- [x] **S-HM. Kanto HM licenses** (2026-11-14): Kanto badges are now their own flags (`FLAG_KANTO_BADGE01..08_GET`,
      FireRed's gym scripts set them — before this they set Hoenn's badge flags, which a post-game save already had).
      `src/field_move.c` HasBadgeForFieldMove: in Kanto maps each field move needs FireRed's badge for it (Flash Boulder,
      Cut Cascade, Strength Rainbow, Surf Soul, Rock Smash Marsh, Dive/Waterfall Volcano); Fly always; Hoenn unchanged.
      The player's HM items are their Hoenn ones (owner: HMs are a licence). Test: `state fieldmove ID`.
      Old notes: (owner 2026-11-14: Rock Tunnel needs Flash — the player must have a way to get
      Flash from a Kanto Gym Leader before ch. 3b; Brock's badge allows Flash in FireRed.) on Kanto maps, Cut/Surf/Strength/Rock Smash/Waterfall/Flash/Dive need the Kanto
      badge from the Gym Leader who gives that HM (FireRed's badge→HM table); Fly always works; Hoenn unchanged.
- [x] **S-LEAGUE. Kanto League lineup** (already decided, design doc §1): Viridian Gym leader Giovanni → **Blue**
      (8th badge, ch. 10); Fuchsia → **Janine**; Elite Four (owner 2026-11-14): **Lorelei, Bruno, Agatha, Koga** (Koga in Lance's room);
      **Lance = Champion** in the Champion's room (replaces FireRed's Blue there). Giovanni is hidden only in the Rocket Hideout (S0 step 2); the Viridian Gym and Champion's room
      objects are untouched until this task. Mt. Moon: fossils kept (ch. 2) — Miguel's fossil scene stays.
- [ ] **S-LEAGUE2. Kanto boss type coverage rework** (owner, 2026-11-15: "max coverage ... there shouldn't be one kind
      of Pokemon or typing that can just carry somebody through"). **Decided 2026-11-15:** Blue = Flying Gym (Mega Pidgeot),
      his title defense = real Blastoise team; Sabrina promoted to the E4 (Agatha out); Copycat = Normal Saffron Gym;
      **Janine = Ghost** (owner: "Janine can be ghost"). Lineup in KANTO_PROJECT_ARIA §1. Coverage now: Ground still hits 4
      bosses (Brock, Surge, Blaine, Koga) - unavoidable with those types, so each carries an in-type Ground answer
      (Aerodactyl, Air Balloon, Charizard, Crobat/Weezing) + coverage moves; no other type hits more than 3 of 13; the
      E4+Lance block: no type hits more than 2 of 5. **E4 order decided 2026-11-15: Koga → Lorelei → Bruno → Sabrina → Lance**
      (owner: "Koga, Lorelei, Bruno, Sabrina, Lance"). To do: move the E4 trainers/scripts to their new rooms; Koga's approved
      after-battle line "Go. The Champion awaits." no longer fits room 1 — draft a new ending (KANTO_SCRIPT L.2) for approval. **Open:** **Janine's Ghost team needs Pokémon from other regions** (Gengar is Kanto's only Ghost family) —
      proposed story reason, NOT yet approved: ghosts from elsewhere slip through the tears and she takes them in, finding
      her own path apart from her father's Poison. Before showing any team: run the coverage checker (to be written as
      tools/aria_league_coverage.py) over all Kanto boss teams.
- [ ] **S-COPYCAT. Saffron Gym -> the Copycat, Normal type** (owner by voice, 2026-11-15, with the Sabrina-to-E4
      plan: asked who takes Sabrina's place; Copycat chosen; "we still want her to be strong" — copying is her flavour, not
      every Pokémon). She is a little girl in FRLG; our story is after FRLG, so she is **older** (teen/young adult) and needs
      a new overworld sprite + a battle picture (drafts shown first; research how good Gen 3 sprites are made first).
      **Team DRAFT (owner OK'd putting it in her draft):** Mega Kangaskhan (ace, Parental Bond), Snorlax (Curse), Tauros
      (Intimidate), Porygon-Z (Adaptability; owner: never two Porygon on one team), Blissey (special wall), Ditto
      (Imposter, her one copying signature). Kanto families only. Moves/items/natures still to do + coverage check.
      **Sprites (2026-11-15):** overworld draft OK'd ("her ingame sprite looks good": FRLG Lass recoloured, pink hair,
      straight bangs + long sides, white top/red skirt — `tools/aria_copycat_sprites.py`). **Battle picture: all four
      drafts rejected** (v1/v2 too close to the Lass, v3 "looks like shit", v4 "looks awful") — parked, come back later
      **RESOLVED 2026-11-15: Treston supplied the battle art himself** (`docs/kanto_art/copycat_battle_art_owner.png`) ->
      `TRAINER_PIC_COPYCAT`, via `tools/aria_copycat_from_art.py` + graphics/hicolor. **Name: Mimi** (owner, 2026-11-15: "Mimi is good"). Picture now cut at the thighs (figure 88 px) so the art's own eyes and Poké Ball survive; the hand-redrawn
      eyes/ball at full-body size were rejected ("You made it worse").
      (owner: "let's move on and return to this later"). Wanted: Hinata-style hair (blunt bangs, long side strands, long
      in the back), not recognisable as an existing trainer. Read `docs/SPRITING_NOTES.md` first; consider asking Treston
      for reference art (Janine/Varnell battle art came from his own art).
- [ ] **ENG-HIRES. Battle pictures with more pixels — ON HOLD** (owner, 2026-11-15: option 1, "but put that on hold").
      Same on-screen size as every trainer (keeps his 2026-11-14 rule "same size so they don't stick out"), but drawn at
      2x detail: a 128x128 full-colour picture in the 64x64 spot. Needs the PC renderer to draw those sprites at output
      resolution (src/platform/hicolor.c + sdl2.c — shared engine files, claim in WORK_CLAIMS first). Then Mimi goes back
      to full body from his art (eyes/ball/boots intact); Janine's and Varnell's art can use it too.
- [ ] **S-SAFFRON-GYM. Redesign the Saffron Gym for the Copycat** (owner, 2026-11-15) — it is Sabrina's teleporter maze now.
- [ ] **S-SABRINA-ROOM. New Elite Four room for Sabrina** (owner, 2026-11-15) — the 4th room (FireRed's dragon room, Lance's old one), alongside S-KOGA-ROOM.
      **Rule for all three (owner, 2026-11-15): before designing any map, research how people make good Gen 3 maps**
      (mapping guides, Porymap practice, well-regarded hacks) and write the findings down; don't just try.
- [ ] **S-KOGA-ROOM. Redesign Koga's Elite Four room (now the 1st room, FireRed's ice room) — later** (owner, 2026-11-15: "eventually we need 
      [to] redesign Koga's E4 room"). Koga sits in Lance's old room (S-LEAGUE), which still has FireRed's
      dragon-themed layout. Not started, no design decided yet — ask Treston what he wants before drafting.
- [ ] **S-ANNE. The S.S. Anne voyage** (owner, 2026-11-14): the trip from Slateport is played on the real ship —
      explore and battle like normal (boarding puts you in 1F corridor; stepping off = Vermilion, Oak's scene).
      Its trainers get teams for a Hoenn Champion arriving in Kanto (L4 level curve) and new lines — passengers and crew talk about the anomalies, Pokémon
      from other regions suddenly showing up (the ship carried the player from Hoenn). The captain is **not sick**
      and **does not give Cut** (HMs come from Kanto Gym Leaders); he gets his own new lines. Draft in
      `docs/KANTO_SCRIPT.md` first.
- [x] **S-ITEMS (TMs done 2026-11-14 — new TM51–TM100, see `docs/KANTO_TM_LIST.md`; HM spots and low-value ground items still open). No repeat TMs from Hoenn** (owner, 2026-11-14): TMs are infinite use, so no TM the player could
      already get in Hoenn is given again in Kanto (Gym Leader TMs, gifts, ground items) — replace each with a TM
      the player can't have yet. Battle Frontier items don't count. Useful items **may** repeat where it makes sense:
      nature mints, Bottle Caps, good healing items, Full Heals, Rare Candies, etc. But nothing weak for a Champion
      (no Potions healing 20 HP) — Kanto's ground items fit a Champion. Make a list (Hoenn TMs vs. Kanto planned)
      for the owner first.
- [ ] **S-WILD. Kanto wild encounters overhaul, same as Hoenn's** (owner, 2026-11-14): every non-legendary,
      non-mythical species obtainable across the game's wild encounters, placed believably (habitat, climate,
      terrain — the Hoenn rules in `WALLY_JOURNEY_OWNER_REQUIREMENTS.md`). Levels fit the Kanto curve (L-tasks).
- [ ] **S-NUGGET. Nugget Bridge** (owner, 2026-11-14): the five bridge trainers and the man at the end stay
      (the man wasn't dressed as a Rocket in FireRed). New, non-Rocket dialogue for all of them; the man becomes
      an ordinary trainer (no recruiting, no Rocket trainer class/team). His FireRed object is still visible now
      with his Rocket script — fix in this task. Teams for the player's level.
- [x] **S-FOSSIL. Fossil revival in the Pewter Museum** (approved ch. 2 text: Miguel says the museum in Pewter can
      bring the fossil back to life; FireRed did it at the Cinnabar Lab, which is closed in ARIA until ch. 9).
      Done 2026-11-14: FireRed's fossil scientist (same script, `CinnabarIsland_PokemonLab_ExperimentRoom_EventScript_FossilScientist`)
      now stands in `PewterCity_Museum_1F_Frlg` at (8,2). The "go for a walk" wait is finished by
      `Aria_PewterFossilReady` in `PewterCity_OnTransition` (revive state 1 -> 2), so leaving the museum and
      coming back is enough. His text never names Cinnabar. Only Helix/Dome/Old Amber are handled (FireRed's list);
      Hoenn fossils are revived in Rustboro as before.
- [ ] **S-BLOCK. Road blocks** from `docs/KANTO_PROJECT_ARIA.md` §4 (after the owner confirms the table).
- [ ] **S-TALK. New dialogue for every Kanto NPC and trainer** (owner: each trainer unique and believable, Gym
      Leaders with their own story, longer is fine). Drafts go in `docs/KANTO_SCRIPT.md` for approval first.
- [x] Ch. 2, 3, 3b, 4 in game (2026-11-14). Script: `docs/KANTO_SCRIPT.md` (status per chapter).
- [ ] Ch. 5 (Celadon office — new building, show owner a picture first), then ch. 6+.
- [x] **S-ARIA-CANON-LOCK (DOCUMENTATION ONLY — owner 2026-11-15):** Three historical points locked
      in `docs/KANTO_PROJECT_ARIA.md` and `docs/KANTO_NARRATIVE_AUDIT.md`: (1) young Varnell was **forced
      outside Pokémon Tower** during Rocket's raid; (2) Clefairy remained **captive with a Rocket member
      inside** while Aria searched for it (until the **post-jump** collapse began; clarified 2026-11-16: Clefairy then escapes and runs back to Aria); (3) Lavender residents **speculate the unrecognizable adult
      man found in the rubble might have been a Rocket member**. This is **not** approval of exact NPC
      dialogue and **not** a claim the current game scripts implement these decisions.
- [ ] **S-ARIA-LOOP-IMPLEMENTATION (STORY / SCRIPT — unimplemented; owner-approved facts only, wording DRAFT):**
      First read `docs/KANTO_NARRATIVE_AUDIT.md` and the locked §2 history in
      `docs/KANTO_PROJECT_ARIA.md`. Draft and obtain approval for: (a) Chapter 3 Kiyoshi's failed
      stabilizing rescue instead of the old forced-seal scene (current `data/scripts/aria_ch3.inc` is
      **superseded but still in game**); (b) the understated Lavender NPC account of a girl seeking
      captive Clefairy, unexplained collapse, severe damage to the unidentified man, and locals' **mistaken
      Rocket-member speculation**, without revealing any relationship; (c) later subtle clue placement;
      (d) Chapters 12–13 staged time-loop finale, where the **one-way retrieval portal stays stable
      until Varnell jumps**, and his reverse crossing FIRST triggers the old Tower collapse.
      **Owner-clarified 2026-11-16 final sequence:** stable window and Aria's refusal while Clefairy
      is captive → Varnell jumps backward → FIRST shaking / roof fall → Clefairy escapes
      Rocket's custody and runs to Aria DURING the collapse → Aria, Clefairy and future
      Varnell die together. **Owner-locked motivation 2026-11-16:** Varnell believes the historically recorded time of Aria's death is almost here; she will not leave without Clefairy, so he jumps because he thinks he must save her himself. He does not know his jump causes the death he fears. Never start shaking or release Clefairy before Varnell jumps. Exact countdown/portal-time behavior and spoken wording remain DRAFT.
      **Unresolved blocking:** decide who forced young Varnell outside, exact placement and
      mechanism of Clefairy's escape from the Rocket member, and why the historical body
      cannot be identified by surviving effects. These details are DRAFT, not in game.
      Keep all new dialogue labeled **DRAFT** until the owner approves exact text. Build/test
      WallysJourney.exe for branch-specific flags, old saves, optional interactions, skipped dialogue,
      revisits, and the exact cutscene timeline; **do not** mark this done before game verification.
- [ ] **S-ARIA-CONTINUITY-REVIEW (DRAFT, 2026-11-16):** Review `docs/KANTO_CONTINUITY_SOLUTIONS_DRAFT.md`
      sections P01–P09 with the owner. Proposed, **not canon**: who forced young Varnell outside;
      how Clefairy escapes **after Varnell jumps and roof starts falling**; Lavender echo's hidden
      targeting role; public vs hidden Blue research; Palkia/Dialga/Giratina causal tests;
      credible League vault transfer and delayed audit; Orb-to-Giratina capture timing;
      independently charged one-way receiver; final choreography with only approved assets.
      Do not edit live script or mark approved without owner sign-off; do not expose to Polish.


## Phase 3 — Levels and balance

- [ ] **L1. Hoenn post-game rules:** remove May/Brendan lab battle (and from the E4-rematch unlock); lower
      Steven to Wally's strength; Gym Leader rematch EXP 5% (or none); repeat Elite Four runs 5% EXP;
      Champion defenses locked until Kanto is beaten.
      **Done 2026-11-14:** lab battle removed (May/Brendan still say what they decided; E4-rematch unlock no
      longer needs it); Steven in Meteor Falls cut to ace 67 = Champion Wally (`WallyJourney_ScaleMeteorFallsSteven`,
      his title-defense team unchanged); Leader rematches 5% EXP; every Hoenn E4 battle after the first League
      win 5% EXP (`WallyJourney_IsRepeatEliteFourBattle`). **Open: Champion defenses lock** — waiting for the
      owner to say what the Champion's Room does on a repeat run before Kanto is beaten.
- [ ] **L2. Re-run `tools/exp_curve_full.py`** with L1's rules; update the arrival level in the design doc.
- [ ] **L3. Kanto trainer and wild levels** per the design doc; check with `tools/exp_curve_kanto.py`.
- [ ] **L4. Kanto Gym Leaders / Elite Four / important teams** (Kanto families only, no regional forms) —
      show the owner before adding.

## Done

(nothing yet)
