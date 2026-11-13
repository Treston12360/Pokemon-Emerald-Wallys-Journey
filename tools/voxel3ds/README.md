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
