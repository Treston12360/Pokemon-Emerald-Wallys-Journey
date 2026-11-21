#!/usr/bin/env python3
"""Lay Route 119's Acro Bike rails just over the river, in place.

usage: lower_rails.py RELIEF.bin [LAYOUT_ID=35] [OVER_PX=4]
  OVER_PX 0 (Brennan 17:53 "Build real 3D rail models"): every rail cell flat at the water, no ramps - the rails
  are drawn as 3D models over it (voxel_mesh_builder.c EmitRailCell).

Brennan 2026-11-19 (POL-53): the river stands on Route 119 "look like thick raised blocks sitting high out of the
water with exposed boxy side walls. Lower their height profile closer to the water surface"; 2026-11-20 16:11: they
are "the long thin grey/white rails over the river". In relief.bin they were cells at the bank's height (16 px over
the water) with walled cut records. Each rail cell over the water (see TOP / UNDER) now stands OVER_PX over the water
nearest it (the lowest water cell - 0x206, 0x207, 0x170 - within 3 cells), the cells drawn under a rail (its side
and shadow) lie at the water's level, the top rises onto the bank where it meets it, and the cells' cut records (walls)
are dropped. Run on relief.bin as it was before this tool (git show dceeabd4~1:pc_data/voxel/relief.bin).
"""
import json, os, struct, sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
# Brennan 17:01 review: the bank's own cells drawn with a rail's end (cliff 0EC 0EE 0EF 0F0 0F2 0F8 0FA, grass 0F6)
# stay the bank; only the cells over the water are the rail's: its top (0ED 0F1 0F3 0F4 0F7 0FD) and what is drawn
# under it on the water (side, posts, shadow: 0F5 0F9 0FB 0FC)
TOP = {0x0ED, 0x0F1, 0x0F3, 0x0F4, 0x0F7, 0x0FD}
UNDER = {0x0F5, 0x0F9, 0x0FB, 0x0FC}
RAIL = TOP | UNDER
WATER = {0x206, 0x207, 0x170}


def main():
    path = sys.argv[1]
    lid = int(sys.argv[2]) if len(sys.argv) > 2 else 35
    over = int(sys.argv[3]) if len(sys.argv) > 3 else 4
    layouts = json.load(open(os.path.join(ROOT, 'data/layouts/layouts.json')))['layouts']
    lay = layouts[lid - 1]
    W, H = lay['width'], lay['height']
    blk = open(os.path.join(ROOT, lay['blockdata_filepath']), 'rb').read()
    mt = lambda x, y: struct.unpack_from('<H', blk, 2 * (y * W + x))[0] & 0x3FF
    b = bytearray(open(path, 'rb').read())
    n = struct.unpack_from('<H', b, 4)[0]
    for j in range(n):
        l, cells, w, h, off = struct.unpack_from('<HHHHI', b, 8 + 14 * j)
        if l == lid:
            break
    assert not (w & 0x4000)
    unit = 2 if h & 0x4000 else 1
    at = {(b[off + k * 27], b[off + k * 27 + 1]): off + k * 27 + 2 for k in range(cells)}
    get = lambda x, y: [v * unit for v in struct.unpack_from('25b', b, at[(x, y)])]
    level = {}
    for (x, y) in at:
        if mt(x, y) not in RAIL:
            continue
        near = [min(get(i, j)) for i in range(x - 3, x + 4) for j in range(y - 3, y + 4)
                if 0 <= i < W and 0 <= j < H and (i, j) in at and mt(i, j) in WATER]
        if not near:
            print('no water near', x, y)
            continue
        wl = min(near)
        level[(x, y)] = wl
    # the rail's top: OVER px over the water; where it meets the bank (a cell beside it that is neither water nor
    # rail), it rises across its cell to the bank's edge, so it runs onto the bank's top instead of into its cliff
    for (x, y), wl in level.items():
        px = wl if mt(x, y) in UNDER else wl + over
        g = [[px] * 5 for _ in range(5)]
        if mt(x, y) in TOP and over > 0:
            for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1)):
                nx, ny = x + dx, y + dy
                if (nx, ny) not in at or mt(nx, ny) in WATER or mt(nx, ny) in RAIL:
                    continue
                nb = get(nx, ny)
                for k in range(5):
                    # the bank's samples along the shared edge
                    edge = nb[k * 5 + 4] if dx == -1 else nb[k * 5] if dx == 1 else nb[20 + k] if dy == -1 else nb[k]
                    if edge <= px:
                        continue
                    for t in range(5):
                        f = (4 - t) / 4.0       # 1 at the bank's side, 0 at the far side
                        v = px + (edge - px) * f
                        r, c = (k, t) if dx == -1 else (k, 4 - t) if dx == 1 else (t, k) if dy == -1 else (4 - t, k)
                        g[r][c] = max(g[r][c], v)
        struct.pack_into('25b', b, at[(x, y)], *[int(round(g[r][c] / unit)) for r in range(5) for c in range(5)])
    open(path, 'wb').write(b)
    # no walls on the rail's cells (Brennan 17:01: "long, thin black lines/bars" beside the rails were their walls)
    import subprocess
    subprocess.check_call([sys.executable, os.path.join(os.path.dirname(os.path.abspath(__file__)), 'drop_relief_cuts.py'),
                           path, str(lid)] + ['%d,%d' % c for c in level])
    print('rails laid:', len(level), 'cells')


if __name__ == '__main__':
    main()
