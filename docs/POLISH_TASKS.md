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

- [ ] **POL-11 (PRIORITY). Freezing in 3D.** "There's freezing and pausing when in 3D making the whole game stop
      for a second or two then start again."
- [ ] **POL-14 (PRIORITY). Crash in 3D after about 10 minutes.** "When in 3D mode after about 10min the game
      crashes after a lot of skipping and pausing."
- [ ] **POL-25 (PRIORITY). Unlearn HM moves.** "Make it to where you can unlearn an HM move."
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
- [ ] **POL-7. Grass, flowers, flowerbeds, trees still flat.** "The grass in 3D is not 3D it's still 2D alongside
      flowers, flowerbeds, trees."
- [ ] **POL-8. Black lines on indoor floors.** "There are black lines on the flooring inside of buildings."
- [ ] **POL-9. Game Corner.** "You can walk through the left and right columns of the game corner building,
      inside of the game corner the gatcha machines need to be fixed as they are in parts or not fully 3D and the
      game corner is very skippy/laggy."
- [ ] **POL-10. Walking through the top of doors.** "When walking in entrances you walk through the top of the
      door this should be fluid and not be cutting off the top when walking through the door."
- [ ] **POL-12. Water is still.** "Water is not moving in 3D mode it is still."
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
- [ ] **POL-24. Route 112 into Lavaridge.** "Route 112 into lavaridge town rocks in background appear and then level."
- [ ] **POL-26. Rustboro lights blink.** "Lights in rustboro city blink in and out while walking."
- [ ] **POL-27. Route 113 ash.** "Route 113 ash on the floor after walking through it shows blue boxes and does
      not show crash or anything." (asked Brennan whether "crash" means the ash puff — see Questions.)
- [ ] **POL-28. Petalburg Woods black lines.** "Bunch of black lines inside of petalburg woods."
- [ ] **POL-29. Separate music and gameplay volume.** "Add a feature in the volume setting to differentiate music
      volume and gameplay volume incase you want to turn off music but keep gameplay audio."

## Questions for Brennan (and his answers)

(none yet)
