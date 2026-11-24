#!/usr/bin/env python3
"""A cave's rock raised into walls over its floor (the Cave tileset's caves), in place in relief.bin.

Wally's Journey, Brennan 2026-11-24 10:50: "Stairs, ladders, pokeballs in caves should be 3D" - and, asked how (the
caves were flat, their cliffs only painted on the floor): "Raise the cliffs". The way Fiery Path was raised
(fiery_path_relief.py, 2026-11-22), read here off the map's own collision instead of a list of rock tiles:

  floor  every cell the player can stand on (collision 0), and the cells of the floor's own details (the rocks,
         crystals, stalagmites and ladders that stand as models: cave_detail_spec.py)          level 0
  face   a blocked cell with the floor straight south of it: the rock's face, drawn toward the floor - a slope FACE px
         over the cell from the floor to the rock top (voxel_relief.h: a point stands as far south as it is high,
         so that is an upright wall at the floor's edge, its drawing standing on it)
  top    every other blocked cell: flat at the rock's height, filled smoothly between the faces' tops and the belt
         at the map's edge; where it meets the floor west, east or south with no face between it drops to the floor
         in that edge (as Fiery Path's)
The map's west, east and south edges come down to the floor's level at the edge (nothing beyond: a raised edge stood
a tall stretched cut face along them).

Brennan 2026-11-24 12:39 (the test on Victory Road B1F): "Do all caves this way, but fix the dark shadows that show and
keep the textures the same". Done 2026-11-24 for (layout ids):
  Cave tileset: 420 281 401 400 158 162 157 159 160 161 131 389 132 133 134 288 280 413 407 311 147 148 149 150 151
                152 155 156 304 284 168 169 164 328 165 167 166 320 409 408 163 285 286
  Meteor Falls: 126 127 128 431 (125 already had its own relief, kept)   Navel Rock: 422-428
  Seafloor Cavern rooms 6-7 (Pacifidlog tileset): 153 154                 Magma Hideout: 336-341 379 380
  (Fiery Path 293 and Rusturf Tunnel 129 had relief already.)
Floors stand at their elevation's level (Shoal Cave's upper floors a rock's height up per elevation; a face rises a rock's
height over the floor at its foot); a rock top dropping to the floor beside it drops to that floor's height; two rock
cells meet at the same heights along their shared edge (else the renderer stood a stretched dark wall in the step).
And the renderer: a cave's ground is lit as its floor whatever way it faces, and casts no sun shadow (voxel_gl.c
BuildFinish, voxel_lighting.c VoxelLighting_Sample) - so its walls carry the drawing's own colours.
usage: cave_relief.py RELIEF.bin LAYOUT_ID [LAYOUT_ID ...] [--face 16] [--belt 24]
Needs 3ds_port/scripts (voxel_relief_fixes).
"""
import argparse, json, os, struct, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, os.path.join(ROOT, "3ds_port", "scripts"))
import voxel_relief_fixes as vrf  # noqa: E402

# the floor's details standing as models (cave_detail_spec.py): floor under them
DETAILS = {0x202, 0x203, 0x250, 0x262, 0x266, 0x330, 0x2D5, 0x35A, 0x21F, 0x227, 0x31F, 0x326, 0x348, 0x34D, 0x34E,
           0x217, 0x37F, 0x35B}


