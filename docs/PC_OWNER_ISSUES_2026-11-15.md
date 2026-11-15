# PC rendering and battle-entry reports — owner, 2026-11-15

**Owner:** Treston. **Branch examined:** `kanto-port` at `7215461e4dd5904f3787ea691696fdc6be1ff3fa` (before this report).
**Status: TRIAGED / NOT FIXED / NO BUILD TESTED.** These are the six symptoms the owner reported in his own words, preserved here so no contributor invents or silently changes a requirement.

**Screenshot:** Owner provided a 1548×1149 PNG in the ChatGPT conversation on 2026-11-15 (filename `image(20261115-173757).png`). It shows the native 3D coastal town with buildings, beach, trees, and sea. The screenshot is an external conversation attachment; **not** committed to the repo, so do not claim it is accessible from the source tree. Ask the owner for the original file if pixel-level reproduction/comparison is needed.

**Strict rules:** Work on the correct project/branch. Review `AGENTS.md`, `docs/WALLY_JOURNEY_OWNER_REQUIREMENTS.md`, `docs/TASKS.md`, `docs/WORK_CLAIMS.md`, `docs/POLISH_TASKS.md` on `polish`, and relevant `docs/DELUXE_WORK_QUEUE.md` references first. Do not edit claimed engine/voxel files while another session owns them. No spoiler material in shared documents. Test both 2D and 3D, in the complete Windows package, without breaking imported GBA saves. Do not claim fix until executable test.

## Owner's six issues and evidence

### PC-1115-01 — Houses losing their color (3D)

**Owner wording:** "the houses lost their color for some reason".

**Observed:** Uploaded screen shows multiple blue-roof/gray-wall houses; without a matching old screenshot, the exact intended colors and whether daytime/season/Gamma settings were active are **not known**.

**Possible investigation (NOT a proven cause):** The building texture palettes / seasonal reload path (DLX-17) and 3D day-night/window lighting (POL-26b) are relevant. Compare loaded model/albedo texture vs. output, with `gammaLight=0/1` and day/night tint switch, then compare with the same map's 2D colors. Do not simply increase saturation as a guess.

**Files/ownership:** `src/platform/voxel3d/voxel_gl.c`, `voxel_world.c` and related building textures are in active ARIA/POLISH work claims. **No changes made.**

**Acceptance:** Match building colors to the actual intended tiles/palette across day/night, seasons, Gamma on/off; no new mismatch in 2D.

### PC-1115-02 — Black seams in Granite Cave and Pokémon Center (3D)

**Owner wording:** "there is black lines inside of certain areas like granite cave, pokemon center".

**Existing reports:** `docs/DELUXE_WORK_QUEUE.md` DLX-22 documents black lines in **those exact places**, and says a tile-coordinate quantization fix was made, still needing owner GPU verification. `docs/POLISH_TASKS.md` POL-8 black lines on indoor floors, POL-28 Petalburg Woods black lines are still open.

**Investigation:** Test precise cave and Pokémon Center positions with a real GPU. Determine whether the lines are depth cracks between coplanar meshes, cracks between atlas textures, thin geometry holes, or transparent texture edges; do not assume all black seams share one cause.

**Acceptance:** No lines or light leaks while walking and rotating camera; no broken texture sampling; inspect 2D for regression.

### PC-1115-03 — Ability to exit battle before the 3D intro transition is finished

**Owner wording verbatim:** "Allow us to leave the battle before the 3d battle transition happens once entering battle".

**Interpretation NOT LOCKED:** Could mean allow battle commands/Run immediately rather than waiting for a cinematic intro, or allow skipping/canceling the 3D intro to leave the battle sooner. **Do not pick one behavior silently.** At minimum, examine early input/interrupt handling and the transition-to-battle state. Do not permit wild-battle Run to skip a mandatory trainer battle or corrupt battle startup. Keep double-battle and scripted encounters correct.

**Known code:** `docs/DELUXE_WORK_QUEUE.md` DLX-4 (2D transition) and DLX-5 (3D transition), `src/platform/sdl2.c`, 3D battle transition paths. **No changes made.**

**Acceptance (pending exact owner interpretation):** A permitted exit/skip behavior can be triggered promptly, while mandatory trainer/scripted battles remain mandatory, and no transitions leave dangling battle state.

### PC-1115-04 — 2D zoom not responding

