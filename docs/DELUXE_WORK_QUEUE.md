# Deluxe Work Queue (PC version of Wally's Journey)

Owner's handoff list. To hand work to an AI, say: **"Work on DLX-7 from the Deluxe Work Queue"**
(file `docs/DELUXE_WORK_QUEUE.md`, repo `Treston12360/Pokemon-Emerald-Wallys-Journey`).

Before starting any item:
1. Read `docs/PROJECT_PC_PORT.md` ("START HERE") and `docs/WALLY_JOURNEY_OWNER_REQUIREMENTS.md`.
2. Work on branch **`deluxe-3d`** (3D work, newest) unless the item says `deluxe`. Other sessions push too:
   `git fetch` and rebase before every push.
3. Owner rules: old GBA `.sav` files must import into the PC game (that is the only hard rule now).
   **Show the owner pictures and ask before any new wording or visual change.** Be direct; no guessing.
4. When an item is done, tick it here (move it to "Done" with the date and commit) in the same push.

Status key: TODO = not started, WIP = started (see notes), ASK = needs the owner's answer first.

## Open items

### DLX-1 Lavaridge gym in 3D looks flat (DONE - owner 2026-11-12: "DLX-1 looks good"; ship the regenerated data, see note)
Status 2026-11-12 13:40 (session 019khs4X): the owner was shown the fixed gym (picture) and then reported only
the folded sign (DLX-19) - treat the model as accepted. The working voxel data with the fix is NOT in git
(voxel/ is not in the repo); whoever builds a release must regenerate buildings.bin/relief.bin with the
`lavaridge_gym_exterior` spec line below added to Emerald 3DS's `voxel_building_specs.py`:
`{"name":"lavaridge_gym_exterior","layout":"LAYOUT_LAVARIDGE_TOWN","rect":(2,11,6,5),"match_rows":(0,4),"ground":[GRASS],"parts":gym,"exact":GYM_EXACT}`
(insert before the `dewford_gym_exterior` entry; also remove the `sand_bath` spec).
Fix: Emerald 3DS had no exterior model for Lavaridge's gym. Added spec `lavaridge_gym_exterior`
(rect (2, 11, 6, 5), `parts: gym`, `match_rows (0, 4)`, ground GRASS) before `dewford_gym_exterior` in
`voxel_building_specs.py`, regenerated `buildings.bin` and `relief.bin`. Picture sent 2026-11-12.
Not in a build yet.
Second session (2026-11-12): the fix above lives only in the first session's workspace (spec + regenerated
.bin files are not in the repo, and the voxel data folder is not in the repo either). Rebuilt from scratch here:
`voxel/*.bin` from Emerald 3DS's own builder on a vanilla Emerald ROM (vtree.build_tree + the generators; trees.rgba5551
from the .3dsx RomFS). With that standard data, deluxe-3d already draws Lavaridge's gym as a full 3D gym, identical
to Dewford's (picture sent). So the flat gym comes from data generated against our own tree, where WJ's Lavaridge
art (metatiles 29F/2A7 in the gym's east column) no longer matches the generic `gym` spec. Running the generators
on our tree also skips the Cycling Road and all interiors (art differs) - needs a decision on which data the
release ships. Owner asked which build showed the flat gym.
Owner: "it should look like how the gym looked in Dewford in 3D", and like Emerald 3DS's.
The 3D model comes from Emerald 3DS's generator (`gym()` in `voxel_building_specs.py`, built by
`gen_voxel_buildings.py`). Compare Lavaridge's gym to Dewford's in game (`WJPC_3D=1`, `warp 0 12 8 16`
vs `warp 0 11 ...`) and to Emerald 3DS screenshots; find why Lavaridge's is drawn flatter (wrong model
picked, art differs from vanilla so the model did not match, or the roof is drawn as ground).

### DLX-3 Smooth battles and menus (TODO, approved "option 1")
In-between frames so battles and menus feel high frame rate, like the overworld's smooth mode.

### DLX-4 Battle transitions on the zoomed-out 2D view (TODO - OWNER SAYS: GO AHEAD AND FINISH IT)
Owner (2026-11-12): any Claude may finish DLX-4 without asking first; send him pictures/a short clip when done.
Problem: in 2D with the zoomed-out "wide view", when a trainer/wild battle starts the screen snaps back to the
small GBA picture and plays the transition there. Wanted: the transition plays over the wide field instead.
How the 3D version already does it (copy this approach):
- `src/platform/gba_easy_draw.c` records, for every GBA pixel, which layer drew it in `gPcPixelSource[240*160]`
  (0 backdrop, 1-4 BG0-3, 5 sprite, 6 priority-0 sprite) and the brightness fade in `gPcPixelFade[]`
  (BLDY, blend modes 2/3: + = to white, - = to black, 0..16).
