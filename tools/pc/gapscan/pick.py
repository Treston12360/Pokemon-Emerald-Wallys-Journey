#!/usr/bin/env python3
"""usage: pick.py DUMPDIR MVP.bin [BELT.bin] PX PY [RADIUS] [SHOT_W SHOT_H]

Region audit 2026-11-23: which 3D faces lie round a pixel of a test picture (a hole in the gap scan). Run the game
with WJPC_DUMP_TERRAIN_ALL=DUMPDIR (every map's terrain/trees/models + <g>_<n>.origin, voxel_gl.c),
WJPC_DUMP_MVP=MVP.bin (the camera's matrix, last frame: 16 floats column-major, then belt shift x z, width, height)
and optionally WJPC_DUMP_BELT=BELT.bin (the background past the maps' edges), take the picture on the last frame,
then give the pixel (in the picture's own size SHOT_W x SHOT_H, default the 3D picture's). Prints the faces whose
projection comes within RADIUS px of it, nearest the camera first: source (map_layer or belt), the map cell, its
height and corners (map cells) - enough to find the relief cell or model to fix."""
import glob
import os
import struct
import sys

args = sys.argv[1:]
D, MVP = args[0], args[1]
belt = args[2] if len(args) > 2 and not args[2].lstrip('-').isdigit() else None
rest = args[3:] if belt else args[2:]
PX, PY = float(rest[0]), float(rest[1])
R = float(rest[2]) if len(rest) > 2 else 6
m = struct.unpack('<20f', open(MVP, 'rb').read(80))
M, (bsx, bsz, W, H) = m[:16], m[16:]
SW, SH = (float(rest[3]), float(rest[4])) if len(rest) > 4 else (W, H)


def project(x, y, z):
    c = [M[0 * 4 + r] * x + M[1 * 4 + r] * y + M[2 * 4 + r] * z + M[3 * 4 + r] for r in range(4)]
    if c[3] <= 1e-6:
        return None
    nx, ny, nz = c[0] / c[3], c[1] / c[3], c[2] / c[3]
    return ((nx + 1) / 2 * SW, (1 - ny) / 2 * SH, nz)   # flip chosen by FLIP env below


FLIP = os.environ.get('PICK_FLIP', '1') == '1'   # the picture is stored upside down from the projection (proj's y flipped for SDL)


def tris(path):
    d = open(path, 'rb').read()
    n = len(d) // 24
    v = struct.unpack('<%df' % (n * 6), d[:n * 24])
    for t in range(n // 3):
        yield [v[(3 * t + k) * 6:(3 * t + k) * 6 + 3] for k in range(3)]


def dist_point_tri(p, a, b, c):
    def inside(p, a, b, c):
        d = (b[1] - c[1]) * (a[0] - c[0]) + (c[0] - b[0]) * (a[1] - c[1])
        if abs(d) < 1e-9:
            return False
        l1 = ((b[1] - c[1]) * (p[0] - c[0]) + (c[0] - b[0]) * (p[1] - c[1])) / d
        l2 = ((c[1] - a[1]) * (p[0] - c[0]) + (a[0] - c[0]) * (p[1] - c[1])) / d
        return l1 >= 0 and l2 >= 0 and 1 - l1 - l2 >= 0
    if inside(p, a, b, c):
        return 0.0

    def seg(p, a, b):
        ax, ay = b[0] - a[0], b[1] - a[1]
        L = ax * ax + ay * ay
        t = 0 if L == 0 else max(0, min(1, ((p[0] - a[0]) * ax + (p[1] - a[1]) * ay) / L))
        dx, dy = a[0] + t * ax - p[0], a[1] + t * ay - p[1]
        return (dx * dx + dy * dy) ** 0.5
    return min(seg(p, a, b), seg(p, b, c), seg(p, c, a))


out = []
sources = []
origins = {}
if os.path.exists(MVP + '.origins'):   # where each map was drawn on the frame the matrix is from
    for line in open(MVP + '.origins'):
        g, n, x, z = (int(v) for v in line.split())
        origins['%d_%d' % (g, n)] = (x, z)
for o in glob.glob(os.path.join(D, '*.origin')):
    key = os.path.basename(o)[:-len('.origin')]
    if origins and key not in origins:
        continue
    ox, oz = origins.get(key) or tuple(int(v) for v in open(o).read().split())
    pre = o[:-len('.origin')]
    for f in glob.glob(pre + '.*'):
        if f.endswith('.origin'):
            continue
        sources.append((os.path.basename(f), f, ox, oz))
if belt:
    sources.append(('belt', belt, bsx, bsz))
for name, f, ox, oz in sources:
    for tri in tris(f):
        pts = []
        for (x, y, z) in tri:
            q = project(x + ox, y, z + oz)
            if q is None:
                break
            if FLIP:
                q = (q[0], SH - q[1], q[2])
            pts.append(q)
        if len(pts) < 3:
            continue
        dd = dist_point_tri((PX, PY), *pts)
        if dd <= R:
            cx = sum(v[0] for v in tri) / 3
            cz = sum(v[2] for v in tri) / 3
            out.append((min(q[2] for q in pts), dd, name, int(cx // 1), int(cz // 1),
                        [tuple(round(c, 3) for c in v) for v in tri]))
out.sort()
for z, dd, name, cx, cz, tri in out[:40]:
    print('%-14s cell %3d,%3d  px-dist %4.1f  depth %.5f  %s' % (name, cx, cz, dd, z, tri))
print(len(out), 'faces within', R, 'px')
