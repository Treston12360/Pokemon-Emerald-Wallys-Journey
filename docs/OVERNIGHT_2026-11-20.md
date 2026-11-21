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
### 5. Mt Pyre summit north-west trench: dark slanted panel (POL-60 "black vertical streaks ... left trench")
Pictures: `polish_reports/OVN_05_mtpyre_summit_trench_BEFORE_AFTER.png` (close-up), `OVN_05_..._BEFORE.png`,
`OVN_05_..._AFTER.png` (warp `24 22 12 24`, clock +6 h from the midday test clock, nearer the light of Brennan's
00:43 picture).
- Cause: in the generator's data, cell column x3 rows 7-12 (metatile 070, between two rock ridges) starts 24 px below
  the ridge west of it and dips to -28 px, a V-trench; its west side stood as a dark slanted wall.
- Fix: those six cells re-laid as one slope between the ridge on each side (`blend_relief_region.py 303 3 3 7 12`;
  added to `rebuild_mtpyre_relief.sh`, which still reproduces relief.bin byte for byte).
- Tried also row 13: it opened a small sky sliver at row 14, so row 13 was left as it was. A small dark triangle stays
  at the foot of the trench (row 13).
- The rest of the summit at this hour (centre, east): no black streaks seen (`OVN_05_..._BEFORE.png` covers the west;
  centre and east pictures are in the scratch folder, nothing to fix).

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
  rows 43-53) a jagged mix of heights - single cells at 0..+16 px among rock at -80..-96,
  each with its own cliff walls. (The west side's dark slots are something else: its heights are a smooth slope with no spikes; cause not found yet.) Options: (a) re-lay those slopes as smooth steps falling to the sea (like the Mt Pyre
  corner, region by region; changes how Ever Grande's mountain sides look); (b) only flatten the worst single spikes
  (fewer dark walls, the slope stays bumpy); (c) leave it. Affects only Ever Grande's mountain sides.
- Q7 Mossdeep south (`OVN_Q7_mossdeep_trees_water.png`): the trees by the south shore stand on light water. The map
  itself has those trees growing out of the shallows (tree-in-water metatiles 2F0/2F1/2F8/2F9 among water 19E), so
  this matches the original. Keep, or give them a patch of ground?
- Q8 Route 126 (`OVN_Q8_route126_crater.png`): the floor inside Sootopolis's crater is a dark purple surface (it is
  the map's own tile, cloud shadows cross it, not sky), and a thin red line runs along the crater's west wall at the
  water. Both look like the original tiles; say if either looks wrong to you.
- Q9 Route 108 / 109 (`OVN_Q9_route108_sea_line.png`; also in the Batch 2 notes as "dark dashes across the sea"):
  a dark zig-zag line runs across the open sea north of Route 108. Cause (measured with WJPC_FIELD_DUMP): the
  background sea round each map lies flat at one level, the level most of that map's own sea has (DLX-26). Route 108's
  own sea is not flat: its relief rises in 3-column steps from -15 px in the west to about 0 in the east, every step
  equally common, so its background sea took the first, -15 px. Route 109's background sea is at 0. Where the two
  meet (a diagonal halfway between the two maps' edges) the 15 px step shows as the dark zig-zag; it has nothing to
  do with the models (same with them hidden).
  Options: (a) lay Route 108's own sea flat at Route 109's level (one data change to Route 108 only; the line goes,
  but the Route 107 side of Route 108 would no longer slope; not yet checked what that does at the 107/108 seam);
  (b) give Route 108's background sea a height that follows its ramp (a code change in how the background sea is laid
  for a map with a sloped sea - only such maps change; the owner chose a flat background sea in DLX-26 to avoid steps,
  so this is a design point); (c) leave it.

## Brennan's answers (2026-11-20 12:27)
Recorded in docs/WALLY_JOURNEY_OWNER_REQUIREMENTS.md ("Brennan's answers to the overnight questions").
Q2 leave, Q3 round leafy, Q4 (a) cutouts like the trees, Q5 OK, Q6 (a) smooth steps, Q7 keep, Q8 keep, Q9 (a) flat at
Route 109's level, Q10 Route 121 east fence upright + coast dirt/sky, Q11 berry seeds 3D in the middle of the dirt.

## Daytime fixes after Brennan's answers (2026-11-20)
### 6. Route 121 east railing corner standing (Q10, Brennan 12:27)
Pictures: `polish_reports/Q10_route121_fence_BEFORE_AFTER.png`.
- Cause: cell (73,4), metatile 0x341 (Lilycove tileset: the railing piece where the rail turns down the column), was
  not in Emerald 3DS's railing spec, so it was drawn flat on the ground.
- Fix: 0x341 added to the spec, Route 121's models regenerated and spliced in (tools/voxel3ds/README.md, "Route 121
  railing corner"). The corner now stands with the rails on both sides.
### 7. Route 108: dark zig-zag line across the sea gone (Q9 (a), Brennan 12:27)
Pictures: `polish_reports/Q9_route108_sea_line_BEFORE_AFTER.png` (warp `0 23 45 6`), `Q9_route108_107seam_BEFORE_AFTER.png`
(warp `0 23 4 8`, the Route 107 side).
- Route 108's relief (sea and land alike) rose in steps from -16 px in the west to 0 in the east; now laid flat at 0,
  Route 109's level (`tools/voxel3ds/flat_relief_layout.py relief.bin 24 0`). Its background sea is at 0 too, so the
  step against Route 109's background sea (the zig-zag) is gone. The Route 107 side checked: no line at the seam.
### 8. Mt Pyre summit: green square on the ridge end made rock (Brennan 12:30 "make it rock")
Picture: `polish_reports/MTPYRE_summit_green_square_BEFORE_AFTER.png` (warp `24 22 25 22`, +6 h clock).
- Cells (27,15) 068 and (27,16) 078, the ridge's west end east of the upper stairs, are drawn as the ridge's rock
  (kArtSwap in VoxelMesh_TileUV; the map itself is unchanged) and laid between their neighbours
  (`blend_relief_region.py 303 27 27 15 16`). The green slope is gone; a faint light line is left at the foot of the
  wall behind it (smaller than before).
### 9. Ever Grande east mountain side: smooth slope down to the sea (Q6 (a), Brennan 12:27)
Picture: `polish_reports/Q6_evergrande_east_BEFORE_AFTER.png` (warp `0 8 18 41`).
- Cells x32-39 rows 33-53 (layout 9) re-laid as one smooth surface between the rock round them
  (`tools/voxel3ds/smooth_relief_region.py relief.bin 9 32 39 33 53`: a harmonic fill on the relief lattice; the map
  edge side held at the straight line between rows 32 and 54). The generator's single raised cells and their dark
  walls are gone; their cut records keep their ground but lose their walls (as on the even slope of rows 14-32).
- West side: done the same way (fix 12).
### 10. Fortree's palms bigger, rising above the wood (Q4 (a) / POL-52)
Picture: `polish_reports/Q4_fortree_palms_BEFORE_AFTER.png` (warp `0 4 19 11`).
- The palms stay Emerald 3DS's leaning cutout of the original art; the card is 1.5 times as big about the trunk's foot
  (voxel_fortree.py palm(), WJ_PALM_SCALE), so the five palms stand above the trees round them. Generated and spliced
  as tools/voxel3ds/README.md "Fortree palms bigger" says.
### 11. Route 121 northern woods: all round leafy trees (Q3, Brennan 12:27 "Round leafy trees")
Pictures: `polish_reports/Q3_route121_woods_BEFORE_AFTER.png` (warp `0 36 65 4`), `Q3_route121_woods_west_AFTER.png`
(warp `0 36 20 4`).
- Cause (WJPC_TREE_DUMP): the background wood north of Route 121 is drawn from the nearest map's border. Its west
  part is nearest Route 120, whose border is the pine-like small tree (0C6/0C7); the east part takes Route 121's
  own round tree (1D4/1D5/1DC/1DD), so the two met in a diagonal.
- Fix: Route 120's east edge (the stretch beside Route 121, rows 20-79) is filled with Route 121's round tree
  (`add_relief_edge.py relief.bin 36 E 0x1d4,0x1d5,0x1dc,0x1dd 20 79`). Route 120's own trees are unchanged.
### 12. Ever Grande west mountain side smoothed too (Q6 (a))
Picture: `polish_reports/Q6_evergrande_west_BEFORE_AFTER.png`.
- Its dark slots were walled cut records on a stepped slope (wall 07C/073, all sides). Cells x0-6 rows 38-56 re-laid
  smooth (`smooth_relief_region.py relief.bin 9 0 6 38 56`), their cut records without walls. One small dark notch is
  left by the flower ledge at the slope's top (outside the block).
### 13. Berry seeds and sprouts sit in the middle of the soil (Q11, Brennan 12:27)
Picture: `polish_reports/Q11_berry_seeds_BEFORE_AFTER.png` (test: `plant 51 1`, `plant 54 2`, warp `0 36 65 11`).
- The 3D seed mound (POL-49) stood on its cell's south edge, at the soil's front. Tried 0.5 (cell middle: the soil bed
  is drawn over two cells, so the sprout's stem sank out of sight), 0.85 and 0.75; 0.75 puts the seed and the sprout
  in the middle of the dirt with the whole stem showing. WJPC_BERRY_SEED_Z / WJPC_BERRY_SEED_EDGE to test others.
- New test command: `plant TREE_ID STAGE` (sdl2.c).
### 14. Route 121 east / Lilycove coast wall: brown rock instead of dark dirt (Q10, Brennan 12:27)
Picture: `polish_reports/Q10_route121_coast_BEFORE_AFTER.png` (warp `0 36 65 4`).
- The wall that closes Lilycove's west edge down to the sea took the cliff face of another map's background terraces
  (a dark one). Over the sea it now uses the map's own cliff face (EdgeOverBelt; WJPC_EDGE_WATER_FACE_OLD=1 reverts).
- Left: a small light triangle at the wall's north-west corner, at the waterline. Tried drawing the north wall from
  both sides: no change (reverted). WJPC_EDGE_DUMP shows the wall itself closed down to the water there; cause not
  found yet.

## After Brennan's 16:22 review
Closed by Brennan: railing (6), Route 108 (7), summit square (8), Ever Grande east (9).
### 15. Ever Grande west: the stepped block at the slope's top (his red box)
Picture: `polish_reports/Q6_evergrande_west_top_BEFORE_AFTER.png`. `smooth_relief_region.py relief.bin 9 7 12 35 43`
(the flower ledge and the rock round it, one slope now; cell 8,40 has no relief record and was left).
### 16. Background woods: each large tree drawn whole at one height
Picture: `polish_reports/R121_woods_whole_trees_BEFORE_AFTER.png` (Route 121 north woods, warp `0 36 65 4`).
- Brennan: "many overlapping trees ... they should be standalone trees making up a forrest"; at the coast "2 random
  tops of trees". Each quarter of a large belt tree stood at its own cell's belt height, so on slopes a tree came
  apart into rows over its neighbours, and at the sea's edge a tree cut in half stood alone. Now a large belt tree is
  drawn only when its four quarters are all there, all at the tree's top-left cell's height (voxel_tree.c
  VoxelTree_EmitBorder; WJPC_BELT_TREE_CELLWISE=1 reverts). Applies to every background wood.
### 17. Route 119's Acro Bike rails lowered to just over the river (Q5 / POL-53)
Picture: `polish_reports/Q5_route119_rails_BEFORE_AFTER.png` (warp `0 34 20 11`).
- Not building models: they were relief cells at the bank's height (16 px over the water) with walled sides. Each rail
  cell now stands 4 px over the water nearest it, the cells drawn under a rail (its side, its posts' drawing) lie at the
  water, and the walls' feet go down to the water (`tools/voxel3ds/lower_rails.py relief.bin`, 82 cells). Thin dark
  edge lines are left at a few rail ends. Separate 3D posts are not added yet (the drawn posts show under the rails).
### 18. Route 119/120 wide shrubs stand as 3D bushes; Fortree's short palms raised (Q4)
Pictures: `polish_reports/Q4_route120_shrubs_BEFORE_AFTER.png` (warp `0 35 19 23`, Brennan's two boxes),
`Q4_fortree_short_palms_raised_BEFORE_AFTER.png` (warp `0 4 19 11`). How: tools/voxel3ds/README.md "Route 119/120
shrubs and taller short palms".
### 19. Route 119 rails v2 (Brennan 17:01 boxes)
Picture: `polish_reports/Q5_route119_rails_v2_BEFORE_AFTER.png`. lower_rails.py v2 on the pre-rails relief: the black
bars (the rail cells' walls) gone; the bank's cliff cells drawn with a rail's end stay at bank height (they had been
lowered: the raised/odd squares); each rail's last cell ramps up onto the bank's top instead of running into the cliff.
### 20. Woods of large trees on their own flat ground (Brennan 17:01: "full, unclipped 3D tree model ... individual")
voxel_mesh_builder.c BeltTreeOwn: a background wood of four-cell trees stands at its edge's level like the woods inside
maps, not on the terraced field (POL-46 rule B kept for small-tree woods). WJPC_LARGE_TREES_ON_FIELD=1 reverts.
### 21. Lilycove / Route 121 coast corner sealed (Brennan 17:01)
Picture: `polish_reports/Q10_coast_corner_sealed_BEFORE_AFTER.png`. The sky triangle was a strip under Lilycove's
north-west wall (the wall stands as far south as it is high) where the belt laid no sea; the sea is now laid into it
(EdgeOverBelt; WJPC_EDGE_SEA_STRIP_OFF=1 reverts). The black squares at the wall's foot are gone with it. Tried the
wall in faces 0A9 (same look) and 075 (striped, worse): kept the map's own 07C.
### 22. Route 119 river: raised water squares flattened, black pole gone (Brennan 17:53)
Picture: `polish_reports/Q5_route119_river_flat_BEFORE_AFTER.png`. Water cells (16,16) and (16,17) stood at the bank's
height with walls (the raised square and the pole); every water cell x3-21 rows 3-23 is at the river's 128 px now and
water cut records there are dropped. Rail ends / hop rails: 3D rail models next (Brennan's answer).
### 23. Route 119 rails as 3D models (Brennan 17:53 / 18:57)
Pictures: `polish_reports/Q5_route119_3D_rails_BEFORE_AFTER.png`, `Q5_route119_rail_posts_BEFORE_AFTER.png`.
Rail cells flat at the river and drawn as river (lower_rails.py OVER_PX 0; art swap to 0x206); each rail cell over the
water gets a 3 px bar at the bank's height (the drawing's white bar for its top) and a dark grey post (the drawing's
post grey) down to the water (voxel_mesh_builder.c EmitRailCell; WJPC_RAIL_OLD=1 off). Hop rails stay separate.