- `src/platform/sdl2.c` (search `s3DTransition`): when `WideView_BattleTransitionOnField()` is true and the
  frame is not in field mode, it builds a 240x160 ARGB overlay: pixels drawn by BG1-3 / normal sprites
  (sources 2-5, i.e. the map) become transparent carrying only the fade as black/white alpha; everything
  else (the wipe's black, the transition's BG0 graphics) is the opaque GBA pixel. `DrawScene` then draws
  the 3D picture full screen and the overlay "cover"-scaled on top.
- `WideView_BattleTransitionOnField()` (`src/platform/wide_view.c`) = callback2 is CB2_Overworld/
  CB2_OverworldBasic, a layout is loaded and `IsOverworldBgSetup()`.
For 2D: during the same condition keep drawing the wide field (`DrawFieldScene`, `sFrameInfo.mode ==
WIDE_VIEW_FIELD` path) instead of switching to the GBA framing, then draw the same overlay on top. Two choices
to make (pick what looks right, show the owner): (a) overlay only over the GBA-sized centre at field scale and
extend the wipe's black/fade outward to the screen edges, or (b) scale the overlay to cover the whole screen
like 3D. Wavy/ripple transitions (they scroll BG lines) do not distort the wide field yet, in 3D or 2D - nice
to have: apply the per-line BG offsets the transition writes to the wide field rows.
Test: save `/home/claude/keep/owner_brawly.sav`-style save (any save), script `trainerbattle 774` (Brawly) or
walk into grass; `winshot` frames during the transition (`WJPC_SCRIPT`, see "How to build and test").

### DLX-5 3D battle backgrounds + settings option (WIP - session 014PwRDj; owner: "you can choose the wording")
NOTE 2026-11-12: nothing to copy - only `voxel_battle.h` (Emerald 3DS header) exists; no battle code here or in
pokeemerald-multiplatform. Build it: voxel world from a low battle camera behind the battle (in place of BG3).
Emerald 3DS builds 3D arenas from the battle background art (`src/platform/voxel3d/voxel_arena.c`,
already copied). Owner wants it as its own on/off option in settings. See also `docs/PROJECT_NEW_3D_BGS.md`.

### DLX-6 In-game options for the 3D overworld (Done - session 014PwRDj, 2026-11-12; owner: "you can choose the wording")
Settings > Video > "3D World" On/Off (last row). Takes effect at once, no restart: the renderer is OpenGL whenever
`voxel/regions.bin` is next to the game, and the 3D data loads the first time 3D is turned on (`Try3DInit`,
sdl2.c). Saved as `world3d=` in WallysJourney.cfg (the "Play in 3D.bat" `WJPC_3D` still overrides it at start).
Shows "N/A" when the voxel folder or OpenGL is missing. `PLATFORM_SETTING_WORLD3D`, `Platform_Is3DAvailable()`.
Tested in Wine: Off -> On (3D at once) -> walk -> Off (2D at once); video sent.

### DLX-8 3D: chunky people (ASK - owner 2026-11-12: show an example before deciding to implement)
Owner asked for trainers/NPCs to look 3D like the houses: extrude each sprite frame into blocks.
Make one test picture, owner decides.

### DLX-9 Lighting like Gamma Emerald (WIP - OWNER SAYS: GO AHEAD AND FINISH IT)
Owner (2026-11-12): any Claude may finish DLX-9; send him before/after pictures (2D and 3D) when done.
Tools and data to redo any reading: `tools/gamma_lighting/README.md`; cloud textures `docs/gamma/gamma_clouds.bin`
(copy into the game's `voxel/` folder).
The owner got Gamma Emerald's files from its developer, who said we may use them (including textures) for
this project. Everything read from them is in `docs/GAMMA_LIGHTING_VALUES.md` (exact numbers and shader math;
how they were read is at the top of that file). Program file `PokemonEmerald-Win64-Shipping.exe` (owner's PC,
connected folder `gamma-emerald-ea-windows`) settled: auto-exposure "extend luminance range" default 0,
exposure bias default 1.0 -> Gamma's screen multiplier 0.36; GI (Lumen) default 0 = Gamma has NO bounce light;
blur kernels capped at 32 samples (r.Filter.LoopMode 0) so every bloom stage is 31 texels of its level.
Built (commit on deluxe-3d, off by default, turn on with `WJPC_3D_GAMMALIGHT=1`), all in `voxel_gl.c` +
`voxel_lighting.c`: Gamma's sun direction/colour/intensity (bake keeps only sun visibility + contact,
`gVoxelGammaLight`), sky light from Gamma's cubemap as SH, haze formula, cloud shadows with Gamma's two
textures (`voxel/gamma_clouds.bin` = T_NoisePuddles + T_NoiseRough, 1024x1024 8-bit each; copy of it in
`docs/gamma/`),
half-float scene + screen pass: 6-stage bloom (UE BloomSetup, threshold 0.4, intensity 6, UE default tints),
depth of field with Gamma's lens, saturation 1.25 in AP1, UE FilmToneMap (slope 1, toe 0.3, shoulder 1).
Decision (owner: "whatever looks best"): exposure 2.06 instead of Gamma's 0.36, because GBA art is drawn
fully lit; 2.06 returns sunlit ground to its own colours through the same curve.
Constants already in code (do not re-derive): sun dir to-sun (east 0.1747, up 0.6428, south 0.7459), sun
colour (0.788,0.738,0.700) x 3/pi, sky tint (1.0,0.701,0.271) x 0.32, SH table `sGammaSh`, exposure `sExposure`.
Open:
- 3D: faint vertical light streaks over buildings in Littleroot with the mode on (also with bloom off,
  reflections off, shadows off) - find the cause (suspect depth-of-field gather or the depth texture).
