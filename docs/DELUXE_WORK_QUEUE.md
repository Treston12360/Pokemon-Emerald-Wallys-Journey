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

### DLX-2 Continue screen: shiny Pokemon icons (ASK - works in current builds; which build did the owner see?)
Checked 2026-11-12 (session 014PwRDj): shiny icons on the Continue screen were added in 979563a0 (0.18.15, Nov 10)
and are in `deluxe` and `deluxe-3d`. Tested owner's save (shiny Breloom) and a test save with six different shinies
(scenario tools/wj_testfw/scenarios/shiny6.json): GBA (mGBA) and PC identical, all shiny; PC window in 2D and 3D
too. Waiting for the owner to say which build/save showed normal colours.
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

### DLX-7 3D: edge scenery rebuild cost (Done - session 014PwRDj, 2026-11-12)
`BuildBelt` in `src/platform/voxel3d/voxel_gl.c` took ~5-12 ms every 2 chunks walked.
Done: the belt is now 8x8 pieces kept in a cache (`BeltChunk`, 256 slots). Walking builds only the
new pieces, at most 10 a frame, and puts the belt together on a frame of its own (one-pass page sort).
New map / new world digest / new atlas still build it all at once, as before. Checked in Wine (software
GL): worst frame from ~8 ms to ~2 ms typical (3-6 ms peaks); 73 step-built belts compared byte for byte
with a full rebuild: all identical. Nothing on screen changes. Test aid: `WJPC_TEST_WINDOW=1` makes a
`WJPC_SCRIPT` run draw the 3D view every frame (otherwise it only draws on `winshot`).

### DLX-8 3D: chunky people (ASK, show a test picture first)
Owner asked for trainers/NPCs to look 3D like the houses: extrude each sprite frame into blocks.
Make one test picture, owner decides.

### DLX-9 Lighting like Gamma Emerald (WIP)
2026-11-12: the owner got Gamma Emerald's unpacked files from its developer (Unreal Engine 5 project,
"Gamma-Pak-Recovery" archive, 51 parts). Study how its lighting is set up (sun, sky, day/night,
post-process) and recreate the look in our OpenGL 3D view (`voxel_gl.c` shader, `voxel_lighting.c`).
Do not copy their assets. Show the owner before/after pictures.
Findings (2026-11-12, from the level files; values are binary, only the setup was read):
- One shared level (`MAP_Hoenn_Persistant`) holds the outdoor lighting for every town: a sun
  (directional light), a sky light (ambient from the sky), exponential height fog and a post-process volume.
- The sky is a stylized-sky kit (SoStylized `BP_StylizedSky`: "Dreamy" sky colours, cartoon clouds, a moon,
  **cloud shadows drifting over the ground** via a light function, a stylized fog post-process) plus weather
  presets (clear, light rain, distant thunderstorm).
- Colour grading by LUTs (Chameleon pack: day-to-night, moonlight, candle light).
- Towns (Littleroot, Dewford) add point/spot lights and animated **god-ray planes**; caves switch lighting
  with a trigger (`BP_CaveLightTrigger`). Sprites use a lit sprite material, so people get the sun's light too.
Exact numbers now read: see `docs/GAMMA_LIGHTING_VALUES.md` (sun, sky light, fog, cloud shadows, sky).
Plan for our view (needs owner's OK on pictures): sun shading per face (warm top, cooler sides), sky-tinted
ambient, distance fog in the sky colour, drifting cloud shadows (our own noise texture), a light colour
grade, day/night from the game clock, darker caves. Each as a separate on/off so the owner can compare.

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
- Fast forward to 20x with music; NPC/sprite limits raised; pop-in fix; Dewford corner; text-box holes.
- 3D overworld in the PC game (Emerald 3DS voxel code + our OpenGL backend): tiles, buildings, signs,
  trees, cliffs, shadows, zoom, battle transitions in 3D, no rebuild stutter at borders, Lavaridge
  (west side, cliffs, sand piles).
- Hex froze the game against a statused target (PC-only crash).
