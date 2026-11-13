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

**NO GUESSING, EVER (owner, 2026-11-13).** Every 3D fix must come from how Emerald 3DS does it: find the exact
handling in its renderer (`3ds_port/src/3ds_video.c`, 13k lines, in the Emerald 3DS source the owner provided) and port
that code. Gamma-only effects (lighting, glow) must use values read from Gamma's files
(`docs/GAMMA_LIGHTING_VALUES.md`). If neither source covers it, ask the owner and show options; never invent a fix
or a number. Earlier sessions rewrote much of the 3DS renderer instead of porting it; that is where the seams,
water lines and other polish bugs came from.

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

### DLX-3 Smooth battles and menus (DONE 2026-11-13, session 019khs4X)
In-between frames so battles and menus feel high frame rate, like the overworld's smooth mode.
Done (fdf25c34): sdl2.c end of VDraw. With Frame rate 120+ (Settings), GBA-screen frames (battles, menus,
also the 3D-battle overlay) get (rate/60 - 1) in-between pictures: sprites in the same OAM slot with the same
picture/shape/size that moved 1..16 px, and BG0-3 H/V scroll that moved 1..16 px, are put part of the way and the
frame is redrawn with gba_easy_draw.c DrawFramePlain (no interrupts/DMA); DrawScreen(t) shows them before the
frame. Frames whose registers change between lines (HBlank/VCount effects, HBlank DMA: gPcFrameHadLineEffects,
e.g. the battle intro slide) are left alone. Test aids: WJPC_TEST_INTERP=N (force N steps), WJPC_WINSHOT_T=t,
WJPC_NO_INTERP=1. Checked: send-out sparkles, party balls, healthbox bob drawn half way (pictures).

### DLX-4 Battle transitions on the zoomed-out 2D view (DONE 2026-11-13, session 019khs4X; owner sent pictures + clip)
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
Done (5b... see git log "DLX-4"): option (b). sdl2.c: s2DTransition = no 3D, last frame was the field, now
not field and WideView_BattleTransitionOnField(). The map texture keeps the last field picture (sLastField /
sFieldImage); palette fades/flashes of the transition are carried to it by a per-channel line fitted from the field
picture to the GBA picture's map pixels; the overlay (as 3D) is drawn cover-scaled on top; the UI layer is hidden.
WJPC_OLD_2D_TRANSITION=1 = old behaviour. Note: test scripts must set WJPC_TEST_WINDOW=1 to see it (otherwise
frames between winshots are not drawn and there is no "last field picture").
Test: save `/home/claude/keep/owner_brawly.sav`-style save (any save), script `trainerbattle 774` (Brawly) or
walk into grass; `winshot` frames during the transition (`WJPC_SCRIPT`, see "How to build and test").

### DLX-5 3D battle backgrounds + settings option (Done, first version - session 014PwRDj, 2026-11-12)
REDONE 2026-11-13 (session 019khs4X, owner: "make sure they are exactly how 3DS Emerald does it"): the first
version was a stand-in (own stage pick, fixed camera, nothing else). Now ported from Emerald 3DS: voxel_battle.c
(stage search with its scores, battler shadows), ctr_voxel.c BattleCamera (glide: 100 frames, crane 16, dolly zoom
0.40, push 0.16, quake 0.07, bars), 3ds_video.c RenderBattleWorld rules (world only while BG3 is on and holds the
scenery; a move's background = 2D battle; BG1/BG2 hidden while the intro slides, not in link battles; window
curtain shows the backdrop; BG3 scroll shakes the camera) and its effects (speed lines, indigo vignette, landing
flash, cinema bars, zoom blur). The 3DS view (400x240, GBA picture at 1.4x, (120,112) on (200,192)) is drawn
cropped to the GBA picture (off-axis projection in voxel_gl.c). Differences kept on purpose: the 3DS moves its
battlers (enemy up 8, player down 2) - ours are not moved, so the stage search uses the GBA positions; the bars sit
on our picture's edges (on the 3DS they are its screen's edges, mostly outside the GBA picture); the 3DS's own
bloom/diorama blur and HD text box are not part of our renderer. Game hooks: PcBattleIntro_Sliding
(battle_intro.c), PcBattleBg_MoveBgShown (battle_bg.c, set in LoadMoveBg).
Owner: "you can choose the wording". Settings > Video > "3D Battles" On/Off (`battle3d=` in the cfg, default On - owner 2026-11-12: "Starts on";
independent of "3D World"). In a battle the 3D world is drawn behind the battle: sdl2.c draws `sdl3DTexture` (3:2)
then the GBA picture with BG3 and the backdrop left transparent (`gPcPixelSource` 0/4; brightness fades kept as
in the 3D transition). voxel_gl.c `Voxel3D_SetBattle`: Emerald 3DS's battle camera (VOXEL_BATTLE_PITCH 30,
DISTANCE 7, FOV 35, voxel_battle.h), no overworld people; the field camera is saved and put back after.
Stage: `VoxelBattle_PickStage` (voxel_world.c) scores spots within 8 tiles of the player: fewest blocked tiles,
water (unless surfing) and height changes in the camera's view, near ones most; picked once per battle.
Palette fades: `VoxelWorld_ScreenFade` on PC fits the fade of BG palettes 2-4 in battles (3DS logic).
Checked in Wine: Slateport, Route 119, Route 104, Lilycove, Route 110, Fortree; intro, move, win -> field camera
back as before. Not yet: contact shadows under the battlers (`VoxelBattle_Shadows`), camera always faces north,
link/contest/frontier facilities untested. Test aid: `battlerhp BATTLER HP` script command.

