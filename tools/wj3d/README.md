# 3D battle backdrop capture tools (Emerald Overhaul, unfinished)

Goal: per-map GBA battle backgrounds made from Emerald 3DS's 3D battles.
Game side is done (`src/battle_bg.c`, `src/data/wj3d_backdrops.h`, `WJ_3D_BATTLE_BACKGROUNDS`).
`tools/wj3d_backdrop.py capture.ppm out_dir` converts one 3DS frame to tiles/tilemap/palette.

What is needed and NOT in the repo (download again, never commit):
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
