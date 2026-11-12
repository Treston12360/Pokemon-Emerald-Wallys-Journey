# Project: Wally's Journey as a PC program (.exe)

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
- Saves must stay importable both ways (PC <-> mGBA). The save file is the raw 128 KB flash image.

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