### DLX-6 In-game options for the 3D overworld (Done - session 014PwRDj, 2026-11-12; owner: "you can choose the wording")
Settings > Video > "3D World" On/Off (last row). Takes effect at once, no restart: the renderer is OpenGL whenever
`voxel/regions.bin` is next to the game, and the 3D data loads the first time 3D is turned on (`Try3DInit`,
sdl2.c). Saved as `world3d=` in WallysJourney.cfg (the "Play in 3D.bat" `WJPC_3D` still overrides it at start).
Shows "N/A" when the voxel folder or OpenGL is missing. `PLATFORM_SETTING_WORLD3D`, `Platform_Is3DAvailable()`.
Tested in Wine: Off -> On (3D at once) -> walk -> Off (2D at once); video sent.

### DLX-8 3D: chunky people (ASK - owner 2026-11-12: show an example before deciding to implement)
Owner asked for trainers/NPCs to look 3D like the houses: extrude each sprite frame into blocks.
Make one test picture, owner decides.
2026-11-12: mock-up sent (flat now / even-thickness blocks / rounded blocks, game camera and turned 35 degrees;
player, Hiker, Lass, Wally). Waiting for the owner to pick one or say no.

### DLX-9 Lighting like Gamma Emerald (DONE except lamps, 2026-11-13, session 019khs4X)
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
DONE 2026-11-13 (session 019khs4X, pictures sent):
- 3D streaks: cause = cloud shadow looked up at (x,z) ignoring height, so walls/roofs repeated one ground sample
  upward. Fix: project along the sun (a directional light function is in light space) - voxel_gl.c clouds().
  Also: no gamma_clouds.bin -> a black 1x1 texture is bound (it used to read whatever was on unit 6/7).
  Debug aids: WJPC_3D_GDEBUG=2 (vShade), 3 (normals), 4 (cloud term); WJPC_3D_NODOF=1.
- 2D: src/platform/gamma2d.c (+ include/platform/gamma2d.h), called from sdl2.c on the field picture when
  WJPC_3D_GAMMALIGHT=1 and 3D is off. Same light as 3D seen from above, UE bloom pyramid (half size, 6 stages,
  31/3 sigma per level, tints), exposure 2.06, AP1 saturation 1.25, film curve via a 48^3 table. Threaded (SDL
  threads, up to 3 workers); ~6 ms/frame at 426x240 on a slow 2-core VM. WJPC_GAMMA2D_TIME=file logs timing.
- Vignette: read from Gamma's compiled tonemap shaders (global shader archive, VS 3670_f0 + PS 3584_f3):
  p = (sx, sy*H/W) * sqrt2/sqrt(1+(H/W)^2), mask = 1/(|p*I|^2+1)^2. Engine default I = 0.4 (FPostProcessSettings
  ctor stores 0.4 at offset 0x5C0 = VignetteIntensity, offset read from its reflection params in the exe).
  Applied in 2D and 3D. Unverified link: that the C++ passes the setting to the shader unscaled.
- Caves: BP_CaveLightTrigger (only placed in Granite Cave) and BP_BrawlyLightTrigger: gain 0.49, vignette 3 / 2.
  WideView_GammaRoom(): Granite Cave 1F/B1F/B2F/Steven's room = 1, Dewford gym = 2. 2D and 3D.
