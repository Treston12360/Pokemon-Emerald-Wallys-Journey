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

## Questions (numbered, for when you are awake)
1. N02 Route 114 west: the plateau west of Route 114 is plain mountain ground. You asked for scattered mountain trees on it too - want a few (like the thinned woods at its north end), or keep it plain?
2. N04 Meteor Falls: the 3D floor rocks come out darker than the flat drawing was (they are lit like the Rusturf ones). Keep them darker, or brighten them to the drawing's own grey?
