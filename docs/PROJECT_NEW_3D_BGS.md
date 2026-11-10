# Project: New 3D battle backgrounds (from our own maps)

Status: **planned, not started.** Only Route 102 was tried, in throw-away test builds. Nothing in the game uses this yet.
Owner asked for this plan on 2026-11-09. Ask the owner before replacing any background in the real game (visual-change rule).

## Why

The current "3D" battle backgrounds (`graphics/battle_environment/wj3d/`, 178 of them) are captures from the 3DS port, shrunk from 336 to 240 pixels wide and squeezed into 3 colour sets (45 colours). The owner finds them blurry and blotchy ("pixelated").

Tested and ruled out:
- **A better converter on the 3DS captures.** It came out only about 19% closer to the original, and dithering looked grainy. Most of the blur comes from shrinking the 3DS picture.
- **The original full-colour captures are gone.** Redoing them takes about 30 minutes per area through the 3DS emulator.

The new idea: build each background from **our own map art**, tilted into perspective, with trees standing up. It stays sharp because it's the game's own pixels at about the right size.

## What the Route 102 test looked like (owner-reviewed)

Pictures are in `docs/new_3d_bgs/`:
- `in_battle_min_zoom.png`: the latest version in a real battle, next to the 3DS background.
- `grass_options_zoom.png`: four ways to draw tall grass. **B, flat on the ground**, is recommended; the owner hasn't picked yet.
- `tree_tops_fixed.png`: whole trees, not sliced flat.
- `colour_sets_compare.png`: 3 vs 4 colour sets, old vs smart grouping.

How the test was built (scripts in `tools/wj3d_ownmap/`):
1. **Render the map from the game's own data.** `maprender.py` builds it from layout, metatiles, tiles and palettes.
2. **Ground pass.** Tree and tall-grass metatiles are replaced by plain grass. The ground is then tilted: each screen row samples the map at a depth, with the scale running from `near` at the bottom to `far` at the top. Nearest-pixel sampling is used, because averaging blurred the grass.
3. **Standing trees.** One clean whole tree is drawn from the tileset (canopy, middle and trunk rows, 32×48). It is stood up at every tree's trunk spot, scaled for depth and drawn back to front. Trees that would poke past the top of the screen are left out.
4. **Ground spots.** Trees are cleared around each Pokémon's feet (enemy about screen 176,76; player about 64,108), and a soft dark oval is drawn there. The owner did **not** like the solid grey shadow; keep the soft one.
5. **Colour pass.** `smartpal.py` gives trees, tall grass and ground their own colour sets, with a 4th set when the game supports it. It writes `tiles.png`, `tilemap.bin` and `palette.pal` in the same format as `tools/wj3d_backdrop.py`.

Settings in the last test:
- `near=0.53, far=1.666` (about 1.9× zoom at the front, tilt ratio about 3.1).
- Grass mode `flat`.
- Camera on Route 102 metatile (12,13), shifted +16/+8 px.
- That is the least zoom with no blank corner at that camera spot. The owner wants "only enough zoom to cut off the blank spots".

To run it:
```
NEAR=0.53 FAR=1.666 GMODE=flat GSQ=1 TAG=x python3 tools/wj3d_ownmap/render_backdrop.py
```
It writes `g_x_rgb.npy` and `g_x_lab.npy` in the current folder. Then:
```
exec(open('tools/wj3d_ownmap/smartpal.py').read())
convert(rgb, labs, 4, True)
write(...)
```
Warning: `render_backdrop.py` briefly overwrites `data/layouts/Route102/map.bin` to render the ground pass, and restores it in a `finally`. Run `git status` afterwards. A real tool should render from an in-memory copy instead.

Test-only game changes that were used and then reverted (needed for real if the 4th colour set is adopted):
- `src/battle_bg.c` (`WJ3D_LoadBackdropPalette`): load the 4th set into BG palette **6**, and run the seasonal foliage on slot 6 too. Slot 6 is only used by the link-battle VS screen. Slots 8/9 are used by battle animation backgrounds, and 10–13 by the move boxes.
- `src/palette.c` (battle background day/night tint): add slot 6 to `PALETTES_BATTLE_BG` and copy it to the unfaded buffer. Without this, the 4th set stays bright at night and shows as a pale stripe.
- Every flash and fade that covers the battle background must include palette 6. This is the same kind of bug as the blocky Poké Ball flash fixed in 0.18.7: `GetBattlePalettesMask`, the ball-open fade, and battle_anim_normal 605. Re-test them all, like the 0.18.7 flash audit (scenarios `fxA-D.json`).
- In the tool, the tilemap palette numbers for the 4th set must be 6, not 5: `slots=(2,3,4,6)`.

## Known problems to solve
- **Overhead vs low camera.** The 3DS camera is low and shows a wall of trees across the top. Ours looks down from higher up, so open spots look bare. Fix: pick a camera spot per area with trees or walls in the back, and pull in the **neighbouring maps** (map connections) so wider views don't run off the map edge.
- **Zoom.** At about 1.9× zoom the pixels are chunkier. Matching the 3DS back-tree size would need about 3×, which is too chunky.
- **Each tileset needs a list** of which metatiles stand up (trees, rocks, signs...) and which lie flat. Each tree type also needs its own clean "canonical" drawing.
- **Caves and buildings.** Walls and cliffs are drawn from above and will look like walls lying on the floor. They need extra work, and may look worse than the 3DS versions. Keep the 3DS versions where they do.
- **Winter.** The winter recolour washes everything out, including the current 3DS backgrounds. A dark shadow only survives winter if it is grey (`SeasonalColor` skips low-saturation colours), but the owner didn't like the grey shadow look.

## Plan (agreed order)
1. **Tool (about 2–3 h):**
   - Make `render_backdrop.py` a real tool: any map, in-memory edits, neighbouring maps, a camera spot per area, and per-tileset stand-up lists.
   - Write straight into `graphics/battle_environment/wj3d/<name>/` in the existing format.
2. **Game side for the 4th colour set (about 1–2 h):**
   - Palette 6 load, seasons, day/night tint, every flash and fade.
   - Run the flash audit again. Test on the **release** build.
3. **Routes and water first: 48 backgrounds (about one session).** Show the owner, ship, and let the owner play with them.
4. **Cities.**
5. **Try 2–3 caves and buildings.** Only convert the rest if they look right; otherwise those keep the 3DS backgrounds.

Total estimate given to the owner: **about 15–25 hours** of work over several sessions. That breaks down as tool 2–3 h, game side 1–2 h, tileset setup 5–10 h, and making and checking all 178 at 6–8 h.

## Rules that apply
- Ask before visual changes, and show test pictures first (the owner has been reviewing every step).
- No new hand-drawn art. Everything comes from the game's own tiles; recolours are OK.
- Test freezes and visuals on the release build. Log each step in `docs/WALLY_JOURNEY_OWNER_REQUIREMENTS.md`.
