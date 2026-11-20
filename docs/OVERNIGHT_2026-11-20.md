# Overnight log, 2026-11-20 (session 014PwRDj) - for Brennan

Rules (his 02:48 message): safe POL fixes only, BEFORE/AFTER PNG for each, questions held until "I am awake now".

## Changelog

### 1. Mt Pyre summit cleanup (POL-51 / POL-60, Brennan 00:43 red boxes, 01:29 "fix whats left on the summit")
Picture: `polish_reports/OVN_01_mtpyre_summit_cleanup.png` (all at midday - his 00:43 picture was evening light,
which darkens wall bases).
- NE flat light-blue sea square: the summit map's own sea (north rows 0-8) sat at the map's base, 64 px over the
  sea round the island. Laid at -64 px.
- Light-blue band along the north: Mt Pyre's own sea tile is 0x0A1 (its border block, lighter); the fills I added
  used 0x170. Both Mt Pyre maps' sea fills now use 0x0A1 - **the sea round Mt Pyre outside is that lighter colour
  too now (the original game's)**.
- NE rock flap over the sea: cells (48,8) and (49,9) re-laid (49,9's corner stood at 0 over the -64 sea).
- Dark notch at the boulder by the flower bed, and cracks where the east ridges bend: the lowered ground used to
  fade out over x32-36, leaving sloped ground against flat rock feet. Every terrace west of the x42 wall column is
  now lowered whole, the wall takes the step.
- One-wall range north of row 18: x13-19 and x27-34 (x35-41 kept their own walls: sky slivers appeared in the
  walls of the path there when they were one face).
- How to rebuild all of it: `tools/voxel3ds/rebuild_mtpyre_relief.sh` (checked: it reproduces the committed
  relief.bin byte for byte).
- Left, small: a green square on one wall top east of the upper stairs.
### 2. Route 120 south-west sky triangle (POL-56, Brennan 14:23)
Picture: `polish_reports/OVN_02_route120_sw_gap.png`.
- Cause: the map's south edge cell (10,99) slopes a level down across the cell (48 -> 32 px) over a belt cell of
  trees. The wall that closes a map edge over the background (EdgeOverBelt) refused tree cells, so the triangle
  between the sloped edge and the flat tree ground showed sky. Now a tree cell south of an edge is closed too.
- Checked unchanged: Lavaridge, Route 115 (side by side; Route 115's colour difference is the test's fixed midday
  clock, not this change).
- Revert switch: WJPC_EDGE_SKIP_WOODS=1. Commit f18189f1.
### 3. Route 121 east: strip of sky by the sea (POL-58 part, Brennan 14:23)
Picture: `polish_reports/OVN_03_route121_sea_edge.png`.
- Cause: Lilycove's west edge stands over the background sea north of Route 121; the wall that closes a map edge
  over the background refused sea cells, so the edge was open down to the water. Now an edge over the sea is closed
  down to the water's level.
- This touches every map edge over sea, so it was compared with it on/off at Route 121, Mt Pyre outside (both
  coasts) and Route 109: only Route 121's strip changed (Mt Pyre / Route 109 identical but for the moving sea).
- Revert switch: WJPC_EDGE_SKIP_WATER=1. Commit 2cce44aa.
### 4. Ever Grande City east edge: thin rock frame standing out of the sea (Batch 4 audit)
Pictures: `polish_reports/OVN_04_evergrande_post_BEFORE_AFTER.png` (close-up), `OVN_04_evergrande_BEFORE.png`,
`OVN_04_evergrande_AFTER.png` (whole view, warp `0 8 18 41`, midday).
- Cause: two relief cells on the map's east edge, (38,25) and (39,24) (metatile 0x0BE), were left by the generator at
  0 px while all the rock round them has fallen to -96..-112 px. Their cliff walls stood up as a thin post and bar
  over the sea.
- Fix: both laid as one surface with their neighbours (`blend_relief_region.py 9 38 38 25 25`, and for the edge cell
  `blend_relief_region.py 9 39 39 24 24 --cols`, new option: blends down the column, as the edge cell has no east
  neighbour). Only these two cells changed; the edge is closed (no sky) in the after picture.
- Checked first that my edge changes (fixes 2 and 3) were not the cause: the post is the same with
  WJPC_EDGE_SKIP_WOODS=1, and the relief data shows the spike directly.

## Questions / proposals held for Brennan (so far)
- Q1 Route 120 north bridge (POL-54): at midday I could not find the sky gap or doubled bridge under the main bridge
  over the pond; the dark band under it looks like the bridge's own shadow. Where exactly is it (a picture with a box)?
- Q2 Route 121 pier stairs (POL-51 "doubled stairs", `OVN_Q2_route121_pier_stairs.png`): the original map has two
  flights (rows 10-11 and row 13) with ONE row of grass between them. Your 21:17 rule keeps one grass row at the top
  of every flight, so the lower flight has no room to be set back into the cliff and lies as a flat pale face.
  Options: (a) treat both as one flight from the top grass to the pier, the middle grass row becoming steps (one
  clean staircase, but the middle grass landing is gone in 3D only; walking is unchanged); (b) let only this lower
  flight cut into the landing row (no top landing for it); (c) leave it. Affects only this spot.
- Q3 Route 121 northern woods (POL-58 "mismatched northern trees", `OVN_Q3_route121_north_trees.png`): the woods
  north of Route 121 mix the tall pine-like trees (left) and round leafy trees (right). Which one should the whole
  wood use? Changing the background's tree type is a look decision, so I left it.
- Q4 Fortree's big fern/palm clumps (POL-52) and Route 120's wide shrubs (POL-55): these need new 3D models (taller
  ferns, 3D bushes) - a design job. Do you want me to model them in the same style as the existing 3D trees (cards
  of the original art, leant like the trees), or something else?
- Q5 Route 119 walkways (POL-53): making them lower with thin posts means changing those building models (a
  model regeneration, Emerald 3DS's generator). OK to do that?
- Fences on Route 121 east (POL-58): at midday they already stand upright (`OVN_03_route121_sea_edge.png`). If you
  still see flat ones, where?
- Berry seeds (POL-49): in the game, but my test save has none planted, so not seen yet.
- Q6 Ever Grande City east side (Batch 4, `OVN_Q6_evergrande_ridges.png`): dark striped walls stand in the rock
  slopes (red boxes, both sides). Cause: the generator gave the diagonal rock slope (metatiles 074/07D/075, x34-38
  rows 43-53 and the matching west side) a jagged mix of heights - single cells at 0..+16 px among rock at -80..-96,
  each with its own cliff walls. Options: (a) re-lay those slopes as smooth steps falling to the sea (like the Mt Pyre
  corner, region by region; changes how Ever Grande's mountain sides look); (b) only flatten the worst single spikes
  (fewer dark walls, the slope stays bumpy); (c) leave it. Affects only Ever Grande's mountain sides.
- Q7 Mossdeep south (`OVN_Q7_mossdeep_trees_water.png`): the trees by the south shore stand on light water. The map
  itself has those trees growing out of the shallows (tree-in-water metatiles 2F0/2F1/2F8/2F9 among water 19E), so
  this matches the original. Keep, or give them a patch of ground?
- Q8 Route 126 (`OVN_Q8_route126_crater.png`): the floor inside Sootopolis's crater is a dark purple surface (it is
  the map's own tile, cloud shadows cross it, not sky), and a thin red line runs along the crater's west wall at the
  water. Both look like the original tiles; say if either looks wrong to you.
