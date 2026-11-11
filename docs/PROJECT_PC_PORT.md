# Project: Wally's Journey as a PC program (.exe)

Status: **alpha 1 works** (branch `pc-port`, based on 0.18.19). Owner asked for this on 2026-11-11:
"a .exe file at the end of it with all those cool features that normally come with recomps".

## What it is
The game's own C code compiled for PC with SDL2, not an emulator. The platform layer
(screen drawing, sound mixer, input, save chip, clock) comes from the pokeemerald PC port
(Kurausukun / NTx86 `pc_port`, plus the Windows/Linux work in the owner's Emerald-PC-Engine,
taken at commit 2f35335ef, before that repo switched to loading content from the ROM).

Saves are the same 128 KB flash image mGBA uses, so saves move both ways
(tested: friend's save loads on PC; a save made on PC loads in mGBA).

## Rules kept
- **The GBA ROM must not change.** Every change to shared code is behind `#ifdef PORTABLE`
  (C) or `.ifdef PORTABLE_ASM` (asm). Check: `make release` on this branch gives the same
  SHA-1 as the normal build (0.18.19: 21f006adcc5d884c4992805296e171f45428c46b).
- PC-only files live in `src/platform/`, `include/platform/`, `tools/pc/`; the GBA Makefile skips `src/platform/`.

## Building
Run the normal GBA build first (`make release`) so all generated files exist, then:
```
make -f Makefile.pc            # Linux 32-bit: ./wallys-journey (needs gcc-multilib + i386 SDL2)
make -f Makefile.pc WIN=1      # Windows 32-bit: WallysJourney.exe (needs mingw-w64 + build/pc-deps/SDL2)
```
`build/pc-deps/SDL2/i686-w64-mingw32` = SDL2 2.30.9 + SDL2_image 2.8.2 official MinGW dev packages.

## How the hard parts were solved
- **Struct layout:** the GBA compiler (apcs-gnu) aligns every struct to 4 bytes. `tools/pc/align4.py`
  adds `aligned(4)` to every game struct on PC. Windows also needs `-mno-ms-bitfields -mno-align-double`.
  `tools/pc/layoutcmp.py` compares all 779 struct layouts against the GBA debug build: only the
  sound engine's internal structs differ (on purpose).
- **Asm data:** `tools/pc/asmconv.py` turns GBA assembly data (scripts, maps, battle scripts, songs) into x86 assembly (`--coff` for Windows).
- **Tagged script pointers:** the GBA marks "effect" functions by pointing into ROM mirror 1. On PC the
  `effect_ptr` macro keeps them in a list; `tools/pc/effect_funcs.sh` turns that list into a C table.
- **Busy-waits on VBlank:** the PC only runs VBlank between frames, so `IsDma3ManagerBusyWithBgCopy` drains the DMA queue itself.
- **ARM vs x86 shifts:** shifting by 32 gives 0 on ARM but not on x86 (`ARM_LSL32`/`ARM_LSR32`, used in text drawing).
- **NULL reads** that are harmless on GBA (it reads BIOS) crash on PC; fixed where found (battle transition end).

## Testing
`WJPC_SCRIPT=script.txt` runs the game frame-exact as fast as possible: `wait N`, `press KEYS N`,
`shot file.ppm`, `quit`. `WJPC_SAVE=` picks the save, `WJPC_AUDIO_DUMP=` writes raw sound.
`tools/pc/harness2script.py` converts the mGBA harness scripts. The Windows .exe is tested under Wine.

Tested so far: intro, title, main menu, friend's save, overworld, start menu, party menu,
wild battle (moves, EXP, back to map), saving, save back into mGBA. Not tested yet: real-time
sound/controllers on a real Windows PC, trainer battles, long play.

## Next (agreed order)
1. Owner tests alpha 1 on Windows.
2. Go system by system against the ROM (battles, menus, 3D backgrounds, seasons, sound, gacha).
3. Widescreen and zoom (overworld drawn from map data), then smooth frame rate, then 4K.
