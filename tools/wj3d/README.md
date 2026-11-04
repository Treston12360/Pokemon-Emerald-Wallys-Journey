# 3D battle backdrop capture tools (Emerald Overhaul, unfinished)

Goal: per-map GBA battle backgrounds made from Emerald 3DS's 3D battles.
Game side is done (`src/battle_bg.c`, `src/data/wj3d_backdrops.h`, `WJ_3D_BATTLE_BACKGROUNDS`).
`tools/wj3d_backdrop.py capture.ppm out_dir` converts one 3DS frame to tiles/tilemap/palette.

What is needed and NOT in the repo (download again, never commit):
- Emerald 3DS releases: https://github.com/ZallaxDev/pokeemerald-3Ds-dualscreen (Releases: v0.5.0 `Emerald3DS-WebPayload.zip`).
- Azahar libretro core: the libretro buildbot `azahar_libretro.so` for Linux x86_64 (it was fetched as `core.zip`).
- Emerald 3DS v0.5.0 web payload (builder + `payload/`), and its `.3dsx`.
- The Azahar libretro core (`azahar_libretro.so`).
- A clean Emerald (USA/Europe) ROM, SHA-1 f3ae088181bf583e55daf962a92bb46f4f1d07b7 (owner's own dump).

Files here:
- `azahar_fe.c`: headless libretro frontend. `fe core.so game.3dsx script.txt sysdir savedir`.
  Script lines like the mGBA harness (`N`, `N KEYS`, `shot NAME`). Output frames are 400x480 (top+bottom).
  The SD card is `savedir/Azahar/sdmc/3ds/emerald3ds/` (pak, sav, settings.txt with `voxel=1 voxel_battle=1`).
- `mkpak_clean.py PYDIR ROM OUT.pak`: builds the data pack with battle graphics blanked (regex in `BLANK_RE`)
  so a battle shows only the 3D stage. Keeps followers (`follower_fetcher=True`); without them the pack differs.
- `try.sh REGEX name`: builds a pack and boots it 1500 frames, to bisect which blanking breaks boot.
- `example_route104_script.txt`: continue a save, walk in grass, screenshot a wild battle.

Findings so far (2026-11-04):
- A pack built by `mkpak_clean.py` with nothing blanked is byte-identical to the real one (pipeline OK).
- Blanking to all-zero pictures: the game stops at boot with `FATAL: [FS] graphics/pokemon/rayquaza/a...`
  (the title screen's Rayquaza). Last unfinished test: same regex but skipping `rayquaza/`.
  Pictures are now one opaque pixel instead of empty; that alone did not fix the Rayquaza error.
- If blanking keeps failing, fallback: capture the same spot twice with different wild Pokémon and
  take the per-pixel median, or crop around the HUD.
- Camera: GBA (x,y) sits at 3DS (200+(x-120)*1.4, 192+(y-112)*1.4); the converter already crops for this.
- The 3DS port has day/night on (`day_night=1`): set it to 0 for neutral colours (the GBA side tints them).
- Saves per map: hook vanilla pret `WarpToTruck()` to the map/x/y, play the new game in mGBA, save, copy the
  .sav to the 3DS SD folder as `emerald3ds.sav`.

## Step-by-step to finish (do these in order)

1. **Get a clean capture of one route.** Rebuild the blanked pack skipping `rayquaza/`
   (`BLANK_RE='^graphics/pokemon/(?!rayquaza/)[^/]+/(front|back|anim_front)\.4bpp\.lz$'` with `try.sh`).
   If it boots, add trainer back pics and `battle_interface/` back in one group at a time with `try.sh`
   to find any other picture the port needs at boot. Then run the Route 104 script and look at the frames.
   If blanking can't be made to work, use the fallback (median of several captures) from "Findings".
2. **Convert it:** `python3 tools/wj3d_backdrop.py frame.ppm graphics/battle_environment/wj3d/route104`.
   Check `preview.png`: the ground where the Pokémon stand must be clear; paint out leftover shadows/dots
   by copying nearby ground pixels in the 240x160 image (the tool also accepts a 240x160 PNG).
3. **Hook it up:** add a row to `src/data/wj3d_backdrops.h`:
   `{MAP_GROUP(MAP_ROUTE104), MAP_NUM(MAP_ROUTE104), sRoute104Tiles, sRoute104Map, sRoute104Pal}` with
   `INCGFX_U32(".../tiles.png", ".4bpp.lz")`-style includes (tiles.png → .4bpp.lz, tilemap.bin → .bin.lz,
   palette.pal → .gbapal); follow how `src/data/battle_environment.h` includes its graphics.
4. **Test in the GBA harness** (wild battle on that route at noon, at night, and in each season), and check
   the stock "entry" grass that slides in at the battle start still looks right over it.
5. **Show the owner screenshots and get approval before doing more maps.** Then batch: Routes 101–103,
   then the other grass routes. Gyms/Elite Four need trainer battles in the 3DS game (later).
6. Credit Emerald 3DS (Zallax and contributors) in CREDITS.md and add a ledger row.
