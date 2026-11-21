We tested the latest build across **Route 121, Route 120, Fortree City, Route 112, Lavaridge, Route 110, and Mauville City**. Here is the full list of visual, streaming, and gameplay issues found (marked in the attached screenshots and video):

---

### 1. Gameplay & Global Streaming / NPC Bugs
- **Auto-Run From Battle Is Too Slow:**
  - When triggering auto-run from a battle, the game pauses for ~1 second, pulls up the text box, and then exits. Make auto-run trigger and exit battles **immediately/snappily** without that extra stall.
- **NPCs Disappearing & Reappearing Across Map Borders (Mauville/Route 110 & Fortree City — Last 2 Screenshots):**
  - Look at the last two screenshots (`ROUTE 110` vs. `MAUVILLE CITY` border): the three NPCs inside the **red boxes** appear when standing just south on Route 110, and **completely vanish** the moment you step north across the Mauville City border (and reappear when stepping back).
  - The exact same NPC disappearing/reappearing bug is happening in **Fortree City** and other map transitions.
  - **Fix:** Audit object/NPC culling and `MAP_CONNECTION` event loading across **all towns and routes** so NPCs in connected neighbor maps never pop in and out when crossing map borders.
- **Lavaridge <-> Route 112 Border Streaming Pop-In & Shifting Props:**
  - When walking from **Route 112 into Lavaridge**, the entire west side flashes **blue skybox** for a moment before the rocks load in, and some trees/rocks visibly shift position when walking back and forth between the two maps.
  - **Fix:** Pre-load/keep connected neighbor map geometry seamless across the Route 112 / Lavaridge boundary with consistent world coordinates so zero blue sky flashes and props stay locked in place.

---

### 2. Route 121 (`Images 1 & 2`)
- **Black Lines / Grid Seams at the Base of Trees & Fences (All Small Red Boxes in Images 1 & 2):**
  - Look inside the red boxes around the bases of the trees and along the blue fence line: there are **thin black grid lines/seams** cutting into the ground around tree bases and fences. Remove these base seam lines so trees and fences sit cleanly on the ground.
- **Berry Plants Off-Center North of Dirt Patch (Wide Red Box in Image 2):**
  - On Route 121 East, the 3D Berry plants inside the wide red box are shifted slightly **north (up)** out of the center of the brown dirt soil patch. Center all 3D Berry models squarely in the middle of their dirt patches.

---

### 3. Route 120 (`Images 3, 4, 5, 6` & Attached Video)
- **Tree Shadows Flickering / Z-Fighting Constantly (Image 3 Red Boxes & Attached Video):**
  - As shown in the attached video and the red boxes in Image 3, the **shadows under/around the trees across Route 120 are flickering rapidly** (Z-fighting with the ground plane) even while standing completely still. Stabilize all tree shadow/canopy layers across Route 120 so shadows sit completely still with zero flickering.
- **Black Grid Lines Around Tall Grass (Image 4 — 4 Red Boxes):**
  - Look inside the 4 red boxes in Image 4: the **tall grass patches** have thin black grid lines/borders cutting around their base edges. Remove these black outline seams around tall grass.
- **Missing Water Squares & Skybox Leaks in South & North Ponds (Images 5 & 6 Red Boxes):**
  - **Image 5 (Bottom-Center Red Box):** At the bottom-right corner of the pond, there are missing water squares flickering and showing **blue skybox** instead of water. Fill those squares with solid water.
  - **Image 5 (Far-Right Small Red Box):** There is a small **blue skybox triangle leak** along the far-right cliff/ground seam. Seal that vertex gap.
  - **Image 6 (Bottom-Right Red Box — Northern Pond):** At the bottom-right corner of the northern pond (next to the small rock island), there are **missing water squares** where ground/cliff corners cut into the water instead of clean water tiles.
- **Half-Missing Underwater "Second Bridge" Artifact (Image 6 Under Main Bridge):**
  - Look directly underneath the wooden bridge in Image 6: there is a faint/half-missing **duplicate bridge reflection/mesh** sitting halfway across the water under the real bridge. Remove that broken under-bridge artifact.

---

### 4. Route 112 (`Image 7` — 3 Tall Red Boxes)
- **Stretched Sky-High Rock & Tree Spikes (Image 7):**
  - Look inside all three red boxes in Image 7 on Route 112: massive **stretched vertical pillars/spikes of pink rock, yellow/red cliff textures, and trees** are shooting straight up into the sky in the foreground/midground.
  - **Fix:** Clamp/rebuild those corrupted vertex elevations on Route 112 so all rocks, cliffs, and trees sit flush with the surrounding terrain.
