Here is the next batch of visual and 3D fixes found across **Rusturf Tunnel, Verdanturf Town, Pokémon Centers, Route 119, Oldale Town, Route 104, and Winter House Shaders** (all 10 screenshots attached with red boxes for reference):

---

### 1. Rusturf Tunnel (`Image 1` — 3 Red Boxes)
- **Wrong Green Color Palette (Large Right Red Box):** The entire cave floor and wall coloration inside Rusturf Tunnel is tinted neon/bright green instead of its authentic brown/earthy cave palette from GBA *Pokémon Emerald*. Check the original GBA *Emerald* palette/tileset for Rusturf Tunnel and recolor the cave walls and floor accurately.
- **2D Rocks Still Flat on the Floor (2 Small Center Red Boxes):** The small rocks scattered across the tunnel floor (as well as the breakable Rock Smash boulders) are still flat 2D ground textures—convert them into proper **3D rock models**.

---

### 2. Verdanturf Town (`Image 2` — 3 Red Boxes)
- **Grey Cliff/Ground Blocks Clipping Through West Trees (Wide Left Red Box):** Along the western forest edge, grey/snow-covered cliff blocks are jutting straight through the middle of the 3D trees. Clean up that cliff-to-forest boundary so rock blocks don't clip through the trees.
- **Exposed Dirt Strip Along East Mountain Wall (Tall Upper-Right Red Box):** Between the Poké Mart / Pokémon Center and the eastern tree/cliff line, a thin vertical strip of raw brown dirt ground is exposed. Mesh/blend that ground cleanly into the mountain edge so the bare dirt strip doesn't show.
- **Leftover Flat 2D Tree on the Ground (Small Lower-Right Red Box):** Just south-east of the Pokémon Center (in front of the tree line), there is still a flat 2D tree graphic lying on the ground—remove it.

---

### 3. Pokémon Center Interiors (`Image 3` — 3 Red Boxes)
- **Black Grid/Seam Lines on Floor & Walls Even When Standing Still:** Look inside the three red boxes inside the Verdanturf Pokémon Center (along the left wall/escalator, around the yellow/orange floor cushions, and along the right floor edge): thin **black seam lines** appear even when standing completely still.
- **Fix:** Check for **overlapping/double ground or wall surfaces** (or UV-edge bleed on interior tiles) inside Verdanturf's Pokémon Center, and **audit all other Pokémon Center interiors** across the region for the exact same issue.

---

### 4. Route 119 (`Images 4 & 5` — Red Boxes)
- **Black Lines Around South Tall Grass (`Image 4` — Small Center Red Box):** Along the northern edge of the tall grass on South Route 119 (just left of the cyclist), thin **black border lines** are showing around the grass tiles—check for double ground / UV-edge bleed there.
- **Flickering Trees on Middle-West Cliff Terrace (`Image 5` — Wide Left Red Box):** Along the middle-west cliff ledge (`Image 5` red box), the row of trees is **flickering/strobing** when moving—check for **double ground / overlapping tree-base sheets** at the same height and fix them globally.

---

### 5. Oldale Town (`Image 6` — Center Red Box)
- **Black Lines Appearing Both Stationary & When Moving:** Look inside the red box in the center of Oldale Town (between the Poké Mart, house, and sign): horizontal **black ground-seam lines** show up both while standing still and while moving. Check for double ground / tile-edge UV bleed across Oldale Town.

---

### 6. Winter Season Building Shaders (`Images 9 & 10` — Red Boxes)
- **Keep House Front Walls Their Original Colors in Winter (Only Roofs Should Be Snowy/White):**
  - Look at `Image 9` (Route 104 / Mr. Briney's cottage front wall inside the red box) and `Image 10` (Petalburg City houses inside the 3 red boxes): the winter snow/frost tint is bleaching/washing out the **front walls and facades** of the houses (turning yellow/wood walls pale grey/washed out, or tinting red roofs inconsistently).
  - **Fix:** Check **all houses and buildings across the entire region during Winter**—keep the **front walls, doors, and windows 100% their original non-winter colors** across all seasons. **Only the tops/roofs** of houses should have white snow on them in Winter.

---

### 7. Route 104 West (`Images 7 & 8` — Red Boxes)
- **Replace Floating West Cliff & Trees With Open Sea (`Image 7` — Tall Left Red Box):** Along the western edge of Route 104 (`Image 7` red box), there is a floating strip of vertical white/grey cliff wall and trees hanging over the water. Remove that floating land/tree strip completely so the entire west side of Route 104 is **open blue sea** (matching our Route 110 / Route 117 coastal cleanups).
- **Spikes / Dark Artifacts Jutting Off the North Side of Water Rocks (`Image 8` — 2 Red Boxes):** Look inside the two red boxes in the water on Route 104: the sea rocks have **sharp spikes / dark jagged slivers sticking off their north/side edges** into the water. Clean up those 3D sea-rock meshes so they sit smoothly in the water with no spikes.
