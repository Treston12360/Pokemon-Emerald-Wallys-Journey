# New Mauville in 3D (POL-31): where we left off

Project Polish (Brennan), branch `polish`. This is the reference to use if New Mauville needs more changes.
Brennan, 2026-11-17 18:02: "from what i can see in the test picture things look to be fine, if there are anymore
things to fix i will let you be aware. put all notes and fixes for new mauville in the github so if we need more
changes we have it for a reference of where we left off."

## State

| Part | State |
|---|---|
| Glare (Gamma lighting) | Done. Brennan: "Good as is". |
| Inside (layout 185, `LAYOUT_NEW_MAUVILLE_INSIDE`) walls | Done. Every wall is 24 px, the tops are even, and the walls south of the barriers are whole (17:06 fix). |
| Props (machines, computers, shelves, cabinets, crates, tanks, generator, bench) | Done. They are boxes, and a prop standing against a wall leaves that row to the wall. |
| Electric barriers (blue and green) | Done. Each is a solid bar 8 px wide and 24 px tall. It shows only while its corridor cell holds the lit bar, and the hidden state is handled. |
| Floor switches | Flat, as bright as the original art. Brennan's fallback: "If you can't get it to work right then we can leave them flat on the floor and as bright as the original art". Raised ones rendered dark. They can be turned back on with env `NM_RAISED_BUTTONS=1` at generation. |
| Entrance (layout 184, `LAYOUT_NEW_MAUVILLE_ENTRANCE`) | **Not modelled yet** (flat). Ask Brennan before starting. |

## Brennan's requests, in his words (in order)
- 11-17 10:43: "work on the new mauville glare and 3d generation first since new mauville is all flat still".
- Glare: "Whole room too bright". Then on gain 0.7 and bloom x0.35: "Good as is".
- Models wanted: machines and computers, electric barriers, floor buttons/switches, walls ("Walls look wrong").
- Walls: "Too short", "Try it this way, I want to see how it will look. They just look flat like 2D and not 3D."
  First walls picture: "Yes, keep going".
- Switches: "Try to keep them raised and brighten them. If you can't get it to work right then we can leave them flat
  on the floor and as bright as the original art."
- 16:00 (`polish_reports/BREN_1600_POL-31_new_mauville_walls.png`): walls short or cut off, a cabinet cut off
  at the top with the wall behind it short. "raise them to be even across the top".
- 17:06 (`polish_reports/BREN_1706_POL-31_south_of_barriers.png`): "to the south of each switched walls the wall
  to the bottom of it is still a half wall ... check every wall in new mauville and check if their tops align with
  one another make it make sense and look clean."
- 18:02: looks fine (above).

## Pictures (`docs/polish_reports/`)
- `POL-31_new_mauville_now.png`: before any work.
- `POL-31_new_mauville_glare.png`: the glare fix.
- `POL-31_new_mauville_3d_walls.png`: first walls.
- `POL-31_new_mauville_barriers.png`: the barriers.
- `POL-31_new_mauville_switches.png`: the switches.
- `POL-31_new_mauville_walls_even.png`: the 16:00 fix.
- `POL-31_south_of_barriers_fixed.png`: the 17:06 fix (the latest).

## Code and data

### Glare
- `src/platform/wide_view.c`: `WideView_GammaRoom()` returns 4 for `MAP_NEW_MAUVILLE_ENTRANCE` and `_INSIDE`.
- `src/platform/voxel3d/voxel_gl.c`: in room 4 the colour gain is 0.7 (env `WJPC_ROOM_GAIN` to try others), and
  bloom is multiplied by 0.35.
- `src/platform/gamma2d.c`: `job.gain = ... room == 4 ? 0.7f : ...` (the 2D path).

