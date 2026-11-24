#!/usr/bin/env python3
"""usage: coplanar_all.py DUMPDIR [THRESH] [--no-terrain-self]

Region audit 2026-11-23 (Brennan 18:34: "scan outdoors, indoors, and caves while moving for any flickering/strobing
caused by overlapping double ground, double walls, or tree/object base sheets at the same height").
Reads the dumps written by WJPC_DUMP_TERRAIN_ALL=DIR (voxel_gl.c): DIR/<group>_<num>.<page> (terrain), .trees, .models
- VoxelVertex: x y z u v shade, 6 floats, map cells. For every flat (horizontal) triangle, four points inside it
(the centre and halfway to each corner) are looked up in every other flat triangle of the same map: one covering the
point at a height within THRESH (default 1/32 tile) is a double surface (two faces the depth buffer cannot keep apart
far from the camera: flicker). The same for upright faces facing the same way in the same plane (double walls).
Prints per map: cells with doubles, by which layers (terrain page / trees / models)."""
import collections
import glob
import os
import struct
import sys

SKIP_TERRAIN_SELF = '--no-terrain-self' in sys.argv
ARGS = [a for a in sys.argv[1:] if not a.startswith('--')]
D = ARGS[0]
TH = float(ARGS[1]) if len(ARGS) > 1 else 1.0 / 32


def load(path):
    d = open(path, 'rb').read()
    n = len(d) // 24
    v = struct.unpack('<%df' % (n * 6), d[:n * 24])
    return [v[i * 6:i * 6 + 6] for i in range(n)]


def bary(px, pz, a, b, c):
    (x1, z1), (x2, z2), (x3, z3) = a, b, c
    d = (z2 - z3) * (x1 - x3) + (x3 - x2) * (z1 - z3)
    l1 = ((z2 - z3) * (px - x3) + (x3 - x2) * (pz - z3)) / d
    l2 = ((z3 - z1) * (px - x3) + (x1 - x3) * (pz - z3)) / d
    return l1, l2, 1 - l1 - l2


def look(px, pz, tr):
    l = bary(px, pz, tr[2], tr[3], tr[4])
    a = tr[5]
    return tuple(sum(l[k] * a[k][m] for k in range(3)) for m in range(3))


def inside(px, pz, a, b, c):
    (x1, z1), (x2, z2), (x3, z3) = a, b, c
    d = (z2 - z3) * (x1 - x3) + (x3 - x2) * (z1 - z3)
    if abs(d) < 1e-9:
        return False
    l1 = ((z2 - z3) * (px - x3) + (x3 - x2) * (pz - z3)) / d
    l2 = ((z3 - z1) * (px - x3) + (x1 - x3) * (pz - z3)) / d
    return l1 > 1e-4 and l2 > 1e-4 and 1 - l1 - l2 > 1e-4