- 2D mode: only an offline preview exists (`tools/gamma_lighting/gamma2d.py`; owner liked it). Build it in
  the 2D renderer (`sdl2.c` field/wide view, GL or software): per screen pixel, colour = linear(GBA colour) x
  (sun x 0.6428 x clouds(world pos) + sky(up)), + bloom, x 2.06, saturation 1.25 in AP1, UE film curve, sRGB.
  World position = map pixel / 16 tiles x 64 cm (Gamma X = north = up the map, Y = east). Clouds scroll with
  game time; they must stay fixed to the map while the camera moves. Indoors: no sun/clouds (see caves).
  Same on/off switch as 3D (`WJPC_3D_GAMMALIGHT`, later a menu option - ask the owner for wording).
- Not yet built: light rays (god-ray planes, math in the doc), dark caves/gyms trigger (colour gain 0.49,
  vignette 3/2), lamps. Show the owner pictures before turning anything on by default.

### DLX-20 3D: black person-shaped silhouette on the ground in Littleroot (TODO)
Seen in every 3D screenshot of Littleroot north of the player's house (before and after the lighting work): an
all-black sprite-shaped card standing on the grass. Probably a person/object whose sprite frame or palette
is not loaded for the 3D card. Find which object event it is and fix.

### DLX-21 Next owner test package (TODO)
Last package sent was `alpha8c-3d-test.zip` (exe + DLLs + `voxel/` + "Play in 3D.bat"). It does NOT have the
Lavaridge gym data (DLX-1), the Hex fix, DLX-7, or the lighting. Build a new Windows package with
`make -f Makefile.pc WIN=1`, regenerated voxel data (DLX-1 note), `gamma_clouds.bin`, and a second .bat with
`WJPC_3D_GAMMALIGHT=1` so the owner can compare.

### DLX-10 Encore: cursor on the forced move (Done - session 014PwRDj; owner 2026-11-12: "Yes, jump to the move")
Done: `InitMoveSelectionsVarsAndStrings` (battle_controller_player.c) puts the cursor on `encoredMovePos` while
Encored (GBA and PC). Tested on PC: cursor moved to slot 2, B, Fight -> back on the Encored move; without Encore
it stays on slot 2.
Offered: while Encored, the move cursor jumps to the locked move. No new wording.

### DLX-11 Trainer spotting with many people loaded (TODO)
Limits raised to 64 people; test that trainers still spot the player correctly with extra people.

### DLX-12 Pop-in: people on neighbouring maps; maps with a different tile set (TODO)
2D wide view: people on connected maps are not drawn; about 70 map links use another tile set and are
still blank zoomed out.

### DLX-13 Other empty map corners (TODO)
Find and fix corners like Dewford's (zoomed out shows bare filler). Show pictures.