- Light rays: Gamma has the material but NO ray actors placed in any of its maps (Littleroot, Dewford, the
  persistent map were read) - nothing to copy.
- RECHECK 2026-11-13 (owner: "double check the gamma lighting"), against Gamma's files again:
  * 3D sky light table (sGammaSh) was wrong: ~3x too strong, up-term sign flipped (3D looked washed out and
    too blue in shade). Refitted from cube16 (irradiance/pi, 400 directions); now equals the 2D value up.
  * Gamma's LUT shader (global CombineLUTs, e.g. 3416_f5) also does UE's ExpandGamut (1.0) and BlueCorrection
    (0.6); its matrices are literals there and match UE's math exactly. Added in 3D and 2D. Greys unchanged,
    so exposure 2.06 still holds.
  * Cloud textures: instance overrides "Cloud Texture" = T_NoisePuddles; distortion = parent's T_NoiseRough
    (code was right, doc label fixed).
  * Checked OK: sun dir/colour/intensity, sky tint, saturation 1.25 (camera beats the volume's 1.20/1.19/1.12),
    bloom, haze (no visible effect at our distances), vignette 0.4, cave gain/vignette, cloud maths.
  * Known small differences: no specular (UE Default Lit, roughness 1, ~4%); the static SkyLight actor and
    ShadowAmount 0.99 not modelled; DOF focuses on the player (Gamma: fixed 450 with a 500 arm); cave gain
    applied before expand gamut (UE: after).
- NOT DONE: lamps (Dewford street lamps/campfire point lights: positions in MAP_Dewford, BP_StreetLamp values in
  the doc). Needs UE point-light units -> our scale worked out exactly before building.

### DLX-20 3D: black person-shaped silhouette on the ground in Littleroot (RESOLVED - test-script artifact, see below)

**RESOLVED 2026-11-13: not a game bug.** The silhouette was the player, drawn black because the test script warped while still on the title screen, so the save never loaded and the player graphics were never set up. It shows in plain 2D too. Fix for test scripts: `wait 500`, `press START 5`, `wait 200`, `press A 5`, `wait 200`, `press A 5`, `wait 400`, then `warp`. With that, the player draws correctly in 2D and 3D.
Tried in Wine with the 3D data from vanilla Emerald: late-game save (go.sav) and an early-game Littleroot save
(trucks, Mom, Birch; wj_testfw scenario MAP_LITTLEROOT_TOWN 7,10 + FLAG_SYS_POKEMON_GET + FLAG_SYS_POKEDEX_GET),
2D and 3D, around and north of the player's house: no black figure. Most likely it is in the data generated from
our maps (session 019khs4X / DLX-9 screenshots), e.g. a reused building piece whose texture page is missing
(see DLX-19: our-map data drew with missing building textures here). Needs that data, or a screenshot with the
spot, to go further.
Seen in every 3D screenshot of Littleroot north of the player's house (before and after the lighting work): an
all-black sprite-shaped card standing on the grass. Probably a person/object whose sprite frame or palette
is not loaded for the 3D card. Find which object event it is and fix.

### DLX-21 Next owner test package (Done 2026-11-13: alpha10-3d sent after DLX-3/4/9/19; alpha9-3d sent - exe, SDL2 DLLs, regenerated voxel/ with gamma_clouds.bin, bats: 3D, 3D+Gamma lighting, 3D+Gamma+chunky test)
Last package sent was `alpha8c-3d-test.zip` (exe + DLLs + `voxel/` + "Play in 3D.bat"). It does NOT have the
Lavaridge gym data (DLX-1), the Hex fix, DLX-7, or the lighting. Build a new Windows package with
`make -f Makefile.pc WIN=1`, regenerated voxel data (DLX-1 note), `gamma_clouds.bin`, and a second .bat with
`WJPC_3D_GAMMALIGHT=1` so the owner can compare.

### DLX-10 Encore: cursor on the forced move (Done - session 014PwRDj; owner 2026-11-12: "Yes, jump to the move")
Done: `InitMoveSelectionsVarsAndStrings` (battle_controller_player.c) puts the cursor on `encoredMovePos` while
Encored (GBA and PC). Tested on PC: cursor moved to slot 2, B, Fight -> back on the Encored move; without Encore
it stays on slot 2.
Offered: while Encored, the move cursor jumps to the locked move. No new wording.

### DLX-11 Trainer spotting with many people loaded (Done - session 014PwRDj, 2026-11-12)
Limits raised to 64 people; test that trainers still spot the player correctly with extra people.
Code: CheckForTrainersWantingBattle loops all OBJECT_EVENTS_COUNT (64) objects; nothing sized 16 on the path.
Test (Linux, early-game save, 1080p scene, zoom 7, WJPC_TEST_WINDOW=1): Route 116 at (28,5): 25 people active
(slots up to 25, `objcount`), School Kid Jerry (18th object on the map, sight 4) spotted the player and walked up;
Route 110 Psychic Edward spotted too. Test aid fix: script `set` now applies through Platform_SetSetting (the zoom
and resolution only take effect that way, and only with WJPC_TEST_WINDOW=1, which draws every frame).

### DLX-12 Pop-in: people on neighbouring maps; maps with a different tile set (Done - session 014PwRDj, 2026-11-12)
2D wide view: people on connected maps are not drawn; about 70 map links use another tile set and are
still blank zoomed out.
Tile sets DONE (2026-11-12): a neighbour with another secondary tile set was drawn with the current map's tiles
(wrong pictures, e.g. Lavaridge cones where Route 113's ash is). wide_view.c now keeps the neighbour's own tiles
(decompressed) and colours (season-adjusted, then moved like the current map's colours this frame: a per-channel
line fitted from stored to shown colours, so time of day, weather and fades follow) and draws its tiles/palettes
from them (`GetForeignTiles`, `sEntryForeign`); its layer types come from its own attributes. Checked Route 112 ->
113 (ash) against standing on Route 113: same. Test aid: WJPC_TEST_NO_FOREIGN=1 draws neighbours as before.
People DONE: wide_view.c `AddNeighbourPeople` draws the people of the nearby maps (sNearMaps) at their
start spot and first facing, unless their flag hides them (berry trees, lights, follower/OW mons and clones
skipped), with their picture and colours in room the sprite allocators leave free on PC (sprite.h
WJPC_GHOST_TILES 512 / WJPC_GHOST_PALETTES 8; palette tag ^0x4000 so real sprites never share a slot), so they
layer, fade and tint like everyone else; the real ones take over on crossing. Checked: Dewford north edge shows
Route 106's two Fishermen, same look as standing on Route 106. Test aid: WJPC_TEST_NO_GHOSTS=1.

### DLX-13 Other empty map corners (Done - session 014PwRDj, 2026-11-12: none left)
Find and fix corners like Dewford's (zoomed out shows bare filler). Show pictures.
Checked by script: every connected map group (15 groups: Hoenn 49 maps, Kanto, islands...) laid out by its
connections; a Dewford-type corner (maps continue past the corner on both sides, the diagonal covered by none)
exists nowhere now (Dewford's was fixed by widening Route 106 to 90 columns). The three open corners (above
Fallarbor between Routes 114 and 113, Route 108 south-east, Dewford south-east) were looked at zoomed out:
forest and sea continue naturally (picture sent). The light squares in Route 108's water are the map's own
shallow-water metatiles (175/176/17D/17E, General tile set), same with or without DLX-12.

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

### DLX-17 3D: a ground texture draws wrong in Slateport and many other places (Done - session 014PwRDj, 2026-11-12)
Found: the 3D view ignored Wally's Journey's seasons. 2D recolours the tilesets as a map loads
(`WallyJourney_ApplySeasonalFoliage`: hand-made General palettes + `SeasonalColor`); 3D baked its textures from
the tilesets' stored palettes, so it was always summer (green grass/trees in autumn, pale paths).
Owner: "Choose the live, season adjusted ones." Fix: `Voxel_ResolvePalettes` returns season-coloured copies
(`WallyJourney_SeasonalTilesetPalettes`), atlases carry the season key (`VoxelWorld_SeasonKey`) and rebake when
it changes; trees (`trees.rgba5551`) and building-model pages are recoloured with `VoxelWorld_SeasonTexels`
(General's own colours -> its hand-made seasonal ones, the rest by the season formula) and reload on a change.
Checked in Wine (autumn): Slateport ground/trees/bushes and Route 119 grass/trees match the 2D colours.
Note: my Wine test folder uses 3D data generated from vanilla Emerald (vanilla layouts); the generators fail on our
tree in buildings (IndexError in voxel_building._cell_image, frontier_arch) and relief - see DLX-1 (which data
ships). `reuse_everywhere` now skips a layout it cannot read (3ds_port copy only, not in git).

### DLX-18 3D: route switching still a little jarring; no pop-in at all (Done - session 014PwRDj, 2026-11-12)
Owner: "just load everything so nothing has to pop in". Preload connected maps' meshes, people and
trees ahead of the border (map meshes are already built one a frame; the edge scenery rebuild is
DLX-7). People on neighbouring maps should already be drawn.
- Map meshes and tile sheets of maps that are not underfoot are built in the background, about 3 ms a
  frame (a frame's worth while the screen is dark), old mesh shown meanwhile; meshes of maps that left
  the view are kept (32 slots) for when they come back. Was: one whole map a frame (15-30 ms each).
- Crossing a seam moves the camera and the old edge scenery into the new map's coordinates instead of
  snapping the camera and rebuilding the whole belt on that step (new belt: a few pieces a frame).
- People: drawn up to 48 tiles from the camera's aim (was 24), and the game loads them that far out
  while 3D is on (gWjPc3DField); the neighbouring maps' people are drawn as stand-ins (first spot and
  facing, hidden by their flags) until the crossing loads the real ones. 2D zoomed-out stand-ins no
  longer double a person the old map still has loaded.
- Found and fixed on the way: (1) the PC has 64 object events but the 3D sprite sheet had 32 slots -
  objects 32-63 wrote past the tables; now 128 slots (1024x512), uploaded only when a picture changes.
  (2) Reading the clock for the season cost 4-20 ms (Wine) and was done every frame by the 3D and the
  zoomed-out view, and on each crossing: the hour is now kept ~17 s on PC (WJ_RealClockHour).
- Measured (Wine, Oldale -> Route 103): the step that crosses 4.5 ms of map/belt work (was ~13 ms plus
  15-20 ms on each of the next frames). Test aids: WJPC_VOXEL_LOG logs "slow frame" over 4 ms;
  WJPC_3D_BUILD_MS=N sets the budget; WJPC_3D_OLD_CROSSING=1 the old snap; WJPC_TEST_NO_GHOSTS=1 no stand-ins.
- GBA ROM unchanged (PC code only).

### DLX-19 3D: Lavaridge gym sign folded in half; check the gym door (DONE 2026-11-13, session 019khs4X)
The new gym model (DLX-1) shows the sign bent. The model is visual only, but confirm in game that
the door still leads into Lavaridge's gym.
- Door: checked in 3D (Wine): walking up from (5,17) enters Lavaridge Gym 1F (Flannery). OK.
- Sign: cannot be reproduced here - the bent sign is in session 019khs4X's gym model, whose data is not in git.
  Needs that session (or its data + spec) to fix the model.
- Tried to regenerate 3D data from our tree (3ds_port, not in git): regions/signposts OK; relief OK after a guard in
  voxel_terraces._lay_rough; buildings OK after guards (71 models skipped, 4 unplaced; missing tileset anim
  .4bpp made with gbagfx). Result: our layouts are right (Slateport, Lavaridge match 2D) but NO buildings are
  drawn (shadows only) and magenta faces appear - the generated buildings.bin does not draw with our loader.
  Found on the way (committed): MAX_PAGES 512 -> 1024 (our-tree data has 564 pages); season reload re-uploads
  building pages into the same texture (meshes keep the handle).

DLX-19 sign fix (data, not code): the sign record at Lavaridge (layout 13) cell (7,15) in voxel/signposts.bin had
no head, so the sign's top (drawn in the gym wall's cell above) stayed painted on the wall. Patched its head mask:
row 9 bits 3..13, row 10 bits 2..14, rows 11..15 bits 1..15 (read from the 2D picture). Result: one standing
sign like Dewford's (picture sent). The patched file is in the alpha10 package's voxel/ folder; whoever
regenerates signposts.bin must re-apply it (or teach the sign generator about signs on a building's edge).

### DLX-22 3D: thin seam lines between tiles (FIX IN - owner to check on his GPU, 2026-11-13 session 019khs4X)
Not reproducible here (Mesa software GL is pixel-exact; tried 2560x1440 and walking). Ported Emerald 3DS's rule
(ctr_voxel.c GPU vertex format + Quantise): static meshes' positions rounded to 1/512 tile from the whole-tile
origin, half up, so shared edges land on exactly the same place (voxel_gl.c MeshSetQuantised). Our atlas already
matches 3DS (1/32-texel v inset). Our coordinates are small (camera ~7-30 tiles from the map corner), so the 3DS
anchor-matrix fix is not needed. If lines remain on the owner's GPU, next step: get a picture at the same spot.
Black lines in Granite Cave and right outside it, and inside the Pokémon Center; light lines across water on Route 118 / sea routes. Pictures: 82dd3767, 38db5a80, ce8fe2a3, fd24e386, 7e0beffa (Pokémon Center), 1d0a3e73/945abb13 (Route 118 water). Images in `docs/dlx_reports/2026-11-13/`.

### DLX-23 Indoor glow too strong (Pokémon Center and houses) (TODO, owner report 2026-11-13)
Bright bloom on walls/edges indoors. Owner: 'the glow in the pokemon center is too strong', 'weird glow inside of houses as well'. Pictures: b5ceb537, 0fd2c32c, 7e0beffa.
WIP 2026-11-13 (session 019khs4X): cause = our interiors are drawn unlit at full colour, then the outdoor bloom
makes them glow. Emerald 3DS turns bloom off indoors (ctr_voxel.c BaseLightFor). Gamma keeps the global bloom but
lights interiors with six rect lights (values in GAMMA_LIGHTING_VALUES.md 'Interiors'). Owner approved the 3DS rule (2026-11-13): indoor bloom now 0 by default
(WJPC_3D_INDOOR_BLOOM overrides). Full Gamma interior lighting (rect lights + glow maps) = DLX-23b, later. Next: read whether Gamma's sun reaches interiors + interior materials, then port. Images in `docs/dlx_reports/2026-11-13/`.

### DLX-24 3D: heal-machine Poké Balls drawn in the wrong spot (DONE 2026-11-13, session 019khs4X - owner to confirm)
Fix ported from Emerald 3DS (0005-port-hooks.patch FieldEffect_IsMachineSprite; 3ds_video.c PlaceScreenObject;
ctr_voxel.c CtrVoxel_ProjectPictureTile): the machine's balls/monitors (and the Hall of Fame monitors) are placed
by hand around the player on the GBA picture, so in 3D they are projected to the machine's tile one tile up.
Code: field_effect.c, wide_view.c BuildWideSprites, sdl2.c Pc_MachineSpriteSpot, voxel_gl.c
Voxel3D_ProjectPictureTile (note: our field is drawn upside down, so NDC y points down).
During healing the balls float above the wall instead of on the machine. Picture: 0fd2c32c. Images in `docs/dlx_reports/2026-11-13/`.

### DLX-25 3D: Dewford pop-in when switching routes (TODO, owner report 2026-11-13)
Trees at the outside edges of the map change when crossing into the next route. Pictures: 72382506, f63f2bd1, d88db846, d32e7b46, ee23a2c6. Images in `docs/dlx_reports/2026-11-13/`.

### DLX-26 3D: water routes show grey squares and lines (TODO, owner report 2026-11-13)
Grey/transparent squares under rocks and other objects, and long light lines on the sea (Route 118 and others). Picture: fd24e386. Images in `docs/dlx_reports/2026-11-13/`.

### DLX-27 3D: the ship on the sea renders wrong / partly flat (TODO, owner report 2026-11-13)
Picture: fa34c54f, fd24e386. Images in `docs/dlx_reports/2026-11-13/`.

### DLX-28 3D: furniture and half walls flat inside houses (and odd lines) (TODO, owner report 2026-11-13)
Chairs and tables look flat, half walls show lines. Pictures: f1550854 (Silph-style railings), 7e0beffa (house). Images in `docs/dlx_reports/2026-11-13/`.

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

### 2026-11-13 owner round (session 019khs4X), alpha13-3d sent
- Blue squares (Granite Cave floors, Slateport): an atlas built for earlier maps on the same tile sets lacked
  later maps' metatiles; now AtlasCovers() makes it again (voxel_gl.c), meshes follow by Atlas.gen. DONE.
- Caves (MAP_TYPE_UNDERGROUND, inst->underground): no belt plateau/walls round them, backdrop behind, no haze;
  Gamma: no clouds in caves/indoors (Gamma's clouds are its sun's light function). DONE.
- Cloud pattern jumping at map borders (3D): clouds use VoxelWorld_SteadyOrigin(). DONE. (2D not yet.)
- Trees "changing a lot" in Dewford: asked the owner what exactly changes - OPEN.
- Battle transition over the 3D world as Emerald 3DS (RenderTransition): the world moved line by line by BG1's
  per-line scroll (gPcLineScrollX/Y from gba_easy_draw.c), in bands. DONE (2D field still not moved).
- Gamma Lighting is a setting: Options > Settings > Video (cfg gammaLight); env WJPC_3D_GAMMALIGHT overrides. DONE.
- Owner's next big idea: add Kanto with story sense (linked PCs after Sevii, or lower EXP in Kanto). Discussing.
