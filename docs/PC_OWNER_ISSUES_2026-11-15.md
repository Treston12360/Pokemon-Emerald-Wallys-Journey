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

**OWNER-CONFIRMED 2026-11-15:** The **3D battle introduction/cinematic must be skippable**, allowing the player to get immediately to the **normal battle command menu**, including **Run** for encounters that normally permit fleeing. This is **not** permission to flee before a battle is initialized, cancel mandatory trainer encounters, bypass normal wild Run success/failure logic, or skip scripted battle restrictions. Skip should end/start the battle intro in a consistent initialized state, rather than forcibly abandoning a battle or teleporting back to the overworld.

**Exact skip input (A/B/Start/other), frame at which the skip is accepted, and visual treatment are not yet owner-approved.** Investigate existing skip/fast-forward handling and show the actual control plan before changing it. Keep trainer, double, scripted, and wild battle state intact.

**Known code:** `docs/DELUXE_WORK_QUEUE.md` DLX-4 (2D transition) and DLX-5 (3D transition), `src/platform/sdl2.c`, 3D battle transition paths. **No changes made.**

**Acceptance (OWNER-CONFIRMED desired behavior, NOT tested):** Player can skip the 3D cinematic and immediately access the fully initialized command menu, including the normal Run option if battle type permits; trainer battles still cannot be escaped when normally prohibited. Wild Run retains all ordinary mechanics. No dangling battle state, duplicated entry sequences, interrupted music/scene effects, or 2D regression. Test skip at earliest permitted moment, multiple points in the cinematic, and no-skip baseline.

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
- **3D battle-intro requirement (OWNER-CONFIRMED 2026-11-15):** The cinematic must be skippable to the initialized battle command menu, including ordinary Run in wild encounters; never skip a mandatory trainer/scripted battle. Determine/approve exact skip key and test every battle type before code edits.
- **Verification:** Run the native Windows executable from a complete packaged folder; save logs/PNG comparisons, test both `world3d=0` and `world3d=1`, enable/disable Gamma, verify story flags/old `.sav` import, and record build commit and reproducible test conditions. This report contains no claimed code fixes or new builds.

## Investigation pass 2026-11-15 — scoped source changes and evidence (ChatGPT / Treston)

**Code commit:** `cd66f0f8f4a896d16039f856f20618dc088ceb17` (follow-up comment correction: `e6ca9d7`). **Math-level regression script:** `tools/pc/test_display_geometry.py`, commit `aaa4d37`. **NOT an executable fix verification.**

- **PC-1115-04, 2D zoom — identified a concrete defect and patched it:** `src/platform/sdl2.c` previously computed integer `k = round(outputHeight / desiredWorldHeight)`, which collapses distinct zoom levels onto the same screen scale. At **1280×720** there were only **5 distinct scales for 8 settings**; at **1920×1080** there were **7**. `ComputeFieldView` now computes a fractional `sFieldZ` with nearest-neighbour pixels and clamps the source view to `WIDE_VIEW_MAX_W/H` (640×360); no other rendering source files were altered. For 720p and 1080p the mathematical model now yields **8 distinct scales**. **Still unverified in WallysJourney.exe;** this does not prove that the owner's original failure was exclusively quantization rather than `WideView_SetEnabled(FALSE)` fallback or a different setting path.
- **PC-1115-05, clipping — identified an actual small-render-target overflow and patched it:** `DrawFieldScene` forced UI scale >=1 even if effective output dimensions were smaller than 240×160, placing UI content outside the render target. It now permits fractional UI downscale **only in the too-small case**; the generic non-overworld integer-scaling branches no longer force 1x when 1x cannot fit. Normal-size integer scaling remains unchanged. **Still unverified in the owner's actual settings**; if the clipping happens with targets >=240×160, a different cause remains to be found.
- **Source-model validation:** simulated all 8 zoom levels at 720p,1080p,1440p,4K; checked view buffer bounds and fractional UI placement at 10 resolutions, across normal/indoor zoom. **180 geometry assertions and 4 source checks passed** using source-derived arithmetic in an isolated JS check. The repository includes a separate Python regression script for a real checkout; that script **has not been run in this session**. No PC build, Wine test, GPU screenshot comparison, or save test occurred.
- **PC-1115-01 house colors — not fixed:** User's screenshot has gray/blue 3D building materials, but there is no comparable known-good prior frame or matched 2D screenshot; a root-cause claim would be guessing. Investigate the building-page grade/reload and night lighting on a common save. Claimed by other sessions.
- **PC-1115-02 black lines — not fixed:** DLX-22 records the exact Granite Cave/Pokémon Center locations and an earlier quantization attempt, still pending GPU proof. Distinguish atlas artifacts from geometry cracks using controlled render toggles before altering claimed renderer code.
- **PC-1115-03 battle intro skip — not fixed:** `src/battle_main.c` runs a multi-stage `DoBattleIntro` with controller callbacks; `voxel_gl.c` separately moves the camera for up to 150 intro frames plus a glide. Force-terminating either stage without synchronized battle controller state risks broken encounters. Need choose and implement an input hook and state-safe skip that advances to the **initialized** command menu; retain normal Run restrictions. 3D file currently claimed. **Do not fake a fix by only hiding the 3D camera while the engine still blocks input**.
- **PC-1115-06 route-edge trees — not modified here:** `polish` already has a `POL-30` border fix in active review, so no cross-branch duplication or overwrite is authorized. Its own task still identifies outlier links requiring review.

**CI/build limitation:** The repository's workflows currently build GBA/ROM targets rather than the owner's PC package on `kanto-port`, and this session's container cannot resolve GitHub to clone the tree. A native whole-game build and package could not be produced here. **Do not tick any of these six as fully fixed until the complete Windows executable has been rebuilt and tested on both modes**.
