# Project ARIA — working notes for AI sessions

Everything a new session needs that isn't a design decision (those are in `docs/KANTO_PROJECT_ARIA.md`).

## Mockups (owner has seen these)

| File | What it is |
|---|---|
| `varnell_facility_v2.png` | Top: Varnell Institute public floor (FireRed Cinnabar Lab tiles). Bottom: the hidden ARIA core (Emerald Aqua Hideout facility tiles). Owner rejected an earlier Weather Institute version as "too close to the Weather Institute". |
| `bay2_mons_big.png` | The legendary holding bay: four sealed tanks (Dialga, Palkia, Giratina, Hoopa), pods in the walls, central spine to the ARIA core. Owner asked for "bigger, cooler, glass around them". |
| `bay_schemes.png` | The approved recolour direction: **Void** (calm, violet fluid) and **Red Alert** (ARIA firing). Live palette swap = story trigger. |
| `bay_pulse.gif` | Tanks "breathing" (fluid brightness cycles), then the snap to Red Alert. |

Owner rules for these rooms: only FireRed/Emerald tiles; recolours allowed as **new Varnell-only tileset
copies** (never change palettes other maps use); "make it creative, not just another Emerald place".
Mockups are from our renderer, not in-game screenshots. Doorways are plain gaps; no collision/events yet.

### Tools (`tools/`)
- `render.py` — draws any layout or metatile grid exactly as the game does (reads tilesets via the
  tileset headers, handles FRLG 640-tile primaries). Set `WJ_ROOT` to the repo if not run from here.
- `compose.py` — `Map` helper: stamp whole objects from real maps (keeps edges correct), fill, save.
- `front2.py`, `coreA.py`, `bay2.py` — the three rooms. `recolor.py` — Void / Red Alert palettes
  (per-colour rules: fluid glows, screens stay cyan, metal dark with bright edges). `pulse.py` — the GIF.
- Legendaries are drawn from `graphics/pokemon/<name>/overworld.png` (first frame), tinted by the fluid.

## Testing the game (Linux build)

- Build: `make release` **first** (generates map/palette data), then `make -f Makefile.pc`. Makefile.pc
  does not track data files — after changing maps/layouts, `touch data/maps.s data/map_events.s` or the PC
  build keeps old map data.
- Scripted runs: `tools/run_test.sh DIR SAVE [env…]` runs the game with `DIR/s.txt` as a script
  (`lockrng 1`, `wait N`, `press A 5`, `warp GROUP NUM X Y`, `winshot name.ppm`, `quit`; see
  `src/platform/sdl2.c` `TestStep`). Put a `WallysJourney.cfg` in DIR (`world3d=0/1`, `gammaLight=0/1`).
- **Always set `WJPC_TEST_WINDOW=1`** (otherwise frames are skipped: false holes, missing transitions).
- 3D needs the voxel data folder (`WJPC_VOXEL_DIR`). **It is not in git** (generated from Emerald 3DS;
  see DLX notes in `docs/DELUXE_WORK_QUEUE.md`). Without it the game falls back to 2D.
- The owner's test save (`owner_brawly.sav`) is not in git either; ask the owner for a save if needed.
- Map numbers: `include/constants/map_groups.h` is `(num | (group << 8))`; `warp` takes GROUP then NUM.
  Pallet Town = `warp 37 0 X Y` (crashes until K1–K3 are done).

## Crossroads port references

- Repo: github.com/eonlynx/pokecrossroads (Beta 1.4, commit e05c8286). Permission from eonlynx (see design doc).
- `crossroads_commits.txt` — their 22 own commits (everything else in their history is upstream expansion).
- `crossroads_files.txt` — the 213 files those commits touch. Read the relevant ones for each K-task.
- What we already learned:
  - `tools/mapjson/mapjson.cpp`: they removed the three "skip the other game's maps/layouts" filters
    (done on `kanto-port`, K1).
  - `data/event_scripts.s`: they removed `.if IS_FRLG` around the Kanto scripts (done on `kanto-port`, K1).
  - Cross-region Fly: `SetForcedFlightRegion` in `src/region_map.c` + a "Flight Call" key item. We do the
    forced-region part inside normal Fly instead (owner: no key item). Their public code has **no**
    overworld soaring between regions; that's our own later stretch goal.
  - Their known fixes worth reading: PR #44 (FRLG flags moved out of 0x500..0xB55), PR #45 (save within
    32 flash sectors), PR #46 (cross-region script softlocks), PR #47 (HM forgetting, etc.). Fetch with
    `git fetch origin '+refs/pull/*/head:refs/remotes/pr/*'`.

## Owner's style
Plain language, direct, no guessing, call out your own mistakes. Ask before any wording or visual change and
show a picture. The level curve is the top priority. Keep `docs/KANTO_TASKS.md` ticked as you go.
