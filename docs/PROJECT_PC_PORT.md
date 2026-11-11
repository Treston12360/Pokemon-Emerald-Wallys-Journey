# Project: Wally's Journey as a PC program (.exe)

**Branch:** `deluxe` (PC work started on `pc-port`) (based on 0.18.19). Read this whole file before working on the PC version.
**Status (2026-11-11):** alpha 4 (wide view) sent to the owner as `WallysJourney.exe`. Steps 1 (PC settings) and
2 (widescreen) are done. **Next: step 3 (smooth high frame rate).** The alpha 4 .exe could not be run before sending
(no Wine in that container), so the owner's first report on it matters: if it does not start, look at the
Windows-only build steps under "Build environment" first.

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
3. **Smooth high frame rate** — game logic stays at 60 updates/s (so nothing breaks); extra frames are
   drawn in between on 120/144/240 Hz monitors. Walking + camera first, then sprites, menus, battle
   animations. Line-by-line effects (water ripple, some battle intros) stay at 60. Needs step 2's renderer.
   Setting: "60 (original) / match monitor / cap". Estimate 5–15 h.
4. **4K** — clean upscaling + optional filters. (High-res replacement art would be new art; ask the owner first.) 2–4 h.
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

## Owner feedback log
- alpha 1: crashed at Birch's "This world is widely inhabited by..." (fixed alpha 2); Space made it do
  "weird stuff" (fast-forward stutter, fixed alpha 2).
- step 2 test pictures: owner said "Do whatever you think will look the best and download whatever tools you think
  is necessary to finish". Alpha 4 (wide view) sent with 9 screenshots from the real PC build.
