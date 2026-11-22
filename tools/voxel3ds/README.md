# 3D data (the `voxel/` folder) — how it is made

The PC game's 3D buildings/terrain come from **Emerald 3DS's own generators**
(github.com/ZallaxDev/emerald-3ds, `3ds_port/scripts/`), run against THIS repo's map data.
The output folder `voxel/` (buildings.bin, relief.bin, regions.bin, signposts.bin,
trees.rgba5551, gamma_clouds.bin) is NOT in git (≈45 MB). Every release zip must ship the
current one — on 2026-11-13 a zip went out with an old copy and broke the lighting and furniture.

## Setup (what session 019khs4X used)
1. Clone emerald-3ds; copy this repo's `data/ graphics/ include/ src/ tools/` next to its
   `3ds_port/` (so the scripts read OUR maps).
2. Apply `our_changes.patch` here (`patch -p1` from the emerald-3ds root):
   - Lavaridge gym exterior spec (DLX-1), WJ layout reader fixes (skip layouts it can't read),
   - `PLAIN_FURNITURE` + `dewford_hall()` (DLX-28, **WIP, not yet generated or checked**).
3. `cd 3ds_port && python3 scripts/gen_voxel_buildings.py --verify --output romfs/voxel/buildings.bin`
   (≈6 min). Preview one room: `--only dewford_town_hall --preview OUTDIR`.
4. relief/regions/trees: the other `gen_voxel_*.py` the same way (see 3ds_port/full.mk rules).
5. `gamma_clouds.bin` is Gamma's cloud texture (docs/GAMMA_LIGHTING_VALUES.md) — copy from the
   last release zip.

## Route 119 relief (POL-36, 2026-11-15, session 014PwRDj)
relief.bin generated against OUR tree (2026-11-14, in git as pc_data/voxel) lifted Route 119's ground into
tall walls (bridges in pieces, Weather Institute flat, terrain "shooting up into the sky"). The relief.bin made
by Emerald 3DS's builder from a vanilla Emerald ROM (2026-11-12) draws Route 119 right. Only 5 layouts differ
between the two: Lilycove, Route 106, 119, 120, 121. Route 106's map is changed in WJ (keep ours); the other four
maps are byte-for-byte vanilla, so their records were spliced in from the vanilla-built file with
`tools/voxel3ds/splice_relief.py OURS DONOR OUT 35 36 37 6` (round-trip checked: with no layouts it rewrites
OURS byte-for-byte). **Whoever regenerates relief.bin from our tree must re-run that splice** until the
generator is fixed for these maps (likely cause: WJ's Fortree/General tileset art and metatile layers differ
from vanilla - pokeemerald-expansion lighting - and the generator reads the drawing).

## Weather Institute exterior (POL-36, 2026-11-15, session 014PwRDj)
Emerald 3DS has no model for the Weather Institute on Route 119 (drawn flat on the ground). Spec + model:
`tools/voxel3ds/weather_institute_spec.py` (paste into voxel_building_specs.py as it says). A whole buildings.bin
made from this session's 3ds_port does NOT match pc_data's (53 models fewer), so only this placement was moved in:
`tools/voxel3ds/splice_buildings.py OURS DONOR OUT 35 1 26` (DONOR = a buildings.bin generated with the spec).
The renderer binds one page per map, so the tool replaces Route 119's page with the donor's (same drawings, checked
texel by texel, plus the Institute's) and moves the page's other models to their new place; round trip checked.
Whoever regenerates buildings.bin must add the spec first.

## Trainer Hill (POL-5, 2026-11-16, session 014PwRDj)
Spec changes in `tools/voxel3ds/trainer_hill_spec.py` (a door model on Route 111 - Brennan's save runs the
Route111_NoMirageTower layout 392, so the model is placed on 27 and 392 - and the lobby room: back wall 20 tall on
row 132 instead of 16 on 128, the healing machine as a box on the counter). Spliced into pc_data/voxel/buildings.bin:
`splice_buildings.py OURS DONOR OUT 27 30 112`, again for 392, then `splice_buildings.py --layout ... 414` (the
whole room: every piece, a page of its own). relief.bin: `patch_relief_cells.py relief.bin 27 0 30,113 31,113
32,113` and the same for 392 (the model's foot cells; their relief lifted the model 8 px - a light line at its foot).

## Game Corner prize machines (POL-9, 2026-11-16, session 014PwRDj)
Spec change in `tools/voxel3ds/game_corner_spec.py` (the three "plant" cards and the purple machine painted on
the wall become one box, as the arcade cabinets beside them). Spliced: `splice_buildings.py --layout
pc_data/voxel/buildings.bin DONOR OUT 91` (DONOR = a full gen_voxel_buildings.py --output with the spec).

## Route 121 railing corner (Q10, 2026-11-20, session 014PwRDj)
Brennan 12:27: "one of the fences knocked over, this needs to be upright" (Route 121, cell 73,4, metatile 0x341 of
gTileset_Lilycove: a railing piece with the post that turns down the column). The spec "railing_lilycove" in
`3ds_port/scripts/voxel_building_specs.py` lacked 0x341, so the cell was drawn flat. Change (make it in any regenerated
spec): `"tiles": {0x249, 0x250, 0x251, 0x252, 0x258, 0x25A, 0x341}`. Generated a full donor
(`gen_voxel_buildings.py --output DONOR`, 6 min) and spliced Route 121 whole: `splice_buildings.py --layout
pc_data/voxel/buildings.bin DONOR OUT 37`. Checked first that the donor's Route 121 differs from ours only in the railing
blocks (7 of ours -> 3 donor 4x4 blocks at 64,4 / 68,4 / 72,4; the other 49 placements at the same places).

## Fortree palms bigger (Q4, 2026-11-20, session 014PwRDj) - APPLIED
Brennan (POL-52, 2026-11-19): Fortree's palms "taller, larger ... so they rise above the surrounding regular trees";
Q4 answer 2026-11-20: "flat cutouts of the original art, leaning like the trees". Change in
`3ds_port/scripts/voxel_fortree.py` `palm()`: the card (PALM_TILT, the drawing projected) is emitted as
`ScaledProjPoly`, grown `PALM_SCALE` times (env `WJ_PALM_SCALE`, default 1.5) about the trunk's foot (W/2, 0, H), each
corner keeping the unscaled texel (u = x, v = z - y). Donor generated in full; Fortree (layout 5) spliced whole
(`splice_buildings.py --layout ... 5`; donor's Fortree has the same 66 placements as ours).

## Route 122 relief (2026-11-20, session 014PwRDj) - IN TEST
Brennan 12:30: "work on route 122 and slateport". Route 122 (Mt Pyre's island seen from the route) was left flat:
gen_voxel_relief.py's DRAWN_EXCLUDED has "route122" ("a massif of one band tile: counted a level a band it spikes").
Patch: `drawn_ok()` and the candidates list accept `VOXEL_DRAWN_OK=group,group`. Run:
`VOXEL_DRAWN_OK=route122 VOXEL_SPREAD_OK=route122 python3 scripts/gen_voxel_relief.py --layouts LAYOUT_ROUTE122
--output X.bin` (about 6 min), then `splice_relief.py pc_data/voxel/relief.bin X.bin OUT.bin 38`.

## Route 119/120 shrubs and taller short palms (2026-11-20, session 014PwRDj) - APPLIED
Brennan 16:11 (shrubs picture) / 16:23 (short palms "raise the 3 ... the trunk ... still hides behind the forest
trees"). In `3ds_port/scripts`: `voxel_fortree.shrub(layout)` (the Fortree shrub 281-283 over 270-272 as one leaning
card of its upper layer, grass pixels clear) with one spec per shrub (`wj_shrub_*`, 5 on Route 120, 4 on Route 119;
Route 119's two wider ones done too as wj_shrub4_*, voxel_fortree.shrub(layout, 4), spliced --layout 35 2026-11-20 19:3x); `palm(..., extra_trunk=1)` for fortree_palm_short (one more trunk
cell, 0x273's lower layer, foot kept on the rect's last row). Full donor; spliced `--layout` 5, 35, 36.

## Fiery Path in 3D (2026-11-22, session 014PwRDj) - IN TEST
Brennan: Fiery Path "into full 3D cave walls, raised ledges and 3D boulders". Emerald 3DS has no 3D Fiery Path and its
drawn-relief solver does not know the cave's tiles (Lavaridge 0x268-0x313), so the cells are made by
`tools/voxel3ds/fiery_path_relief.py pc_data/voxel/relief.bin` (needs 3ds_port/scripts for voxel_relief_fixes; replaces
layout 293's cells, sets its belt to 32 px and its drawn flag). **Re-run it after regenerating relief.bin.** Options:
--face (px a cell of face, 16), --belt (32), --boulder (10).
