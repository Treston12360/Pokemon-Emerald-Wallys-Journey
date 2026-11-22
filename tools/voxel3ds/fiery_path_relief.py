#!/usr/bin/env python3
"""Fiery Path in full 3D: its rock raised into walls over the cave floor, in place in relief.bin.

Brennan 2026-11-22 12:39 ("continue on the next big job": Fiery Path "into full 3D cave walls, raised ledges and 3D
boulders, matching the 3D overworld style"). Emerald 3DS has no 3D Fiery Path (docs/VOXEL_STATUS.md: not started) and
its drawn-relief solver does not know Fiery Path's tiles (Lavaridge's cave cliffs, 0x268-0x313, none of them in its
ROCK_TILES), so the cave is carved here the way Emerald 3DS carved Rusturf Tunnel by hand (relief_fixes.json: rock at
the belt's height, the faces as slopes from the floor to it).

Each cell is read off its metatile:
  floor  0x308-0x311 (and the boulder 0x30D, a mound on the floor)           level 0
  face   the cliff faces drawn toward the floor (0x270 0x272 0x274 0x278 0x27A 0x27B 0x27C 0x306 0x307 0x30E 0x30F):
         a slope, FACE px a cell of face between the floor and the rock top (two cells of face: twice as high)
  top    every other blocked cell (rock top 0x271/0x312/0x313, the rims 0x30A-0x30C 0x268-0x26A 0x27E 0x27F, the
         exits 0x20D-0x20F): flat at the rock's height, which is filled smoothly between the faces' tops (and the
         belt at the map's edge); where a top cell meets the floor with no face between (a rim, the rock's back seen
         from the north) it drops to the floor in the cell's outer quarter, as Rusturf's do.
The heights are worked out at the cells' corners; a cell's 5x5 lattice is laid between its four (voxel_relief.h: a
point stands as far south as it is high, so FACE px over a cell of a north face is an upright wall).

usage: fiery_path_relief.py RELIEF.bin [--face 16] [--belt 32] [--boulder 10]
Needs 3ds_port/scripts (Emerald 3DS's generators, tools/voxel3ds/README.md) for voxel_relief_fixes.
"""
import argparse
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, os.path.join(ROOT, "3ds_port", "scripts"))
import voxel_relief_fixes as vrf  # noqa: E402

LAYOUT = "LAYOUT_FIERY_PATH"
FLOOR = {0x308, 0x309, 0x310, 0x311}
BOULDER = {0x30D}
# a face more than this many cells from the floor (the corner tiles chain faces of a stair of ledges together) counts
# as this deep: the drawing never stacks more than two cells of face
MAX_DEPTH = 2
# A rock top with the floor north of it (a rim: the rock's back) is left whole: the renderer stands its back
# (EmitNorthFace: the floor run on under it, then an upright wall). Dropped to the floor in its north quarter instead
# (FP_NORTH_DROP=1) the drop showed from above as a dark stretched strip along every rim.
NORTH_DROP = os.environ.get('FP_NORTH_DROP') is not None
FACE = {0x270, 0x272, 0x274, 0x278, 0x27A, 0x27B, 0x27C, 0x306, 0x307, 0x30E, 0x30F}


