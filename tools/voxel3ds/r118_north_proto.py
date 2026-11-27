#!/usr/bin/env python3
"""r118_north_proto.py [OUT.png] - draws the parts of the belt north of Route 118 as a cell map (N82, 2026-11-27,
session 014PwRDj): the same maths as voxel_world.c VoxelWorld_R118North (R118NBanks, R118NBand, the trees north of
the map's own wood, the drift back past the river's end), for trying a new lake/river shape in seconds before
changing the C. Colours: rock brown, water blue, bank trees green, woods dark green; the bottom row is Route 118's own
row 0. Keep KEYS and the noise in step with the C (Brennan 2026-11-27 03:48 asked to keep how this was done, to do it
elsewhere: docs/WALLY_JOURNEY_OWNER_REQUIREMENTS.md, N82 note)."""
import math, sys
from PIL import Image, ImageDraw
def hash01(i, seed):
    h = (i * 374761393 + seed * 668265263) & 0xffffffff
    h = ((h ^ (h >> 13)) * 1274126177) & 0xffffffff
    return ((h ^ (h >> 16)) & 0xffff) / 65535.0 * 2.0 - 1.0
def noise1(t, seed):
    i = math.floor(t); f = t - i
    a, b = hash01(i, seed), hash01(i + 1, seed)
    f = f * f * (3 - 2 * f)
    return a + (b - a) * f
def smooth(a, b, t):
    t = max(0.0, min(1.0, (t - a) / (b - a)))
    return t * t * (3 - 2 * t)
def lerp(a, b, t): return a + (b - a) * t
# piecewise profile by v (rows north of the map's edge): (v, left, right)
KEYS = [(0.0, 5.5, 36.5), (3.0, 5.7, 36.3), (6.0, 6.6, 35.4), (9.0, 8.3, 33.7), (11.0, 10.1, 31.9),
        (13.0, 12.9, 29.1), (14.5, 16.0, 26.0), (16.5, 19.0, 25.0), (22.0, 19.5, 25.0), (27.0, 21.0, 25.5),
        (33.0, 18.5, 23.0), (39.0, 17.0, 21.5), (44.0, 18.5, 22.0), (48.0, 20.0, 20.5)]
END = 48.0
def banks(v):
    if v >= END: return None
    for k in range(len(KEYS) - 1):
        v0, l0, r0 = KEYS[k]; v1, l1, r1 = KEYS[k + 1]
        if v0 <= v <= v1:
            t = (v - v0) / (v1 - v0)
            l, r = lerp(l0, l1, t), lerp(r0, r1, t)
            break
    fade = smooth(2.0, 6.0, v)            # the mouth matches the map exactly
    l -= fade * 1.6 * noise1(v / 3.5, 11)
    r += fade * 1.6 * noise1(v / 3.5, 23)
    return l, r
def band(v, side):
    return 1.6 + 1.1 * noise1(v / 2.8, 37 if side == 0 else 41)
MAPROW0 = 'TTTTTCRRRRRRRRTTTTTTCSSWWWSSSCTTTTTTTTTR'   # Route 118 row 0, x 0..39 (T tree, C cliff, R rock, S sand, W water)
def cls(lx, ly):
    v = -ly - 0.5; u = lx + 0.5
    b = banks(v)
    if b is not None:
        l, r = b
        if l < u < r: return 'W'
        if u <= l:
            if u > l - band(v, 0): return 'T'
            # trees north of the map's own trees, 1..3 rows
            if MAPROW0[lx] == 'T' and v < 1.5 + 1.2 * (noise1(lx / 2.5, 53) + 1) / 2 * 2: return 'T'
            return 'M'
        return 'F'
    # past the river's end: the cut between rock and woods drifts back to the straight line at 20 (Voronoi) by v 64
    cut = lerp(20.25, 20.0, smooth(END, 64.0, v)) + (1 - smooth(END, 64.0, v)) * 1.5 * noise1(v / 4.0, 59)
    if u < cut - band(v, 0) * (1 - smooth(END, 64.0, v)): return 'M'
    return 'F' if u >= cut else 'T'
S = 12
img = Image.new('RGB', (40 * S, 70 * S), (0, 0, 0)); d = ImageDraw.Draw(img)
col = {'W': (60, 110, 220), 'T': (30, 120, 40), 'F': (20, 90, 30), 'M': (170, 120, 90)}
for ly in range(-1, -71, -1):
    for lx in range(40):
        c = cls(lx, ly)
        y = (70 + ly) * S
        d.rectangle([lx * S, y, lx * S + S - 1, y + S - 1], fill=col[c])
# the map's row 0 at the bottom for reference
for lx in range(40):
    m = MAPROW0[lx]; c = {'T': (90, 200, 90), 'C': (120, 80, 60), 'R': (200, 160, 120), 'S': (230, 210, 120), 'W': (90, 150, 255)}[m]
    d.rectangle([lx * S, 69 * S, lx * S + S - 1, 70 * S - 1], fill=c)
for k in range(0, 70, 5): d.line([0, (70 - k) * S, 6, (70 - k) * S], fill=(255, 255, 255))
img.save(sys.argv[1] if len(sys.argv) > 1 else 'r118_north_proto.png')