**Owner wording:** "the zoom isn't working in 2d for some reason".

**Code inspection, NOT a reproduction:** In `src/platform/sdl2.c`, `ZoomStep` updates `PLATFORM_SETTING_ZOOM`; `ComputeFieldView` reads the value, calculates `sFieldZ` and requests `WideView_SetViewSize` from `wide_view.c`. `GetMode` returns `WIDE_VIEW_FIELD` only during normal overworld callbacks with the BG map set up; other screens use native-size mode. Zoom values are quantized to integer world-pixel scales, so adjacent settings can render identically at some window sizes; **this does not yet establish the owner's actual bug**.

**Investigation:** Test `world3d=0`, outdoor and indoor map, several sizes (720p,1080p,1440p,4K, narrow/resized), all Zoom 1–8 and `-`/`=`/mouse-wheel. Log setting, computed `sFieldZ`, view dimensions, `WideViewFrame.mode` and map texture dimensions. Check `WJPC_NOWIDE` and `PcGuard` fallback, compare settings UI with actual visible tile count.

**Acceptance:** Every supported zoom level has the intended clear effect in 2D when enough screen resolution exists (otherwise UI should explain constrained levels); 3D remains intact.

### PC-1115-05 — Start menu off-screen / oversized under some screen settings

**Owner wording:** "sometimes with certain screen settings the start menu goes off the screen and is too big".

**Existing report:** Polish `POL-6` describes oversized menus with Widescreen off, tested at 960×640 without reproduction; fullscreen/real monitor configurations remain untested.

**Code inspection:** `src/platform/sdl2.c` `DrawFieldScene` currently chooses UI scaling as `s = min(outW / 240, outH / 160)` (whole number), with a minimum of 1, and positions the opened start-menu UI at `outW - 240*s`. At sub-native render targets this can clip. This alone does **not** explain a 960×640 or larger case; verify effective scene/render target sizes, resolution setting, aspect lock, integer scaling, fullscreen/high-DPI and settings combinations before altering UI.

**Acceptance:** Full start menu, selection cursor, and any nested menus fit fully within viewport with Widescreen both on/off, windowed/fullscreen, 720p/1080p/1440p/4K, integer scaling on/off, no misplaced touch/controller hitboxes.

### PC-1115-06 — Edge/outskirts trees change when crossing routes

**Owner wording:** "the trees outside usually on the outskirts of the screen changes when you change routes".

**Existing reports:** DLX-25 in `docs/DELUXE_WORK_QUEUE.md` previously documented trees vs sea at Dewford/Route 106/107. Polish `POL-30` generalizes this to all route transitions; a proposed nearest-map border fix is **already pushed on branch `polish`** on 2026-11-15, with remaining exceptions Route 113 to 112/111, Route 118/123, Verdanturf and unverified links. Its border-height blending is marked as an owner-unapproved design choice. **Do not duplicate or overwrite it from `kanto-port`.**

**Acceptance:** Edge terrain and trees do not swap abruptly when crossing route boundaries in either direction, and geometry continuity holds; check all connected routes with camera at typical 3D/zoom positions. Coordinate integrating POL-30 only after owner/project review.

## Work assignment / safe next steps

- **Currently claimed 3D areas:** `src/platform/voxel3d/voxel_gl.c`, `voxel_world.c`, belt/backdrop paths and `pc_data/voxel/`, with active entries in `docs/WORK_CLAIMS.md`. No renderer edits by this report's author.
- **Potential unclaimed but shared PC files:** `src/platform/sdl2.c`, `src/platform/wide_view.c`, `src/option_menu.c`; acquire a work claim and check latest cross-branch edits before changes. Avoid speculative fixes to zoom or menu sizing until reproduced.
- **POLISH coordination:** POL-6, POL-8, POL-28 and POL-30 overlap four symptoms. Do not mark those tasks completed based on documentation or earlier attempts.
- **3D battle-intro requirement:** keep verbatim until the precise permitted input behavior is confirmed; do not conflate ability to **run from a wild battle** with an ability to skip required trainer/scripted battles.
- **Verification:** Run the native Windows executable from a complete packaged folder; save logs/PNG comparisons, test both `world3d=0` and `world3d=1`, enable/disable Gamma, verify story flags/old `.sav` import, and record build commit and reproducible test conditions. This report contains no claimed code fixes or new builds.
