#!/usr/bin/env python3
"""Birth Island's and Navel Rock's islands raised (relief.bin, in place).

Brennan 2026-11-24 14:28: "Stairs in birth island, birth island still looks 2D", "Naval rock exterior looks 2D" -
"All should be 3D". The game types these islands as rooms (MAP_TYPE_INDOOR), so they got no relief: flat on the sea.
Here, read off the map's own collision:
  sea and pier   level 0 (the pier's cells: walkable at the foot of the island's stairs)
  plateau        the walkable cells up the stairs: PLATEAU px
  rock           a blocked cell that is not sea: a cliff face rising from whatever stands south of it (sea, the
                 plateau) - FACE px for each rock cell between it and that foot, up to MAX cells - so a cliff drawn
                 three cells tall is three courses tall, and the rock ring round Navel Rock's sand stands over the
                 sand
  stairs         (STAIRS) laid as a cliff face, so they climb from the pier to the plateau (the renderer stands
                 them as real steps: voxel_mesh_builder.c CaveStairCell)
A cell's 5x5 lattice runs between its corners; each corner is the lowest of what its four cells put there.
usage: island_relief.py RELIEF.bin
"""
import json, os, struct, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, os.path.join(ROOT, "3ds_port", "scripts"))
import voxel_relief_fixes as vrf  # noqa: E402

FACE = 16
MAX = 3
# the sea drawn round them: every General metatile of MB_OCEAN_WATER (its open water and its edges by the rock;
# 14:55 "the islands need to be natural": a shore edge read as rock stood a blue slab beside Navel Rock)
SEA = {0x170, 0x172, 0x174, 0x180, 0x181, 0x189, 0x190, 0x192, 0x198, 0x19A}
FALL = 2.5   # tiles over which the land falls to the sea west, east and north of it (south: its drawn cliff)
MAPS = {
    # layout id: (stair metatiles, plateau px, walkable cells drawn as rock - the peak's top, which the collision
    # leaves open: stood as the rock it is drawn)
    404: ({0x0AF}, 48, set()),     # Birth Island: a cliff three cells tall under its triangle of grass
    421: ({0x0CF}, 16, {(9, 5), (10, 5), (11, 5), (9, 6), (10, 6), (11, 6), (9, 7), (10, 7), (11, 7), (10, 8)}),
}


# cells the collision blocks that lie as the plateau: Birth Island's peak, under its tree's crown (17:14: the tree
# stands there in 3D; the crown's drawing had stood up as rock round its foot)
LEVEL = {404: {(14, 3), (15, 3), (16, 3), (14, 4), (15, 4), (16, 4)}}