### DLX-14 Sprite colour / graphics memory limits (Done on PC - session 014PwRDj, 2026-11-12)
Owner 2026-11-12: "Because we are a native windows code now we have no limitations and are allowed to change
the rom however we want as long as the original .sav files are compatible or at the least importable" -> PC build:
no sprite palette / sprite VRAM limits.
Done (PC build only; the GBA keeps its hardware limits): 64 sprite palettes (was 16) and 4096 sprite tiles / 128 KB
(was 1024 / 32 KB). `OBJ_PALETTE_COUNT`, `OBJ_PLTT_EXTRA_SIZE`, `VRAM_SIZE`, `OBJ_VRAM0_SIZE`, `TOTAL_OBJ_TILE_COUNT`
(include/gba/defines.h); `struct OamData` on PC has tileNum:12 / paletteNum:6 (12 bytes, `OAM_SIZE` 128*12).
The extra palettes sit right after the GBA's 16 in PLTT / gPlttBuffer*, the extra tiles right after OBJ VRAM,
so the renderers and OBJ_PLTT_ID(n) just work. Allocators hand out the extra room only once the GBA's is full.
Fades/blends/tints/time of day/weather (palette.c, field_weather.c, overworld.c): the extra palettes follow
any effect that covers every GBA sprite palette; per-palette masks (`1 << (16 + n)`) cannot name them (only
matters on screens that never fill 16). Fixed `< 16` / `> 16` "found" checks (event_object_movement.c,
battle_transition/intro.c, field_weather_effect.c, LoadCustomWeatherSpritePalette, seasonal grass).
Test aid: `WJPC_TEST_EXTRA_FIRST=1` puts every sprite in the extra room. `WJPC_TEST_GBA_LIMITS=1` puts the GBA caps back;
`WJPC_TEST_CROWD=N` acts as if N palettes were taken (crowded scene); `WJPC_TEST_PAL_LOG=1` prints when slots >15 /
tiles >1023 are used or none is free. Normal play on Slateport/Lilycove/Routes 104/110/119 (zoomed out) stays under
16 palettes and 1024 tiles today; the demo (crowd 13, Slateport) loses the autumn leaves with GBA caps, not on PC. Checked: Slateport, Route 119 (autumn
grass, rain), warp fades, a trainer battle: pixel-identical to a normal run (only falling leaves, which move
differently every run anyway); 3D Slateport the same.

### DLX-15 Test on real Windows hardware (TODO, owner)
3D builds were tested in Linux and Wine only. Owner tests `alpha8c` on his PC.

### DLX-16 Merge `deluxe-3d` into `deluxe` (TODO, after owner OK on 3D)
The Hex crash fix is already on `deluxe` too (applied separately). Merge everything else after the owner OKs 3D.

### DLX-17 3D: a ground texture draws wrong in Slateport and many other places (TODO)
Owner report 2026-11-12. Find the metatile(s) (compare 2D and 3D at the same spot) and fix.

### DLX-18 3D: route switching still a little jarring; no pop-in at all (TODO)
Owner: "just load everything so nothing has to pop in". Preload connected maps' meshes, people and
trees ahead of the border (map meshes are already built one a frame; the edge scenery rebuild is
DLX-7). People on neighbouring maps should already be drawn.

### DLX-19 3D: Lavaridge gym sign folded in half; check the gym door (TODO)
The new gym model (DLX-1) shows the sign bent. The model is visual only, but confirm in game that
the door still leads into Lavaridge's gym.

## How to build and test (short)
- Linux: `make -f Makefile.pc -j3`; Windows: `make -f Makefile.pc WIN=1`.
- 3D data (`voxel/` folder next to the exe) comes from Emerald 3DS's generators run on this repo's
  maps (see PROJECT_PC_PORT.md). `sand_bath` model is removed on purpose.
- Test scripts: `WJPC_SCRIPT=file` with commands `warp G N X Y`, `wait N`, `press KEY N`,
  `winshot FILE.ppm` (follow with `wait 2`), `set zoom N`, `trainerbattle ID`, `partyswap A B`,
  `enemyswap A B`, `enemymoves SLOT m1 m2 m3 m4`, `enemyhp SLOT HP`, `lockrng 1`, `quit`.
  3D under Xvfb needs `+extension GLX`.

## Done (2026-11-12)
- DLX-2 Continue screen shiny icons: already worked (979563a0); verified by two sessions with the owner's
  save and a six-shiny save, picture sent. Reopen only if the owner sees it again (ask which build).
- DLX-7 Edge scenery belt built piece by piece (a0360473, session 014PwRDj): worst frame ~8 ms -> ~2 ms.
- Heap text-printer crash on map load/reload (f89e68f6); shake-animation pointer crash; contest move
  background overrun; empty party icon slots (session 014PwRDj).
- Fast forward to 20x with music; NPC/sprite limits raised; pop-in fix; Dewford corner; text-box holes.
- 3D overworld in the PC game (Emerald 3DS voxel code + our OpenGL backend): tiles, buildings, signs,
  trees, cliffs, shadows, zoom, battle transitions in 3D, no rebuild stutter at borders, Lavaridge
  (west side, cliffs, sand piles).
- Hex froze the game against a statused target (PC-only crash).
