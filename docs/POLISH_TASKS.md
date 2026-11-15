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
      - Not reproduced yet. Stress run in Wine + Xvfb (software GL), Gamma lighting off, the shipped `pc_data/voxel`,
        a late-game save, script = warp to each of the 56 outdoor Hoenn maps and walk a square (`WJPC_TEST_NO_BATTLES=1
        WJPC_3D=1 WJPC_VOXEL_LOG=1 WJPC_TEST_WINDOW=1`): 26,000 frames, 45+ warps, worst frame 77 ms of map/belt
        work (a warp: the map underfoot + whole belt built at once), memory flat at ~480 MB. Nothing near 1-2 s.
      - Still to try: Gamma lighting on (bloom, depth of field), walking across seams for a long time instead of
        warping, battles in between, real-time speed with sound. Asked Brennan for his build name, his
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
- [ ] **POL-1. People look like paper.** "People's silhouettes are still like paper. They need to be rounded out
      to look like people."
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
- [ ] **POL-5. Trainer Hill.** "Trainer hill entrance is missing the top of the door. Inside of trainer hill
      building the things inside at the front of the building are cut off and doesn't look 3D, the pc is cut off
      at the bottom, the top of the healing station and back wall looks crunched and cut off like the back wall
      is down too far."
- [ ] **POL-6. Boxes too big outside widescreen.** "When not in widescreen mode, the text box and main menu box
      is too big for the screen and gets cut off."
      - Not reproduced yet (Claude 019khs4X, 2026-11-14). Linux build, Brennan's settings from his cfg but
        `widescreen=0` (zoom=5, resolution=3 / 1440p, lockAspect=1, integerScale=1, border=1), 960x640 test window:
        the new main menu (Continue / New Game / Option) and the new text box both show whole, nothing cut off
        (picture: `docs/polish_reports/POL-6_not_reproduced_widescreen_off.png` = the menu; the text box was checked in
        an earlier run, not kept). zoom 3 vs 5 makes no difference there.
      - Not checked: his real window size/monitor (windowScale=8, fullscreen), the Classic text box, the Windows exe.
      - Need from Brennan: a screenshot of the cut-off box, and whether he was in fullscreen.
- [ ] **POL-7. Grass, flowers, flowerbeds, trees still flat.** "The grass in 3D is not 3D it's still 2D alongside
      flowers, flowerbeds, trees."
- [ ] **POL-8. Black lines on indoor floors.** "There are black lines on the flooring inside of buildings."
- [ ] **POL-9. Game Corner.** "You can walk through the left and right columns of the game corner building,
      inside of the game corner the gatcha machines need to be fixed as they are in parts or not fully 3D and the
      game corner is very skippy/laggy."
- [ ] **POL-10. Walking through the top of doors.** "When walking in entrances you walk through the top of the
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
- [ ] **POL-15. Route 110 into Mauville.** "Walking into mauville city from route 110 everything below disappears
      and reappears."
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
- [ ] **POL-17. Seed mounds.** "Seed mounds look like shrubs and are above the dirt."
- [ ] **POL-18. Pokémon are flat.** "Pokemon in 3D are still 2D."
- [ ] **POL-19. Falling leaves.** "Leaves appear only in certain areas and randomly disappear or speed up."
- [ ] **POL-20. Boats are flat.** "Boats in the water are 2D not 3D."
- [ ] **POL-21. Route 110 to 103 backdrop.** "Going from route 110 to 103 the background is not fluid and changes
      from water to trees and skips into frame."
- [ ] **POL-22. Breakable rocks.** "Rocks that are breakable disappear and reappear and also 2D not 3D."
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
- [ ] **POL-26. Rustboro lights blink.** "Lights in rustboro city blink in and out while walking."
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
- [~] **POL-26b. 3D night (Treston's decision 2026-11-14, from POL-26).** Port Emerald 3DS's day/night to the 3D
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
      - Lamps: the 2D game's lamp-glow sprites show in 3D as flat yellow discs that pop in and out.
        **Treston, 2026-11-15: "Yes, take it over"** - Claude 019khs4X takes `voxel_entities.c` over from 014PwRDj
        for the lamps and draws them the way Emerald 3DS does (UpdateNightLamps).
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
