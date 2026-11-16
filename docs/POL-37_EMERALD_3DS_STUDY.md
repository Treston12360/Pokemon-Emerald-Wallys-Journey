# POL-37: how Emerald 3DS makes its 3D, and what that means for ours

Brennan, 2026-11-16 08:46: "Emerald 3DS doesn't seem to have 3D issues, please research how they were able to
generate everything in 3D so cleanly and use what you learn to help polish our 3D world. please review their code
to learn how they do it, no guessing or pretending". 08:49: "Prioritize POL-37 before anything else".

Study by Claude session 014PwRDj, 2026-11-16. Source read: github.com/ZallaxDev/emerald-3ds at `bcae4b7`
(2026-10-30, the newest on `main` when checked), cloned at `/home/claude/emerald-3ds`. Everything below was checked
by reading their code and docs or by running their own tools on our data, as said for each point.

## 1. How Emerald 3DS makes its 3D (from their docs and code)

- **Nothing is worked out on the console.** Python generators (`3ds_port/scripts/gen_voxel_*.py`) read the game's
  own maps, tilesets and metatiles on a PC and write `voxel/*.bin`; the console only draws what they wrote
  (`docs/VOXEL_GUIDE.md`). Our game uses the same files, made by the same generators (`tools/voxel3ds/README.md`).
- **"The drawing is the truth."** Every model must match its GBA drawing pixel for pixel at 45 degrees (the
  generator proves it and the build fails otherwise); every texel is one pixel; the simplest geometry that follows
  the drawing; a rule names tiles and shapes, never a map; a spot no rule fixes gets a hand correction **as data**
  (`assets/voxel/relief_fixes.json`), never a special case in code (`VOXEL_GUIDE.md`, `.claude/skills/voxelize`).
- **Mountains** (`docs/VOXEL_MOUNTAINS.md`, `voxel_terraces.py`): heights are counted from the tiles (a face tile
  = one level, a band = a 45-degree slope of one level), not solved; each cell is one fixed piece; pieces settle
  together. **Only the maps in `voxel_terraces.GROUPS` get this.** The rest keep the older generic relief.
  Route 118 and Route 119 are NOT in GROUPS ("mountains (group 118 + 119) not enabled", `VOXEL_STATUS.md`).
- **A strict loop for every fault**: mark it on a screenshot, reproduce it with the engine's own code
  (`voxel_engine_view.py`, `voxel_scene.py`), make a checker report it (`voxel_view_scan.py`, `voxel_defect_scan.py`,
  `cracks.py`), fix with a general rule, check every other map for regressions, look on a real console, and report
  numbers. Each map's state is tracked in `docs/VOXEL_STATUS.md`.