def check(prefix):
    flat = []   # (layer, y, (x,z)x3)
    walls = collections.defaultdict(list)   # plane key -> [(layer, 2D tri in plane)]
    for f in glob.glob(prefix + '.*'):
        layer = f[len(prefix) + 1:]
        if not os.path.getsize(f):
            continue
        vs = load(f)
        for t in range(len(vs) // 3):
            a, b, c = vs[3 * t], vs[3 * t + 1], vs[3 * t + 2]
            ys = (a[1], b[1], c[1])
            if max(ys) - min(ys) < 1e-4:
                up = (b[0] - a[0]) * (c[2] - a[2]) - (b[2] - a[2]) * (c[0] - a[0])
                if abs(up) > 1e-6:
                    flat.append((layer, ys[0], (a[0], a[2]), (b[0], b[2]), (c[0], c[2]), (a[3:], b[3:], c[3:])))
                continue
            xs = (a[0], b[0], c[0]); zs = (a[2], b[2], c[2])
            nx = (b[1] - a[1]) * (c[2] - a[2]) - (b[2] - a[2]) * (c[1] - a[1])
            nz = (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0])
            if max(xs) - min(xs) < 1e-4:     # wall facing east/west in plane x (back-to-back faces apart)
                walls[('x', round(xs[0] * 64))].append((layer, (a[2], a[1]), (b[2], b[1]), (c[2], c[1])))
            elif max(zs) - min(zs) < 1e-4:   # wall facing north/south in plane z
                walls[('z', round(zs[0] * 64))].append((layer, (a[0], a[1]), (b[0], b[1]), (c[0], c[1])))
    hits = collections.Counter()
    grid = collections.defaultdict(list)
    for i, tr in enumerate(flat):
        xs = [p[0] for p in tr[2:5]]; zs = [p[1] for p in tr[2:5]]
        for gx in range(int(min(xs) * 2) - 1, int(max(xs) * 2) + 2):
            for gz in range(int(min(zs) * 2) - 1, int(max(zs) * 2) + 2):
                grid[(gx, gz)].append(i)
    for i, (layer, y, a, b, c, _) in enumerate(flat):
        cx = (a[0] + b[0] + c[0]) / 3; cz = (a[1] + b[1] + c[1]) / 3
        for px, pz in ((cx, cz), ((cx + a[0]) / 2, (cz + a[1]) / 2), ((cx + b[0]) / 2, (cz + b[1]) / 2),
                       ((cx + c[0]) / 2, (cz + c[1]) / 2)):
            for j in grid.get((int(px * 2), int(pz * 2)), ()):
                if j <= i:
                    continue
                l2, y2, a2, b2, c2, _ = flat[j]
                if SKIP_TERRAIN_SELF and layer.isdigit() and l2.isdigit():
                    continue   # (--no-terrain-self: ground against ground - the water's own layers - left out)
                if sorted((round(p_[0], 3), round(p_[1], 3)) for p_ in (a, b, c)) == \
                   sorted((round(p_[0], 3), round(p_[1], 3)) for p_ in (a2, b2, c2)) and abs(y2 - y) < 1e-5:
                    continue   # the same corners: the same depth exactly, the later one always wins (no flicker)
                if abs(y2 - y) < TH and inside(px, pz, a2, b2, c2):
                    u1, v1, s1 = look(px, pz, flat[i]); u2, v2, s2 = look(px, pz, flat[j])
                    if layer == l2 and abs(u1 - u2) + abs(v1 - v2) < 1.0 / 2048 and abs(s1 - s2) < 0.03:
                        continue   # the same picture and light twice: no visible flicker
                    hits[('ground', int(px) if px >= 0 else int(px) - 1, int(pz) if pz >= 0 else int(pz) - 1,
                          '+'.join(sorted((layer, l2))), round(abs(y2 - y) * 64))] += 1
    for key, tris in walls.items():
        if len(tris) < 2:
            continue
        g = collections.defaultdict(list)
        for i, tr in enumerate(tris):
            us = [p[0] for p in tr[1:]]
            for gu in range(int(min(us) * 2) - 1, int(max(us) * 2) + 2):
                g[gu].append(i)
        for i, (layer, a, b, c) in enumerate(tris):
            cu = (a[0] + b[0] + c[0]) / 3; cv = (a[1] + b[1] + c[1]) / 3
            for j in g.get(int(cu * 2), ()):
                if j <= i:
                    continue
                l2, a2, b2, c2 = tris[j]
                if sorted((round(p_[0], 3), round(p_[1], 3)) for p_ in (a, b, c)) == \
                   sorted((round(p_[0], 3), round(p_[1], 3)) for p_ in (a2, b2, c2)):
                    continue   # the same corners (no flicker)
                if inside(cu, cv, a2, b2, c2):
                    u = cu if key[0] == 'z' else key[1] / 64
                    w = key[1] / 64 if key[0] == 'z' else cu
                    hits[('wall', int(u), int(w), '+'.join(sorted((layer, l2))), 0)] += 1
    return hits


def main():
    prefixes = sorted({f.rsplit('.', 1)[0] for f in glob.glob(os.path.join(D, '*_*.*'))})
    for p in prefixes:
        h = check(p)
        name = os.path.basename(p)
        if not h:
            print(name, 'clean')
            continue
        cells = collections.defaultdict(int)
        for (kind, x, z, layers, dy), n in h.items():
            cells[(kind, layers, dy)] += 1
        print(name, ' '.join('%s[%s dy=%d/64]:%d' % (k, l, dy, n) for (k, l, dy), n in sorted(cells.items())))
        for (kind, x, z, layers, dy), n in sorted(h.items())[:12]:
            print('   ', kind, x, z, layers, dy)


if __name__ == '__main__':
    main()
