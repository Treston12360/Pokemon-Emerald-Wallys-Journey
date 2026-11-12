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

### DLX-1 Lavaridge gym in 3D looks flat (WIP - fix made, waiting on owner's OK)
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

### DLX-2 Continue screen: shiny Pokemon icons (WIP - session 014PwRDj, 2026-11-12 10:35, `deluxe` too)
On the title screen's Continue/save summary, party icons of shiny Pokemon must use their shiny colours.

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

### DLX-7 3D: edge scenery rebuild cost (TODO)
`BuildBelt` in `src/platform/voxel3d/voxel_gl.c` takes ~12 ms every 2 chunks walked. Spread it over
frames or build it incrementally. Map meshes are already one per frame.

### DLX-8 3D: chunky people (ASK, show a test picture first)
Owner asked for trainers/NPCs to look 3D like the houses: extrude each sprite frame into blocks.
Make one test picture, owner decides.

### DLX-9 Lighting like Gamma Emerald (ASK)
Gamma Emerald's data is encrypted; we will not break it. Owner to send screenshots, or approve designing
it from Emerald 3DS's lighting (`voxel_lighting.c`).

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
Includes the Hex crash fix (`src/battle_util.c`), which should reach `deluxe` regardless (cherry-pick
commit "Fix freeze: Hex on a statused target...").

## How to build and test (short)
- Linux: `make -f Makefile.pc -j3`; Windows: `make -f Makefile.pc WIN=1`.
- 3D data (`voxel/` folder next to the exe) comes from Emerald 3DS's generators run on this repo's
  maps (see PROJECT_PC_PORT.md). `sand_bath` model is removed on purpose.
- Test scripts: `WJPC_SCRIPT=file` with commands `warp G N X Y`, `wait N`, `press KEY N`,
  `winshot FILE.ppm` (follow with `wait 2`), `set zoom N`, `trainerbattle ID`, `partyswap A B`,
  `enemyswap A B`, `enemymoves SLOT m1 m2 m3 m4`, `enemyhp SLOT HP`, `lockrng 1`, `quit`.
  3D under Xvfb needs `+extension GLX`.

## Done (2026-11-12)
- Fast forward to 20x with music; NPC/sprite limits raised; pop-in fix; Dewford corner; text-box holes.
- 3D overworld in the PC game (Emerald 3DS voxel code + our OpenGL backend): tiles, buildings, signs,
  trees, cliffs, shadows, zoom, battle transitions in 3D, no rebuild stutter at borders, Lavaridge
  (west side, cliffs, sand piles).
- Hex froze the game against a statused target (PC-only crash).
