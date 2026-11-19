# Project Polish — Brennan's fix list

Project Polish is Brennan's half of the one game (see AGENTS.md "Who works on what"): the existing game —
Hoenn in 3D, lighting, and the bugs he finds. Branch: **`polish`** (cut from `kanto-port` 5f0aae61 on
2026-11-14, so it has everything; `git fetch` and merge `origin/kanto-port` into it often). Older Polish items
are the DLX entries in `docs/DELUXE_WORK_QUEUE.md`.

Rules that apply here (AGENTS.md has them in full): never guess — ask Brennan; show him a PNG of every fix
("With all fixes send me pngs of the fixes and if there are any questions ask me"); **never tell Brennan
anything about the Kanto story**; test on `WallysJourney.exe`; builds handed over are the whole zipped folder.
**Test every fix in both 2D and 3D** (Treston, 2026-11-14: "Make sure you're testing everything in 2D too all fixes
need to be checked for both"): a 3D fix must not break the 2D game, and the reverse. Say in each item which modes
were tested (`world3d=0` / `world3d=1` in `WallysJourney.cfg`).

## The list (Brennan, 2026-11-14, his words). Three are marked "prioritize this".

Status: `[ ]` open, `[~]` in work (who, date), `[x]` done (date, commit, what was sent to Brennan).
Add under each item: what was found, what was changed, files, how to test, what is left.

- [~] **POL-11 (PRIORITY). Freezing in 3D.** "There's freezing and pausing when in 3D making the whole game stop
      for a second or two then start again." (Claude 014PwRDj, 2026-11-14, in work together with POL-14)
      - **Taken over by Claude 019khs4X, 2026-11-15 (Treston: "Yea you can take over"; 014PwRDj had pushed nothing
        on it for ~11 hours).** Lead: Brennan's `WallysJourney_errors.txt` (below, "Files Brennan sent") lists four
        callbacks the NULL guard (src/platform/null_guard.c) skipped. The guard logs each callback ONCE, but a
        broken sprite / task / vblank callback runs every frame: the guard may be catching a fault 60-144 times a
        second, and Windows exception handling is slow - a possible cause of "freezing and pausing" and of the
        crash after "a lot of skipping". Not proven yet. His build is unknown (save made after fc0c34d283,
        11-13 15:35; report 11-14 15:45: ~26 code commits), so the addresses are not resolved.
      - Not reproduced yet. Stress run in Wine + Xvfb (software GL), Gamma lighting off, the shipped `pc_data/voxel`,
        a late-game save, script = warp to each of the 56 outdoor Hoenn maps and walk a square (`WJPC_TEST_NO_BATTLES=1
        WJPC_3D=1 WJPC_VOXEL_LOG=1 WJPC_TEST_WINDOW=1`): 26,000 frames, 45+ warps, worst frame 77 ms of map/belt
        work (a warp: the map underfoot + whole belt built at once), memory flat at ~480 MB. Nothing near 1-2 s.
      - **More from Claude 014PwRDj (2026-11-14 evening, written up 2026-11-15; for whoever has POL-11/14 now):**
        - Gamma lighting ON can be stress-tested here if the window is tiny: Brennan's cfg with `resolution=0`,
          `windowScale=1` and Xvfb `-screen 0 640x400x24` gives a 320x180 3D picture at ~25-35 fps in software GL
          (at 1280x720 it is ~3 fps). With that, his save, Gamma on, the same 56-map script x3, `WJPC_TEST_NO_BATTLES=1`:
          20,000+ frames (about 14 minutes), no crash, no errors file, memory flat (~360 MB RSS).
        - So neither Gamma nor warping/walking reproduces it **in the test-script loop**. That loop is not the
          real one: with `WJPC_SCRIPT` the main loop runs frame by frame as fast as it can - no frame-rate setting,
          no 144 fps in-between frames (`smooth` is forced off under a script), no vsync pacing, no sound-timing
          gate. Brennan plays at frameRate=144 with interframe=1. The freeze may live in that real-time path.
        - Route 110 -> Mauville (POL-15) recorded every 2nd frame for 220 frames with his cfg (Gamma on) at
          640x360: frame-to-frame difference is smooth, nothing disappears in the current build.
        - **Unfinished patch, NOT applied** (files are claimed by 019khs4X): `docs/polish_reports/
          POL-11_stall_log_and_realtime_script.patch` (against d46130bf, `git apply --3way`). It adds
          (1) `WallysJourney_stalls.txt`: always-on log, a line whenever one trip round the real main loop takes
          over 150 ms saying where the time went (waiting for the game thread, VDraw, of which 3D: maps / belt /
          drawing, DrawScreen, present) plus map, mesh and page counts and the free 32-bit address space
          (`GlobalMemoryStatusEx`), and one line a minute - so Brennan can just send the file after a freeze;
          (2) test aid `WJPC_SCRIPT_REALTIME=1`: the script only presses buttons while the game runs its normal
          real-time loop. It compiled; the first real-time run was interrupted before any result. Use or drop it.
      - Still to try: walking across seams for a long time instead of warping, battles in between, real-time
        speed with sound (see the patch above). Asked Brennan for his build name, his
        `WallysJourney.cfg`, and `WallysJourney_crash.txt` / `WallysJourney_errors.txt` from his game folder.
- [~] **POL-14 (PRIORITY). Crash in 3D after about 10 minutes.** "When in 3D mode after about 10min the game
      crashes after a lot of skipping and pausing." (Claude 014PwRDj, 2026-11-14)
      - Not reproduced yet (see POL-11's run: 12 minutes, no crash, memory flat). The game writes
        `WallysJourney_crash.txt` next to the exe when it crashes: Brennan's copy says exactly where. Look the
        addresses up with `llvm-nm -n WallysJourney.exe` on the unstripped exe of the same commit.
      - Found on the way (a test-script artefact, but a real weak spot): a test `warp` during a wild battle
        crashes in `VoxelBattle_Shadows` (`voxel_battle.c`) called from sdl2.c `BattleEffects` while callback2 is
        `CB2_LoadMap`: `gMain.inBattle` is still set but the battle's heap is gone (`gBattleSpritesDataPtr`
        dangling). Normal play clears inBattle before a map load, so this should not be Brennan's crash; the test
        aid `WJPC_TEST_NO_BATTLES=1` (no wild encounters, trainers don't spot) keeps scripts out of battles.
- [x] **POL-25 (PRIORITY). Unlearn HM moves.** "Make it to where you can unlearn an HM move." Done 2026-11-14
      (Claude 014PwRDj); picture sent to Brennan (`docs/polish_reports/POL-25_hm_move_replaced.png`).
      - Found: the game's setting already allows it (`P_CAN_FORGET_HIDDEN_MOVE TRUE`, include/config/pokemon.h), and
        battles / evolution / the old summary screen ask `CannotForgetMove()`. But the summary screen the game
        actually uses (`src/bw_summary_screen.c`, `CanReplaceMove`) asked `IsMoveHM()` directly, so picking an HM
        move to replace still said "HM moves can't be forgotten now." Changed to `CannotForgetMove()`.
      - Safe because field HMs never need a Pokémon to know the move (owner requirements, "Quality-of-life").
      - Tested in the exe (Wine): test script `state setmove 1 57` (Surf in slot 2), `state learnscreen 85`
        (the "which move to forget" screen for Thunderbolt, as a TM opens it), Down, A -> slot 2 is Thunderbolt.
        Both `state` sub-commands are new test aids (`src/platform/wj_pc_glue.c`).
      - Open question to Brennan: the Lilycove Move Deleter still refuses to delete the last Surf in the party
        (`IsLastMonThatKnowsSurf`, vanilla rule so you can't strand yourself; not needed here since Surf works
        without a Pokémon knowing it). Remove that refusal too? Waiting for yes/no.
- [x] **POL-1. People look like paper.** "People's silhouettes are still like paper. They need to be rounded out
      to look like people."
      - 2026-11-15 (014PwRDj): Emerald 3DS draws people flat too. Prototyped two looks (thick pixel slices; shading
        darker to the edges) and showed Brennan `docs/polish_reports/POL-1_flat_thick_round.png`; from the game's
        camera both change little. **Brennan: "Keep them flat"** - closed, no change.
- [ ] **POL-2. Background trees end in blue.** "The background trees go up in the sky revealing a blue background."
- [ ] **POL-3. Mountain edges show blue.** "The mountains edges reveal a blue background."
- [ ] **POL-4. People vanish behind you.** "People disappear after walking away a few steps then come back when
      you turn around, they need to be there at all times."
      - Not reproduced yet (Claude 019khs4X, 2026-11-14), 3D and 2D (`world3d=1` / `0`, Windows exe, Wine):
        Slateport (warp `0 1 19 20`, 6 tiles down and back) and Oldale -> Route 101 -> Littleroot (warp `0 10 10 12`,
        walk down, then back up): everyone in view stayed in view.
      - Checked in the code: the 3D view does not hide people the 2D game marks off-screen (`voxel_entities.c`
        ~1719, as Emerald 3DS); in 3D, people load and stay loaded 42-44 tiles out (`wide_view.c` UpdatePads,
        DLX-18), far past what the camera shows. Differences from Emerald 3DS that could matter and are NOT
        changed: the 3DS sizes that box from the 3D camera's real view and reloads people when the view changes
        without a step (`UpdateObjectEventsForViewChange`); PC loads the extra ring only while more than 4
        object slots are free (`WJPC_SpawnOk`), so a crowded map could leave far people unloaded.
      - Need from Brennan: where it happens (map), 2D or 3D, a screenshot or short video, and what "turn around"
        means (the player facing the other way, or walking back?).
- [x] **POL-5. Trainer Hill.** "Trainer hill entrance is missing the top of the door. Inside of trainer hill
      building the things inside at the front of the building are cut off and doesn't look 3D, the pc is cut off
      at the bottom, the top of the healing station and back wall looks crunched and cut off like the back wall
      is down too far."
      - **Fixed 2026-11-16 (014PwRDj), data only** (Brennan: "Continue working on trainer hill until it's complete"):
        - Door: a 3D door frame (cap with the Poke Ball, posts, doorway) and the cliff face beside its foot as a
          model on Route 111 (and Route111_NoMirageTower, which Brennan's save runs); its foot cells' relief set
          flat so it stands on the ground. The cap was lost before because those cliff cells are relief.
        - Room: the lobby's back wall stands 20 px tall (its green panels down to the counters, rows 112-131) instead
          of 16 (the panels' last rows lay flat, "crunched"); the healing machine is a box on the counter, not
          painted flat on it. The PC already stands (a box); the stools are 3D (they are cut only by the screen edge).
        - How: specs `tools/voxel3ds/trainer_hill_spec.py`, splice/patch tools + notes `tools/voxel3ds/README.md`.
          Pictures: `docs/polish_reports/POL-5_trainer_hill_door_before_after.png`,
          `POL-5_trainer_hill_room_before_after.png`.
- [x] **POL-6. Boxes too big outside widescreen.** "When not in widescreen mode, the text box and main menu box
      is too big for the screen and gets cut off."
      - Not reproduced yet (Claude 019khs4X, 2026-11-14). Linux build, Brennan's settings from his cfg but
        `widescreen=0` (zoom=5, resolution=3 / 1440p, lockAspect=1, integerScale=1, border=1), 960x640 test window:
        the new main menu (Continue / New Game / Option) and the new text box both show whole, nothing cut off
        (picture: `docs/polish_reports/POL-6_not_reproduced_widescreen_off.png` = the menu; the text box was checked in
        an earlier run, not kept). zoom 3 vs 5 makes no difference there.
      - Not checked: his real window size/monitor (windowScale=8, fullscreen), the Classic text box, the Windows exe.
      - Need from Brennan: a screenshot of the cut-off box, and whether he was in fullscreen.
- [x] **POL-7. Grass, flowers, flowerbeds, trees still flat.** "The grass in 3D is not 3D it's still 2D alongside
      flowers, flowerbeds, trees."
      - 2026-11-16 (014PwRDj), Brennan's answers: flat ones = "Flowers and flowerbeds", "Short grass too"; look =
        "Same as tall grass" (a card standing, leant back as the long grass's tuft); prototype picture
        `docs/polish_reports/POL-7_flowers_and_grass_prototype.png` -> "Yes, keep both".
      - **Done (General tileset):** flowers (0x004) stand as their own drawing less the grass, 0.75 tile long at
        50 degrees; every plain grass tile (0x001) a low tuft (0.3 tile) of blade tips. Art: two cards added to
        `pc_data/voxel/trees.rgba5551` (free bottom row) by `tools/voxel3ds/add_tree_cards.py`; renderer:
        `voxel_tree.c` EmitGroundCard (not on building cells or sloping relief). Emerald 3DS has neither.
      - Brennan: "The flower patches along with the planter boxes were 2D ... The planter boxes inside mauville and
        petalburg. Also whatever planter boxes you can find inside cities." Done: the General planter boxes (0x007:
        Petalburg's garden, the Battle Frontier) stand as two cards a tile (trees texture 32,112). Mauville's map
        has only the 0x004 flower patches (done). Brennan 2026-11-16 07:41: "We can close POL-7 and move onto POL-8".
        Pictures: `POL-7_flowers_and_grass_prototype.png`, `POL-7_petalburg_planters.png`.
- [ ] **POL-8. Black lines on indoor floors.** "There are black lines on the flooring inside of buildings."
      - 2026-11-16 (014PwRDj): not seen here (Petalburg Center, Oldale Mart, Brendan's house, Devon Corp, 1280x720;
        Brennan's 4K scene setting is too slow to run here). Brennan: "Try a fix anyway". Likely cause fixed:
        `voxel_atlas.c` VoxelAtlas_SlotUV insets a slot's west/east edges a 1/32 texel too (only north/south were),
        so a tile edge on a pixel centre can't take a column of the next slot (black void tile indoors).
        **Waiting for Brennan to check it at 4K.**
- [x] **POL-9. Game Corner.** "You can walk through the left and right columns of the game corner building,
      inside of the game corner the gatcha machines need to be fixed as they are in parts or not fully 3D and the
      game corner is very skippy/laggy."
      - 2026-11-16 (014PwRDj): prize ("gatcha") machines: three were flat cards in Emerald 3DS's spec (named
        "plant_0..2") and the purple one was left painted on the wall. Now one 3D box like the arcade cabinets
        (tools/voxel3ds/game_corner_spec.py, layout 91 respliced). Picture: docs/polish_reports/POL-9_prize_machines_3d.png.
        Still open: "walk through the left and right columns" (not yet clear which columns) and the lag.
- [x] **POL-10. Walking through the top of doors.** "When walking in entrances you walk through the top of the
      door this should be fluid and not be cutting off the top when walking through the door."
- [x] **POL-12. Water is still.** "Water is not moving in 3D mode it is still." Done 2026-11-14 (Claude 019khs4X);
      picture for Brennan: `docs/polish_reports/POL-12_water_moves.png` (Route 103, two frames 16 apart).
      - Cause: our 3D renderer never redrew the animated tiles; the 3DS code for it (`VoxelAtlas_RefreshAnimatedPageRows`)
        was already ported in `voxel_atlas.c` but nothing called it.
      - Fix (`voxel_gl.c`, `AnimateAtlasTiles`, after the maps' meshes each frame, not in battle): compares BG VRAM's
        two tile sets (0x8000 bytes) with last frame's copy; once a frame passes with no write (the game writes
        its animation frames on frames 0-4 of every 16 - 3DS's VOXEL_ANIM_SETTLE), every slot made from a changed
        tile is composed again (3DS rule, ctr_voxel.c AnimateTiles/AnimBegin) in the current map's atlas and in
        the atlases of maps beside it with the same primary tile set (primary tiles only - 3DS AnimShares), then
        the page is graded and uploaded again. Each atlas now keeps its pages as composed (`Atlas.raw`, 256 KB a
        page) to compose into. Flowers and sea edges move too.
      - Tested in the Windows exe (Wine, 3D on, Route 103): the sea differs between frames 16 apart; with
        `WJPC_3D_NO_TILE_ANIM=1` (new switch: turns it off) the frames are identical. `WJPC_VOXEL_LOG=1` logs
        the first redraws. Test scripts need `WJPC_TEST_WINDOW=1`, or the 3D view is only drawn on screenshots.
      - Not measured: cost per redraw on a real GPU (a whole page is untiled and uploaded per change, every 16
        frames per atlas in view).
      - 2D checked 2026-11-14 (`world3d=0`, Windows exe, Wine): Route 103's water moves. Same pictures, pixel for pixel, as an exe
        built with this session's 3D files put back (only a randomly walking person differs): the fix changes
        nothing in 2D. Picture: `docs/polish_reports/2D_check_POL-12_16_23_27.png`.
- [ ] **POL-13. Background mountains are blocks.** "Mountains in background are blocks and not like mountains."
- [x] **POL-15. Route 110 into Mauville.** "Walking into mauville city from route 110 everything below disappears
      and reappears."
      - **Brennan 2026-11-15 15:52, PRIORITY:** "going from route 110 to mauville city the whole bottom of the
        screen goes blue and then the whole game starts becoming laggy and not smooth. Make this a priority fix
        and when fixed send me the newest zip file."
      - 014PwRDj, 2026-11-15: not reproduced yet. Test-script loop (frame by frame) Route 110 (14,8) -> Mauville,
        Brennan's save, cfg with Gamma: nothing disappears, no rebuild or atlas churn in the log after the
        crossing (the only map/belt work after it: the belt pieces again over ~8 frames of 3-11 ms, then the
        water/flower animation's atlas redraw every 16 frames, 5-11 ms here, also before the crossing). Route 110,
        Mauville, 111, 117, 118 and Verdanturf all share one atlas (General + Mauville tile sets); 7 tile set
        pairs loaded, 12 atlas slots, 12 maps: no slot thrash. The real-time loop (144 fps) cannot be tested
        here: software GL draws ~5 frames a second. So the build now writes `WallysJourney_stalls.txt` (stalls
        over 150 ms, LAGGY seconds, what the 3D work was) - asked Brennan to send it after it happens.
      - **Brennan 17:06: stall log + phone video; "It still does it with the old backgrounds as well."** Video: at the
        crossing ALL of Route 110 (south half of the screen) is gone for ~1.3 s while Mauville is drawn. Log: seconds
        of ~11 trips/s (~100 ms each) with almost no 3D map/belt/drawing time - the split of the rest was not logged
        (now it is: game/frame/screen/present in LAGGY lines).
      - **Cause (code):** FindAtlas threw an atlas away whenever a map that came into reach used a metatile it
        lacked. Route 110, Mauville, 111, 117, 118 and Verdanturf share one atlas (General + Mauville), so all of
        them lost their meshes until laid again one at a time (only the map underfoot at once). The test run that
        visits 12 towns first logs 15 such throw-aways in one tour.
      - **Fix (2026-11-15, ported idea from Emerald 3DS `extendPending`):** the atlas is grown (same slots, pages
        recomposed into the same textures, `VoxelAtlas_JobBegin` with `base`); meshes on it stay drawn
        (`Atlas.layoutGen`) and are laid again behind it. Picture `POL-15_route110_to_mauville_after_fix.png`
        (walking north from Route 110 into Mauville, Route 110 stays). The lag part is NOT proven fixed: waiting
        for Brennan's next stall log.
- [x] **POL-16. People turn black at doors.** "When walking into doors all npcs turn black and disappear then
      reappear whenever you come back out."
      - Done 2026-11-14 (Claude 019khs4X). Picture for Brennan: `docs/polish_reports/POL-16_door_fade.png`.
      - Reproduced in the exe (Wine, 3D, Oldale Town = warp `0 10 6 18`, walk up into the Pokémon Center): the
        screen fade at a door/warp darkened only the people (their colours come from the game's faded palettes);
        the 3D world never faded, so they turned into black figures on a lit town, then the picture cut to the
        room, where the people came back from black on a fully lit floor.
      - Cause: `voxel_world.c` `VoxelWorld_ScreenFade` only reported battle fades on PC (the comment said field
        fades were "drawn over the 3D view instead"; nothing did that). Fix: the field branch Emerald 3DS has
        (palette fade active / weather screen fade) now runs on PC too, so the 3D world fades with the screen.
      - `voxel_gl.c`: as on the 3DS (`PrepareFades`: sWorldFade for the world, no extra fade for sprites), the
        people's pass gets no world fade (their palettes already carry it). With Gamma lighting the fade is done
        in the screen pass (after the tone curve, as battles already did); the people's pass first takes their
        palette fade back off (`unfade` uniform), so they are faded once, the same as the world. Done in the
        world shader instead, the tone curve kept a lit room bright for half the fade and the people's haze
        stayed blue at full black - tried and dropped.
      - Tested Gamma lighting on and off: fade out, full black, fade in all even. Not tested: battle fades
        (that path is unchanged), fades to white, real GPUs.
      - 2D checked 2026-11-14 (`world3d=0`, Windows exe, Wine): going into Oldale's Pokémon Center fades the town and the people out together, and the room in. Same pictures, pixel for pixel, as an exe
        built with this session's 3D files put back (only a randomly walking person differs): the fix changes
        nothing in 2D. Picture: `docs/polish_reports/2D_check_POL-12_16_23_27.png`.
- [x] **POL-17. Seed mounds.** "Seed mounds look like shrubs and are above the dirt."
- [x] **POL-18. Pokémon are flat.** "Pokemon in 3D are still 2D."
- [x] **POL-19. Falling leaves.** "Leaves appear only in certain areas and randomly disappear or speed up."
- [x] **POL-20. Boats are flat.** "Boats in the water are 2D not 3D."
- [x] **POL-21. Route 110 to 103 backdrop.** "Going from route 110 to 103 the background is not fluid and changes
      from water to trees and skips into frame."
- [x] **POL-22. Breakable rocks.** "Rocks that are breakable disappear and reappear and also 2D not 3D."
- [x] **POL-23. No sandstorm.** "There's no sandstorm in the desert."
      - Found (Claude 019khs4X, 2026-11-14): 2D shows the sandstorm on Route 111's desert (same save, same spot);
        3D shows plain sand. Cause: `voxel_world.c` `VoxelWorld_Weather()` / `VoxelWorld_WeatherAmount()` only
        work under `#ifdef PLATFORM_3DS`, so on PC the 3D world always gets "clear" weather, and the 3D view has no
        port of Emerald 3DS's `DrawWeather` (`ctr_voxel.c` lines 8005-8137: rain, sand and ash drawn in the world,
        `CtrVoxel_DrawsWeather` / `CtrVoxel_DrawsSand`). Same cause for rain and ash in 3D (likely POL-27's ash puff
        too). Fix = port those into `voxel_world.c` + `voxel_gl.c` (claimed by 014PwRDj) - waiting like POL-12.
      - Done 2026-11-14 (Claude 019khs4X; Treston OK'd taking voxel_gl.c over). Picture for Brennan:
        `docs/polish_reports/POL-23_3d_weather.png` (rain, volcanic ash, sandstorm in 3D, Windows exe).
        `voxel_world.c`: the three weather functions now also run on PC (`#if PLATFORM_3DS || PORTABLE`).
        `voxel_gl.c`: Emerald 3DS `DrawWeather` ported (same streak/flake kinds and numbers: rain, sand, ash;
        768 streaks, 512 flakes, camera-relative box, fog and screen-fade aware), drawn after the world, outdoors,
        not in battle. Off switch for testing: `WJPC_3D_NO_WEATHER=1`. Test: `WJPC_FORCE_WEATHER=3/7/8`.
        Not ported: 3DS's sand "dust sheets" (`DrawBanks`) - the streaks are there, the haze layer is not.
        Not tested: weather during battle transitions; real GPUs (only Wine/llvmpipe here) - Brennan please check.
        Did not fix POL-27 by itself; that was a separate cause (see POL-27).
      - 2D checked 2026-11-14 (`world3d=0`, Windows exe, Wine): rain, ash and Route 111's sandstorm show as before (2D rain looked sparse with the forced test weather after ~3 s - the same in the older exe, not looked into). Same pictures, pixel for pixel, as an exe
        built with this session's 3D files put back (only a randomly walking person differs): the fix changes
        nothing in 2D. Picture: `docs/polish_reports/2D_check_POL-12_16_23_27.png`.
- [ ] **POL-24. Route 112 into Lavaridge.** "Route 112 into lavaridge town rocks in background appear and then level."
- [x] **POL-26. Rustboro lights blink.** "Lights in rustboro city blink in and out while walking."
      - Fixed 2026-11-15 (Claude 019khs4X): `src/event_object_movement.c` UpdateLightSprite removed a lamp glow as
        soon as it was outside the GBA's 15x20-tile window, but TrySpawnLightSprites makes them in the padded PC
        area (widescreen / 3D, WJ_SPAWN_PAD_X/Y). So lamps near the view's edges were made and removed again
        every step, and in 3D lamps still in view went out. UpdateLightSprite now uses the same padded area.
        Tested at night (WJPC_RTC_EPOCH, 10 PM), Rustboro, walking left, 3D and 2D: 3D lamp glows now stay on
        (yellow-pixel count steady, ~5,300; the older exe fell from ~1,650 to ~0 half way); 2D shows no change
        in what it shows. Picture: `docs/polish_reports/POL-26_lamps_no_blink.png`.
      - 3D look of the lamps: done in POL-26b (lamps step).
      - Looked at 2026-11-14 (Claude 019khs4X), Windows exe (Wine), Rustboro (warp `0 3 16 40`), walking left then
        up, a picture every 4 frames, 3D and 2D, at the test machine's night (01:xx): no blinking seen in either.
      - Found instead: **the 3D view has no night at all.** At the same moment 2D shows night (lamps and windows
        lit) and 3D shows full daylight with the lamps dark - Gamma lighting on or off. Emerald 3DS does have it
        in 3D (`ctr_voxel.c`: `UpdateNightLamps`, window glow from `buildings.bin`, moonlight, CtrDayNight);
        our port never brought that over, and Gamma's own files are fixed at day (GAMMA_LIGHTING_VALUES.md).
        The owner's wish list asks for night street lights and lit windows (OWNER_REQUIREMENTS "Atmospheric
        day/night"). Which lights Brennan saw blink is unknown - asked (below).
      - Need from Brennan: 2D or 3D, time of day, which lights (street lamps, windows, the Pokémon Center/Mart
        signs), a screenshot or video.
      - **Treston, 2026-11-14: "Yes, port 3DS night"** - the 3D view follows the game clock and gets Emerald 3DS's
        night (night tint, lit windows, street lamps, moonlight), ported as the 3DS does it; pictures first.
        Claude 019khs4X working on it (task POL-26b below).
      - Test aid for later: `WJPC_RTC_EPOCH=<unix time>` fixes the game clock (sdl2.c UpdateInternalClock).
- [x] **POL-26b. 3D night (Treston's decision 2026-11-14, from POL-26).** Port Emerald 3DS's day/night to the 3D
      view: `3ds_port/src/3ds_daynight.c` + `ctr_voxel.c` (UpdateNightLamps, window glow from buildings.bin,
      moonlight, glow pass). Claude 019khs4X, in work. Test 3D and 2D (2D must stay as it is).
      - **Treston, 2026-11-14 (night colours): "Match our 2D"** - the 3D world takes its time-of-day colour from the
        game's own 2D tint (what the palettes show vs what is stored, as wide_view.c FitPaletteShift does), so
        switching 2D/3D never changes the look; lit windows, street lamps and moonlight as Emerald 3DS does them.
      - Steps (picture after each): 1 world tint from the 2D tint; 2 the 3DS daylight factor (sun rays, clouds,
        dapples fade at night) + moonlight; 3 lit windows (buildings.bin glow) and night lamps (VoxelSign_EmitNight).
      - Step 1 done 2026-11-14: `voxel_world.c` `VoxelWorld_TimeTint` works out the game's time blend (gTimeBlend:
        tint = multiply by tint/256, blend = move towards its colour by coeff*2/32, start blend weight/256) as one
        line per colour channel; `voxel_gl.c` puts it on the world's texels (`tintMul`/`tintAdd`, not the people:
        their palettes carry it), the sky colour, the haze. Indoors / no natural light: none. Off switch for tests:
        `WJPC_3D_NO_TIME_TINT=1`. Picture: `docs/polish_reports/POL-26b_step1_time_of_day.png` (Rustboro 1 PM,
        6:30 PM, 10 PM; 2D left, 3D right; `WJPC_RTC_EPOCH`). 1 PM 3D mean colour 93,114,124 vs 2D 96,110,113
        (untinted 3D was 121,136,129: November = winter daylight tint). 2D at 10 PM identical, pixel for pixel,
        to the exe before this change.
      - Seen, for the next steps: night windows not lit in 3D yet (step 3); the game's lamp-glow sprites show in
        3D as flat yellow discs (they are the 2D game's light sprites drawn as cards - `voxel_entities.c`,
        claimed by 014PwRDj; Emerald 3DS draws its own lamp pools instead, UpdateNightLamps) - maybe POL-26's
        "blinking" (they appear/disappear as the 2D view's light sprites come and go). Not changed yet.
      - Step 3 (windows) done 2026-11-15: building pages' window-glass masks (buildings.bin L8 glow, already read
        by `voxel_building.c`) uploaded as a second texture (`voxel_gl.c` GlowPage, unit 1) and lit as Emerald 3DS
        does (BuildingGlowEnv: after the light, towards 255,216,160 by mask x strength; Gamma path: that colour in
        linear light, c*c/exposure). Lit at the 2D game's own times (`WallyJourney_WindowsLitNow`, overworld.c:
        outdoors, 20:00-04:59), strength 1. Off switch: `WJPC_3D_NO_WINDOWS=1`. Picture:
        `docs/polish_reports/POL-26b_step3_lit_windows.png`. Tested 10 PM and 1 PM, 3D and 2D: 2D both times and
        3D at 1 PM pixel-identical to before; 3D 10 PM windows lit. Difference from 2D: the 3DS masks also light
        the Pokémon Center / Mart door glass (kept: it is the 3DS's data).
      - Step 2 not done yet: Gamma's sunlight direction and cloud shadows still run at night in 3D. The 2D game
        also keeps its cloud shadows at night (see the picture).
        **Treston, 2026-11-15: "Do it like the 3DS"** - sun effects fade at night and soft moonlight comes in, as
        Emerald 3DS does it, even though 2D keeps its cloud shadows.
      - **Step 2 undone (Treston, 2026-11-15, after seeing it: "It looked better before with the clouds and gamma
        lighting at night")**: 3D keeps Gamma's sunlight and cloud shadows at night (like 2D keeps its clouds).
        The step-2 code (commit e0629d0b5b: VoxelWorld_NightLight, the 3DS daylight / sun / moon curves) was
        reverted; its picture stays as a record: `docs/polish_reports/POL-26b_step2_night_sun_moon.png` (left =
        what he chose). Do not bring it back without asking him.
      - Lamps: the 2D game's lamp-glow sprites show in 3D as flat yellow discs that pop in and out.
        **Treston, 2026-11-15: "Yes, take it over"** - Claude 019khs4X takes `voxel_entities.c` over from 014PwRDj
        for the lamps and draws them the way Emerald 3DS does (UpdateNightLamps).
      - Lamps done 2026-11-15: the game's night-light sprites (UpdateLightSprite: lamp balls, PC / Mart sign lights)
        no longer go into the solid card mesh. `voxel_entities.c` puts their cards in a builder of their own
        (VoxelEntities_SetLampBuilders), and `voxel_gl.c` draws them as the GBA blends them (OBJ blend
        Weather_SetBlendCoeffs(7, BASE_SHADOW_INTENSITY = 12): scene x 12/16 + glow x 7/16; shader mode 3).
        Every lamp ball also gets Emerald 3DS's pool of light (voxel_sign.c LampGround, now public as
        VoxelSign_LampPool; 3DS MakeLampPool texture (1-r^2)^2, LampTexEnv: ground x (1 + falloff x 0.5 x
        255,150,60), shader mode 4), centred on the post: the light object is on the lamp's head tile, the post
        one tile south (3DS: centred on the post). The 3DS's lamp "glass" is not used: it comes from its sign data
        (signs.bin lanterns), not from our maps' light objects. Off switch: `WJPC_3D_FLAT_LAMPS=1` (old look).
        Picture: `docs/polish_reports/POL-26b_lamps.png`. Tested Rustboro 10 PM, 3D (Gamma on and off) and 2D
        (unchanged; only randomly walking people differ). The pools are faint on the night-blue ground (the 3DS
        formula adds mostly red) - ask Treston if they should be stronger.
      - Treston, 2026-11-15: "The lights look like it's behind the lamp in 3d". Placed from the screen, the glow's
        card stood ~half a tile north of the lamp and up-left of its head, so the 3D lantern hid it. Now centred
        where Emerald 3DS lights the lantern's glass (voxel_sign.c VoxelSign_LanternGlass: head rows 6..10, the
        lantern's front face, as VoxelSign_EmitNight) and stood just in front of it. Lamps with no 3DS lantern data
        keep the screen placement, moved a tile towards the camera along the line of sight. Not handled: maps
        with eased rows / raised bases (z stretch) - Rustboro is flat. Picture updated: POL-26b_lamps.png.
      - Treston, 2026-11-15: "Still doesn't look good, maybe it's too low, and also idk like look at how the 2D
        looks". Three differences from 2D fixed: (1) the glow's centre is now the 2D game's (the head cell's middle,
        6 px under its top: SpawnLightSprite x + 8, y + 22 - 16), not the glass; (2) the lantern's glass is lit as
        Emerald 3DS lights it (VoxelSign_EmitNight glass, LampTexEnv 255,191,107 at alpha 220/255; mode 5), as the 2D
        lamp's glass turns yellow at night; (3) with Gamma lighting the glows are drawn over the finished picture
        (after PostFinish, with the scene's depth sHdrDepth on the output), as the GBA blends over its finished
        picture - added to the linear light before the tone curve they washed the lamp head out. Picture:
        POL-26b_lamps.png (2D vs 3D, close-ups Gamma on/off).
- [x] **POL-27. Route 113 ash.** "Route 113 ash on the floor after walking through it shows blue boxes and does
      not show crash or anything." (Brennan, confirmed: he meant the ash puff when you step in it.)
      - Done 2026-11-14 (Claude 019khs4X). Picture for Brennan: `docs/polish_reports/POL-27_ash_trail_fixed.png`.
      - Reproduced in the exe (Wine, 3D, Route 113 = warp `0 28 38 4`, walk right): every ash cell walked over
        turned into a light-blue box. Cause: stepping in ash changes the cell from ash grass (metatile 0x20A) to
        normal grass (0x212), which the 3D texture sheet (atlas) did not hold, so the cell drew nothing and the sky
        showed through. `voxel_gl.c` `AtlasCovers` (which has the atlas made again when a map uses a metatile it
        lacks) remembered its answer per map and atlas build only, so it never looked again after the change.
      - Fix: for the map underfoot it now also remembers the state of the live grid (`VoxelWorld_LiveDigest`), so
        any cell a field effect or script changes is checked; a new metatile has the atlas made again once (log:
        `atlas lacks metatile 212 of 0:28: made again`, `WJPC_VOXEL_LOG=1`). Same fix covers other mid-play tile
        changes in 3D (e.g. Lavaridge's ash grass, script `setmetatile`s), not tested one by one.
      - Cost: one atlas rebuild the first time a new metatile appears (a short hitch, like entering a map); the
        map's cells are re-scanned only on frames the grid changed. Not measured on a real GPU.
      - The ash puff sprite itself and the ash weather (POL-23) were already fine; the boxes were the ground.
      - 2D checked 2026-11-14 (`world3d=0`, Windows exe, Wine): walking through Route 113's ash leaves the plain grass, as the game always did. Same pictures, pixel for pixel, as an exe
        built with this session's 3D files put back (only a randomly walking person differs): the fix changes
        nothing in 2D. Picture: `docs/polish_reports/2D_check_POL-12_16_23_27.png`.
- [ ] **POL-28. Petalburg Woods black lines.** "Bunch of black lines inside of petalburg woods."
- [x] **POL-29. Separate music and gameplay volume.** "Add a feature in the volume setting to differentiate music
      volume and gameplay volume incase you want to turn off music but keep gameplay audio." Done 2026-11-14
      (Claude 019khs4X); picture for Brennan: `docs/polish_reports/POL-29_music_sound_volume.png`.
      - Settings > Audio now has **Volume** (main, as before), **Music volume** and **Sound volume** (0-10 each,
        saved as `musicVolume=` / `soundVolume=` in `WallysJourney.cfg`, default 10). Music = whatever a music
        player is playing as a MUS_ song (background music, fanfares, MP3 replacements); sound = sound effects and
        Pokémon cries. Music/sound volume x main Volume.
      - Code: `include/platform.h` (2 settings), `src/platform/sdl2.c` (defaults, cfg), `src/option_menu.c` (2 rows),
        `src/platform/mp3_glue.c` (`Audio_ChannelGain`, per-player music check, GB channel gains),
        `src/platform/sound_mixer.c` (gain per DirectSound channel), `src/platform/cgb_audio.c` (`gCgbGain`).
      - Tested (Linux build, audio dump): music 0 / sound 10 -> silent except the menu sounds; music 10 / sound 0
        -> the town music with no menu sounds; both 10 -> same as before. The rows change and save (`musicVolume=7`).
      - Not tested in the Windows exe yet (same code).

      - 2D/3D: the volume code has no 2D or 3D path (it mixes the sound itself), so it is the same in both.
- [ ] **POL-30. Backdrop changes between routes, everywhere** (Brennan, 2026-11-14, later the same day): "Also fix
      the background in every route as it is changing from one background to the other when going to and from
      routes." The general case of POL-15, POL-21 and POL-24 (and DLX-25, which fixed Dewford's): check every
      Hoenn map link, not only the ones he named.
    - **2026-11-15, 014PwRDj - fix in 3D, pushed to `polish`, waiting for Brennan's look (not ticked yet).**
      Cause: everything past the maps' edges (the "belt") was decided by the map the player stands on - its
      border art, its edge heights, its edge fills - so at every crossing the belt beside the next map was
      redrawn as the new map's. Now each belt cell goes with the **nearest loaded map**, whichever map the
      player is on:
      - art: `VoxelWorld_BorderMetatile` / `NearestBeltMap` (DLX-25's rule, widened to maps on another secondary
        tile set whose border is the shared primary set's; ties to the lower map number);
      - height and base: `VoxelWorld_BeltRing` in `BorderWaterLevel`, `VoxelMesh_BorderLevel`, seam skirts,
        tree belt; `VoxelMesh_EmitBorder` sets the builder's base per cell;
      - edge fills (`EdgeFillAt`) the same; atlas marks them and is made again if it lacks them.
      - **My own design choice, NOT from Emerald 3DS (which has the same swap, hidden by its narrow view) -
        Brennan/Treston please confirm:** between two maps whose edges stand at different heights the belt
        height is blended (every map's nearest edge pulls, 1/distance^3) and step faces are tiled a tile a level.
        Without the blend a tall wall stood where the two maps' halves met (behind Lavaridge).
        `WJPC_BELT_NO_BLEND=1` turns only the blend off; `WJPC_BELT_OLD=1` gives the whole old belt back.
      - Check of all 64 Hoenn links (`docs/polish_reports/POL-30_belt_sweep.py`, belt cells within 24 tiles of
        each link, seen from both sides; 34029 cells): **old 8318 differ in art, 9661 in height; new 641 in art,
        70 in height.** What is left: Route 113 <-> Route 112 / Route 111 art (Route 113's ash-tree border is on
        Fallarbor's tile set, which Route 112/111's atlas cannot draw - would need a second atlas on the belt);
        a few cells of height near Route 118/123 and Verdanturf (far maps coming and going). Two links gave no
        table (Route 119/118, Route 111/Mauville) - **not checked by the sweep.**
      - **Brennan 2026-11-15 13:15:** (1) the blended slope "is okay for now"; (2) "fix route 113"; (3) "fix the
        small blue sawtooth edges in the fallarbor area / routes"; then go on with Polish, keep sending
        pictures/video.
      - (3) done 2026-11-15: the saw teeth were sky between Fallarbor's filled edge (stands the belt's 2 courses
        up) and Route 113's tree belt (trees stand on the level, no ground laid). A standing belt cell now
        shows its faces beside a tree cell, a flat cell steps down to a lower one that is not flat
        (`BeltFlatSteps`), side faces lean to the lower neighbour's row, and the first tree cells get ground.
        Picture `docs/polish_reports/POL-30_fallarbor_sawteeth_fixed.png`. Only looked at in Fallarbor so far.
      - (2) Route 113, 2026-11-15: **belt layers** (`BELT_LAYERS` in `voxel_gl.c`). The belt is emitted once per
        atlas: layer 0 the current map's, layers 1-3 the ready atlases of loaded maps on other tile sets
        (`VoxelWorld_SetBeltPairs`, `VoxelWorld_BorderArtMap`, `VoxelMesh_SetBeltPass`). A new set of layers
        rebuilds the belt a few pieces a frame (no whole-belt build). Sweep after this: **0 of 34029 cells differ
        in art, 70 in height** (Route 118/123 30, Verdanturf/116 21, 117/Verdanturf 14, Lavaridge/112 5).
        Picture `POL-30_route111_sees_route113_old_vs_new.png`.
        Not checked by the sweep: Route 119/118 and Route 111/Mauville (the test `warp` cannot place the player
        past y=127 on those tall maps).
      - Pictures: `docs/polish_reports/POL-21_route110_to_103.png`, `POL-24_route112_to_lavaridge.png`
        (top old, bottom new; left before the crossing, right after). 2D does not use this code.


## Brennan's clarifications and new items, 2026-11-15 18:05 (his words; numbered as he sent them)
- **POL-31 (new). New Mauville in 3D:** "the glare whenever having gamma lighting on is way to bright needs to be tuned
  down a bit, also the models inside are still 2D whenever in 3D."
- **POL-32 (new). PC mass select:** "When trying to mass select pokemon within PC it turns the screen a solid color and
  sorta glitches out, also mass select only works on when you have sword and shield boxes on via settings."
  - **Fixed 2026-11-15 (014PwRDj), in 2D renderer code:** reproduced with Brennan's save (test aid `state storage 0`
    opens the PC box menu; `state classicgfx 5 1` switches to Classic boxes): starting a multi-select turned the
    whole screen black in BOTH Sword/Shield and Classic boxes. Cause: `src/platform/gba_easy_draw.c` drew colour 0
    of 256-colour backgrounds as solid (on the GBA it is see-through, and wide_view.c already skipped it); the
    multi-select draws the picked group on such a layer covering the screen. Fix: colour 0 skipped. Side effect,
    also correct: the boot intro's "POWERED BY" line now shows (it was hidden by the same bug). 2D field and 3D
    untouched (other renderer). Picture: `docs/polish_reports/POL-32_pc_multiselect_fixed.png` (Classic left,
    Sword/Shield right, a group picked up). Mass RELEASE (Start while holding a group) exists only in the
    Sword/Shield boxes (WJ 0.18.9) - asked Brennan whether he wants it in Classic too.
  - **Classic mass release added 2026-11-15 (014PwRDj)**, Brennan: "Yes make it the same way in the classic boxes as
    well." `src/pokemon_storage_system.c` now has the same Start-while-holding-a-group release as the Sword/Shield
    boxes (same rules: Eggs, the last Pokémon knowing a protected move, and a Pokémon whose item won't fit in the
    bag stay; held items go to the bag). Messages shortened to fit the narrower Classic message box. Checked with
    Brennan's save (test aid `state boxmon BOX*30+SLOT`): a 3x2 group of 6, 5 released, his Swampert stayed (it is
    the last one knowing a protected move). Picture: `docs/polish_reports/POL-32_classic_mass_release.png`.
- **POL-6 (clarified):** "The sword and shield start menu is too big and goes off the screen when widescreen is turned
  off for some reason."
  - Brennan 20:47 (with his photo): "The sword and shield menu gets cut off on left side. It needs to flow from left to
    right. On the bottom of the screen. As you can see in the photo that it starts about halfway then goes to the
    right." **Fixed 2026-11-15 (014PwRDj):** on a screen wider than the GBA's, the menu sits against the right edge
    and its bottom bar started where the GBA screen starts. `src/platform/sdl2.c` (DrawFieldScene) now carries the
    bar on to the left edge (its first column, GBA rows 124-159: edge lines and green, no text) while the
    Sword/Shield start menu is open (`Vol_StartMenu_IsSwshOpen`, `src/vol_start_menu.c`). Classic menu untouched.
    Picture: `docs/polish_reports/POL-6_start_menu_bar_full_width.png` (1280x720 window, Brennan's save).
  - Brennan 20:58: "the right side top it also runs from top to bottom". Done: when the window is taller than the
    menu, the GBA screen's first row (the right column's border and green, no icons) carries on up to the top edge.
    Same picture, updated.
- **POL-35 (new, priority after POL-6). Fishing bites:** Brennan 2026-11-15 20:12: "when fishing in the water for
  pokemon, make it only one "Oh! A bite!" before you can get into battle with it. currently it makes you wait 3 times
  in order to get into a battle with the pokemon."
  - **Fixed 2026-11-15 (014PwRDj):** Gen 3 rule made each rod need several rounds of dots + "Oh! A bite!" (Good Rod
    1-3, Super Rod 1-6, plus a chance of more). `src/fishing.c` Fishing_CheckMoreDots now goes straight to "A
    Pokémon's on the hook!" after the first bite, every rod. Test aids: `state fish ROD`, `state rng SEED`,
    WJPC_TEST_AUTO_REEL=1. Picture: `docs/polish_reports/POL-35_super_rod_one_bite.png` (Super Rod, Petalburg pond:
    dots, one bite, on the hook; the battle followed).
- **POL-36 (new, PRIORITY). Route 119 / Weather Institute (3D):** Brennan 2026-11-15 21:14 (6 photos, his save
  `WallysJourney.sav` of 21:14 kept for future checks, stalls + errors files): "On route 119 west of fortree city the
  whole ways from north to south where the waterfall is located needs to be looked at and fixed. It is shooting up in
  the sky and the bridges are in pieces. The weather institute is flat and in pieces alongside the terrain around it
  as well as the background. If you go south of the weather institute the terrain also shoots up in the sky. There
  are also black lines around the tall grass in this area. It seems elevation is the problem in this area."
  - **Elevation fixed 2026-11-15 (014PwRDj), data only:** reproduced with his save (Weather Institute door, the
    waterfall). Cause: `pc_data/voxel/relief.bin` (generated against our tree on 2026-11-14) differs from the one
    Emerald 3DS's builder makes from vanilla Emerald in 5 layouts - Lilycove, Route 106, 119, 120, 121 - and its
    Route 119 is the broken one (with the vanilla-built file Route 119 draws right). Route 106's map is changed in
    WJ, the other four are byte-for-byte vanilla, so their records were spliced in from the vanilla-built file
    (`tools/voxel3ds/splice_relief.py`, notes in `tools/voxel3ds/README.md`). Checked in 3D: Weather Institute,
    waterfall + bridges, south of the Institute, Route 120, Lilycove. Picture:
    `docs/polish_reports/POL-36_route119_before_after.png` (top before, bottom after).
  - Brennan 22:23 (photo): "you didn't make the weather institute 3D it is 2D flat against the ground."
    **Fixed 2026-11-15 (014PwRDj), data:** Emerald 3DS has no model for it. New model (spec
    `weather_institute_exterior`: a rounded body 51 px tall, the pillared front 62 px, two domes; every face reads
    the drawing), put into `pc_data/voxel/buildings.bin` with `tools/voxel3ds/splice_buildings.py` (notes in
    `tools/voxel3ds/README.md`). Picture: `docs/polish_reports/POL-36_weather_institute_3d.png`.
  - Black lines around the tall grass: faint dark specks seen at grass tile edges in my shots too - separate
    from the elevation, still to do.
- **POL-33 (new). Pokédex name search:** "When searching in pokedex to find pokemon under name using the alphabetical
  search, it shows pokemon you havent found yet, allowing players to find locations of all pokemon without having
  encountered them before which was not intended, you're suppose to encounter a pokemon before they show in your dex
  even when searching using the alphabetical search in filters"
- **POL-34 (new; same as Treston's PC-1115-01). Houses lost their colour (3D):** "All the houses now lost their color
  for some reason and jus appear to be pale white for the most part, for some reason most of their color was removed"
  - Brennan 19:40: Mauville, winter at night; reference = the GBA Mauville picture (houses with tan/gold posts,
    the Gym's orange roof). **Fixed 2026-11-15 (014PwRDj), 3D only:** my own DLX-17 bug - the season recolour of
    the building-model pages ran AFTER the colour grading (inside `VoxelBuildings_ReadPage`), so the season
    formula saw brightened colours; in winter its snow cover (light earth tones -> white, owner choice 2026-10-29:
    snow on every outdoor map) caught most wall/post/roof colours. Now season first, then grading
    (`voxel_gl.c` LoadBuildingPage), as for the trees and as 2D does on the tile sets' own colours. Picture
    `docs/polish_reports/POL-34_mauville_houses_before_after.png` (left before, right after; winter night).
    Treston's PC-1115-01 ("the houses lost their color") is likely the same; tell him.
- **POL-33: Brennan 19:40 "I was wrong disregard the Pokédex search."** Closed, no change.
- **POL-30 (reopened, quality pass):** "Since we introduced the zoom feature the out of boarder areas that the player
  could not see before were "filled" with a bunch of trees added and then they would change when switching routes ...
  but now like in Dewford for example you jus put a bunch of trees in the middle of the ocean to "fill" the out of
  bounds area we couldn't see before, we need a quality pass making sure we are filling the out of bound areas with
  terrain that makes sense for the area and to make sure we don't reintroduce the popping in when switching routes."
- **POL-15 lag, PRIORITY:** "my game is lagging but my buddy game is not lagging at all he has a intel cpu and I have
  a amd cpu and we both have nvidia gpu's, could that have anything to do with it?" His cfg at 17:48:
  `resolution=4` (3D drawn at 3840x2160), `frameRate=240`, `zoom=7`, `gammaLight=1`, `interframe=0`, `bilinear=0`.
  Stall log (build 17:08): whole seconds of 11-20 loop trips (~50-120 ms each); of a ~100 ms trip, the 3D work
  (maps/belt/drawing calls) is 5-20 ms and the rest is in the frame step outside it - not yet split further (the
  2D-picture time is now logged too). Here, his cfg at Route 111: the 2D picture costs ~6 ms a frame.
  - 18:28: his friend's cfg has the same graphics settings (only controls, fast-forward, music/sound volume 0
    differ). GPUs: Brennan RTX 3080 + AMD CPU, 144 Hz; friend RTX 3060 + Intel, 180 Hz. Lag only above 60 fps.
  - 18:42 log (build 18:32): "GL waits" 1-6 ms - not the GPU. The CPU-drawn 2D picture took 13-60 ms a frame
    (7-10 ms here with his cfg on Route 111, 2.1 GHz Xeon under Wine) and the 3D CPU work was inflated alike,
    varying frame to frame: looked like the thread waiting for a CPU. Build 18:44 logs the thread's own CPU time.
  - **18:53: "It seems to not be lagging anymore ... nothing has changed on my end."** His 18:44-build log has no
    LAGGY line at all (only map-entry stalls of 150-190 ms on Route 110, mostly building the maps). Cause of the
    earlier lag not found; it went away without a change on his side - most likely something outside the game
    taking the CPU. The stall log stays in the game; if it comes back, the LAGGY line's "thread CPU" says whether
    the game was starved. The blue-bottom fix (atlas grown) stays.
  - His `WallysJourney_errors.txt` (build 18:44): `skipped: callback exe+198be0 = Task_HandleExpansionIntro,
    c0000005 at 0x2c, callback2 exe+198bc0 = CB2_ExpansionIntro` - a NULL read in the boot "powered by" intro,
    skipped by the guard (the game goes on). Not looked into yet.

## Brennan's new items, 2026-11-16 08:46 (his words; numbered here by session 014PwRDj)

- [ ] **POL-37. Learn from Emerald 3DS.** "Emerald 3DS doesn't seem to have 3D issues, please research how they
      were able to generate everything in 3D so cleanly and use what you learn to help polish our 3D world. please
      review their code to learn how they do it, no guessing or pretending, if you need anything to accomplish
      this please ask me"
      - 2026-11-16 (014PwRDj): study written up in `docs/POL-37_EMERALD_3DS_STUDY.md` (what they do, what was
        checked, how to run their engine view on our data, the plan). Pictures:
        `docs/polish_reports/POL-37_route118_ours_vs_3ds_engine.png`, `POL-37_pokecenter_ours_vs_3ds_engine.png`.
        Short version: our relief data is byte-for-byte theirs on every unchanged map; their own status list has
        the same faults on Route 118/119/111 and the Game Corner; the lines on the sand and the Pokemon Center
        are not in their engine's scene - they are our renderer's texture-edge sampling at high resolution.
- [x] **POL-38. Sandstorm north of Mauville (Route 111).** "sandstorm north of mauville looks weird not like a
      sandstorm. Rocks within the sandstorm, the top right of the rocks so a blue box"
      - 2026-11-16 (014PwRDj): two causes. (1) With Gamma lighting the weather colours went into the linear
        scene as they were and the exposure pushed them to white: the sand's streaks read as rain. Now converted
        to linear light / exposure (rain's too). (2) Only the streaks of Emerald 3DS's sandstorm were ported; its
        dust (ctr_voxel.c DrawBanks with sSandBanks: two drifting noise sheets near the ground) now too
        (voxel_gl.c DrawSandBanks, values as the 3DS's; WJPC_NO_SAND_DUST=1 off). Picture:
        docs/polish_reports/POL-38_sandstorm.png. The "blue box" at the rocks not found in the test shots yet.
- [x] **POL-39 (PRIORITY). White flowers.** "The white flowers around the region have been turned into 3D, but
      now they have black lines and rough textures." (the POL-7 flower cards)
      - 2026-11-16 (014PwRDj): his crop shows a dark tick at both ends of every card: the card's u ran right to
        its edge, so a screen pixel there read the next card's column in trees.rgba5551 (dark outline). Fixed:
        the card's uv 1/32 texel inside (voxel_tree.c EmitGroundCard), as the tree crowns' top already was. Route 118
        at 1280x720: no ticks with the fix - but 16c shows none at that size either (the ticks need his
        resolution), so this waits for Brennan's check. "Rough textures" not changed yet - waiting for his look at the new build.
- [ ] **POL-40. Route 118.** "look within the red boxes in the picture provided. There are black lines along the
      beaches and on the ground near the trees. Also the background mountains are blocks instead of like the
      mountain south of it that looks correct." (picture not received yet - asked 08:50)
      - 2026-11-16 (014PwRDj): the blocks are past Route 118's north edge: our belt (POL-30) stood every cell flat at
        the height blended from the nearest maps, a wall a tile a level between neighbours, sky between columns.
        Belt land now lies on its corners (each the mean of the land cells round it) as one slope; walls only
        towards water/trees, cut a tile a level (voxel_mesh_builder.c BeltCorner/BeltSlopeSteps; WJPC_BELT_BLOCKS=1
        the old way). Lavaridge and Fallarbor checked unchanged. Picture: docs/polish_reports/POL-40_route118_background_blocks.png.
        Black lines on the beaches: the 16c/16e fixes (atlas inset, safety floor, centroid) - waiting for Brennan.
- [ ] **POL-41. Route 119 trees.** "look within the red boxes in the picture provided. The trees in the back ground
      look to be higher than the others showing a block look to them. All trees should align together to show a
      full forest." (picture not received yet - asked 08:50)
      - 2026-11-16 (014PwRDj): the walls are in the wood past Route 119's west edge (the belt, Route 119's border
        trees). POL-30's belt took each cell's level from every map's edge, blended - Route 118's mountains to the
        south pulled the wood up column by column, and each step showed as a wall of tree crowns. Belt tree cells
        now keep their own map's edge level, unblended (voxel_mesh_builder.c BorderLevelRaw; WJPC_BELT_TREE_BLEND=1
        old way). Picture: docs/polish_reports/POL-41_route119_tree_walls.png (the walls Brennan boxed, then even).
- POL-8 (2026-11-16 08:34): Brennan's photo of the lines (Pokemon Center 1F, build 16b, before the atlas fix):
  thin dark lines along the floor in front of the counter and the mat, and dashed lines on the side walls and
  around the stools. Those are the room's own models (page textures, not the atlas), so the 16c atlas inset
  does not reach them; the same 1/32-pixel inset is now applied to every model face
  (`VoxelBuildings_InsetTriUV`, building chunks and static models).

## Questions for Brennan (and his answers)

Asked 2026-11-14, answered the same day:
1. Crash files (`WallysJourney_crash.txt`, `WallysJourney_errors.txt`, `WallysJourney.cfg`): "yes I can provide
   that" - not received yet.
2. Which build: "I can't remember what version. But I start in 3D and have gamma on." -> test POL-11/14 with
   Gamma lighting ON.
3. Move Deleter refusing to delete the last Surf: "disregard 3" -> leave the Move Deleter as it is.
4. POL-27 Route 113: "yes you are right I misspoke" -> he meant the ash puff when stepping in the ash.
5. Who merges Polish fixes into the game: "by you" -> this session merges finished `polish` work into
   `kanto-port` (fetch + rebase first, never force-push, check WORK_CLAIMS; AGENTS.md rules).

### Files Brennan sent, 2026-11-14 17:09 (kept by session 014PwRDj in `/home/claude/keep/brennan_2026-11-14/`, not in git: his own save)
- `WallysJourney.cfg`: `world3d=1 battle3d=1 gammaLight=1 frameRate=144 resolution=3 (1440p) zoom=5 bilinear=1
  interframe=1 integerScale=1 vsync=1 lockAspect=1 widescreen=1 fullscreen=0 windowScale=8 volume=0`.
  -> test POL-11/14 with exactly these (Gamma on, 144 fps in-between frames, interframe blend, 1440p).
- `WallysJourney.sav`: 131,788 bytes = a PC save that already has the Kanto block with K4's vars/trainer flags,
  so his build is from 2026-11-13 evening or later. Player in Mauville. Loads fine in the current build (3D, Wine).
- `WallysJourney_errors.txt` (no `WallysJourney_crash.txt` was sent - ask again after the next crash). Addresses
  are exe-relative for HIS build, which we cannot identify yet, so they are not resolved:
  ```
  skipped: callback exe+19c110, c0000005, address 0000002c, callback2 exe+19c0f0
  skipped: callback exe+37c970, c0000005, address 00000000, callback2 exe+2395a0   (3 times)
  skipped: callback exe+2c76f0, c0000005, address 00000008, callback2 exe+239c60
  skipped: callback exe+1a40e0, c0000005, address 0000002c, callback2 exe+1a40c0
  ```
  (NULL reads the guard skipped: one callback skipped each time, not a crash by itself. To resolve them: get the
  zip name of his build, rebuild that commit unstripped, `llvm-nm -n`.)

## Brennan 2026-11-16 14:56: build 16f "laggy and there was also a crash" (+ "revert back all the white flowers to the 2D form")
- His files kept in /home/claude/keep/brennan_2026-11-16_1456.sav (save). cfg: resolution=4 (4K), antialias=4, gammaLight=1.
- Crash: c0000005 in the GL driver, stack through MakeBanks (POL-38's sand dust, first frame of Route 111's sandstorm):
  the texture upload ran with SDL's GL_UNPACK_ROW_LENGTH still set, reading past the texels. Unpack state reset first.
- Lag (LAGGY lines, 2D picture 18-25 ms + 3D 18-65 ms on Mauville/Route 111): option 5's 4x MSAA on an RGBA16F target
  at 3840x2160 (~265 MB, resolved each frame) - the change these builds added. MSAA now off above 1440p, 2x max at 1440p
  (WJPC_MSAA overrides). Waiting for his check on 16g.
- White flowers (General 0x001, the grass's flowers) back to 2D; red flowers (0x004) and planters (0x007) stay cards.
- POL-40 belt terraces work in progress on branch `polish-belt-wip` (not in 16g).

- [x] **POL-42. Ground moves vs Orthworm.** Brennan 2026-11-16 15:39: "ground type moves arent super effective against
      steel type as they should be. this happened when battling against Orthworm in the desert."
      - 014PwRDj, checked: the type chart is right (src/data/types_info.h, Ground -> Steel x2). Orthworm (Route 111's desert,
        wild_encounters.json) has the ability Earth Eater (gen_9_families.h: { EARTH_EATER, NONE, SAND_VEIL }) - as in
        Scarlet/Violet, a Ground move heals it instead of hurting it. Asked Brennan whether to keep that (official) or change it.
      - Brennan 2026-11-16: "Keep Earth Eater". Closed, no change.
- 16h (Brennan 15:34, 16g at 4K/240 fps "the lag is back whenever walking ... the whole game lags behind"): a catch-up
  spiral in sdl2.c's main loop - every game frame of a catch-up batch was drawn (CPU picture + whole 3D world), only the
  last shown; one slow trip made the next run up to 6 frames, each drawn. Now only the batch's last frame is drawn (as
  fast-forward did; WJPC_DRAW_EVERY_FRAME=1 old way). Measured here with his save in Mauville at frameRate=240 (software
  GL, so slow overall): old ~3000 ms trips (3D 2900 ms = ~6 world draws each), new ~250-500 ms (one draw). Stall log now
  says "N drawn" per worst trip. His save: /home/claude/keep/brennan_2026-11-16_1534.sav.

## Brennan 2026-11-16 ~19:00, after test builds 16i-test / 16j-test (his words) - HARD, PARKED FOR LATER
Brennan: "take all this info and update the github on these POLs, after updating lets try and complete the easier
tasks and we can leave these harder ones for later. if any questions ask me first."
- **POL-41 (Route 119 belt wood):** 17:56 - trees look good, but "where the mountain edge and trees start theres a black
  line with blue showing ... needs to be meshed together"; west of the Weather Institute "the trees have rock squares
  showing and blue showing through the ground". ~19:00 (16j-test): "Route 119 there is now more mountain blocks in
  between each tree cell, this needs to be only trees."
  - State: `polish` has the unblended belt wood (570268b8), BeltTreeSeam + edge mean +-4 (2eec5a4e). The 16j-test also
    carried the POL-40 WIP (branch `polish-belt-wip`: mirrored belt heights, BeltTopTile mountain tops, face-tile walls) -
    likely the source of the "mountain blocks between trees" (land belt cells next to wood drawn as rock). Not confirmed.
- **POL-40 (background mountains):** Route 118: "the rock still shows pillars on the most recent build, just make a
  mountain round itself out instead of pillars." Earlier: "make it like the mountains that appear in front of them and to
  the left ... staggered as it is getting higher". Lavaridge: "it is way incorrect make mountains where it makes sense to
  flow with the background"; earlier: "mirror whats on its west side and make it horizontal like the rest of it until it
  meets the trees and have it slope up from there".
  - Tried on `polish-belt-wip` (none shipped in a release build): flat blocks per cell at blended heights (pillars);
    corner-sloped tops (16f, "still pillars"); terraces: envelope + whole levels + rock rise (16i-test, pillars); mirrored
    heights (16j-test, pillars - each cell still a flat block); mirrored relief grid + face walls (picture
    POL-40_mirror_try.png: terraces' backs face the camera, light gaps at the water). Lavaridge's belt is a *drawn* belt
    (VoxelRelief_Belt != 0, relief_fixes) - none of the belt-land changes reach it.
  - Idea not tried: model the belt's mountains as their own relief (gen_voxel_relief / voxel_terraces over a padded map)
    rather than in the renderer.
- **NEW, numbered POL-43: Route 112 -> Lavaridge:** "when going from route 112 to lavaridge the backgrounds disappear in the red boxed
  areas" (pictures not received in this message).
- **NEW, numbered POL-44: Lavaridge cable-car stairs:** "the stairs leading up to the lifts east of lavaridge go straight up instead of
  staggering like real stairs."
- **Brennan's pictures for the above, 18:41** (`docs/polish_reports/BREN_1841_*`):
  1. Route 119, west of the Weather Institute: a dark/black box where belt trees should be (red box).
  2. A town by the sea (pink-roof houses, beach east): the mountain north of the town over the water is a block of
     stepped rock with vertical seams (red box). Brennan 18:44: "the town with the beach and pink roofed houses is route 118 left of mauville." So this is POL-40's Route 118 mountain (the belt past Route 118's north edge, x 6-13) - POL-45 folded into POL-40.
  3. Lavaridge: the background top-left (behind the Mart) has sky-blue holes and floating slabs; the bottom-right edge
     has a sky-blue strip.
  4. Same view on arriving from Route 112 (sign "ROUTE 112"): same holes top-left, and the bottom-left edge shows flat
     grass instead of the mountain.
  5. The "stairs" east of Lavaridge: the grass ledges run as long straight strips up the slope, not staggered steps.

## STATUS BOARD, 2026-11-16 18:45 (session 014PwRDj) - for Brennan to choose what comes next
Brennan 18:42: "update the github on everything that we have accomplished and the fixes that are left to do. give me a
list of all the POLs left so i can choose which ones we need to complete."

**Done (ticked):** POL-1 people round, POL-5 Trainer Hill, POL-7 3D flowers/grass/planters, POL-12 water moves,
POL-16 people black at doors, POL-23 sandstorm, POL-25 unlearn HMs, POL-26 + 26b Rustboro lights / 3D night,
POL-27 Route 113 ash, POL-29 music/sound volume, POL-42 Orthworm (keep Earth Eater).

**Done 2026-11-16 by 014PwRDj, not ticked (waiting for Brennan's eyes on his GPU):**
- POL-8 black lines indoors: 1/32-texel inset on the atlas and every model face (needs his 4K check).
- POL-37 Emerald 3DS study (docs/POL-37_EMERALD_3DS_STUDY.md); from it: safety floor under every map, MSAA (off above
  1440p), sand dust.
- POL-38 sandstorm look (colours + dust sheets). Left: the "blue box" at the rocks (not found).
- POL-39 white flowers back to 2D; red flowers/planters keep the inset fix. Left: "rough textures" if he still sees them.
- POL-9 prize machines now 3D. Left: walk-through columns (which ones?), the Game Corner lag.
- Crash fix (sand dust upload) and lag fixes (MSAA at 4K; draw once per catch-up batch at 240 fps).
- POL-41 belt wood level and seam (Brennan: "the trees look good"). Left: see below.

**Left - easier / need one answer from Brennan:**
- POL-4 people vanish behind you (not reproduced; need map + video).
- POL-6 boxes cut off outside widescreen (not reproduced; need screenshot + fullscreen or not).
- POL-10 walking through the top of doors.
- POL-17 seed mounds look like shrubs, sit above the dirt.
- POL-19 falling leaves appear/disappear/speed up.
- POL-15 Route 110 -> Mauville (atlas fix in; lag not proven fixed - need a new stall log).

**Left - bigger 3D model work:** POL-18 Pokemon flat, POL-20 boats flat, POL-22 breakable rocks flat / pop in.

**Left - hard, parked (the belt / backdrop past map edges):** POL-2 trees end in blue, POL-3 mountain edges blue,
POL-13 background mountains are blocks, POL-21 Route 110->103 backdrop, POL-24 + POL-43 Route 112 -> Lavaridge
backdrop, POL-28 Petalburg Woods lines, POL-30 backdrop changes between routes, POL-40 Route 118 pillars + Lavaridge
mountains, POL-41 leftovers (black box west of the Weather Institute, rock blocks between trees),
POL-44 Lavaridge cable-car stairs straight. (POL-45 = Route 118, folded into POL-40.)

## Brennan 2026-11-16 19:12 - his checks on test build 16j-test, every open POL (his words)
Closed by Brennan (ticked above): POL-6 ("close this one out"), POL-10 ("close this one out"), POL-15 ("the lag is no
longer there"), POL-18 ("close this one out"), POL-19 ("you can close this one out"), POL-20 ("boats have been fixed"),
POL-21 ("close this one out"), POL-38 ("the blue boxes are no longer there, and the sandstorm looks good"), POL-39 ("the
white flowers look good"), the 16f sandstorm crash and lag ("crash and lag from the sandstorm has been fixed").

Still open:
- **POL-4:** "people no longer disappear, only things are the kids in fortree city with the fake tree in front of them
  when walking away the fake tree disappears."
- **POL-8:** "the lines still appear as you are walking when at 4K."
- **POL-9:** "you can still walk through the columns at the front of the game corner, it is not so laggy anymore, the
  prize machines look scrunched down they need to be raised at the top to make it look full, the vending machines also
  do this same thing. imagine them as tall rectangles instead of squares"
- **POL-17:** "the picture provided shows the seeds sitting back behind off of the dirt, the seeds should be remaining in
  the middle of the dirt as if they were planted." Picture: `docs/polish_reports/BREN_1912_POL-17_berry_plants_behind_dirt.png`.
- **POL-22:** "mauville to route 111, the breakable rocks at the passageway to go north is still a 2D object it needs to
  be 3D". Picture: `docs/polish_reports/BREN_1912_POL-22_route111_rock_still_2D.png`.
- **POL-2/3:** "this still continues around the region, the bases around trees or some edge lines still show either
  black or blue. try using a green safety screen behind it so the colors match and dont show backgrounds"
- **POL-13:** "this is a continuing issue, all mountains we have added in are not 3D but more so 2D blocks. try
  researching into how the other mountains were created so that the mirrored mountains are meshing well with one
  another and make sense along side one another."
- **POL-24 & POL-43:** "this continues as a problem". **POL-30:** "this continues happening going from route 112 and
  lavaridge".
- **POL-28:** "the black lines appear at the base of the trees inside of the woods."
- **POL-40:** "try taking information from what i said in POL-13 to fix this issue." **POL-45** (= north of Route 118):
  "these mountains are located north of route 118, use what i have stated in POL-13 to fix this issue."
- **POL-41:** "this is still an issue". **POL-44:** "the stairs go straight up instead of stagger" (picture
  `BREN_1841_5_lavaridge_stairs_straight_up.png`).
- POL-37: Brennan asked what is left on it (answered in chat: the study is done; what is left are the plan items it
  produced, which are now POL-13/40/2/3/28).
Brennan: "lets focus on the easier POLs first, if any questions ask me." Order taken (014PwRDj): POL-17, POL-22, POL-9,
POL-4 (Fortree), POL-2/3 green safety screen, POL-28, POL-8; then the mountains (POL-13/40/45/41/24/43/30/44).

## Easier POLs, 2026-11-16 evening (session 014PwRDj) - what was done (build 16k-test)
- **POL-17 berry plants:** the GBA stands a berry plant's 32-row frame on its cell's south edge (its base, rows 26-30,
  in the soil's middle: the soil is drawn across its cell and the one south of it). Our cards stand on the cell's
  centre with the feet (pad measured on the graphics info's frame 0 - not the berry's own picture table) there: half a
  tile north, "behind off of the dirt". Now berry plants stand on the cell's south edge with their whole frame
  (`voxel_entities.c` BerryPlant, VOXEL_BERRY_STAND; WJPC_BERRY_CENTRE=1 old way).
- **POL-22 breakable rocks (and Strength boulders):** a mound instead of a card (`voxel_entities.c` EmitMound): column
  by column the sprite's rows are laid on a quarter round so at 45 degrees every texel is where the card had it
  (the 3DS rule "the drawing is the truth"), back face at 45 degrees, shaded by facing. WJPC_FLAT_ROCKS=1 old way.
- **POL-9 machines:** prize machines and the two back-left machines as tall boxes (gacha 14->24 px, arcade 14->22),
  same drawing, generator exact.
- **POL-9 front columns:** the Mauville Game Corner portal stood 8 px forward with its posts on row 14's cells, which
  the cartridge lets you walk on. Asked Brennan: "Pull the porch back" (not blocking the squares). The portal is now
  flush with the facade and the posts' feet are painted on the ground (spec game_corner(): portal part off; keep/exact
  without rows 64-72). Spliced with the new `splice_buildings.py ... 3 5 10 --replace`.
- **POL-4 Fortree ninjas:** Route 119's two ninja boys by Fortree hide under a tree / rock disguise (a field effect). When
  Route 119 is a neighbouring map their stand-in was drawn from the boy's own picture - the tree "disappeared". Now a
  disguised ninja's stand-in is the disguise's first frame (`wide_view.c` WideView_DisguiseInfo, used by the 3D
  stand-ins and the 2D wide view's neighbour people).
- **POL-2/3/8/28 crack fill:** in the final screen pass (Gamma lighting path), a pixel nothing was drawn on between two
  drawn pixels (1-2 px apart, across or down) takes their colour - the "safety screen" Brennan asked for, made of the
  ground's own colours instead of one green (so sand, water, floors and snow match too). WJPC_NO_CRACK_FILL=1 off.
- Checked here (Windows exe, Wine, 1280x720): berries planted (`POL-17_berry_plants_planted.png`), rock mound
  (`POL-22_breakable_rock_3d.png`), porch flush (`POL-9_game_corner_porch.png`), machines (`POL-9_machines_tall.png`).
  Not checked here: the Fortree ninja stand-ins (the test script's warp to Fortree times out under Wine) and the crack
  fill at 4K (cannot run 4K here) - both for Brennan's check on 16k-test.

## Brennan 2026-11-16 20:33, checks on 16k-test
- Closed: POL-17 ("berries are now fixed close this one out"), POL-22 ("that will suffice for now, close this out"),
  POL-9 ("Game corner now looks good, close this out").
- **POL-2/3, 8, 28:** "still show black lines inside of buildings." Picture: `BREN_2033_POL-8_house_lines.png` (a house,
  two thin dark lines across the floor/rug at tile-row boundaries, behind the table - floor seams, not screen rows).
  The crack fill did not catch them: being looked at with a 4K run here.
- **POL-4:** "the fake tree is still disappearing whenever walking away." Picture: `BREN_2033_POL-4_route119_ninja.png`.
  Cause found: the disguise field effect copies its object's sprite's `invisible` flag (UpdateDisguiseFieldEffect),
  which the game sets once the object is off the 2D screen; our 3D view draws the object itself further out (it
  ignores that flag for objects) but honoured it for effects - so the boy stood there without his tree. Fix:
  `voxel_entities.c` DisguiseShown - a disguise is hidden only when its object is (obj->invisible).
- 16l-test (2026-11-16 ~22:40) = 16k-test + the POL-4 disguise fix. Not checked here: since ~20:30 this machine's
  Wine test runs to Route 118 / Fortree end without a picture even with the older 16h exe (a test-setup problem here,
  not the game). POL-8 house lines: not reproduced at 1280x720 (zoom 1 and 4) and a 4K run here is too slow to reach
  the field; the floor of LAYOUT_HOUSE1 has no ground patches and chunks are quantised to 1/512 tile (no chunk
  cracks), so the cause is still open. Brennan (asked): the lines are "Both" - some stay put, more appear while walking.

## Harder POLs, night of 2026-11-16/17 (session 014PwRDj) - code pushed to `polish`, only partly seen
Brennan 23:17: "Continue onto the harder ones for now." 23:27: "Provide me pictures not a test build for right now".
- **POL-13/40/45 (belt mountains, Route 118 north):** `voxel_mesh_builder.c` BeltMountain / BeltRoundOff: a land cell
  of the belt takes the map's edge heights averaged along the edge over +-(1+2d) cells (d = distance out), held a
  cell, then rounded down to the ground over 7 cells (quarter cosine), and down to the water's level within 4 cells
  of water; Route 118's north edge FILL (relief.bin "EDGE", a plateau at 64 px out to the horizon - the "pillars":
  a stair of dark walls down to the sea, row by row) is rounded the same way; belt corners next to belt water are
  pulled down to the water's surface (BeltCorner); sloped belt tops laid with the map's mountain-top tile
  (BeltTopTile), not the face's streaks. Not near a drawn belt (NearDrawnBelt, 8 cells). Off: WJPC_BELT_COLUMNS=1,
  WJPC_BELT_FACE_TOP=1. Seen: `POL-40_route118_mountain_round_try1.png` (try 1); later tries: walls shorter, a low
  step still along the shore (m57); the last two changes not seen yet.
- **POL-24/30/43 (Route 112 <-> Lavaridge):** the drawn belt (VoxelRelief_Belt, Lavaridge's) was chosen by the map the
  player stands on, so crossing switched the whole belt. Now per cell, by its own map (VoxelWorld_BeltRing)
  (WJPC_BELT_DRAWN_GLOBAL=1 old). Seen: `POL-24-43_lavaridge_belt_before_after.png` - pillars / floating slabs gone,
  thin light lines left top right. With the round-off a sky gap opened at the drawn/undrawn seam -> NearDrawnBelt
  (not seen yet).
- **Test machine problem (not the game):** since ~20:30 the Wine test runs end with exit code 0 shortly after a warp
  to Lavaridge / Route 118 / Fortree (2D too, and with the 16h build), at varying frame counts; a fresh Wine prefix
  did not help. Earlier lesson: never `pkill -f wine...` from a command line that itself contains those words (it
  kills its own shell) - `scratchpad/cx/clean.sh` now kills by process name only. Not solved.
- Not started: POL-41 leftovers, POL-44 (Lavaridge cable-car stairs).

## GROUPS (Brennan 2026-11-17 10:43: "Combine all POLs that have to do with 3D generation together and then combine
## all POLs that have to do with the out of bounds terrain that the zoom feature allowed us to see, then work on the
## new mauville glare and 3d generation first since new mauville is all flat still, then once finished with that work
## on the remaining pols, no guessing or pretending please ask if you have any questions")
POL-2/3 asked: "One each" (POL-2 out of bounds, POL-3 3D generation).

**A. 3D generation (inside the maps)** - order: POL-31 first, then the rest
- [x] **POL-31** New Mauville: Gamma-lighting glare too bright; its models still 2D (Emerald 3DS has not modelled
  New Mauville at all: VOXEL_STATUS "Entrance/Inside" both not started). Done 2026-11-17 (Brennan 18:02: "things
  look to be fine, if there are anymore things to fix i will let you be aware"). All notes, fixes and how to rebuild:
  `docs/NEW_MAUVILLE_3D.md`. Entrance (layout 184) still flat.
- [ ] **POL-3** mountain edges show blue (and "the bases around trees or some edge lines still show either black or
  blue"). Crack fill in the final pass (16k-test) - outdoor result not reported yet.
- [ ] **POL-8** black lines on indoor floors at 4K ("Both": some stay, more while walking). Not reproduced here.
- [ ] **POL-28** black lines at the base of the trees in Petalburg Woods.
- [ ] **POL-44** Lavaridge cable-car stairs go straight up instead of staggering.
  2026-11-17: Brennan, asked where a sloped flight's room comes from: "Half and half". Done in `voxel_relief.c`
  StairFlights (Route 112 x 20-21 rows 41-43: 6 steps, 8 px tread + 8 px riser, top 24 px into the cliff top, foot
  24 px out onto the path; the 2 cliff-top cells north of it laid shorter). WJPC_STAIRS_UPRIGHT=1 = old. Other
  flights elsewhere are untouched (add rows to sStairFlights). Test: warp `0 27 20 39` (on the cliff top above the
  stairs) works; warping to the path below (`0 27 20 46`, `24 47`) ends the game 100-300 frames later (2D too, exit
  code 0, not this change - WJPC_STAIRS_UPRIGHT=1 does the same). Picture: `POL-44_route112_stairs.png`.
  Brennan 2026-11-17 19:33: "i will find more stairs and let you know whenever i have a list." (Other flights wait
  for his list; add each as a row of sStairFlights.)
- [ ] **POL-37** the Emerald 3DS study (done; its findings are these POLs) - close when Brennan says.

**B. Out of bounds terrain (the belt past the map's edge, seen when zoomed out)**
- [ ] **POL-2** background trees go up into the sky showing blue.
- [ ] **POL-13 / POL-40 / POL-45** background mountains as blocks / pillars (Route 118 north) - rounded mountain
  (BeltMountain / BeltRoundOff), not seen in a test build yet.
- [ ] **POL-24 / POL-30 / POL-43** backgrounds change / disappear going Route 112 <-> Lavaridge - per-cell drawn belt,
  not seen in a test build yet.
- [ ] **POL-41** Route 119 belt trees: rock blocks between tree cells, black box west of the Weather Institute.

**Other:** - [ ] **POL-4** Fortree ninjas' disguise (16l-test, not checked yet).
- POL-31 progress (2026-11-17): glare done (Brennan: "Good as is"). Walls: tried scaling relief heights for New
  Mauville (VoxelRelief_Cell) - no effect: New Mauville has NO relief cells and NO building models (relief.bin and
  buildings.bin checked); its walls only look raised because the art draws them. Reverted. Now modelling it in
  3ds_port/scripts/voxel_building_specs.py new_mauville_inside() (Mossdeep gym's way: wall runs 24 px, props as
  boxes); barriers/switches next. Brennan (asked about walls): "Too short" + "They just look flat like 2D".
- POL-31, Brennan 2026-11-17 16:00 (picture `BREN_1600_POL-31_new_mauville_walls.png`): "Red Box 1 - the wall is short
  and cut off from top to bottom, make this a full wall going from top to bottom / Red Box 2 - The cabinet is cut of
  at the top of it along with the wall behind being short, make the cabinet 3D model and raise the back wall to align
  with the rest across the top / Red Box 3 - ... the wall is short in this section among other sections check this
  along with others and raise them to be even across the top. / Red Box 4 - ... other walls also dont align at the
  top and are short or cut off." Cause: rect_pieces stood a run a single cell tall 12 px (and a prop against a wall
  took the wall's face row, leaving a 1-cell run). Fix in new_mauville_inside(): props against a wall leave that row
  to the wall (the prop stands in front, against=); every wall run whole=True (24 px). Switches: Brennan "Try to keep
  them raised and brighten them. If you can't ... leave them flat on the floor and as bright as the original art."
- POL-31, Brennan 2026-11-17 17:06 (picture `BREN_1706_POL-31_south_of_barriers.png`): "if you look to the south of
  each switched walls the wall to the bottom of it is still a half wall and not full in line with the top of the
  wall. check every wall in new mauville and check if their tops align with one another make it make sense and look
  clean." Causes: (1) the bar had taken the west half of the south wall's cell under it (only an east-half piece
  stood there, so a dark notch showed); (2) whole-cell wall runs: New Mauville's thin north-south walls are drawn
  8 px wide down one half of their cells (0x26a/272/27a/282, the other half black), and a column's run went on from an
  east-west wall down a thin wall, laying the east-west wall's face flat as a top. Fix (new_mauville_inside()): the
  south wall under each bar is a normal wall (its Bottom cell starts at row 4, below the bar's end); walls are built
  in half cells (8 px): an all-black half is no wall, a half's run ends at its foot (outline in row 15 under face
  colour), so every wall's front is its drawn face and every wall 24 px. Tried and dropped: running the bar on to the
  south wall's foot (the bar went dark). Picture: `POL-31_south_of_barriers_fixed.png`.

## POL-46 (new): Brennan's 3D brief, 2026-11-17 19:42
Brennan sent a brief ("overlook this information and work on this for our 3D POL section"): close every mountain
volume (no open back faces / sky leaks), stepped multi-sided ridges instead of flat blocks (Lilycove north), no black
void boxes behind trees (Route 119 Weather Institute), seamless map edges (Lavaridge south). "Start by fixing the
Lavaridge Town to Route 112/Mt. Chimney northern mountain seam." (The brief names 3DS code/limits; our game is the PC
renderer, src/platform/voxel3d, no 3DS limits.) Asked: stepped or rounded? "Show me both". Pictures: "Use old ones".
- Lavaridge NE seam (voxel_mesh_builder.c): cause = Lavaridge's drawn belt (4 levels, drawn 4 rows south) meeting
  Route 112's belt (2 levels, drawn where tiled) along a diagonal - a pit + a stair of sky. Fix: near a drawn belt
  the other belt starts at its top/shift and eases to its own over 8 cells; BeltCloseSeams walls between the two
  kinds. Terraces: WJPC_BELT_TIERS=1 (whole levels + whole rows, every step walled). Picture:
  `POL-46_lavaridge_seam_smooth_vs_tiers.png` (before / smooth / tiers). Thin slivers still show along the seam in
  both. Waiting: Brennan's pick. Next: Lilycove north ridge, Route 119 black box, Lavaridge south edge.
- Brennan 21:00 on that picture: blue triangle still at the seam; smooth = "stretched curtain" + black tears; tiers =
  "organ-pipe slivers". Wants "horizontal stepped plateaus (terraces)—each tier must have a vertical rock wall of fixed
  tile height (1 to 2 tiles high with tiled rock UVs) followed by a flat horizontal top ledge", like Route 112's own
  behind the Mart. Done for Lavaridge (DrawnTierLift, TerraceBand, BeltCloseSeams in the cliff face a tile a level,
  EdgeOverBelt): picture `POL-46_lavaridge_terraces.png`. Elsewhere still smooth; WJPC_BELT_TIERS_ALL=1 made Route
  118's north columns - the next thing to do right (terraces stepping along rows, not per cell).
- Brennan 22:07 (picture `BREN_2207_POL-46_lavaridge_red_box.png`): left terraces "look much better now and the blue
  sky leak is gone", but in the red box (upper right, where the terraces meet Route 112's east wall and forest):
  (1) missing cap blocks / dark notches on the uppermost tier; (2) a jagged black vertical seam where the terraces
  meet the east wall - needs east-facing cliff walls with rock UVs; (3) the raised trees on the right sit on a dark
  disconnected shelf - align their ground with the terraces so it is one mountain corner. Then: "do not hardcode this
  fix only for Lavaridge" - generalize to all of Hoenn: (A) mountains: horizontal terraces, walls on every exposed
  side, continuous massifs across borders/connections; (B) forests: tree ground aligned with terraces, border trees
  continuing across seams at matching height; (C) seas: sealed shorelines, ocean border extended flat to the horizon
  at sea level.
- 22:07 red box fixed (`POL-46_lavaridge_corner.png`): belt beside a drawn belt takes the higher of the drawn belt's
  rings and its own map's edge stepping down a level every 2 cells (from the edge cell's highest point), all drawn as
  far south as they are high - the terraces wrap the corner up to Route 112's edge, no gully, no sky.
- Generalizing (Brennan: "do not hardcode this fix only for Lavaridge"): `BeltTerraces` in voxel_mesh_builder.c
  (every loaded map's edge stepping down a level every 2 cells, highest wins; edge height = max over +-5 cells along
  the edge; walls all round; blocks). WIP, OFF by default (WJPC_BELT_TERRACES_ALL=1 turns it on): on Route 118 the
  north mountain came out as tall columns (pictures not committed: Route 118 at 0 33 9 5, shots after 40-120
  frames). Not found yet which heights make the columns (SPAN 2 -> 5 changed nothing). Forests (rule B) and seas
  (rule C) not started. Test note: warps to Route 118 / Lilycove now often close the game 150+ frames in (the old
  test-setup problem), take shots within ~120 frames.
- Brennan 2026-11-18 13:33: Lavaridge corner "blocky 90-degree Lego stair cutouts", a vertical strip of tree tiles in the
  rock; Route 118 "long, parallel extruded planks/beams with deep vertical grooves"; asked for a unified heightmap with
  shared vertices and chamfered (45 degree) terrace corners; and to fix "Route 118 crashing". Done (voxel_mesh_builder.c
  BeltField / EmitFieldCell, default on; WJPC_BELT_BLOCKS_OLD=1 old): one height per grid corner (maps' edge heights
  blended 1/d^3, exact on an edge, + a level every 2 cells by distance to the nearest map off its N/W/E sides, max 6),
  each cell two terraced triangles (flat ledges, 1-level walls along level crossings, 45 degrees at turns), walls down
  to the ground where the field meets the belt's sea or trees. Picture `POL-46_heightfield_lavaridge_route118.png`.
- The "Route 118 crash" was NOT the game: the test script's last screenshot before `quit` was lost (the game quit
  before it was written). With a `wait 60` after the last shot every shot is there (Route 118 ran 540+ frames fine).
  So zoomed-out shots work too. (3DS limits do not apply: PC game, NO LIMITS; BELT_LAYERS/PAIRS raised 4 -> 16.)
- Left: tree belt ground not yet on the field (rule B), seas (rule C), the small dark notch north-west of Lavaridge's
  hot spring, Lilycove / Route 119 not checked yet.
- Rule B (forests): belt woods stand on the field (BorderLevel: mean of BeltField's 4 corners, not floored;
  WJPC_BELT_TREES_OWN=1 old). Route 119 west of the Weather Institute: the black box is gone
  (`POL-46_route119_woods.png`); some crowns there are darker in squares - they are crowns in the shadow of taller
  ones, not holes (same with floored or smooth ground). Lilycove (Dept. Store spot): sea to the north, nothing open.
  Test note: `set zoom 5` in the script did not zoom out in these runs.