def build(entry, face_px, belt_px):
    W, H = entry["width"], entry["height"]
    raw = open(os.path.join(ROOT, entry["blockdata_filepath"]), "rb").read()
    v = struct.unpack("<%dH" % (len(raw) // 2), raw)
    mt = [x & 0x3FF for x in v]
    blocked = [(x >> 10) & 3 != 0 for x in v]
    elev = [(x >> 12) & 0xF for x in v]
    cave_set = entry["secondary_tileset"] == "gTileset_Cave"   # (DETAILS are its metatiles)
    kind, base = {}, {}
    # the floor's levels, from the map's own elevations: the lowest floor (3 in most caves; water is 1) at 0, each
    # elevation above it a rock's height higher (Shoal Cave's upper floors, 4 and 5); a bridge (15) at one rock's
    # height over the floor it spans; stairs (0, joining two levels) climb between them (None: worked out below)
    lows = [e for i, e in enumerate(elev) if not blocked[i] and 3 <= e <= 14]
    low = min(lows) if lows else 3

    def level(e):
        if e == 15:
            return face_px
        return max(0, e - low) * face_px if e >= 3 else 0
    for y in range(H):
        for x in range(W):
            i = y * W + x
            kind[(x, y)] = "F" if (not blocked[i] or (mt[i] in DETAILS and cave_set)) else "T"
            if kind[(x, y)] == "F":
                base[(x, y)] = None if (elev[i] == 0 and not blocked[i]) else level(elev[i])
    for (x, y), k in list(kind.items()):
        if k == "T" and kind.get((x, y + 1)) == "F":
            kind[(x, y)] = "A"
    top_floor = max([b for b in base.values() if b is not None] + [0])
    belt_px = max(belt_px, top_floor + 8)

    def touching(i, j):
        return [(x, y) for x in (i - 1, i) for y in (j - 1, j) if (x, y) in kind]

    top, floor_c, fh = {}, set(), {}
    for i in range(W + 1):
        for j in range(H + 1):
            t = touching(i, j)
            if any(kind[c] == "F" for c in t):
                floor_c.add((i, j))
                known = [base[c] for c in t if kind[c] == "F" and base[c] is not None]
                if known:
                    fh[(i, j)] = max(known)
            faces = [c for c in t if kind[c] == "A"]
            if faces:
                # a face rises a rock's height over the floor at its foot
                top[(i, j)] = face_px + max((base.get((c[0], c[1] + 1)) or 0) for c in faces)
            elif i in (0, W) or j in (0, H):
                top[(i, j)] = belt_px
    free = [(i, j) for i in range(W + 1) for j in range(H + 1) if (i, j) not in top]
    val = dict(top)
    for c in free:
        val[c] = belt_px
    for _ in range(300):
        for (i, j) in free:
            nb = [val[n] for n in ((i - 1, j), (i + 1, j), (i, j - 1), (i, j + 1)) if n in val]
            val[(i, j)] = sum(nb) / len(nb)

    # the stairs' corners: between the floor's and the bridge's, smoothly
    stair = [c for c in floor_c if c not in fh]
    for c in stair:
        fh[c] = 0.0
    for _ in range(300):
        for (i, j) in stair:
            nb = [fh[n] for n in ((i, j - 1), (i, j + 1)) if n in fh]
            fh[(i, j)] = sum(nb) / len(nb) if nb else 0.0

    def floor_h(c):
        return fh.get(c, 0)

    def lattice(cor):
        return [(cor[0] * (1 - s / 4.0) + cor[1] * s / 4.0) * (1 - r / 4.0)
                + (cor[2] * (1 - s / 4.0) + cor[3] * s / 4.0) * r / 4.0 for r in range(5) for s in range(5)]

    def is_floor(c):
        return kind.get(c) == "F"

    def floor_at(x, y, s, r):
        """the floor's height at point (s, r) of cell (x, y)'s edge, from the floor cell(s) beside it"""
        hs = []
        for (nx, ny, ns, nr) in ((x - 1, y, s + 4, r), (x + 1, y, s - 4, r), (x, y - 1, s, r + 4), (x, y + 1, s, r - 4),
                                 (x - 1, y - 1, s + 4, r + 4), (x + 1, y - 1, s - 4, r + 4),
                                 (x - 1, y + 1, s + 4, r - 4), (x + 1, y + 1, s - 4, r - 4)):
            if is_floor((nx, ny)) and 0 <= ns <= 4 and 0 <= nr <= 4:
                cor = [floor_h(c) for c in ((nx, ny), (nx + 1, ny), (nx, ny + 1), (nx + 1, ny + 1))]
                hs.append(lattice(cor)[nr * 5 + ns])
        return min(hs) if hs else 0

    cells = {}
    for (x, y), k in kind.items():
        corners = ((x, y), (x + 1, y), (x, y + 1), (x + 1, y + 1))
        if k == "F":
            g = lattice([floor_h(c) for c in corners])
        elif k == "A":
            g = lattice([floor_h(c) if c in floor_c else val[c] for c in corners])
        else:
            side = is_floor((x - 1, y)) or is_floor((x + 1, y))
            if side:
                g = lattice([floor_h(c) if (c in floor_c and c[0] != x + (0 if is_floor((x + 1, y)) else 1)) else val[c]
                             for c in corners])
            else:
                g = lattice([val[c] for c in corners])
            for r in range(5):
                for s in range(5):
                    down = ((s == 0 and is_floor((x - 1, y))) or (s == 4 and is_floor((x + 1, y)))
                            or (r == 4 and is_floor((x, y + 1)))
                            or (s in (0, 4) and r in (0, 4) and any(
                                kind.get((cx, cy)) == "F" and cy >= y
                                for cx in (x + s // 4 - 1, x + s // 4) for cy in (y + r // 4 - 1, y + r // 4))))
                    if down:
                        g[r * 5 + s] = min(g[r * 5 + s], floor_at(x, y, s, r))
            for (dx, dy) in ((-1, 0), (1, 0), (0, -1), (0, 1)):
                n = (x + dx, y + dy)
                if kind.get(n) != "A":
                    continue
                fc = lattice([floor_h(c) if c in floor_c else val[c]
                              for c in (n, (n[0] + 1, n[1]), (n[0], n[1] + 1), (n[0] + 1, n[1] + 1))])
                for kk in range(5):
                    r, s = {(-1, 0): (kk, 0), (1, 0): (kk, 4), (0, -1): (0, kk), (0, 1): (4, kk)}[(dx, dy)]
                    rn, sn = {(-1, 0): (kk, 4), (1, 0): (kk, 0), (0, -1): (4, kk), (0, 1): (0, kk)}[(dx, dy)]
                    g[r * 5 + s] = fc[rn * 5 + sn]
        # the map's west, east and south edges: down to the floor's level at the edge itself - there is nothing
        # beyond (a cave's black), and a raised edge stood a tall cut face of the rock's drawing, stretched, along
        # them (the north edge rises behind, never seen)
        if k != "F":
            for r in range(5):
                for s in range(5):
                    if (x == 0 and s == 0) or (x == W - 1 and s == 4) or (y == H - 1 and r == 4):
                        g[r * 5 + s] = 0
        cells[(x, y)] = [int(round(h)) for h in g]
    # two rock cells side by side meet at the same heights along their shared edge (the lower of the two): where
    # they did not (a top sloping down to the floor beside it, next to a face or a top that did not) the renderer
    # stood a wall in the step between them, its drawing stretched dark - the "dark shadows" at the rock's corners
    low = {}
    for (x, y), k in kind.items():
        if k == "F":
            continue
        g = cells[(x, y)]
        for r in range(5):
            for s in range(5):
                key = (4 * x + s, 4 * y + r)
                low[key] = min(low.get(key, 1 << 30), g[r * 5 + s])
    for (x, y), k in kind.items():
        if k == "F":
            continue
        g = cells[(x, y)]
        for r in (0, 4):
            for s in range(5):
                g[r * 5 + s] = low[(4 * x + s, 4 * y + r)]
        for s in (0, 4):
            for r in range(5):
                g[r * 5 + s] = low[(4 * x + s, 4 * y + r)]
    return cells


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("relief")
    ap.add_argument("layouts", type=int, nargs="+")
    ap.add_argument("--face", type=int, default=16)
    ap.add_argument("--belt", type=int, default=24)
    a = ap.parse_args()
    LL = json.load(open(os.path.join(ROOT, "data/layouts/layouts.json")))["layouts"]
    relief = vrf.parse(open(a.relief, "rb").read())
    for lid in a.layouts:
        entry = LL[lid - 1]
        cells = build(entry, a.face, a.belt)
        index = vrf.layout_index(entry["id"])
        assert index == lid, (index, lid)
        t = relief["layouts"].setdefault(index, {"w": entry["width"], "h": entry["height"], "flags": 0, "base": 0,
                                                "cells": {}})
        t["flags"] |= vrf.DRAWN
        t["belt"] = a.belt // 16
        t["cells"] = {c: g for c, g in cells.items() if any(g)}
        print("%s: %d cells lifted" % (entry["id"], len(t["cells"])))
    open(a.relief, "wb").write(vrf.pack(relief))


if __name__ == "__main__":
    main()
