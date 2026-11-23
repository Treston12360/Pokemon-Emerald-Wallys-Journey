# Overnight work 2026-11-22 → 23 (Brennan 22:03: "keep working continuously ... until I reply with I am awake")

Each fix: picture in this folder (BEFORE left, AFTER right) and what caused it / what changed.
Questions are held in the numbered list at the bottom.

## Fixes

### N01 Fallarbor north woods: level with the mountain, thinned (Brennan 21:54)
Picture: `2026-11-22_N01_fallarbor_north_woods.png`
- Cause: the wood past Fallarbor's north edge stood at 32 px and was rounded down going north (the belt's
  "mountain round-off"), so it sat in a pit under the cliff top (64 px). Every cell of it had a tree.
- Change: its top is 64 px (the cliff top's height) and stays level for 10 rows before rounding off; about half its
  trees are left out by place (6x3-cell clearings and single trees), the clearings showing the soil. Route 113's
  north wood thinned the same way. Gap test (34 north views, 1920x1080): unchanged.

### N02 Route 114 west: stretched cliff ribbons, water drop, sky gap (Brennan 21:56; also C8)
Picture: `2026-11-22_N02_route114_west.png`
- Cause: nothing told the 3D view what lies west of Route 114, so it blended Route 114's mountain (2 to 8 levels
  high) with Route 115's sea level - long stretched slopes running down into open water, with sky through the
  seams.
- Change: the west side is now filled in: the mountain ground carried on level with the route's own heights (the
  wood at the north end at the same level, thinned), and a field point under such a fill stands at its height.
  Gap test: no new gaps on Route 114/115.

### N03 Fallarbor Contest Hall rocks in 3D (C7)
Picture: `2026-11-22_N03_fallarbor_contest_rocks.png`
- Cause: the four mossy boulders beside the Contest Hall are part of the ground drawing, so they lay flat.
- Change: each stands as a rounded rock mound in the same style you picked for Rusturf Tunnel (D: rounded, lower),
  with plain soil under it. The rest of C7 (the sky gap and cut by the tree row north of Fallarbor) was closed by
  22d and N01.

### N04 Meteor Falls: ledge spikes, 3D rocks, 3D craters (C9)
Picture: `2026-11-22_N04_meteor_falls.png`
- Spikes - cause: along each diagonal ledge every cell was its own little ramp, and the next row's ramp started one
  cell further over, so where two rows met their edges were up to 8 px apart; the game stood a thin triangle in each
  of those seams. Change: those seams are welded to one height round the ledge tiles; real cliffs are untouched.
- Rocks: the small floor rocks stand as rock mounds (your style D).
- Craters: their rim now rises round the bowl.
- Still to do here: the dark hole under the east end of the bridge and the two thin dark upright panels by the
  bottom-left drips.

## Where every list item stands (checked tonight against the notes)
Done before tonight (in 22b/22c/22d): B1 Rusturf tint and 3D rocks, B2 Verdanturf woods, B6 winter walls, B7 Route
104, C2 gatehouse winter walls, C3 Dewford sea, C5 flower shop, C6 Toxtricity, D2 Route 119 tree rows, E1/E2 stalls,
the `exe+198bf0` intro error (NULL check in m4a_pc.c), the 3D cable car and line, the 3D Fiery Path and its back
wall, Route 117 / Route 112 / Trainer Hill / Verdanturf items of the 17:25 list.
Done tonight: N01-N04 above (Fallarbor woods, Route 114 west = C8, Contest Hall rocks = rest of C7, Meteor Falls = C9).
Waiting on your eyes (fixed on the test machine, your GPU decides): B3 Pokémon Center lines and B5 Oldale lines
(the 1/8-pixel texture inset in 22b), B4 Route 119 grass lines.
Not reproduced here, need a closer picture from you: C1 Route 103 line, C4 Rustboro sky hole, D1 Weather Institute
east end (reproduced on 22a, solid since).
Blocked: falling leaves with fast-forward - that code is ARIA's file (src/wally_journey.c), the request is written
in docs/WORK_CLAIMS.md for Treston.

## Questions (numbered, for when you are awake)
1. N02 Route 114 west: the plateau west of Route 114 is plain mountain ground. You asked for scattered mountain trees on it too - want a few (like the thinned woods at its north end), or keep it plain?
2. N04 Meteor Falls: the 3D floor rocks come out darker than the flat drawing was (they are lit like the Rusturf ones). Keep them darker, or brighten them to the drawing's own grey?
3. N04 Meteor Falls: the dark patches by the east end of the bridge and the two dark upright panels by the bottom-left drips are the cave's own diagonal rock drawings (dark blue with black in the 2D map) standing as slopes; I tried flattening and welding them and it only made new slivers, so I left them. Do you want them redrawn lighter (as plain rock), or left as the 2D map draws them?