- **Seams they handle in code**: positions are stored in 1/512 tile and every draw's corner is added in whole tiles
  from one anchor, so the edge two chunks share is exactly the same number on both sides (`voxel.v.pica`,
  `ctr_voxel.c SetModelView`). Textures are NEAREST and CLAMP_TO_EDGE. Bridges' drawings get a one-texel gutter
  (a copy of their own edge) so a face's edge never reads the next drawing on the page (`gen_voxel_buildings.py
  _guttered`); a model's crop keeps every texel its faces read (`read_crop`).
- **Lighting**: one fixed sun, worked out per vertex on the CPU (`shade`), plus a colour grade and haze in the
  vertex shader. No per-pixel lighting.
- **The console draws 400x240.** A seam a tenth of a pixel wide there is never seen.

## 2. Emerald 3DS is not clean everywhere - it lists the same faults Brennan found

Their own `docs/VOXEL_STATUS.md` (rows quoted as they are):
- Route 118: "Mountain and ledge faults to fix; mountains (group 118 + 119) not enabled ... Console review (New 3DS,
  2026-10-25): many black lines and boxes. Console review (2026-10-30): Black lines on the ledges."
- Route 119: "Mountains (group 118 + 119) not enabled ... dark abutment strips under the south bridge's ends ... the
  Weather Institute is still flat ... faults under the bridges".
- Route 111: "Terrain not right yet ... the mountains east of the desert look like a sheer wall rather than a mountain."
- Mauville Game Corner: marked as having faults ("The top of the prize counter is flat"). Its four prize machines
  at the back are made as flat cards there too (the spec calls them `plant_0..2`, `card=True`).
- Pokemon Center 1F (x15): "The glass table and stools still to review".
- Route 108: "Black lines in the water above and below the rocks". Trick House 2: "Black lines on the walls".
So the places Brennan reported are, for the most part, places Emerald 3DS itself has not finished.

## 3. Checked: is our 3D data the same as Emerald 3DS's?

`relief.bin` (the ground's heights), compared layout by layout with the one Emerald 3DS's own builder made from a
vanilla Emerald ROM (2026-11-12, `/home/claude/vdata/voxel/relief.bin`), and our maps compared with vanilla
pokeemerald (pret, cloned to `/home/claude/vanilla_pe`):
- 62 layouts with relief: byte-for-byte the same as Emerald 3DS's.
- Different only where we meant it: Route 106 (Wally's Journey changed that map) and Route 111 (both layouts: the
  Trainer Hill door cells, POL-5).
- So Route 118's and Route 119's terrain is exactly what Emerald 3DS has.

## 4. Checked: the same scene drawn by Emerald 3DS's own engine code

Their `voxel_engine_view.py view` compiles the console's own C (`voxel_mesh_builder.c`, `voxel_tree.c`,
`voxel_building.c`, `voxel_relief.c`, ...) on the PC and rasterises exactly what the console would build, gaps in
magenta. Run on our `pc_data/voxel` (setup: section 6). Pictures in `docs/polish_reports/POL-37_*`.
- **Route 118 beach (Brennan's red boxes):** in the 3DS engine's scene the sand has **no** dashes, at 4x scale.
  So the dark dashes on the sand in Brennan's picture are made by our renderer, not by the data.
  The small gaps at the foot of the trees by the house ARE in the 3DS scene too (magenta there): data.
- **Pokemon Center 1F:** the 3DS engine's scene has no lines on the floor or walls at 4x scale. So the thin lines in
  Brennan's photo are our renderer's (how it samples the textures at high resolution), not the data's.
- **Game Corner:** same layout of models as ours; prize machines are flat cards in both.

## 5. Where our renderer differs from theirs (code compared file by file)

Same as theirs (no differences): voxel_relief.c, voxel_camera.c, voxel_regions.c, voxel_arena.c, voxel_grade.c,
voxel_sprite_slots.c. Small changes: voxel_building.c, voxel_atlas.c, voxel_tree.c, voxel_lighting.c (all ours,
commented with the POL/DLX that asked). Bigger: voxel_mesh_builder.c (the belt round the map, POL-30, and DLX-26's
skirts round model cells), voxel_world.c, voxel_entities.c. **The whole drawing layer is ours**: `voxel_gl.c`
(OpenGL) replaces their `ctr_voxel.c` (3DS GPU). Ours adds Gamma Emerald's lighting (light per pixel, its normal
taken from screen derivatives), cloud shadows, bloom, depth of field and tone mapping, and draws at up to 4K.
What that means:
- At 4K a sample a hair past a face's edge or a seam a tenth of a pixel wide becomes a visible line. Their
  console never shows these, so their data and code never had to avoid them everywhere. Where they did need it
  (bridges) they used a gutter; we do the same job in the renderer: every face reads 1/32 texel inside its own
  edges (atlas: POL-8 16c; models and the new flower/grass cards: this change).
- Sandstorm (POL-38): we ported their falling sand streaks (POL-23) but not the rest of their sandstorm: the
  sun turned to "a warm smudge", the shadows filled with dust and the distance in ochre (`ctr_voxel.c` lighting,
  VOXEL_WEATHER_SAND), and the low dust sheets sweeping the ground (`DrawBanks` with `sSandBanks`). That is why
  ours "looks weird, not like a sandstorm".

## 6. How to run their engine view on our data (for the next session)

```
E=/home/claude/emerald-3ds/3ds_port; P=/home/claude/wj-3d/3ds_port     # P is not in git (.git/info/exclude)
mkdir -p $P/tests $P/include $P/src/voxel
cp $E/tests/voxel_scene_dump.c $P/tests/; cp $E/src/voxel/*.[ch] $P/src/voxel/; cp $E/src/3ds_video_decode.c $P/src/
cp -r $E/include/* $P/include/
for f in buildings.bin relief.bin trees.rgba5551; do ln -sf /home/claude/wj-3d/pc_data/voxel/$f $P/romfs/voxel/$f; done
cd $P/scripts && python3 voxel_engine_view.py view LAYOUT_ROUTE118 10,11 out.png --scale 4 --dist 14
python3 cracks.py ../romfs/voxel/relief.bin 34     # their "black dash" (open step) finder
```

## 7. What we take from this (plan, in Brennan's priority order)

1. Every report gets the 3DS-engine picture of the same place first: if the 3DS scene has the fault, it is in the
   data (fix the data the 3DS way: a spec, a rule, or `relief_fixes.json`); if not, it is our renderer.
2. Texture edges: the 1/32-texel inset on every face (done for atlas, models, cards).
3. Route 118 sand dashes: ours - find which of our additions draws them (candidates: DLX-26 skirts on the sloping
   beach, the per-pixel normal on thin faces) and compare against the 3DS scene.
4. Route 118/119 "block" mountains: Emerald 3DS has not switched its mountain rules on for these two routes. Trying
   it means adding them to `voxel_terraces.GROUPS`, regenerating their relief and running their checks
   (`voxel_terraces.py LAYOUT --check`, `cracks.py`, `ledgecheck.py`) - their method, but work they have not done.
5. Sandstorm: port the rest of theirs (light, haze, dust sheets).
6. Game Corner prize machines: a box spec like the arcade cabinets', the way their specs make furniture.
