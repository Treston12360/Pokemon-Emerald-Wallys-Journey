#!/usr/bin/env python3
"""Project ARIA: simulate Wally's Journey's season recolour (src/wally_journey.c SeasonalColor) on Kanto maps,
to see which building colours change with the leaves. Python copy of the C integer maths.
Usage: python3 tools/aria_season_sim.py OUTDIR [LayoutName ...]"""
import sys, os, struct
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '../docs/kanto_mockups/tools'))
import render as R
from PIL import Image
NONE, SPRING, AUTUMN, WINTER, FROST = range(5)
def idiv(a, b):  # C integer division (truncates toward 0)
    q = abs(a) // abs(b)
    return q if (a >= 0) == (b > 0) else -q
def seasonal(c5, mode, slot, protected=False):
    if mode == NONE or protected: return c5
    r, g, b = c5
    mx, mn = max(r, g, b), min(r, g, b); d = mx - mn
    if mx == 0 or d * 100 // mx < 18: return c5
    if mx == r: h = idiv(60 * (g - b), d)
    elif mx == g: h = idiv(60 * (b - r), d) + 120
    else: h = idiv(60 * (r - g), d) + 240
    if h < 0: h += 360
    s = d * 255 // mx; v = mx
    if mode == FROST and 10 <= h < 65 and v >= 20:
        h = 210; s = s * 15 // 100; v += (31 - v) * 70 // 100
    elif h < 65 or h > 170:
        return c5
    else:
        if mode == SPRING:
            if slot % 4 == 1: h = 335 + idiv(h - 110, 8); s = s * 55 // 100; v = min(31, v + 3)
            else: h -= 14; s = min(255, s * 120 // 100); v = min(31, v + 2)
        elif mode == AUTUMN:
            h = [40, 14, 26, 34][slot % 4] + idiv(v - 16, 2) + idiv(h - 110, 8); s = min(255, s * 115 // 100)
        elif mode == WINTER: s = s * 60 // 100; v = v * 92 // 100
        elif mode == FROST: h += idiv(200 - h, 2); s = s * 20 // 100; v += (31 - v) * 60 // 100
    if h < 0: h += 360
    h %= 360
    i = h // 60; f = (h % 60) * 255 // 60
    p = v * (255 - s) // 255; q = v * (255 - s * f // 255) // 255; t = v * (255 - s * (255 - f) // 255) // 255
    r, g, b = [(v, t, p), (q, v, p), (p, v, t), (p, q, v), (t, p, v), (v, p, q)][min(i, 5)]
    return (r, g, b)
def to5(c): return tuple(x >> 3 for x in c)
def to8(c): return tuple((x << 3) | (x >> 2) for x in c)
def season_ts(ts, mode, protect=frozenset()):
    """recolour a render.TS's 16 palettes in place (protect = {(slot, index)})"""
    for slot in range(16):
        ts.pal[slot] = [ts.pal[slot][0]] + [to8(seasonal(to5(c), mode, slot, (slot, i) in protect)) for i, c in enumerate(ts.pal[slot][1:], 1)]
    ts.cache = {}
def render_map(name, mode, protect=frozenset()):
    l = R.layout(name); ts = R.TS(l['primary_tileset'], l['secondary_tileset']); season_ts(ts, mode, protect)
    b = open(R.ROOT + '/' + l['blockdata_filepath'], 'rb').read(); W, H = l['width'], l['height']
    ids = [struct.unpack('<H', b[i*2:i*2+2])[0] & 0x3FF for i in range(W*H)]
    im = Image.new('RGB', (W*16, H*16))
    for i, v in enumerate(ids): im.paste(ts.metatile(v).convert('RGB'), ((i % W)*16, (i//W)*16))
    return im
if __name__ == '__main__':
    out = sys.argv[1]
    for n in sys.argv[2:]:
        ims = [render_map(n, m) for m in (NONE, SPRING, AUTUMN, FROST)]
        w, h = ims[0].size
        o = Image.new('RGB', (w*2+8, h*2+8), (255, 0, 255))
        for k, im in enumerate(ims): o.paste(im, ((k % 2)*(w+8), (k//2)*(h+8)))
        o.save(os.path.join(out, n + '.png'))
