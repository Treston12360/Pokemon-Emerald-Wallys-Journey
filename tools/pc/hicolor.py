#!/usr/bin/env python3
"""ENG-SPRITES (Project ARIA, owner 2026-11-14): full-colour sprites, same size as the game's own.

graphics/hicolor/list.txt, one picture per line (blank lines and # comments allowed):

    FULL_COLOUR.png  GAME.png                 the sprite has its own palette (trainer/battle pictures):
                                              GAME.png is rewritten as the 16-colour version of FULL_COLOUR.png
    FULL_COLOUR.png  GAME.png  PALETTE.pal    the sprite uses a shared palette (overworld people): GAME.png is
                                              rewritten using only that palette's colours

FULL_COLOUR.png must be exactly the size of GAME.png (same frames, same layout); transparent pixels
(alpha < 128) stay transparent. Any number of colours.

The game builds and loads GAME.png exactly as before. On PC, src/platform/hicolor.c spots those tiles in sprite
memory and draws the full-colour pixels instead (and fades them with the palette). This script writes
src/platform/hicolor_tiles.h for that. Run it after changing the list or a picture:

    python3 tools/pc/hicolor.py
"""
import os, sys
from PIL import Image

LIST = sys.argv[1] if len(sys.argv) > 1 else 'graphics/hicolor/list.txt'
OUT = 'src/platform/hicolor_tiles.h'


def rgb555(c):
    r, g, b = c[:3]
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)


def pal_hash(colors555):
    h = 2166136261
    for c in colors555[1:16]:
        h = ((h ^ (c & 0x7FFF)) * 16777619) & 0xFFFFFFFF
    return h


def read_jasc(path):
    lines = open(path).read().split()
    assert lines[0] == 'JASC-PAL', path
    n = int(lines[2])
    vals = list(map(int, lines[3:3 + n * 3]))
    return [tuple(vals[i * 3:i * 3 + 3]) for i in range(n)][:16]


def nearest(c, pal):
    best, bi = None, 1
    for i in range(1, len(pal)):
        p = pal[i]
        d = (c[0] - p[0]) ** 2 * 3 + (c[1] - p[1]) ** 2 * 4 + (c[2] - p[2]) ** 2 * 2
        if best is None or d < best:
            best, bi = d, i
    return bi


def make_indexed(hi, game_png, pal_file):
    w, h = hi.size
    px = hi.load()
    if pal_file:
        pal = read_jasc(pal_file)
        while len(pal) < 16:
            pal.append((0, 0, 0))
        idx = [[0 if px[x, y][3] < 128 else nearest(px[x, y], pal) for x in range(w)] for y in range(h)]
    else:
        # own palette: 15 colours for the opaque pixels, index 0 = transparent
        opaque = Image.new('RGB', (w, h))
        op = opaque.load()
        cols = []
        for y in range(h):
            for x in range(w):
                if px[x, y][3] >= 128:
                    op[x, y] = px[x, y][:3]
                    cols.append(px[x, y][:3])
        q = Image.new('RGB', (len(cols), 1))
        q.putdata(cols)
        q = q.quantize(15, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
        qp = q.getpalette()[:45]
        pal = [(0, 0, 0)] + [tuple(qp[i * 3:i * 3 + 3]) for i in range(15)]
        old = Image.open(game_png)
        if old.mode == 'P':
            pal[0] = tuple(old.getpalette()[:3])  # keep the old "transparent" colour
        idx = [[0 if px[x, y][3] < 128 else nearest(px[x, y], pal) for x in range(w)] for y in range(h)]
    out = Image.new('P', (w, h))
    flat = []
    for c in pal[:16]:
        flat += list(c)
    out.putpalette(flat + [0] * (768 - len(flat)))
    out.putdata([i for row in idx for i in row])
    if not pal_file:
        out.save(game_png)
    return idx, [rgb555(c) for c in pal[:16]]


def main():
    tiles = {}
    for ln in open(LIST):
        ln = ln.split('#')[0].split()
        if not ln:
            continue
        hi_path, game_png = ln[0], ln[1]
        pal_file = ln[2] if len(ln) > 2 else None
        hi = Image.open(hi_path).convert('RGBA')
        g = Image.open(game_png)
        assert hi.size == g.size, f'{hi_path} is {hi.size}, {game_png} is {g.size}: they must match'
        idx, pal555 = make_indexed(hi, game_png, pal_file)
        if pal_file:
            # the game's own picture keeps its pixels: check it still matches what the palette gives
            pass
        ph = pal_hash(pal555)
        w, h = hi.size
        px = hi.load()
        for ty in range(0, h, 8):
            for tx in range(0, w, 8):
                b = bytearray(32)
                pix = []
                for y in range(8):
                    for x in range(8):
                        i = idx[ty + y][tx + x]
                        b[y * 4 + x // 2] |= i << (4 * (x & 1))
                        c = px[tx + x, ty + y]
                        pix.append(rgb555(c) if c[3] >= 128 else 0)
                if not any(b):
                    continue
                tiles[(bytes(b), ph, tuple(pix))] = 1
        if pal_file:
            out = Image.new('P', (w, h))
            g2 = Image.open(game_png)
            out.putpalette(g2.getpalette())
            out.putdata([i for row in idx for i in row])
            out.save(game_png)
    keys = sorted(tiles, key=lambda k: (k[0], k[1]))
    lines = ['// Generated by tools/pc/hicolor.py from graphics/hicolor/list.txt - do not edit.',
             f'#define HICOLOR_TILE_COUNT {len(keys)}',
             'static const struct HiColorTile sHiColorTiles[%d] = {' % max(1, len(keys))]
    for b, ph, pix in keys:
        lines.append('    {0x%08X, {%s}, {%s}},' % (ph, ','.join('%d' % x for x in b), ','.join('0x%04X' % p for p in pix)))
    lines.append('};')
    open(OUT, 'w').write('\n'.join(lines) + '\n')
    print(len(keys), 'tiles ->', OUT)


if __name__ == '__main__':
    main()
