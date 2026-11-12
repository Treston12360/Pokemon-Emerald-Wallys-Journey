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

### DLX-1 Lavaridge gym in 3D looks flat (ASK - fixed in session 019khs4X's data, owner to confirm)
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

### DLX-4 Battle transitions on the zoomed-out 2D view (WIP)
Approved: play the transition across the wide view instead of zooming back to the GBA screen.
The 3D version is done (`gPcPixelSource`/`gPcPixelFade` in `gba_easy_draw.c`, composited in `sdl2.c`
`s3DTransition`). Do the same over the 2D wide field. Wavy/ripple transitions do not distort yet
(3D or 2D). Show pictures before merging.

### DLX-5 3D battle backgrounds + settings option (ASK wording, show pictures)
Emerald 3DS builds 3D arenas from the battle background art (`src/platform/voxel3d/voxel_arena.c`,
already copied). Owner wants it as its own on/off option in settings. See also `docs/PROJECT_NEW_3D_BGS.md`.

### DLX-6 In-game options for the 3D overworld (ASK wording)
3D is on today only with `world3d=1` in `WallysJourney.cfg` or `WJPC_3D=1` ("Play in 3D.bat").
Proposed menu line "3D World: On/Off" - get the owner's OK on wording first.

### DLX-8 3D: chunky people (ASK, show a test picture first)
Owner asked for trainers/NPCs to look 3D like the houses: extrude each sprite frame into blocks.
Make one test picture, owner decides.

### DLX-9 Lighting like Gamma Emerald (WIP - session 019khs4X; owner approved going ahead)
The owner got Gamma Emerald's files from its developer, who said we may use them (including textures) for
this project. Everything read from them is in `docs/GAMMA_LIGHTING_VALUES.md` (exact numbers and shader math;
how they were read is at the top of that file). Program file `PokemonEmerald-Win64-Shipping.exe` (owner's PC,
connected folder `gamma-emerald-ea-windows`) settled: auto-exposure "extend luminance range" default 0,
exposure bias default 1.0 -> Gamma's screen multiplier 0.36; GI (Lumen) default 0 = Gamma has NO bounce light;
blur kernels capped at 32 samples (r.Filter.LoopMode 0) so every bloom stage is 31 texels of its level.
Built (commit on deluxe-3d, off by default, turn on with `WJPC_3D_GAMMALIGHT=1`), all in `voxel_gl.c` +
`voxel_lighting.c`: Gamma's sun direction/colour/intensity (bake keeps only sun visibility + contact,
`gVoxelGammaLight`), sky light from Gamma's cubemap as SH, haze formula, cloud shadows with Gamma's two
textures (`voxel/gamma_clouds.bin` = T_NoisePuddles + T_NoiseRough, 1024x1024 8-bit each, NOT in git),
half-float scene + screen pass: 6-stage bloom (UE BloomSetup, threshold 0.4, intensity 6, UE default tints),
depth of field with Gamma's lens, saturation 1.25 in AP1, UE FilmToneMap (slope 1, toe 0.3, shoulder 1).
Decision (owner: "whatever looks best"): exposure 2.06 instead of Gamma's 0.36, because GBA art is drawn
fully lit; 2.06 returns sunlit ground to its own colours through the same curve.
Open:
- 3D: faint vertical light streaks over buildings in Littleroot with the mode on (also with bloom off,
  reflections off, shadows off) - find the cause (suspect depth-of-field gather or the depth texture).
- 2D mode: only an offline preview exists (`gamma2d.py`, session workspace) - apply the same chain to the
  2D field in `sdl2.c`/wide view (ground lit flat: sun x 0.643 x clouds + sky(up)).
- Not yet built: light rays (god-ray planes, math in the doc), dark caves/gyms trigger (colour gain 0.49,
  vignette 3/2), lamps. Show the owner pictures before turning anything on by default.

### DLX-10 Encore: cursor on the forced move (ASK)
Offered: while Encored, the move cursor jumps to the locked move. No new wording.

### DLX-11 Trainer spotting with many people loaded (TODO)
Limits raised to 64 people; test that trainers still spot the player correctly with extra people.

### DLX-12 Pop-in: people on neighbouring maps; maps with a different tile set (TODO)
2D wide view: people on connected maps are not drawn; about 70 map links use another tile set and are
still blank zoomed out.

### DLX-13 Other empty map corners (TODO)
Find and fix corners like Dewford's (zoomed out shows bare filler). Show pictures.

### DLX-14 Sprite colour / graphics memory limits (ASK - plan first)
Present a plan to raise the sprite palette and sprite VRAM limits before coding.

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
