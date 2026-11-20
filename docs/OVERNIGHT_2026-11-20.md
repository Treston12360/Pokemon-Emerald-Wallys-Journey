# Overnight log, 2026-11-20 (session 014PwRDj) - for Brennan

Rules (his 02:48 message): safe POL fixes only, BEFORE/AFTER PNG for each, questions held until "I am awake now".

## Changelog

## Questions / proposals held for Brennan
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
