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

## Findings 2026-11-04 (second session) — blanking WORKS now
- **Cause of the boot/battle FATALs:** the 3DS port reads every asset at its original stored size; a blanked
  LZ stream is much shorter, so it logs `asset short read` and stops. `mkpak_clean.py` now pads each blank
  back to the original size. With that, blanking Rayquaza (title) and Pokémon/healthbox pictures works.
  Put an empty `debug.txt` in the SD folder to get `port.log` with the exact asset errors.
- **The 3D overworld (`voxel=1`) crashes the Azahar core** (SIGSEGV) after loading a save in this headless
  setup. Use `voxel=0` + `voxel_battle=1`: the overworld is 2D, battles are still 3D.
- **Downloads:** Emerald 3DS `.../releases/download/v0.5.0/Emerald3DS-WebPayload.zip` works; the Azahar core
  is on Azahar's own release: `https://github.com/azahar-emu/azahar/releases/download/2126.2/azahar-libretro-linux-x86_64-2126.2.zip`
  (buildbot.libretro.com may be blocked). Build `azahar_fe.c` with libretro.h from libretro/libretro-common.
- **Route 104 save:** pret pokeemerald (`make modern`) with `WarpToTruck` → `MAP_ROUTE104` (23,60), a Lv30
  Mudkip and the Pokédex/running flags given in `NewGameInitData`, and `Task_NewGameBirchSpeech_Init`
  jumping straight to `CB2_NewGame`. In the mGBA harness: `1200 / 10 S / 300 / 10 A / 400 / 10 A / 300 / 10 A /
  1200`, then Start, 4× Down (SAVE), A, A, wait 1000 frames. Use the first 131072 bytes of the .sav.
- Emulation speed: about 7 frames per second; a walk-into-battle run is ~35 minutes.

## Working pipeline (2026-11-04, used for Routes 101-104)
1. **No 3DS health boxes:** copy `Emerald3DS.3dsx` and rename `hud.bin` in its romfs (UTF-16 name, e.g. to
   `hxd.bin`): the port then logs "battle/hud.bin missing, info boxes stay as the GBA's" and draws the GBA
   boxes, which the pack blanks. Pack regex (with `mkpak_clean.py`, padded blanks):
   `^(graphics/pokemon/[^/]+/(front|back|anim_front)\.4bpp\.lz|graphics/battle_interface/(healthbox_[a-z_]+|enemy_mon_shadow|ball_status_bar|battle_bar)\.4bpp\.lz|generated/assets/build_root_src_graphics__gHealthboxElementsGfxTable\.bin|graphics/fonts/[a-z_]+\.latfont)$`
   What's left: three cream (255,255,222) text-window bars and the foe's drop shadow.
2. **OpenGL, not software:** run `azahar_fe` with `FE_GL=1 FE_GFX=OpenGL` (Mesa llvmpipe). The software
   renderer crashes in `Pica::Texture::LookupTexelInTile` once the 3D overworld is on. `voxel=1 voxel_battle=1`.
3. `grass.py LAYOUT_X` finds a grass tile with grass above/below in both vanilla and our maps.
4. `capture_route.sh name MAP_X x y` (env `GAME3DSX`, `PAK`): builds the pret capture ROM for that spot
   (pret `src/new_game.c` has `WJ_MAP/WJ_X/WJ_Y` defines), makes the save in mGBA, runs the 3DS capture
   (~35 min). `shots/c6.ppm` is the "What will X do?" frame.
5. `autoclean.py shots/c6.ppm clean400.png`: upper bar = copy of the strip above (keeps leaves/grass), the
   other bars and the shadow = OpenCV inpaint (those spots sit under our own health boxes in game).
6. `to_gba.py clean400.png gba.png`, then `tools/wj3d_backdrop.py gba.png graphics/battle_environment/wj3d/<map>`
   (folder = map name lower case without MAP_), then `tools/wj3d_gen_table.py` rewrites the table.

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
