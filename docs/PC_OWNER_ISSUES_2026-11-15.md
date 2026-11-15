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

## Second source audit — 2026-11-15 (inspection only; no new code changes)

This continues the investigation on `kanto-port` while avoiding all voxel renderer/data files claimed by the Polish/ARIA sessions. Evidence below distinguishes **confirmed code behavior** from **hypotheses about the owner's symptoms**. Branch heads at inspection: `kanto-port=79f4d954`, `polish=be4cb4ea`.

### A. 2D zoom has an *additional* on/off gate (CONFIRMED CODE PATH; symptom linkage depends on user's settings)

`src/platform/wide_view.c::GetMode` returns `WIDE_VIEW_GBA` immediately when `!sEnabled` (or `WJPC_NOWIDE`). `src/platform/sdl2.c::ReadConfigFile` sets this from `widescreen=0`, and `Platform_SetSetting(PLATFORM_SETTING_WIDESCREEN)` does the same in Options. `DrawScene` uses `sFieldZ` **only** when `WIDE_VIEW_FIELD`; the standard `WIDE_VIEW_GBA` path renders 240×160 with its general scaling logic and ignores zoom. Therefore, **2D + Widescreen OFF means no overworld zoom**, even after the arithmetic patch. World 3D and Widescreen are independent options. A guarded callback failure in `VDraw` can also permanently call `WideView_SetEnabled(FALSE)` until manually reenabled; inspect `WallysJourney_errors.txt` before assuming this occurred.

**Implication:** The earlier cd66f0f source fix addresses *duplicate levels while wide view is ON*, not the separate gate when wide view is OFF. Keep the user's intent for 2D zoom (independent of Widescreen or only when ON) distinct; no behavior was silently changed here.

### B. Start-menu scale — there are at least two paths, and normal-size clipping is NOT yet explained

`src/platform/sdl2.c::DrawFieldScene` scales the 240×160 **UI layer** using whole integer `s=floor(min(outW/240,outH/160))`; on a 2560×1440 scene, it is **s=9**, so an 80-pixel-wide menu panel appears **720 screen pixels wide**. The start-menu texture is deliberately aligned at the right edge when `Vol_StartMenu_IsOpen()`. It *mathematically fits* ordinary positive output sizes. When Widescreen is OFF, `WideView_GetMode` switches to the separate `WIDE_VIEW_GBA` branch, which draws the complete 240×160 picture through the general scaler. The earlier small-target patch only addresses outputs **smaller than 240×160** (and subnative generic viewports) and cannot alone explain POL-6 at normal 720p–1440p configurations. Existing POL-6 at 960×640 was NOT reproduced on Linux.

**Diagnosis still needed:** determine whether the screenshot complaint refers to a menu disproportionately large within the 3D field, true clipping in 2D GBA mode, or actual coordinates outside a Windows high-DPI/fullscreen viewport. Compare `widescreen=0/1`, `world3d=0/1`, `resolution=0/1/3`, `integerScale=0/1`, fullscreen, actual `SDL_GetRendererOutputSize`, `GetSceneSize`, resulting `sFrameInfo.mode`, `gameViewport`, `ux,uy,s` and Windows scale. Don't claim the earlier fix resolves ordinary-screen cases.

### C. 3D battle introduction — camera is not the battle engine (CONFIRMED)

The 3D-specific camera in `src/platform/voxel3d/voxel_gl.c` has `VOXEL_BATTLE_INTRO_WAIT=150` and `VOXEL_BATTLE_GLIDE_FRAMES=100`, affecting `BattleCamera` and its cinematic bars. `src/battle_main.c::DoBattleIntro` independently waits for controller operations: data requests, BG slide, trainer/wild send-outs, first-turn setup, then `TryDoEventsBeforeFirstTurn` and `HandleTurnActionSelectionState`. **No evidence** that the 3D camera gate directly controls action menu availability. Simply snapping `sBattle.glide` to the end would not guarantee early command/Run access.

