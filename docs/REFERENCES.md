# Reference sources — check these before building anything they already do

Owner rule (2026-11-13): **never guess and never pretend.** If Emerald 3DS or Gamma Emerald already does it,
read how they do it and port that exactly. If neither covers it, or you are unsure, ask the person you work for.
Never claim you can't reach these files without checking first.

## Emerald 3DS (3D world, buildings, terrain, furniture, water)
- Source: **https://github.com/ZallaxDev/emerald-3ds** (public). Clone it:
  `git clone https://github.com/ZallaxDev/emerald-3ds`
- What we use: `3ds_port/src/3ds_video.c`, `3ds_port/src/voxel/*` (renderer reference),
  `3ds_port/scripts/*` (the 3D data generators: `gen_voxel_buildings.py`, `gen_voxel_relief.py`,
  `voxel_building_specs.py` …), `docs/VOXEL_STATUS.md`, `docs/DEWFORD_3D.md`.
- Our changes to its generators: `tools/voxel3ds/our_changes.patch`; how to run them: `tools/voxel3ds/README.md`.
- Release downloads (3DS builds): https://github.com/ZallaxDev/pokeemerald-3Ds-dualscreen/releases

## Gamma Emerald (lighting, Gamma looks)
- The developer gave Treston the full unpacked project and permission to use it in this game. It is **not on
  GitHub** (about 7.5 GB in 150 MB parts — over GitHub's 100 MB file limit, and this repo is public).
  **Ask Treston for the archive** ("Gamma-Pak-Recovery_part01..17.rar"); he can attach it to your session.
- Already extracted into the repo: every lighting value we use, with where it came from —
  `docs/GAMMA_LIGHTING_VALUES.md` (day/night, interiors, Pokémon Center lights, …).
- Tools to read its Unreal files: `/docs/GAMMA_LIGHTING_VALUES.md` explains the dump tool and the `.usmap`
  used (UE 5.6); the maps need their `.uexp` halves.

## FireRed / Kanto (Project ARIA)
- FireRed's maps, scripts and graphics are in this repo (`data/maps/*_Frlg`, `*_frlg` tilesets).
- Pokémon Crossroads (Kanto + Hoenn in one Emerald game, permission from eonlynx):
  https://github.com/eonlynx/pokecrossroads
