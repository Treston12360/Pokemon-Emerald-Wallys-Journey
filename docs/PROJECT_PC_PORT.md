# Project: Wally's Journey as a PC program (.exe)

## START HERE: handoff for a new chat (2026-11-12)
**Open work list: `docs/DELUXE_WORK_QUEUE.md` (items DLX-1 ... DLX-16).**
Read this section, then the rest of the file. Repo `Treston12360/Pokemon-Emerald-Wallys-Journey`, PC branch `deluxe`.
Another Claude session may also push to `deluxe`: always `git pull --rebase` before pushing.

**Done since alpha 5b (all on `deluxe`, GBA ROM still 21f006ad...):**
- Overworld fills the screen edge to edge at any window size; **zoom** like gen1recomp (keys `-` `=`, mouse wheel,
  Settings > Video > Zoom 1-8): whole-number scale, zooming out shows more map. Indoors zooms one step closer.
- **UI layer**: text boxes, menus and the start menu are drawn separately at the biggest whole GBA size that fits
  (they don't grow with zoom), bottom-centred. Start menu open -> UI slides flush right. Map name banner -> always
  top-left. HM "Pokémon shows itself" banner -> runs edge to edge (BG0 drawn wide while FLDEFF_FIELD_MOVE_SHOW_MON).
- **mGBA screen options** (owner-approved wording) in Settings > Video: Lock aspect ratio, Integer scaling, Bilinear
  filter, Interframe blend, Lock frame size, Resolution (Monitor/720p/1080p/1440p/4K), Zoom.
- Frame rate default = steady 60 like gen1recomp (fixed 60 logic, no in-between frames); Smooth 120-240 still optional.
- Surf blur at high frame rate fixed (surf blob classified by base position).
- NPCs load one tile before they scroll into view (fewer pop-ins).
- Test-script commands added: `winshot FILE`, `set NAME VALUE` (zoom, lockAspect, integer, bilinear, resolution,
  interframe, framerate), `warp G N X Y`, `surfon` (real Surf from the shore, face water first), `fieldmove`;
  env `WJPC_WINDOW=WxH`.
- Code: `src/platform/sdl2.c` (DrawFieldScene, DrawScene, DrawScreen, ComputeFieldView, test commands),
  `src/platform/wide_view.c` (UI/overlay split per scanline), `src/option_menu.c` (Video rows).

**Owner decision 2026-11-12 (changes an old rule):** PC saves no longer have to go back into mGBA. mGBA saves must
still **load on PC** (or convert). With that, he wants the GBA limits gone on PC, "without doing weird shit":
NPCs, sprites, colour palettes.

**In progress:** branch `deluxe-limits-wip` (one commit, NOT built/tested yet, do not merge blind):
`OBJECT_EVENTS_COUNT` 64 on PC, `SAVED_OBJECT_EVENTS_COUNT` 16 for the SaveBlock1 array, Save/LoadObjectEvents copy
only 16, extra slots cleared on load (`WJPC_CLEAR_EXTRA_OBJECT_EVENTS`). Slots fill lowest-first, so the player and
nearby NPCs land in the saved 16. Still to check: the player/follower must never sit in slot >= 16 when saving; arrays
sized by OBJECT_EVENTS_COUNT (script_movement.c, trainer_see.c, rotating_tile_puzzle.c, shop.c, LockedAnim localIds)
are fine but check loops that assume 16; `WJPC_SpawnOk` free-slot rule in wide_view.c.

**RULE CHANGE 2026-11-12 (owner):** the GBA ROM does NOT have to stay byte-identical any more ("we can change whatever
we want as long as we can import our old GBA .savs", "no longer bound to GBA limits"). Hard rule now: old GBA/mGBA saves
must load. Still ask before visual/wording changes. Owner also wants to be consulted before any frame-rate change.
Map pop-in fix (wide_view.c: connections 4 maps deep, nearest-map filler) is on `deluxe`. Branch `dewford-corner` widens
Route 106 to 90 columns so the corner NE of Dewford is real map (waiting for owner OK). NOTE Makefile.pc does not notice
changed map .bin files: delete build/pc-linux32/data/maps.o (and pc-win32) and run make twice.
Asked, not started: battle transition played across the zoomed-out view (like gen1recomp: effect applied to the whole
wide frame); neighbour maps' NPCs; per-map tilesets for neighbours; lighting; 3D.

**DONE 2026-11-12, fast forward (on `deluxe`, commit 51a4b697):** only the last game frame of each batch is drawn
(`WideView_SkipFrame` runs the per-line interrupts without drawing); the sound engine runs only as often as the sound
card needs (`Platform_SoundFrameDue` / `Platform_SoundFrameSkipped`), so music keeps normal speed and pitch. Measured
under Xvfb at 20x: 60 fps shown, 1200 game frames/s (before: 2 fps, 240/s, silent). Same game state with/without
skipped drawing. Script command `ff N`. Not yet tried on a real Windows PC by the owner.

**Limits, branch `deluxe-limits` (built, ROM SHA unchanged, partly tested):** the WIP commit's code plus:
`MAX_SPRITES` 128 on PC (keeps 7-bit sprite-id fields and 8-bit sort keys valid), wide view sprite list 1024,
`src/platform/wj_limits.c` `WJPC_PickObjectEventSlot`: player / follower / NPC follower only in slots 0-15 (the NPC
follower's slot number is saved), other people lowest free below 14, else slot 16+. Script command `objcount LABEL`.
Tested: mGBA saves (truck, Slateport) load and play. NOT yet tested: a PC save -> reload round trip with this branch
(test script hit the Bag: this save's start menu order differs), a scene that really needs more than 16 people
(WJ_SPAWN_PAD keeps the count low today; the extra slots only matter if the spawn ring is widened), cutscenes,
trainers spotting. OBJ tile VRAM and OBJ palettes: DONE 2026-11-12 (DLX-14, see docs/DELUXE_WORK_QUEUE.md):
PC has 64 sprite palettes and 4096 sprite tiles; OamData on PC is tileNum:12 / paletteNum:6.

**Update 2026-11-12 (later):** PC save -> reload round trip passes (PC save also still opens in mGBA). Crowd test:
Slateport market at 4K max zoom-out loads 22 people (old build drops part of the Team Aqua line). Random-input hunt
found `script_movement.c` reading past its task data with 64 slots -> fixed (`SCRIPT_MOVEMENT_SLOTS` 16, both builds,
ROM SHA unchanged); five 2500-press hunts after the fix: no skipped reads, up to 30 people. Merged into `deluxe`.
Alpha 6 .exe (fast forward + limits) built with Ubuntu mingw-w64 gcc + official SDL2 2.30.9 / SDL2_image 2.8.2 MinGW
packages (DLLs next to the exe), tested under Wine 10 Xvfb: 20x = 60 fps, 1200 game frames/s, no silence.
Still untested: trainers spotting with extra people loaded (warp test spots in WJ's Route 110 didn't match the map).

**Next, in order:**
1. Owner tries alpha 6. Then OBJ tiles/palettes (design first: OamData tileNum:10 / paletteNum:4).
2. Finish and test the NPC limit (above), then sprites: `MAX_SPRITES` 64 -> e.g. 256 on PC (wide_view draws from
   gSprites, keys use & 0xFF so max 255; OAM buffer is 128). Then OBJ tile VRAM (32 KB) and OBJ palettes (16, the
   4-bit oam.paletteNum) need PC-side room. Playtest trainers spotting, followers, cutscenes, Wally's Ralts catch.
3. New alpha .exe: `make -f Makefile.pc WIN=1`, strip, zip with SDL2.dll + SDL2_image.dll (from
   build/pc-deps/SDL2/i686-w64-mingw32/bin), smoke-test in Wine (`xvfb-run -a wine WallysJourney.exe`), send.
4. Step 5 full check against the ROM: DONE by the other session (2026-11-11/12), see the "Full check" notes at the
   end of this file. Tools: tools/pc/compare (compare.py, steps/*.txt, --epoch, --elf, --pc-only), test saves from
   tools/wj_testfw (scenarios/fullcheck*.json + make_save.txt). PC-only bugs found and fixed: RandomPercentage 50/50
   (if_comptime), Battle Factory palette read, gacha soft-lock, PC storage held-mon sprite, contest move-background
   heap overrun, Continue-screen empty icon slots, shake-animation pointer. After big PC changes run the --pc-only
   sweep over all step files and read the errors it prints.

**Owner (Treston) wants:** plain English, honest feedback, show him pictures, approve any new wording/visual change
first. No new hand-drawn art. Never send his email anywhere. Commits end with the Co-Authored-By / Claude-Session lines.
The old `pc-port` branch on GitHub is his to delete (the proxy refused).


**Branch:** `deluxe` (PC work started on `pc-port`) (based on 0.18.19). Read this whole file before working on the PC version.
**Status (2026-11-11):** alpha 5b sent to the owner (`WallysJourney.exe`), tested under Wine 10 (Kron4ek WoW64 build
from GitHub releases + Xvfb). Steps 1-4 are done (PC settings, widescreen, frame rate 60-240, 4K) plus keyboard keys,
controller mapping (owner confirmed it works) and the bad-read fixes. **Next: step 5, the full check against the ROM.**

## What the owner asked for (2026-11-11, in his words where possible)
- "a .exe file at the end of it with all those cool features that normally come with recomps":
  unlocked/smooth frame rate, widescreen ("allow more screen to be shown", like the Gen 1 recomp,
  bryanthaboi's gen1recomp), 4K, plus a PC settings screen.
- Keep **every Wally's Journey feature exactly as it is**, and keep saves **compatible or at least importable**
  with the mGBA saves.
- He first suggested (with ChatGPT) rebuilding WJ feature by feature on top of his Emerald-PC-Engine
  (github.com/Treston12360/Emerald-PC-Engine, a pokeemerald PC port). We did the reverse instead,
  with his OK: take the whole WJ source and give it the PC layer from that engine. Every WJ feature
  comes along automatically and every future WJ update only needs a rebuild.
- He chose the order of work: **settings, widescreen, smooth frame rate, 4K, then the full check
  against the ROM** (step "1" moved to the end). Crashes he reports still get fixed straight away.
- Settings rows, wording approved by the owner:
  `Window size < 1x..6x >`, `Fullscreen < Off/On >`, `Sharp pixels < On/Off >`, `Volume < 0..10 >`,
  `Fast forward < 2x/3x/5x/Max >` (done in alpha 3).
- 2026-11-11 owner asked for controller button remapping in Options and a fast forward toggle, then for
  "simplified consolidated settings" in sections (controls, audio, video, fast forward/visual toggles); he
  approved the mockup (`tools`-free: drawn from the game font) with "I like the sectioned settings, implement that".
  Built in alpha 4e (PC only, GBA Options menu unchanged): top page Gameplay / Speed & Looks / Controls / Audio /
  Video (+ Cancel); rows `Fast fwd button < Hold/Toggle >`, `Pad A/B/Start/Select/L/R/fast fwd < A B X Y LB RB LT RT
  Back Start LS RS None >`, `Widescreen < On/Off >`. Defaults: pad B = B (was X in the old port), fast forward = RT,
  controller fast forward uses the chosen speed (was fixed 5x). Code: `option_menu.c` (sPcRows/sPcRowPos pages,
  PcMenu_*), `platform.h`, `sdl2.c` (ApplyFastForward, XInput mapping, cfg keys padA.. / fastForwardToggle).
- 2026-11-11 owner: "The max is way too fast, add in intervals of 2x to max speed of 20x". Alpha 4f: Fast forward
  `< 2x 4x ... 20x >` (FF_SPEED_COUNT = 10, value v = (v+1)*2x; Max/100x removed). Owner then: "Make it not stop at the end and go back to 2x" -> the row wraps (4g).

## Plan (owner's order)
1. **PC settings screen** — DONE (alpha 3). Rows at the bottom of Options, PC build only, stored in `WallysJourney.cfg`.
2. **Widescreen and zoom** — DONE (alpha 4), see "Wide view (step 2) as built" below. Draw the overworld from map data so more of the world shows at the
   sides (the gen1recomp approach). Load object events further out so nothing pops in at the edges.
   Battle screens, menus and text boxes need centring or their own handling. Emerald-PC-Engine has design
   docs for a "native overworld renderer" / "native world neighborhood" (docs/NATIVE_OVERWORLD_RENDERER_*.md,
   R11EF_NATIVE_WORLD_NEIGHBORHOOD_*.md) worth reading first. Show the owner test pictures before building it in.
   Estimate 10–20 h.
   - **Test pictures (2026-11-11), sent to the owner:** Rustboro, Petalburg, Fortree, Oldale Pokémon Center, each at
     Original 240x160, A 288x162 (same zoom, sides only), B 320x180 (6x on 1080p, 12x on 4K), C 384x216 (5x/10x),
     D 480x270 (4x/8x). They are mockups stitched from 15 real GBA captures per scene with the camera moved
     ±8 tiles sideways and ±4 up/down (`tools/pc/widescreen_mockup/capture.py` + `compose.py`; the capture run
     temporarily turned off WJ live/seasonal weather with `// WSMOCK` lines in wally_journey.c so every shot has
     clear skies, then restored the file). Small doubled-NPC glitches in them are from the stitching.
   - **Questions put to the owner:** which size (A–D); whether small rooms (they show lots of black at C/D)
     should zoom in automatically indoors; that people further away must be loaded earlier for C/D (gameplay-adjacent,
     see the Emerald-PC-Engine renderer doc §9 "Object-loading limitations") and weather/fog must be stretched.
   - **Design notes from the Emerald-PC-Engine docs (read 2026-11-11):** don't paste the GBA picture into the middle
     and draw margins around it (seams); render the whole view natively from live state (BG ring + backup map grid +
     connections/border, current VRAM/palettes, a sprite command list taken before off-screen culling); fall back to
     the normal 240x160 picture (centred) for battles, menus, text boxes and anything unsupported. Emerald's backup
     map grid has a 7-metatile apron (8 east), enough terrain for A/B/C (C adds 72x28 px per side); D (120 px per side)
     needs a connection resolver past the apron. NPC spawn range is 2 tiles past the GBA screen: C/D need it widened.
   - **Owner's answer:** "Do whatever you think will look the best". Chosen: C (384x216) outdoors, B (320x180)
     indoors (small rooms show less black), NPCs loaded further out, weather repeated into the margins.
3. **Smooth high frame rate** — DONE. Owner then asked for "60 fps to 120 fps 180 fps and 240 fps ... in the options":
   Settings > Video > `Frame rate < 60 / 120 / 144 / 165 / 180 / 240 fps >` (owner added 144 and 165; cfg `frameRate=<fps>`,
   old `smooth=0..3` still read); the draw loop is capped at that
   rate (vsync may cap lower); game logic stays 60 updates/s (measured under Wine with WJPC_FPS_LOG=file:
   "fps 240 game 60"). Above 60 the extra frames are the smooth in-between frames below. How: in the
   field the wide view draws an 8 px bigger canvas (WIDE_VIEW_SMOOTH_MARGIN) of the map, and a separate "still" layer
   holding BG0 (text, pop-ups) plus sprites that kept their screen place while the camera moved (player, follower) —
   diff of two composites per scanline, no extra interrupt runs. sdl2.c draws every loop (every refresh with vsync):
   the map texture slides by (1 - t) * camera step (t = accumulator / dt), the still layer on top. Camera jumps > 8 px
   (warps) and non-field screens aren't slid. NPC walking is still 60 fps steps. Test: WJPC_SMOOTH_TEST=1 + script
   `smoothshot FILE` dumps raw layers + camera. Original plan text: game logic stays at 60 updates/s (so nothing breaks); extra frames are
   drawn in between on 120/144/240 Hz monitors. Walking + camera first, then sprites, menus, battle
   animations. Line-by-line effects (water ripple, some battle intros) stay at 60. Needs step 2's renderer.
   Setting: "60 (original) / match monitor / cap". Estimate 5–15 h.
4. **4K** — DONE: per-monitor DPI aware (SDL_HINT_WINDOWS_DPI_AWARENESS permonitorv2 + ALLOW_HIGHDPI), Window size up
   to 12x (shrunk to fit the screen), game drawn to a render target at a whole-number scale then stretched with linear
   filtering only when the size doesn't divide evenly, `Screen filter < Off / Scanlines / LCD >` (cfg `filter=`).
   Checked under Wine with Xvfb at 3840x2160 fullscreen. Original plan: clean upscaling + optional filters. (High-res replacement art would be new art; ask the owner first.) 2–4 h.
5. **Full check against the ROM** — go system by system (trainer/gym battles, every menu, gacha, contests,
   Battle Frontier, cutscenes, 3D backgrounds in every area, seasons/weather, sound), plus the owner's playtesting.
   Then a stable release. Estimate 15–30 h.

## Wide view (step 2) as built
- `src/platform/wide_view.c` draws each frame. Three modes: FIELD (normal overworld: 384x216 outdoors, 320x180 for
  MAP_TYPE_INDOOR/SECRET_BASE), CENTER (battle transitions, `CB2_OverworldBasic` scenes: GBA picture in the middle,
  black margins), GBA (battles, menus, everything else: normal 240x160, pillarboxed).
- FIELD mode draws BG1-3 for the margins from the RAM tilemap ring where it covers the spot and from the map grid /
  connections / border outside it, BG0 (text, start menu) only in the GBA rectangle, sprites from `gSprites` with
  full coordinates (so NPCs and overworld mons show in the margins), weather sprites repeated (fog/ash/sandstorm
  every 320x256, rain/snow every 240x160). Windows (WIN0/1, used by WJ in the overworld) take the nearest edge
  pixel's setting outside the GBA rectangle, so dark caves stay dark in the margins.
- NPC spawn/despawn range widened on PC only (`WJ_VIEW_PAD_*`, `WJ_SPAWN_PAD_*`, `WJ_SPAWN_OK` in
  `event_object_movement.h`; extra-range NPCs load only while more than 4 object event slots are free).
- Window size setting now steps in 320x180 units; `widescreen=0` in `WallysJourney.cfg` turns it off (no Options
  row yet: ask the owner before adding one, it changes the Options screen).
- Test aids: `WJPC_NOWIDE=1`, `WJPC_WVDEBUG=1`, `WJPC_FORCE_WEATHER=<id>` (3 rain, 4 snow, 6 fog, 7 ash, 8 sandstorm).
- Also fixed: Pokémon Center freeze on PC (wireless "wait for VBlank" spin in main.c never ended;
  `WJPC_WIRELESS_VBLANK_WAIT`). It was in the earlier alphas too.

## Build environment (no apt / only GitHub reachable)
- Linux: `make -f Makefile.pc MUSL=1` builds a static 32-bit binary with clang + musl 1.2.5 (kraj/musl, built with
  clang into /opt/xtc/i686-musl; PAGE_SIZE defines removed from its limits.h; libgcc = compiler-rt builtins from
  llvm-project 18.1.3) + SDL2 2.30.9 / SDL2_image 2.8.2 built static.
- Windows: `make -f Makefile.pc WIN=1 LLVM_MINGW=1 SDL_DIR=/opt/xtc/i686-mingw` with llvm-mingw built from its
  scripts (i686 only, msvcrt) and SDL2/SDL2_image built static, so the .exe needs no DLLs next to it. Needs the
  Linux build first: clang's assembler can't read the game's asm, so the Linux ELF data/midi objects are converted
  with `objcopy -O pe-i386` and `tools/pc/coff_fixrel.py` repairs their relocation types. clang has no
  `-mno-align-double`, so `include/gba/types.h` keeps u64/s64 4-byte aligned for this build (save struct sizes
  checked identical to the Linux build). XInput now links `xinput9_1_0` (ships with Windows) instead of `xinput1_3`.
  Use **mingw-w64 v12.0.0** with LLVM 18: mingw-w64 master's msvcrt.def uses `==` aliases that LLVM 18's dlltool
  turns into self-loops (`atexit` jumped to itself, so alpha 4 hung before opening its window).
  `coff_fixrel.py` rebuilds every relocation from the ELF one (objcopy dropped section symbols, so pointers
  into the same section, e.g. the map group tables, were wrong and loading a save crashed).
- Wine for testing: `curl -L` the Kron4ek/Wine-Builds `wine-10.0-amd64-wow64.tar.xz` release (runs 32-bit .exe with
  no 32-bit Linux libraries), `WINEDLLOVERRIDES="mscoree,mshtml="` to skip Mono/Gecko, run under `xvfb-run`.
- If a machine has gcc-multilib + 32-bit SDL2 and mingw-w64 + Wine, the original commands still work.

## Rules (same as the main project, plus PC ones)
- **The GBA ROM must not change.** Every change to shared code is behind `#ifdef PORTABLE` (C) or
  `.ifdef PORTABLE_ASM` (asm). After every change run `make release` and compare the SHA-1 with a build of
  the base commit (0.18.19 = `21f006adcc5d884c4992805296e171f45428c46b`).
  **Careful:** expansion asserts embed `__LINE__`, so adding lines above an assert in files like sprite.c
  changes the ROM bytes. Keep edits on the same line (see the `CALL_SPRITE_CALLBACK` / `SPRITE_HAS_ANIM`
  macros in include/global.h) when a file has asserts.
- PC-only code goes in `src/platform/`, `include/platform/`, `tools/pc/`; the GBA Makefile skips `src/platform/`.
- Owner rules from the main project still apply: plain language, call out mistakes, new/changed dialogue and
  any visual change must be shown to and approved by the owner first, no new hand-drawn art (recolours OK),
  test on release builds, never send the owner's email anywhere.
- mGBA saves must load on PC (owner 2026-11-12: PC saves no longer need to go back to mGBA). The save file is the raw 128 KB flash image.

## Building
Run the normal GBA build first (`make release`) so all generated files exist, then:
```
make -f Makefile.pc            # Linux 32-bit: ./wallys-journey
make -f Makefile.pc WIN=1      # Windows 32-bit: WallysJourney.exe
```
Needs: `gcc-multilib`, 32-bit SDL2 runtime (`libsdl2-2.0-0:i386`, `libsdl2-image-2.0-0:i386`) + SDL2 headers,
`mingw-w64`, and for Windows `build/pc-deps/SDL2/i686-w64-mingw32` = official SDL2 2.30.9 and
SDL2_image 2.8.2 MinGW dev packages merged (github.com/libsdl-org releases). Python 3 for the tools.
Release package: strip the .exe (`i686-w64-mingw32-strip`), zip it with `SDL2.dll`, `SDL2_image.dll` and README.txt.
Owner's save goes next to the .exe as `WallysJourney.sav`.

## How it works / hard parts already solved
- Platform layer from pokeemerald `pc_port` (Kurausukun/NTx86) via Emerald-PC-Engine commit 2f35335ef
  (before that repo switched to loading content from the ROM): `src/platform/sdl2.c` (window, input, save
  file, clock, main loop), `gba_easy_draw.c` (software GBA renderer), `m4a_pc.c`/`music_player.c`/
  `sound_mixer.c`/`cgb_audio.c` (sound), `agb_flash_pc.c`/`agb_flash_dummy.c` (save chip), `siirtc_pc.c` (clock).
  WJ's own m4a/siirtc/agb_flash files are `#ifndef PORTABLE` and the PC versions replace them.
- **Struct layout:** GBA (apcs-gnu) aligns every struct to 4 bytes. `tools/pc/align4.py` adds `aligned(4)` to
  every game struct on PC. Windows also needs `-mno-ms-bitfields -mno-align-double`.
  `tools/pc/layoutcmp.py GBA.elf PC.o...` compares all struct layouts against a GBA `make DINFO=1` build:
  only the sound engine's internal structs differ (on purpose).
- **Asm data:** `tools/pc/asmconv.py` turns GBA assembly data into x86 assembly (`--coff` for Windows).
- **Tagged script pointers:** GBA marks "effect" functions by pointing into ROM mirror 1. On PC the
  `effect_ptr` macro (constants/gba_constants.inc) keeps a list; `tools/pc/effect_funcs.sh` makes a C table.
- **Busy-waits on VBlank:** PC runs VBlank only between frames, so `IsDma3ManagerBusyWithBgCopy` drains the DMA queue itself.
- **ARM vs x86 shifts:** shift by 32 is 0 on ARM, not on x86 (`ARM_LSL32`/`ARM_LSR32`, used in text drawing).
- **NULL reads** (harmless on GBA, it reads BIOS; crashes on PC): `src/platform/null_guard.c` runs sprite
  callbacks, tasks and VBlank/HBlank callbacks so a read through NULL skips that one callback and logs
  "WJPC: skipped a NULL read" once. Real fixes still preferred where found (battle transition end,
  Pokémon cry animation outside battle, naming screen, NULL first animation in category icons).
- Renderer fix: window 1 was reading window 0's position. Text: ARM-style shifts in GLYPH_COPY.
- Fast-forward: steady speed (old timing clamp made it stutter), sound dropped while held, sound queue capped.
- Saves: written to disk between frames whenever the game saves (`gWjFlashDirty`).

## Testing
- `WJPC_SCRIPT=script.txt` runs the game frame-exact as fast as possible: `wait N`, `press KEYS N`
  (A B S=start s=select U D L R l r), `shot file.ppm`, `quit`. `WJPC_SAVE=` picks the save,
  `WJPC_AUDIO_DUMP=` writes raw sound (f32 stereo 42060 Hz). Use `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy`.
- `tools/pc/harness2script.py` converts the mGBA harness scripts.
- Random-input crash hunts: a script of random button presses from the friend's save for ~50k frames;
  run several seeds after every change. Run under gdb for backtraces.
- Windows .exe is tested under Wine (`wine32:i386`), with `WINEDEBUG=-all` when Wine is available (alpha 4 was not).
- Tested so far: intro, new game (Birch, naming), title, main menu, friend's save, overworld, start menu,
  party menu, Options incl. PC rows, wild battle (moves, EXP, back to map), saving, PC save back into mGBA.
- Not tested yet: real-time sound/controllers on a real Windows PC (owner), trainer/gym battles, long play.
- Random-input hunts from a Rustboro save (3000 presses, seeds 1-5, 7, 11-13) found three PC-only crashes, all
  in every earlier alpha, now guarded behind PORTABLE: Pokédex list asks for dex number 0, HGSS dex sprite id 0xFFFF,
  window id 255 when saving. Watch out: `src/window.c` has asserts that print `__LINE__`, so adding lines above
  line ~60 there changes the GBA ROM.

## Owner feedback log
- alpha 1: crashed at Birch's "This world is widely inhabited by..." (fixed alpha 2); Space made it do
  "weird stuff" (fast-forward stutter, fixed alpha 2).
- step 2 test pictures: owner said "Do whatever you think will look the best and download whatever tools you think
  is necessary to finish". Alpha 4 (wide view) sent with 9 screenshots from the real PC build.
- alpha 4: "it's open on my pc but not showing anywhere other than the task manager" (CRT atexit loop, fixed in 4b
  together with the map pointer and the three crashes above).
- alpha 4b: "It crashes once I hit to play. Speakers and controller work". Could not reproduce under Wine
  (real-time run with key presses from a helper .exe, SDL disk audio, from a save, new game, fullscreen and 6x
  configs). Alpha 4c: `null_guard.c` now skips ANY bad memory access / divide by zero inside guarded calls (was:
  only the first 64 KB), main callbacks are guarded too (`WJPC_CALL_MAIN_CALLBACK` in main.h), the wide view
  falls back to the normal picture if it faults, skips are logged to `WallysJourney_errors.txt`, and an
  unhandled crash on Windows writes `WallysJourney_crash.txt` (exe-relative addresses: look them up with
  `i686-w64-mingw32-nm -n` on the unstripped exe from the same commit) and shows a message box.
- alpha 4c: owner sent WallysJourney_errors.txt + his save: "Once I hit continue save from my save file. It goes to a
  black screen". Save is in New Mauville; its on-load script runs `special DrawWholeMapView` before the overworld
  tilemap buffers are allocated (NULL on GBA = harmless writes to low memory). Fixed in 4d:
  `WJPC_RETURN_IF_NO_TILEMAP_BUFFERS()` (overworld.h) in `DrawMetatile`. Tested with his save + cfg under Wine.
- 2026-11-11 owner: "The controller mapping works good". Asked, in order: 1. keyboard keys in settings, 2. smooth frame
  rate in Video settings, 3. 4K, 4. fix the bad reads; "Send me small videos or pictures ... keep me updated".
  1 done: Settings sections are now Gameplay / Speed & Looks / Controller / Keyboard / Audio / Video. Keyboard rows
  Key Up..Key fast fwd: A = "Press a key" (Esc cancels, a key used elsewhere swaps), cfg `keyUp=<SDL keycode>` ...
  Test script command `typekey NAME`. Checked under Wine with real key presses (W bound to Up walks).
- Owner item 4 "Fix the bad reads so there's no crashes": random-input hunts (tools: scratch hunt2.py, 2500 presses,
  saves s0-s5, t1 and the owner's New Mauville save) found 4 spots, fixed PC-only at the source:
  Trainer Card VBlank after sData freed (trainer_card.c), Pokédex UpdateSelectedMonSpriteId reading gSprites[0xFFFF]
  (pokedex.c), PlayAnimation *argPtr with NULL arg (overworld weather starting a battle; WJPC_ANIM_ARG same-line macro
  in battle_script_commands.h, that file has __LINE__ asserts), CannotUseItemsInBattle GetMonData(NULL) for items used
  from the bag in battle (Poké Balls, X items; item_use.c uses the battler's HP). The last one could cancel a ball
  throw on PC since 4c's guard skipped the whole item use.
  Then the owner's save found an integer divide by zero (Spark animation, `x % 0`). The GBA build's libgcc gives
  x/0 = INT_MAX/INT_MIN/0 by sign (unsigned 0xFFFFFFFF/0) and x%0 = 0 and carries on; null_guard.c now emulates a
  faulting 32-bit DIV/IDIV that way (Windows vectored handler / Linux SIGFPE: decode F7 /6,/7 length, set EAX/EDX,
  continue) instead of skipping the callback (which could freeze a battle animation). Script test `divzero FILE`.
- Owner: "Go ahead and start full check. Send me updates in either a video or pictures and tell me what's been
  found and then the fix". Tool: `tools/pc/compare/compare.py` + `steps/*.txt` (mGBA harness_cmp with poke32 /
  lockrng, PC test commands setrng / lockrng / peekrng; fixed clock WJPC_RTC_EPOCH (local time on both); VBlank
  AdvanceRandom off on both (WJPC_NO_VBLANK_RNG / NOP-patched test ROM copy) because the GBA adds a roll on lag
  frames; "lock on" = same RNG state every frame so the same action gets the same numbers; burst shots
  matched to the closest frame; 1-step colour differences counted as "rounding" (mGBA blends with 8-bit channels,
  the GBA and the PC with 5-bit)).
  Found 1: clang miscompiled `if_comptime` (metaprogram.h): `__builtin_constant_p(cond ? 0 : *(int *)0)` is
  true for clang, so every RandomPercentage(tag, t) was a 50% roll on PC since alpha 4 (first clang build).
  Measured with script `pcttest`: 0%:4980/10000 before, 0 after; 30%: 4983 -> 3061. PC-only fix in
  metaprogram.h. Sent as alpha 5b.

- Full check, found by a clang warning scan: Battle Factory swap screen reads color 37 of the 16-color
  `sPokeballGray_Pal` (past the end) in 3 places. The GBA ROM has 0 there; PC read whatever came next in
  memory, so a fade could use a wrong color. PC now uses 0 (`POKEBALL_GRAY_PAL_37`). Release SHA unchanged.
- Full check, menus (owner save): Start menu, Pokédex, DexNav, Bag, party, Summary (4 pages), Options all match the
  GBA. Only differences are animation timing (bag swirl, cursor blink, Summary grid that scrolls 1 px every 4 frames).
- Full check, catching: wild battle, two turns, Poké Ball from the bag, catch, nickname screen: identical on both.
- Full check, test saves: tools/wj_testfw/scenarios/fullcheck.json (Twins on Route 104, Rustboro Gym next to
  Roxanne, Game Corner with 9000 coins) + tools/wj_testfw/make_save.txt (new game + save via harness_build). Build
  the test ROM with install.py on, make the saves, install.py off, rebuild and check the release SHA.
- FOUND + FIXED (alpha 5c): Game Corner gacha soft-locked on PC after the ball dropped. game_corner_gacha.c calls
  HandleLoadSpecialPokePic(gMonSpritesGfxPtr->spritesGfx[1]) outside battle where gMonSpritesGfxPtr is NULL; the
  GBA writes to a dead address, PC crashed (null_guard skipped the task forever). PC now skips that call.
- Trainer battles: the gym battle matches move for move and damage for damage when the random state is set equal
  before each move. Lesson: do not "lock" the RNG every frame for battles. The GBA has lag frames (big AI and
  graphics work spans more than one frame), so a per-frame lock resets the RNG in the middle of the GBA's work
  and makes false differences. Use "sync" right before each input instead, with long waits (steps/gym_battle.txt).
- Game Corner: the left gacha spot shows a tall magenta column on BOTH GBA and PC (not a port bug; told the owner).
- Owner asked (2026-11-11) whether the reporters (Gabby & Ty) give too little EXP on PC. Checked on both
  (scenarios/rep42.json, steps/reporters.txt): Lv40 Magnezone + Exploud vs Lv42 Groudon/Kyogre gave 102 and 93 EXP
  each and ₽1600 on BOTH. Low EXP is the 5% rule the owner asked for on 2026-11-11 (main branch), not a PC bug.
- Full check, maps (scenarios/fullcheck2.json): Twins double battle (Route 104), Mauville -> Route 110 and
  Route 113 -> Fallarbor map connections, Route 119 walk. All match; differences are only water ripples, the map
  name sign, and random-walking NPCs a step apart after a map change (GBA lag frames during the map load).
- Full check, Battle Frontier / contest / Safari (fullcheck3.json, fullcheck4.json): Battle Factory Lv50 (rent
  three, battle) identical; Lilycove Normal Cool contest identical (audience members are picked at random during
  the hall's map load, so a few look different); Safari Zone entrance, paying, entering and leaving identical. Safari wild
  battle (Oddish, Safari Ball, caught, Pokédex entry, nickname) identical too.
- Lesson (Safari): the GBA lags 9 frames loading the Safari Zone map. Timers that run every game frame (grass
  ripples, overworld-encounter spawns, wandering NPCs) start a few frames apart, so the random numbers drift even
  after a sync. Checked with per-frame random states plus the GBA's own play-time counter (game frames) against
  hardware frames. For tests after a big map load, hold the random state with "lock on" while walking.
- Owner request (2026-11-12): Settings > Reload save (first page, PC only). A asks "Sure? Press A", the second A
  fades out and calls ReloadSave() (src/reload_save.c, the game's own link-error recovery: reloads flash and
  CB2_ContinueSavedGame). B/up/down cancel; "No save yet" when there is none. Owner saw a preview video first and
  approved. Tested: reload returns to the last save; save -> walk -> reload returns to the NEW save; Windows (Wine)
  build too. Sent as alpha 5d.
- Full check, seasons and time of day (compare.py --epoch): Mauville and Route 119 at noon in all four seasons
  (48 h each), at 23:00 and at 06:00 on both. Seasonal palettes, grass/trees, night tint and town lights match;
  only falling petals/leaves (random particles) differ.
- FOUND + FIXED: PC Pokemon storage (swsh_storage_system.c SetMovingMonPriority) wrote to movingMonSprite while no
  Pokemon is held (NULL). GBA: harmless write; PC: crash, the guard skipped the rest of that frame's cursor update.
  PC now returns early. Found by the full check (pc_boxes.txt) via WallysJourney_errors.txt + gdb.
- Full check, features: level-up move learning (two moves, forget prompts) and evolution (Treecko -> Grovyle)
  identical; PC boxes identical. Follower talk lines are random; a per-frame timer started before the sync (GBA
  lag after the battle) shifts them, not a bug.
- Always check pc/WallysJourney_errors.txt after each compare run: the guard hides PC-only crashes on screen.
- FOUND + FIXED (PC) / REPORTED (GBA): contests. LoadMoveBg (battle_anim.c, contest branch) runs
  RelocateBattleBgPal + DmaCopy32 over 0x800 bytes of a malloc'd tilemap that can be smaller (Psychic: 0x500).
  The 0x300-byte overrun corrupts the next heap block on the GBA too (silent; Free() then writes through a garbage
  pointer). PC crashed in Free(); PC now allocates max(size, 0x800) zeroed (extra rows are off-screen, so it looks
  the same). A GBA fix would change the ROM: owner decides.
- Mega Evolution (Swampertite + Mega Ring) works on both. Trainer AI switch decisions are taken at turn start,
  before the test's sync, so the first-turn order can differ (GBA lag drift), not a bug.
- FOUND + FIXED: Continue screen (ui_main_menu.c DestroyMonIcons) called DestroySprite(&gSprites[SPRITE_NONE])
  for empty party slots (party < 6). Out of bounds on the GBA too, but harmless there; on PC, after the 128-sprite
  change, the memory after gSprites looked "in use" and it crashed (guard skipped it). PC now skips empty slots.
- compare.py --pc-only: runs only the PC side of a step file and prints WallysJourney_errors.txt (fast sweep for
  crashes the guard hides). Run it over all step files after big PC changes (limits, layout).
- FOUND + FIXED: battle_anim_normal.c AnimShakeMonOrBattlePlatforms rebuilt the shaken value's address from two
  s16 sprite fields with a signed low half (0xFFFFxxxx when bit 15 is set). Never on the GBA (IWRAM offsets <
  0x8000); on PC it depends on where the linker puts gBattle_BG3_X / gSpriteCoordOffsetX (seen in one build: the
  guard skipped the shake). PC now uses SHAKE_PTR with an unsigned low half (like pokemon_animation.c's ANIM_SPRITE).
- --pc-only sweep after merging the other session's work (64 objects / 128 sprites, zoom, UI layer): all 18 step
  files clean after the fixes above. Frozen GBA reference rebuilt (/home/claude/frozen) because Route 106 changed
  the ROM (owner rule change 2026-11-12).
- FOUND + FIXED (both builds, GBA ROM changes - allowed since 2026-11-12): text printers are allocated in the heap
  (text.c AllocateTextPrinter), but every map load wipes the heap (MoveSaveBlocks_ResetHeap) without forgetting
  the printer list. A message still on screen during a warp left sFirstTextPrinter pointing into overwritten
  memory; the next map's DeactivateAllTextPrinters walked it (PC crash in CB2_LoadMap2, GBA writes to garbage
  addresses). Now ForgetTextPrintersAfterHeapReset() after InitHeap in MoveSaveBlocks_ResetHeap and ReloadSave.
  Found with a warp test script (test warp while a message was open); normal scripts close messages first.
