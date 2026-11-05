# Project "Emerald Overhaul" (Wally's Journey 0.9.0)

The handoff file for the big UI/visuals update. To pick the work up, tell an AI:
**"continue work on Emerald Overhaul"**. Read this whole file first, then
`docs/WALLY_JOURNEY_OWNER_REQUIREMENTS.md` (the full history of owner decisions).

- Repo: `Treston12360/Pokemon-Emerald-Wallys-Journey`, branch `wally-journey-development`.
- Base: pokeemerald-expansion **1.17.2**.
- The last ROM sent to the owner was **0.16.1** (rows 1–32 below).

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
| 35 | Problem 3D backdrops: Phoebe's room, Steven's room, Mirage Tower 4F, Seafloor Cavern 2/8/9, Underwater Routes 124/126 (underwater battles now allowed), Mirage Island, Oceanic Museum 2F, Rustboro City; Abandoned Ship B1F corridor white strip filled; hidden-floor water and Magma Hideout 3F 3R reuse a sibling room's backdrop | ours (Emerald 3DS captures) | The 3DS port runs its own code and only reads our save, so captures set up real encounters (start on grass/seaweed, encounter boost) or the real story battle (flags/vars + trigger). Still stock: Mauville City (Wally) and Space Center 2F (Yes/No prompt needs the 3DS touch screen), Shoal Cave water (no encounter reached). Tested in game: Phoebe, Duncan, Steven's room, Seafloor 2, Rustboro rival. In 0.17.1 |
| 34 | Options → New/Classic for every new graphic: Battle BG (3D backdrops), Battle Screen (BW UI), Summary (BW), Party Menu (Gen 5), Bag (SwSh), PC Boxes (SwSh), Text Box (SwSh box + name tag), Start Menu, Main Menu, Pokédex (HGSS), Ditto Faces. Saved in SaveBlock3 (`wjGfxMagic`/`wjClassicGfx`); old saves read as all New | ours | `WJ_ClassicGfx(feature)`. BW UI configs are runtime (`WJ_BwBattleUi`), Emerald window templates kept beside the BW ones. Emerald's bag (`item_menu.c`) is built again with its public functions renamed `Classic_*`; the SwSh entry points dispatch to it (Battle Pyramid keeps the SwSh bag). Party/textbox/name-tag Emerald art restored under `graphics/*/classic/`. Emulator: every Classic screen opened (field, start menu, party, summary, bag, wild battle menus/moves/run, Pokédex, PC boxes, nurse text, trainer name tag, save box, main menu → Continue), New screens re-checked. In 0.17.0 |
| 33 | Fixes from owner play: Stat Points picture shows shiny colours; dimming moves on 3D backdrops include palette 4 (no blocky tiles while surfing); grass rustle uses the season's grass colours; 19 more trainer/wild 3D backdrops (Abandoned Ship, Aqua Hideout, Mt. Chimney, Trick House, Weather Institute, S.S. Tidal, Seashore House, Meteor Falls) | ours | backdrops tested in trainer battles (Dwayne, Martha, Colton, Thalia). In 0.17.0 |
| 32 | Main menu portrait follows the outfit: the outfit's trainer picture zoomed 2x on head and shoulders (32x32 cut, affine double-size sprite), in its colours; no outfit → Brendan/May portrait | ours | owner picked "Zoomed" over the full picture. Tested Red, Hilbert (default and green/blue), Dawn, and no outfit. In 0.16.1 |
| 31 | New main menu with mugshot: Continue panel (portrait, location, play time, Pokédex, badges, party icons), New Game / Option (Mystery Gift / Events when unlocked), recoloured Emerald green (both the male and female themes) | Archie / Mudskip (TeamAquasHideout `main_menu`) | ported as its diff on vanilla; INCBIN .lz → INCGFX .smol/.smolTM; Birch speech entry uses our DecompressDataWithHeaderVram + gradient palette. With no save, the title goes straight to the Birch speech. Outfit portrait: row 32. Emulator: Continue loads the save, New Game → Birch, Option and back. In 0.16.0 |
| 30 | Outfit faces: the Town Map (PokéNav / Fly map) and Frontier Pass map show the outfit's head in its colours; the Hall of Fame shows the outfit's front picture | ours (owner picked this over Mudskip's outfit system) | `WJ_OutfitMapHead` crops 16x16 from the facing-down walking frame, starting just above the first drawn row. Tested Hilbert + red/blue on both maps. Hall of Fame not reached in a test (same `TRAINER_PIC_WJ_PLAYER` path as the Trainer Card). Link battles still send Brendan/May (would need a link-protocol change). In 0.15.1 |
| 29 | Gacha / Lottery Machines: 4 machines (Basic orange, Great blue, Ultra yellow, Master purple; min bets 50/250/1000/4500 coins) on the Mauville Game Corner's top wall where the 3 plants were, plus a lady (3,4) who explains them; the girl who stood there moved to (4,3) | agsmgmaster64 `gacha-expansion` | owner: "Add all 4 as they are" (Master can give legendaries). Machine art = their metatiles 0x280-0x28b copied as our 0x260-0x267 (their palette 11). Prizes level with badges, capped at the level cap. Gacha music not ported (it needs song renumbering); the script plays the Cinnabar theme as theirs does. Tested Basic (Wooper) and Master (Vaporeon) from the map. Test-only note: warping straight from New Game into an indoor map shows magenta at (4,1..3) — the truck intro's exit-light metatiles, never seen in real play. In 0.15.0 |
| 28 | Fix: mid-battle evolution ran out of memory when going back to the battle ("out of memory, 13288 bytes" stop in test builds; release builds would glitch/crash) | ours | the evolution now frees the battle anim buffers first (`CloseMainBattleScreen`, like opening the Bag); the reshow allocates them again. Tested: right-hand Marshtomp evolves mid double battle and the battle goes on. In 0.14.2 |
| 27 | Pokédex in Emerald green (HGSS Pokédex palettes: reds → green hue 140; type colours and map unchanged) | ours | owner approved the mockup as is ("Perfect looks good"). In 0.14.1 |
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
- ~~**Pokédex in Emerald green.**~~ done in 0.14.1 (row 27). We use the HGSS-style Pokédex (`POKEDEX_PLUS_HGSS`, `HGSS_DECAPPED`). Palette-only recolour (red/blue UI → Emerald greens; keep type colours), mockup first, owner approval before applying.
- **Battle menu size:** owner felt the BW action buttons look big / cut off at the bottom. That is the original art (the lower buttons' bottom edge is off-screen by design). Shrinking needs new art (not allowed). Owner decided (2026-11-04): **leave them as they are.**

## TO DO, in suggested priority order

1. ~~Checks~~ all done 2026-11-04 (test fw `scenarios/checks.json`, per-scenario `"extra"` C setup):
   double battle + mid-battle evo of the right Pokémon (found and fixed row 28), Mega and Z-Move in a trainer battle,
   Safari Zone (BW UI: Ball / Pokéblock / Go Near / Run), Bag in battle and back, Battle Pyramid bag (own items,
   "Pyramid" pocket, Use/Give/Toss), a 300-stack Items pocket scrolled to the end, an Egg's summary, Regirock's
   transition (full with R held, fast-forwarded otherwise), PC menus (consistent mixed case).
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
4. ~~Reports~~ done 2026-11-04. Owner: Outfits → keep ours and add the outfit's face to the Town Map, Frontier Pass,
   Hall of Fame — done (row 30). Gacha → added (row 29).
5. ~~New main menu with mugshot~~ done in 0.16.0 (row 31).

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
