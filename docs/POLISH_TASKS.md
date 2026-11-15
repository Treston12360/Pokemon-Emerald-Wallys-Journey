# Project Polish — Brennan's fix list

Project Polish is Brennan's half of the one game (see AGENTS.md "Who works on what"): the existing game —
Hoenn in 3D, lighting, and the bugs he finds. Branch: **`polish`** (cut from `kanto-port` 5f0aae61 on
2026-11-14, so it has everything; `git fetch` and merge `origin/kanto-port` into it often). Older Polish items
are the DLX entries in `docs/DELUXE_WORK_QUEUE.md`.

Rules that apply here (AGENTS.md has them in full): never guess — ask Brennan; show him a PNG of every fix
("With all fixes send me pngs of the fixes and if there are any questions ask me"); **never tell Brennan
anything about the Kanto story**; test on `WallysJourney.exe`; builds handed over are the whole zipped folder.

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
- [ ] **POL-12. Water is still.** "Water is not moving in 3D mode it is still."
      - Found (Claude 019khs4X, 2026-11-14): reproduced in the Windows exe (Wine, 3D on, Route 103 sea): two frames
        40 apart are pixel-identical in the water. Cause: our 3D renderer never redraws the animated tiles. The
        atlas code for it is already ported (`voxel_atlas.c`: `VoxelAnim_*`, `VoxelAtlas_RefreshAnimated*`) but
        nothing calls it. Emerald 3DS does it every 16 frames in `ctr_voxel.c` (comment block at line ~5462, "The
        animated tiles (water, flowers) of the atlases in view ... VoxelAnim_*") and re-uploads the slots.
        The fix is a port into `voxel_gl.c` (texture uploads live there) - that file is claimed by 014PwRDj for
        POL-11/14, so it is waiting for that claim to be released or for permission. Flowers will animate too.
- [ ] **POL-13. Background mountains are blocks.** "Mountains in background are blocks and not like mountains."
- [ ] **POL-15. Route 110 into Mauville.** "Walking into mauville city from route 110 everything below disappears
      and reappears."
- [ ] **POL-16. People turn black at doors.** "When walking into doors all npcs turn black and disappear then
      reappear whenever you come back out."
- [ ] **POL-17. Seed mounds.** "Seed mounds look like shrubs and are above the dirt."
- [ ] **POL-18. Pokémon are flat.** "Pokemon in 3D are still 2D."
- [ ] **POL-19. Falling leaves.** "Leaves appear only in certain areas and randomly disappear or speed up."
- [ ] **POL-20. Boats are flat.** "Boats in the water are 2D not 3D."
- [ ] **POL-21. Route 110 to 103 backdrop.** "Going from route 110 to 103 the background is not fluid and changes
      from water to trees and skips into frame."
- [ ] **POL-22. Breakable rocks.** "Rocks that are breakable disappear and reappear and also 2D not 3D."
- [ ] **POL-23. No sandstorm.** "There's no sandstorm in the desert."
      - Found (Claude 019khs4X, 2026-11-14): 2D shows the sandstorm on Route 111's desert (same save, same spot);
        3D shows plain sand. Cause: `voxel_world.c` `VoxelWorld_Weather()` / `VoxelWorld_WeatherAmount()` only
        work under `#ifdef PLATFORM_3DS`, so on PC the 3D world always gets "clear" weather, and the 3D view has no
        port of Emerald 3DS's `DrawWeather` (`ctr_voxel.c` lines 8005-8137: rain, sand and ash drawn in the world,
        `CtrVoxel_DrawsWeather` / `CtrVoxel_DrawsSand`). Same cause for rain and ash in 3D (likely POL-27's ash puff
        too). Fix = port those into `voxel_world.c` + `voxel_gl.c` (claimed by 014PwRDj) - waiting like POL-12.
- [ ] **POL-24. Route 112 into Lavaridge.** "Route 112 into lavaridge town rocks in background appear and then level."
- [ ] **POL-26. Rustboro lights blink.** "Lights in rustboro city blink in and out while walking."
- [ ] **POL-27. Route 113 ash.** "Route 113 ash on the floor after walking through it shows blue boxes and does
      not show crash or anything." (Brennan, confirmed: he meant the ash puff when you step in it.)
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
