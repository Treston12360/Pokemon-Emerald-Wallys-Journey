# Project "Emerald Overhaul" (Wally's Journey 0.9.0)

The handoff file for the big UI/visuals update. To pick the work up, tell an AI:
**"continue work on Emerald Overhaul"**. Read this whole file first, then
`docs/WALLY_JOURNEY_OWNER_REQUIREMENTS.md` (the full history of owner decisions).

- Repo: `Treston12360/Pokemon-Emerald-Wallys-Journey`, branch `wally-journey-development`.
- Base: pokeemerald-expansion **1.17.2**.
- The last ROM sent to the owner was **0.14.0** (rows 1–26 below).

## Owner rules (always)

**Talking to the owner**
- Plain, honest language. Call out mistakes and dead ends; never smooth them over.
- Ask before visual/theme decisions. Show screenshots of anything visual before calling it done.

**Dialogue**
- Must be true whenever it's read.
- **Show any new or changed line to the owner and get approval before it ships.**
- Run `python3 tools/lint_wally_dialogue.py --base <vanilla pokeemerald checkout>`.

**Art**
- No patchwork sprites, and no drawing new art: Claude is bad at sprite work.
- Port existing art only. Recolouring a palette is fine.

**Theme**
- "Emerald themed" means swapping blue/teal/yellow UI colours for Emerald greens.
- Keep meaning-carrying colours as they are: type colours, ♂ blue / ♀ red, red/yellow/green/blue battle buttons.
- Nature stats: raised = green with ↑, lowered = red with ↓.

**Game design**
- Wild levels are never below the evolution level.
- Megas only for specific trainer classes.
- Buddies' dev notes must not spoil anything:
  - no mention of Wally being harder or of Wally fights;
  - nothing about Vito, Treston or Cameron;
  - nothing about the important-trainer scaling, or anything "harder".

