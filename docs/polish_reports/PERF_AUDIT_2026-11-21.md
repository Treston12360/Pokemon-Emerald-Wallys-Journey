# Frame-time audit (Brennan 2026-11-21 01:56) - session 014PwRDj

Brennan's request: before coding, find why `frame` sits at 11-14 ms, why `maps` spikes to 15-72 ms
(`GL waits 1`), and whether `meshes`/`pages` (10 -> 28) need LRU eviction. The target is a locked
240 fps (<= 4.16 ms) at 4K. His log, report, pictures and video are in `docs/brennan/2026-11-21_0156/`.
The perf report for the belt is `PERF_BELT_2026-11-20.md`.

## How to read the stall log (sdl2.c StallLog)
- A **trip** is one pass round the main loop, i.e. one presented picture.
- `frame` is the time in `VDraw`. VDraw draws one **game frame** (the GBA is 59.7 Hz, a fixed timestep).
  It runs only on trips where a game frame is due: about 60 a second.
  - At 240 Hz, one trip in four carries a VDraw, and the other three only present.
- A `minute` line is **one trip** (whichever trip crossed the minute), not an average.
- `loops` gives the real rate:
  - 6893 trips in the first 60 s is about 115 a second.
  - Between minutes the count rises by 6000-7400, i.e. 100-125 trips a second.
  - So the game runs at about 115 fps, not 240. The VDraw trips (12-14 ms each) take the time.

## 1. The 11-14 ms `frame` baseline
VDraw = `2D picture` (~7 ms) + `3D` (~3 ms) + converting and uploading the 2D pictures (~2-3 ms).