**Safe fix design to develop:** owner-confirmed B-to-skip was a *suggestion only, not approved binding*. On a skip request, preserve mandatory controller initialization, sprites and battler state, but shorten/advance skippable presentation stages, then finish 3D camera at its final pose. Test wild Run success/failure, trainer cannot Run, double/scripted encounters, 2D unaffected, and repeated B presses. No source changed by this audit.

### D. Black lines — isolate texture-alpha gaps from actual geometry cracks (OPEN but now a narrow experiment)

Existing `DLX-22` port rounds static mesh vertices to **1/512 tile** in `voxel_gl.c::MeshSetQuantised`; it is already used for terrain, trees, buildings, and static models. Earlier software-GL tests couldn't reproduce the owner's GPU artifacts. The shader samples textures then **discards pixels with alpha below 0.5**. `WJPC_3D_NOALPHA=1` sets that test threshold to -1; **if this removes the black lines**, translucent/transparent atlas edges are implicated. If lines remain, investigate mesh gaps, depth precision, tile discontinuities, and back-face/edge geometry. `WJPC_3D_UV=1` visualizes UVs but also changes the alpha-min debug behavior. Test Granite Cave and Pokémon Center at unchanged camera coordinates, with identical settings, while standing and while moving. Do not change all atlas UVs or depth values based on guesswork.

### E. House color — model-specific versus global lighting, with a packaging confound (OPEN)

The user's screenshot `image(20261115-173757).png` shows multiple **blue** roofs, but a large Gym roof that is visually **pale gray / minimally textured**. This does not support claiming every roof's hue was globally removed. Two *confirmed code paths* that could affect colors:
- `VoxelBuildings_ReadPage` grades model texels with `VoxelGrade_Texels`; `voxel_gl.c::LoadBuildingPage` then applies `VoxelWorld_SeasonTexels` to **every baked building texel**. The seasonal LUT's fallback runs `WallyJourney_SeasonalColorNow`, which recolors green-hue material even if part of a *building* (winter reduces saturation, autumn changes green to warm colors). **Potential model color defect**, but not proof this specific roof was originally green.
- Gamma lighting / sky illumination / bloom in `voxel_gl.c` affect surface brightness and cloud patterns; `docs/DELUXE_WORK_QUEUE.md` previously corrected a too-bright/blue sky table. Compare Gamma ON/OFF; `WJPC_3D_NOBLOOM=1` removes bloom; `WJPC_3D_GDEBUG=2` shows vertex shade / =3 normals / =4 cloud term. Keep season fixed and compare house to the 2D tiles.

**Package/build issue:** `tools/voxel3ds/README.md` says 3D models are generated with Emerald 3DS scripts against WJ map data, not derived at runtime, and that an older ZIP once shipped outdated `voxel/` binaries and broke lighting/furniture. `tools/pc/make_package.sh` now copies `pc_data/voxel` to `voxel/` beside the EXE. The currently committed `pc_data/voxel/buildings.bin`, `relief.bin`, `signposts.bin` share the 2026-11-14 add commit `67c92c47df` (later than the 3DS generator patch's latest `c654a59f69`), **so there is no evidence the current repo binaries are automatically older than its generator changes**. The particular owner's local ZIP may still be from an earlier build — cannot know without comparing its hashes. Source documents also record specific mismatches between custom map art and generic gym model specifications. Check the matching binary and source revision **before** altering a shader or recoloring a roof.

### F. Route belt — regression is an existing branch diff, not an unfixed mystery

`POL-30` on `polish` directly identifies the former per-current-map border belt selection as cause of tree/terrain changes at route transitions. Its sweep checks 64 Hoenn links and 34,029 nearby cells: art mismatches 8,318→641 and heights 9,661→70 after nearest-loaded-map border selection and height blending. Remaining outliers include Route 113's ash tree palette needing a second atlas, Route 118/123, and Verdanturf. This is **not yet a 0-difference or owner-GPU-verified result**. Do not copy/replace its claimed `voxel_gl.c` / `voxel_world.c` edits without coordinating, and do not claim finished.