def build(entry, stairs, plateau, rocky=(), level_cells=()):
    W, H = entry["width"], entry["height"]
    raw = open(os.path.join(ROOT, entry["blockdata_filepath"]), "rb").read()
    v = struct.unpack("<%dH" % (len(raw) // 2), raw)
    mt = lambda x, y: v[y * W + x] & 0x3FF
    blocked = lambda x, y: (v[y * W + x] >> 10) & 3 != 0
    # sea: blocked water joined to the map's edge; floor (level 0): sea, and walkable cells joined to the pier
    sea = set()
    todo = [(x, y) for x in range(W) for y in (0, H - 1)] + [(x, y) for y in range(H) for x in (0, W - 1)]
    while todo:
        c = todo.pop()
        if c in sea or not (0 <= c[0] < W and 0 <= c[1] < H):
            continue
        if not (blocked(*c) and (mt(*c) in SEA or mt(*c) >= 0x340)):
            continue
        sea.add(c)
        todo += [(c[0] + 1, c[1]), (c[0] - 1, c[1]), (c[0], c[1] + 1), (c[0], c[1] - 1)]
    stair = {(x, y) for y in range(H) for x in range(W) if mt(x, y) in stairs}
    walk = {(x, y) for y in range(H) for x in range(W) if not blocked(x, y) and (x, y) not in stair and (x, y) not in rocky}
    walk |= set(level_cells)
    # the pier: walkable cells joined to the island's stairs' foot going south (below the lowest stair)
    low = max(y for (_, y) in stair)
    pier = {c for c in walk if c[1] > low}
    level = {}
    for c in sea | pier:
        level[c] = 0
    for c in walk - pier:
        level[c] = plateau
    # faces: rock (and stairs) rise from what stands south
    n, s = {}, {}
    for y in range(H):
        for x in range(W):
            c = (x, y)
            if c in level:
                continue
            d, yy = 1, y + 1
            while yy < H and (x, yy) not in level and d < MAX + 4:
                d, yy = d + 1, yy + 1
            foot = level.get((x, yy), 0)
            if c in stair:
                d = sum(1 for k in range(y, yy) if (x, k) in stair)
                n[c], s[c] = foot + FACE * d, foot + FACE * (d - 1)
            else:
                d = min(d, MAX)
                n[c], s[c] = foot + FACE * d, foot + FACE * (d - 1)
    corner = {}
    for y in range(H):
        for x in range(W):
            c = (x, y)
            for (cx, cy, top) in ((x, y, True), (x + 1, y, True), (x, y + 1, False), (x + 1, y + 1, False)):
                h = level[c] if c in level else (n[c] if top else s[c])
                corner[(cx, cy)] = min(corner.get((cx, cy), 1 << 20), h)
    # Brennan 2026-11-24 14:55 ("the islands need to be natural and not angled blocks"): the land falls to the sea
    # west, east and north of it smoothly, over FALL tiles - a rounded shore, not a cell-wide ramp and stepped
    # grass. Only the sea at the same row or north counts: the cliff drawn on the south stays upright.
    seac = [(sx + 0.5, sy + 0.5) for (sx, sy) in sea]
    top = max(corner.values()) or 1
    for (cx, cy), h in list(corner.items()):
        if h <= 0:
            continue
        d2 = min([(cx - sx) ** 2 + (cy - sy) ** 2 for (sx, sy) in seac if sy <= cy + 0.5] or [1e9])
        d = max(0.0, d2 ** 0.5 - 0.5)
        t = min(1.0, d / FALL)
        t = t * t * (3 - 2 * t)
        # Brennan 2026-11-24 17:14 ("the right and left side of the mountain need fixed"): cut off at the falloff,
        # the cliff's courses stood as a lumpy stair at its two ends. The whole height is scaled down instead -
        # the cliff tapers to the sea at its ends, still upright, its courses together.
        corner[(cx, cy)] = int(round(h * t))
    cells = {}
    for y in range(H):
        for x in range(W):
            cor = [corner[(x, y)], corner[(x + 1, y)], corner[(x, y + 1)], corner[(x + 1, y + 1)]]
            g = [int(round((cor[0] * (1 - sx / 4) + cor[1] * sx / 4) * (1 - r / 4) + (cor[2] * (1 - sx / 4) + cor[3] * sx / 4) * r / 4))
                 for r in range(5) for sx in range(5)]
            if any(g):
                cells[(x, y)] = g
    return cells


def main():
    relief = vrf.parse(open(sys.argv[1], "rb").read())
    LL = json.load(open(os.path.join(ROOT, "data/layouts/layouts.json")))["layouts"]
    for lid, (stairs, plateau, rocky) in MAPS.items():
        entry = LL[lid - 1]
        assert vrf.layout_index(entry["id"]) == lid
        t = relief["layouts"].setdefault(lid, {"w": entry["width"], "h": entry["height"], "flags": 0, "base": 0,
                                              "cells": {}})
        t["flags"] |= vrf.DRAWN
        t["belt"] = 0
        t["cells"] = build(entry, stairs, plateau, rocky, LEVEL.get(lid, ()))
        print("%s: %d cells lifted" % (entry["id"], len(t["cells"])))
    open(sys.argv[1], "wb").write(vrf.pack(relief))


if __name__ == "__main__":
    main()