def build(face_px, belt_px, boulder_px):
    entry = vrf.layout_entry(LAYOUT)
    W, H = entry["width"], entry["height"]
    raw = open(os.path.join(ROOT, entry["blockdata_filepath"]), "rb").read()
    mt = [v & 0x3FF for v in struct.unpack("<%dH" % (len(raw) // 2), raw)]
    kind = {}
    for y in range(H):
        for x in range(W):
            m = mt[y * W + x]
            kind[(x, y)] = "F" if m in FLOOR or m in BOULDER else ("A" if m in FACE else "T")
    # face depth: 1 beside the floor, 2 behind that, ...
    depth, front = {}, [c for c, k in kind.items() if k == "F"]
    d = 0
    seen = set(front)
    while front:
        d += 1
        nxt = []
        for (x, y) in front:
            for c in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
                if c in kind and c not in seen and kind[c] == "A":
                    seen.add(c)
                    depth[c] = min(d, MAX_DEPTH)
                    nxt.append(c)
        front = nxt
    for c, k in kind.items():
        if k == "A" and c not in depth:     # a face cut off from the floor: rock
            kind[c] = "T"

    def touching(i, j):
        return [(x, y) for x in (i - 1, i) for y in (j - 1, j) if (x, y) in kind]

    # corner heights: floor 0; faces FACE x the shallowest face touching; the rest filled
    top = {}        # corners of the rock top (floor ignored): known values
    floor_c = set()
    for i in range(W + 1):
        for j in range(H + 1):
            t = touching(i, j)
            if any(kind[c] == "F" for c in t):
                floor_c.add((i, j))
            faces = [depth[c] for c in t if kind[c] == "A"]
            if faces:
                top[(i, j)] = face_px * min(faces)
            elif i in (0, W) or j in (0, H):
                top[(i, j)] = belt_px
    # fill the rock top's other corners: the mean of their neighbours, repeated (a smooth top between the faces)
    free = [(i, j) for i in range(W + 1) for j in range(H + 1) if (i, j) not in top]
    val = dict(top)
    for c in free:
        val[c] = belt_px
    for _ in range(400):
        for (i, j) in free:
            nb = [val[n] for n in ((i - 1, j), (i + 1, j), (i, j - 1), (i, j + 1)) if n in val]
            val[(i, j)] = sum(nb) / len(nb)

    def lattice(cor):
        g = []
        for r in range(5):
            for s in range(5):
                u, v = s / 4.0, r / 4.0
                g.append((cor[0] * (1 - u) + cor[1] * u) * (1 - v) + (cor[2] * (1 - u) + cor[3] * u) * v)
        return g

    def is_floor(c):
        return kind.get(c) == "F"

    cells = {}
    for (x, y), k in kind.items():
        corners = ((x, y), (x + 1, y), (x, y + 1), (x + 1, y + 1))
        if k == "F":
            g = [0.0] * 25
            if mt[y * W + x] in BOULDER:
                # a mound: highest in the middle, its edge on the floor
                g = [boulder_px * max(0.0, 1 - max(abs(s - 2), abs(r - 2)) / 2.0) ** 0.6
                     for r in range(5) for s in range(5)]
        elif k == "A":
            # a face: its corners on the floor stand on it, the others at the faces' heights
            g = lattice([0 if c in floor_c else val[c] for c in corners])
        else:
            # rock top: the filled heights, but where it meets the floor west, east or south with no face between,
            # the points on that edge stand on the floor
            side = is_floor((x - 1, y)) or is_floor((x + 1, y))
            if side:
                # floor beside it east or west: that side slopes down a whole cell, as a face does (a drop in its outer
                # quarter there was drawn as a grey upright block at a rim's end)
                g = lattice([0 if (c in floor_c and c[0] != x + (0 if is_floor((x + 1, y)) else 1)) else val[c]
                             for c in corners])
            else:
                g = lattice([val[c] for c in corners])
            for r in range(5):
                for s in range(5):
                    down = ((s == 0 and is_floor((x - 1, y))) or (s == 4 and is_floor((x + 1, y)))
                            or (r == 0 and is_floor((x, y - 1)) and NORTH_DROP) or (r == 4 and is_floor((x, y + 1)))
                            or (s in (0, 4) and r in (0, 4) and any(
                                kind.get((cx, cy)) == "F" and (cy >= y or NORTH_DROP)
                                for cx in (x + s // 4 - 1, x + s // 4) for cy in (y + r // 4 - 1, y + r // 4))))
                    if down:
                        g[r * 5 + s] = 0
            # on an edge it shares with a face, the face's heights (the face meets the floor there): no crack
            for (dx, dy) in ((-1, 0), (1, 0), (0, -1), (0, 1)):
                n = (x + dx, y + dy)
                if kind.get(n) != "A":
                    continue
                fc = lattice([0 if c in floor_c else val[c]
                              for c in (n, (n[0] + 1, n[1]), (n[0], n[1] + 1), (n[0] + 1, n[1] + 1))])
                for k in range(5):
                    r, s = {(-1, 0): (k, 0), (1, 0): (k, 4), (0, -1): (0, k), (0, 1): (4, k)}[(dx, dy)]
                    rn, sn = {(-1, 0): (k, 4), (1, 0): (k, 0), (0, -1): (4, k), (0, 1): (0, k)}[(dx, dy)]
                    g[r * 5 + s] = fc[rn * 5 + sn]
        cells[(x, y)] = [int(round(h)) for h in g]
    return cells, kind


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("relief")
    ap.add_argument("--face", type=int, default=16)
    ap.add_argument("--belt", type=int, default=32)
    ap.add_argument("--boulder", type=int, default=10)
    a = ap.parse_args()
    cells, kind = build(a.face, a.belt, a.boulder)
    relief = vrf.parse(open(a.relief, "rb").read())
    index = vrf.layout_index(LAYOUT)
    entry = vrf.layout_entry(LAYOUT)
    t = relief["layouts"].setdefault(index, {"w": entry["width"], "h": entry["height"], "flags": 0, "base": 0,
                                            "cells": {}})
    t["flags"] |= vrf.DRAWN
    t["belt"] = a.belt // 16
    t["cells"] = {c: g for c, g in cells.items() if any(g)}
    open(a.relief, "wb").write(vrf.pack(relief))
    print("%s: %d cells lifted, belt %d px" % (LAYOUT, len(t["cells"]), a.belt))


if __name__ == "__main__":
    main()