**The 2D picture is mostly wasted work in 3D.** `wide_view.c` DrawFieldFrame/DrawWideScanline still does the
following for every line of the whole wide view (576x324 plus margins), every game frame:
- renders all four BG layers (the map's BG1-3 included);
- renders every sprite;
- composites the line up to three times (full, world without UI, map for the smooth overlay).

After that, `sdl2.c` VDraw does all of this:
- converts every pixel to ARGB (`ToArgb`);
- copies it twice (`lastImage`, `sFieldImage`);
- uploads it (`SDL_UpdateTexture`).

When the 3D world is on (`s3DFrame`), `DrawScreen` never shows that picture. It shows `sdl3DTexture` and then only the
**UI layer**, and only the GBA-screen part of it: 240x160, plus the map-name band and the HM banner. What 3D needs from
the 2D pass is:
- the UI pixels (BG0 and UI sprites);
- the per-line interrupts and HBlank DMAs the game relies on (`WideView_SkipFrame` already does those without drawing).

## 2. The `maps` spikes (15-72 ms, `GL waits 1`)
Measured with a walking and warping run on Route 121 (`WJPC_VOXEL_LOG=1`). There are three separate causes.

### a. Animated tiles, every 16 game frames, even standing still (~4 ms on the test box)
`voxel_gl.c` AnimateAtlasTiles does three things for **every atlas that shares the current primary tile set**:
- re-composes the changed rows (good);
- then copies the **whole page** and colour-grades the whole page on the CPU (`VoxelGrade_Texels` over
  `VOXEL_ATLAS_PIXELS`);
- then uploads the whole page (`UploadTiled`).

It already knows which rows changed (`rows` bitmask), but it ignores that for the grade and the upload.

### b. Synchronous rebuild of the map underfoot
The current map (instance 0) is built **at once** when it is stale. It goes stale:
- when you step onto a neighbour whose background build had not finished;
- when its atlas **grows**. The log shows "atlas lacks metatile 75 of 0:36: grown".
  - That is Brennan's 244.6 s spike: 72 ms on 0:36.

A growth makes every mesh on that atlas stale. The map underfoot is then rebuilt in one frame. Whole builds measured:

| Map | Build time |
|---|---|
| 0:26 | 58 ms |
| 0:34 | 52 ms |
| 0:35 | 40 ms |
| 0:38 | 27 ms |

### c. Uploads
A finished map mesh is uploaded with one `glBufferData` of up to ~200k vertices (~5 MB) in the frame it finishes.
New or grown atlas pages are uploaded whole. These are the `GL waits`.

**Pre-fetching neighbours exists already.** It builds in the background, nearest first, with a 3 ms budget.
But the build, its upload and the atlas growth all still land on the main thread. And nothing guarantees the
neighbour is finished before the player arrives.

## 3. `meshes` / `pages` 10 -> 28
`voxel_gl.c` keeps map meshes in a fixed table of `MAX_INSTANCES` = 32 slots with LRU replacement (MeshSlotFor
takes the least recently shown slot). The count therefore climbs to the cap and stops there. It does not grow
without bound:
- free memory in the log stays at 1037-1072 MB;
- per-frame work over the table is a 32-entry loop.

A tighter cap would only make returning to a map rebuild it. No change is needed. A cap on GPU bytes could be
added if memory ever matters.

## 4. A finding that changes the 240 fps goal
In 3D the world is rendered **only in VDraw**, i.e. once per game frame (60 Hz). On the in-between trips,
`DrawScreen` re-copies the same `sdl3DTexture` (no interpolation; the 2D picture slides, the 3D does not).

So at any trip rate the 3D world moves at 60 fps. A real 240 fps in 3D means:
- rendering the 3D world on **every** trip;
- with the camera and the people interpolated between the last two game frames;
- at 4K.

That multiplies the GPU's 3D work by four.

## Plan (in order; each step measured with the stall log and WJPC_VOXEL_LOG before the next)

1. **3D UI-only 2D pass.**
   - While the 3D field is drawn, the CPU picture renders only BG0 and UI sprites, only for the rows/columns
     the UI is shown from.
   - Skip ToArgb, the copies and the upload of the unused world picture.
   - Keep the per-line interrupts (as SkipFrame).
   - Expected: 2D picture 7 ms -> ~1 ms, and frame 12-14 -> ~5 ms.
   - Switch: an env to restore the old path.
2. **Animated tiles: changed rows only.**
   - Grade only the changed rows (a cached graded copy of the page).
   - Upload them with `glTexSubImage2D` per row band.
   - Spread atlases over frames.
   - Expected: the 16-frame 4 ms spike -> well under 1 ms.
3. **No synchronous rebuilds.**
   - (a) Atlases made complete for their tile-set pair up front (no growth during play), or grown while the map
     is still a background neighbour.
   - (b) When the map underfoot is stale, keep drawing its old mesh and rebuild it in the background
     (as the belt now does). Build at once only if there is no mesh at all, which only happens behind a warp's fade.
4. **Background map builds on a worker thread.**
   - The worker builds the CPU vertex arrays from a snapshot of the instance's cells, so it never reads live
     game memory.
   - The main thread uploads the finished arrays in slices of a fixed byte budget per trip:
     - `glBufferSubData` into a pre-sized buffer;
     - or a persistent mapped buffer (`glBufferStorage` + `glFenceSync`) where the driver offers GL 4.4;
     - the SDL GL renderer context is a compatibility one, so a fallback is kept.
   - Neighbours are prefetched as soon as they come into reach, so they are resident before the border.
5. **3D every trip (true 240 fps).**
   - Render the 3D world every trip, with interpolated camera and entity positions.
   - Only after 1-4, because it multiplies GPU cost by four at 4K.
   - Needs a GPU-time measurement on Brennan's PC (GL timer queries in the stall log).

Steps 1-3 should bring both idle and traversal frames under ~5 ms on the test box. Step 4 removes the remaining
build/upload work from the main thread. Step 5 is what makes 3D motion 240 fps.

## Also reported 01:56 (queued, `docs/brennan/2026-11-21_0156/report.md`)
- Auto-run from battle too slow.
- NPCs vanishing across Route 110/Mauville and Fortree borders.
- Lavaridge/Route 112 sky flash and shifting props.
- Route 121 black seams at tree/fence bases; berries north of centre.
- Route 120: shadow flicker (video), black lines round tall grass, pond water squares and sky leaks, half
  under-bridge mesh.
- Route 112 stretched spikes.

## Results (steps 1-5 done 2026-11-21, test box: wine + software GL; GPU time not measurable here)
| Step | Change | Measured |
|---|---|---|
| 1 | 2D pass under 3D: UI only | 2D picture 4.8 -> 1.3 ms; pictures identical |
| 2 | animated tiles: changed rows only | the 4.2-5.7 ms every 16 frames -> ~0 |
| 2b | animated pages in a ring of 3 textures | Ever Grande waterfalls: ~180 ms/update stays on the test box (its software GL waits on any texture upload, before these changes too); expected to help real drivers - check with a stall log there |
| 3 | map underfoot keeps its mesh, laid again in the background | Route 121's 72 ms case (atlas growth) off the main thread |
| 4 | map builds on a worker thread (world snapshot, per-thread caches); finish = upload only; warp destination built on it behind the fade | maps behind warps 16-69 ms -> < 6 ms; finishing a map 2-17 -> 0.1-1.8 ms |
| 5 | 3D drawn on every refresh, camera/people eased between game frames | frame N+1 at t=0 == frame N at t=1 (pixel-identical); t=1 shots identical to before |
Every step has a WJPC_..._OFF style switch back to the old behaviour (see the commits).
Not done: persistent mapped buffers (finished maps are one glBufferData each, 0.1-1.8 ms); GL timer queries in the stall log.