### Models
- `tools/voxel3ds/new_mauville_spec.py`: a copy of the spec, and the one in git. The live copy is in the Emerald
  3DS generator: `3ds_port/scripts/voxel_building_specs.py` (`new_mauville_inside()`,
  `new_mauville_inside_alt()`, and the `NEW_MAUVILLE_*` constants). That file is not in git. Copy the section
  back into the tools file after every change; the section runs from "# Wally's Journey (POL-31, Brennan
  2026-11-15: New Mauville" to "# Fortree's gym".
- The generator's `MAX_TEXTURE` was raised to (512, 1024), because New Mauville's page is 512x536.
- `pc_data/voxel/buildings.bin`: the built models, with layout 185 spliced in.

### How to rebuild
```
cd 3ds_port/scripts
python3 gen_voxel_buildings.py --output DONOR.bin        # about 20-28 min
cd ../..
git checkout pc_data/voxel/buildings.bin
python3 tools/voxel3ds/splice_buildings.py --layout pc_data/voxel/buildings.bin DONOR.bin OUT.bin 185
cp OUT.bin pc_data/voxel/buildings.bin
```
- Preview mode (`--only NAME --preview DIR`) exits 1 on the interior lint (dark sides and stripes). `--output`
  passes.
- buildings.bin grows by about 1.3 MB per splice, because the old pages are left in it.

### How the model is built (new_mauville_inside)

**Metatiles:**
- Void 0x208, floor 0x210.
- 0x21f is a wall top: rows 0-3 black, row 4 outline, rows 5-7 cream, olive face from row 8.
- 0x227 is the wall face.
- 0x237 is drawn the same as 0x21f but is walkable, with floor in rows 0-3.
- Thin north-south walls are drawn 8 px wide down one half of their cells, with the other half black
  (0x268/270/278/280, 0x26a/272/27a/282). Where a thin wall meets an east-west wall, the cell is half each
  (0x28d/295, 0x28e/296).
- 0x290 is two thin walls side by side.

**Walls:**
- A wall is every blocked cell that is not void, not a barrier corridor cell, not a prop, and not the barrier's
  Top or TopMid.
- Walls are built per half cell (8 px):
  - An all-black half is no wall.
  - Each half-column's run is split at its foot (the face colour over the outline in row 15), so every front
    is the drawn face.
- A run's top:
  - If the cell north of it is walkable, the run takes that cell's non-floor bottom rows (`ext`, for 0x237).
  - Otherwise the first cell's black top rows are trimmed (`black_top_h`).
  - The barrier's Bottom cell starts at row 4.
- Every wall is 24 px tall, with its side a patch of wall face (`NEW_MAUVILLE_SIDE`).

**Props:**
- Props are groups of prop cells, built as boxes. Their heights come from `NEW_MAUVILLE_PROPS`: shelves 20,
  computers 16, cabinets 14, crates 10, tanks 24, generator 24, bench 8.
- A prop whose north cell is a wall is "against" it: the wall keeps that row and runs whole behind the prop.

**Barriers:**
- `NEW_MAUVILLE_BARRIERS` holds the 7 barriers, each as (x, y, top metatile). Blue is 2b7/2bf/2c7/2cf and
  green is 2b6/2be/2c6/2ce.
- `bar_X_Y`: the rect (x*16, y*16, x*16+8, (y+3)*16+4), 24 px, shown while the corridor cell (x, y+2) holds
  the lit bar.
- `block_bar_X_Y`: the north wall's east half in the bar's column.
- `new_mauville_inside_alt` `block_hid_X_Y`: the north wall's west half while the barrier is hidden (0x269).

### Tried and dropped
- **Scaling relief heights:** New Mauville has no relief cells. Reverted.
- **Raised switches** (dome, then a block): they rendered dark. They are flat by Brennan's fallback.
- **Running the bar on to the south wall's foot:** the bar went dark.

## Testing (Linux build of the Windows exe under wine)
- Warp `24 53 32 32` (New Mauville Inside) works; the picture is taken from there.
- Some coordinates make the game exit when you warp to them (for example 15,22 in New Mauville).