**Repo**
- Push as you go. Another session also pushes, so always `git pull --rebase` before `git push`.
- Commits end with the Co-Authored-By / Claude-Session lines used in `git log`.
- After a build, run `git checkout src/data/pokemon/all_learnables.json` if it shows as modified (it's generated).
- Credit every ported feature in `CREDITS.md` and add a row to `docs/WALLY_JOURNEY_OWNER_REQUIREMENTS.md`.

## How to build, release and test

- `make -j` builds a test ROM. `make release` builds the ROM that ships.
- Release checklist:
  - `python3 tools/wj_testfw/install.py off`
  - `make release`
  - `arm-none-eabi-nm pokeemerald-release.elf | grep -ci wjtest` must print `0`.
  - The byte pattern `9600640097006400` must not appear in the release ROM.
  - Boot it in the harness: at title time, pixel (120,40) must be RGB (41,16,214).
- **Test framework** (`tools/wj_testfw/`):
  - `install.py on scenarios/<x>.json` puts in TEST-ONLY hooks: a new game starts at the scenario's map and position with its flags and party. `"weak": true` makes trainers' Pokémon level 1.
  - Build, copy the ROM out, then **always** `install.py off` before committing.
  - Scenarios used for this project are in `tools/wj_testfw/scenarios/`:
    - `grass3`: Route 102 grass, wild battles.
    - `pc`: Oldale Pokémon Center PC.
    - `pet` / `pet2`: Petalburg, used for day/night and ripples.
    - `evo`: Roxanne with a Treecko one EXP point from Lv16.
    - `dynt`: Wartortle for dynamic move info.
- **Harness** (`tools/wj_testfw/harness.c`):
  - A headless mGBA runner: `harness rom.gba script.txt <rtc_epoch> [save.sav]`.
  - Build mGBA as a static lib (`cmake -DBUILD_STATIC=ON -DBUILD_QT=OFF -DBUILD_SDL=OFF`) and link `harness.c` against `libmgba.a`.
  - Script lines:
    - a number waits that many frames;
    - `N KEY` holds KEY for N frames: `A B S`(Start) `s`(Select) `U D L R`(D-pad) `l r`(shoulders);
    - `shot name` writes `name.ppm`;
    - `peek ADDR` prints memory.
  - A new game with a test scenario starts with: `400 / 10 A / 200 / 10 S / 900 / 10 s / 60 / 10 s / 1200`.
  - In test new games, game time = harness RTC time minus 6 hours.
- The 0.8.8 save the owner sent (a friend's real save, Lv33–43 team, at Fortree) is the best "real player" test. It's not in the repo; ask the owner for it again if needed.

## DONE (pushed, checked in the emulator)

All the screens below were recoloured to Emerald green, palette only.

| # | Feature | Source (credit) | Notes |
|---|---|---|---|
| 1 | Gen 5 party menu | Archie / TheXaman `gen5ish_party_menu` | released in 0.8.12 |
| 2 | BW summary screen + green/red nature arrows | RavePossum `bw_summary_screen_expansion` | released in 0.8.12 |
| 3 | BW battle UI | mudskip `feature/bwBattleUI` | dark-green text box/bar, green cursor |
| 4 | Dynamic move descriptions | Flash1Lucky | L in move menu; real power/accuracy, green when raised |
| 5 | Sword/Shield message box | montmoguri `swsh_message_box` | green name tag |
| 6 | Sword/Shield PC storage | montmoguri `swsh_storage_system` | lime → Emerald green |
| 7 | Sword/Shield bag | montmoguri `swsh_bag_menu` | hooked to our unlimited bag; counters widened to 16-bit |
| 8 | Hourly day/night + lit windows | ShinyDragonHunter `DayNight` (data/ideas ported) | General pal 5 + Petalburg 6/8 light up 20:00–04:59; General pal 1 skipped (it's also the water) |
| 9 | Hand-made seasonal palettes | BelialClover `seasons` | General tileset only; other tilesets keep our formula; our real-clock calendar kept |
| 10 | Battle backgrounds follow day/night | Kasen `battle-dns` | background only, sprites untinted |
| 11 | Unique 4× / ¼× hit sounds | SolemnStormcloud | |
| 12 | Pond/puddle ripples | OriginalGRE | automatic on every map without its own step callback |
| 13 | Extra battle transitions | rayrobdod (yoshord) | Regirock/Regice/Registeel use the ORAS ones; the rest are available but unused |
| 14 | Mid-battle evolution | shachar700 / Zetraphes | keeps stat stages/effects; B cancels for the rest of that battle |
| 15 | Overworld character shadows | Pawkkie | already built into expansion 1.17 (`OW_OBJECT_VANILLA_SHADOWS FALSE`), nothing to port |
| 16 | Vol's HGSS-style start menu, green | vol8 `start_menu_1`, HashtagMarky's expansion port | Pokédex icon asks Pokédex/DexNav; Pike/Pyramid/link rooms keep the old menu |
| 17 | Stat Points (EV allocator), green | Bivurnum `ev-allocator` on TAH's stat editor | no battle EVs; 10 points per level to Lv51; Party → Stat Points; old saves: all EVs reset to 0 once on Continue (in 0.9.3) |
| 26 | Ditto face sprites: a transformed Ditto keeps its face (Bulbasaur line, Cleffa line, Growlithe, Arcanine, Doduo, Dodrio) | AsparagusEduardo | `P_DITTO_SPRITES`; only the species with art; only real Ditto (Mew etc. using Transform look normal). Tested both sides (our Ditto → Growlithe back, wild Ditto → Arcanine front). In 0.14.0 |
| 25 | Fix: level-scaled trainers whose Pokémon evolve (Norman's Vigoroth → Slaking) hit an "illegal ability" assertion (test builds stop; release silently skipped it) | ours | `wally_journey.c` keeps the old ability only when the evolution has it, else keeps the slot. In 0.13.0 |
| 24 | 3D battle backgrounds in all 8 Gyms (+ Lavaridge/Sootopolis B1F reuse 1F), Sidney, Glacia, Drake and the Champion room | Emerald 3DS | `capture_trainer.sh` + `gyms.txt`: talks to the Leader/E4 member in the 3DS game. Covers Leader, Gym trainer, double (Tate & Liza), E4 and Champion battles. Not done: Phoebe's room (the 3DS port freezes before her battle) — stock background. In 0.13.0 |
| 23 | 3D battle backgrounds for surf battles (50 maps: every route/town/cave with surfing encounters; folders `<map>__water`) | Emerald 3DS | `wj3d_gen_table.py` makes `__water` folders WATER/POND-only rows; plain rows never apply on water. Skipped: Abandoned Ship hidden-floor corridors (the 3DS port draws its water white). The lookup also accepts Gym / Leader / Elite Four / Champion / hideout environments now, for the coming Gym captures (no rows yet). In 0.12.0 |
| 22 | 3D battle backgrounds in caves and indoor wild-battle places (50 maps: Granite Cave, Meteor Falls, Victory Road, Mt. Pyre inside, Magma Hideout, Seafloor Cavern, Shoal Cave, Sky Pillar, Mirage Tower, New Mauville, Fiery Path, Rusturf Tunnel, Desert Underpass, Cave of Origin, Altering/Artisan Cave) | Emerald 3DS | lookup now allows CAVE and BUILDING environments (per-map rows only). Not done: Granite Cave Steven's room, Mirage Tower 4F, Seafloor Cavern rooms 2 and 8 (tiny rooms; the capture gets stuck in a dialogue) — stock background there. Caves' far edges show the 3DS port's fog-grey void. In 0.11.0 |
| 21 | 3D battle backgrounds: every outdoor wild-battle map (27 maps: Routes 101-104, 110-121, 123, Petalburg Woods, Jagged Pass, Mt. Pyre outside + summit, Safari Zone x6) | Emerald 3DS | Route 111 backdrop is desert-only (`environment.txt`); summit fog starts Misty Terrain, whose pink background shows until it ends (engine design). In 0.10.0 |
| 20 | 3D battle backgrounds: Routes 101, 102, 103, 104 | Emerald 3DS (Zallax and contributors) | owner approved the Route 104 look ("continue"); seasons + day/night apply. Pipeline in `tools/wj3d/README.md`. In 0.9.5 |
| 19 | Save screen frames fixed | ours | `Vol_SelectedFunc_Save` used freed menu data; exit now reloads message box/frame gfx. In 0.9.4 |
| 18 | BW move names coloured by effectiveness, green text-box line | ours | green/blue/red names (needs the foe species seen; expansion marks "seen" at the END of the first battle); box keeps type colour; red line + move-box marks → green. In 0.9.2 |

## IN PROGRESS

- **Pokédex in Emerald green:** mockup sent 2026-11-04 (branch `wip/dex-green`, palette-only: reds → green hue 140). Waiting for the owner's OK; then merge.

## ▶ NEXT TASK: 3D battle backgrounds for the rest (water, towns/gyms)

All 27 outdoor wild-battle maps are done (0.10.0). A capture is now ~2-3 minutes (OpenGL), so a whole batch
runs in one background job: `tools/wj3d/routes.txt` + `capture_route.sh`, then `convert_all.sh` with
`shadow_modes.sh`. Still to do, roughly in order:
1. ~~Caves~~ done in 0.11.0 (`caves.txt`, ENC=08,0b,06,25). Left: Steven's room, Mirage Tower 4F, Seafloor rooms 2/8 (need the objects hidden in the pret capture build).
2. ~~Surf battles~~ done in 0.12.0 (`water.txt`, spots must be water in vanilla and ours; warping onto water auto-surfs).
3. ~~Gyms / Elite Four / Champion~~ done in 0.13.0 except Phoebe (3DS port freezes in her room).
4. Left (small): Phoebe, Steven's room, Mirage Tower 4F, Seafloor rooms 2/8, Abandoned Ship hidden corridors (water), Aqua Hideout / trainer-only indoor places.
The owner's clean ROM must be attached each session (never commit it: the repo is public).

## OWNER ORDER (2026-11-04)

The owner picked this order: 1 DexNav check (done), 3 start menu colours (done), 5 EV allocator (done), **9 3D battle backgrounds (in progress)**, then the rest. **Never** do the lit night windows for more towns (owner dropped it).

**New from the owner (2026-11-04), add to the order after the 3D backgrounds unless told otherwise:**
- ~~Save screen looks glitched~~ fixed in 0.9.4 (row 19).
- **Pokédex in Emerald green.** We use the HGSS-style Pokédex (`POKEDEX_PLUS_HGSS`, `HGSS_DECAPPED`). Palette-only recolour (red/blue UI → Emerald greens; keep type colours), mockup first, owner approval before applying.
- **Battle menu size:** owner felt the BW action buttons look big / cut off at the bottom. That is the original art (the lower buttons' bottom edge is off-screen by design). Shrinking needs new art (not allowed). Owner decided (2026-11-04): **leave them as they are.**

## TO DO, in suggested priority order

1. **Checks still not done** (do before the overhaul is called finished):
   - **Battle UI:** a double battle (BW UI + mid-battle evo of the right Pokémon), a trainer battle with Mega/Z triggers, the Safari Zone.
   - **Bag:** the Bag used in battle, the Battle Pyramid bag, and a pocket with more than 255 stacks.
   - **Summary:** an Egg's summary page.
   - **Transitions:** watch one Regi transition with R held. Battle transitions are fast-forwarded unless R is held.
   - **New text:** the Sword/Shield PC menu and bag use mixed-case words ("Store", "Summary"); the decapitalize task will settle this.
2. **Owner decisions needed:** none open right now.
3. **3D battle backgrounds** (the biggest remaining job). Plan:
   - Capture backdrops from Emerald 3DS v0.5.0 in its 3D BATTLE mode (Azahar libretro core).
   - Inpaint out sprites/HUD, convert to GBA (240×160, about 60 colours).
   - Make 4 season versions with our SeasonalColor.
   - Day/night comes free from item 10.
   - The code picks a backdrop per map and falls back to the current backgrounds.
   - Credit "Emerald 3DS by Zallax and contributors" (its port code is MIT; the 3D world is generated from the player's own ROM).
   - Proposed first batch: Routes 101–103, Oldale, Petalburg, Petalburg Gym, Elite Four rooms.
   - No lit-window work in battle (the owner dropped it).
   - **Downloads that work from a session (2026-11-04):** Emerald 3DS `https://github.com/ZallaxDev/pokeemerald-3Ds-dualscreen/releases/download/v0.5.0/Emerald3DS-WebPayload.zip`; Azahar core from Azahar's own release `https://github.com/azahar-emu/azahar/releases/download/2126.2/azahar-libretro-linux-x86_64-2126.2.zip` (buildbot.libretro.com may be blocked). The harness needs mGBA **0.10.5** (the master branch crashes it). Still needs the owner's clean Emerald ROM each session.
   - **Status (2026-11-04):** game side is in: `WJ_3D_BATTLE_BACKGROUNDS`, `src/data/wj3d_backdrops.h` (empty table = stock backgrounds everywhere), hook in `battle_bg.c` (land environments, ordinary battles only; season recolour + day/night tint applied). Converter `tools/wj3d_backdrop.py` (3DS frame → 3 palettes / ≤511 tiles / top 14 tile rows; tested on an old capture). **Blocker:** clean captures. Tools and findings are in `tools/wj3d/README.md` (read it first).
4. **Reports for the owner** (no code):
   - **Outfits:** compare mudskip `feature/outfits-system-rhh` with our Outfit Case.
   - **Gacha:** build agsmgmaster64 `gacha-expansion` and show what the machine/prizes look like before placing it.
5. **New main menu with mugshot** (Archie & Mudskip, pret wiki; vanilla): port and recolour green.

## Source branches (all public GitHub)

- **UI**
  - TeamAquasHideout/pokeemerald `gen5ish_party_menu`
  - ravepossum/pokeemerald-expansion `bw_summary_screen_expansion`, `hgss_battle_ui_expansion` (not used)
  - mudskipper13/pokeemerald `feature/bwBattleUI`, `feature/outfits-system-rhh`
  - Flash1Lucky/pokeemerald-expansion `dynamic-move-descriptions`
  - montmoguri/pokeemerald-expansion `swsh_message_box`, `swsh_storage_system`, `swsh_bag_menu`
- **Visuals**
  - ShinyDragonHunter/pokeemerald `DayNight`
  - BelialClover/pokeemerald `seasons`
  - Kasenn/pokeemerald-expansion-kasen `battle-dns`
  - OriginalGRE/pokeemerald-expansion `ambient_pond_ripples`
  - rayrobdod/pokeemerald `battle_transitions-expansion`
- **Battle and sound**
  - SolemnStormcloud/pokeemerald-expansion `unique-effectiveness-sounds`
  - shachar700/pokeemerald-fork `rhh-13.2-midBattleEvo`
- **Not ported yet**
  - TeamAquasHideout/pokeemerald `AsparagusEduardo/ditto`
  - agsmgmaster64/worldlinkdeluxe-ame `gacha-expansion`
- **Reference:** Emerald Imperium's full source is ei-showdown/emerald-imperium `release`.

How we port: take only the feature's own diff against the expansion release it was built on (`git diff <base> <tip>`), copy its new files, `patch -F3` the changed ones, and then **check every fuzzy hunk landed in the right function**. Several landed in the wrong function silently during this project.
