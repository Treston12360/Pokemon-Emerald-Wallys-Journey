#!/usr/bin/env python3
"""Brennan 2026-11-26 22:09 (Mt. Chimney: "all trees look to be 2D needs to be 3D"; Jagged Pass: "trees on the rocks
in red box are not 3D models"): the Lavaridge tile set's trees as 3D trees, cards in voxel/trees.rgba5551 (256x256
since 2026-11-26; these in its new right half):

  (128, 0)   32x36  the ash-covered large tree's crown (Jagged Pass, Mt. Chimney's slopes: Lavaridge 2C0-2CB, 2CE-2CF):
                    the General large tree's crown card (0, 0) - the ash tree is drawn with the General large tree's
                    tiles in Lavaridge's palette 11 - recoloured by palette index, General 2 -> Lavaridge 11
  (160, 0)   32x32  a large tree's ground on the pink volcanic rock (Lavaridge's green 2D6-2E5 and ash trees): the
                    General large tree's ground (32, 0) with its grass as the rock (Lavaridge 0x271's drawing) and its
                    shadow in the rock's own shades (palette 10 colours 11, 12, and a darker 125,80,68), graded as the
                    General card's over its grass - Brennan 2026-11-26 22:59: the 2D art's purple shadow (palette 11
                    colours 3, 4) read as a pit round the trunk and as cave mouths under the upper trees
  (192, 0)   32x32  the same for the ash-covered tree, its roots and trunk in the ash trunk's dark red-browns
  (128, 48)  16x32  the ash-covered small tree's crown (Mt. Chimney's 2CC/2CD, the General small tree's tiles in palette
                    11): the General small crown card (32, 32) recoloured, 2 -> 11
  (144, 48)  16x16  its trunk's ground: the General small trunk (48, 32), grass -> the rock (Lavaridge 2CC's lower layer),
                    its shadow in the rock's shades (outer ring palette 10 colour 11, middle colour 12)
The ash crowns' two lightest colours are 0.84 / 0.88 as bright as the 2D tiles' (22:59: "glowing white halo/bloom").

The green Lavaridge trees' crown is the General large tree's own card (the same tiles and palette 2, but for their
outermost columns). Renderer: voxel_tree.c (VoxelTree_Part, LargeArtOf, EmitSmallCell; LAV_* / TRUNK_ASH).

usage: add_lavaridge_tree_cards.py TREES.rgba5551 [--redo]   (rewritten in place; --redo: over its own cards)
"""
import os
import struct
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from add_tree_cards import tex, pack, unpack  # noqa: E402


def pal(path):
    lines = open(path).read().split('\n')[3:19]
    return [tuple(int(v) for v in ln.split()) for ln in lines if ln.strip()]


def q5(c):
    """a colour as the texture stores it (5 bits a channel), for matching"""
    return (c[0] >> 3 << 3, c[1] >> 3 << 3, c[2] >> 3 << 3)


def metatile_pixels(metatile, layers):
    """Lavaridge metatile `metatile` (0x200..) drawn from the repo's tile data: 16x16 of (palette, index) or None"""
    gen = os.path.join(ROOT, 'data/tilesets/primary/general')
    lav = os.path.join(ROOT, 'data/tilesets/secondary/lavaridge')
    tiles = open(os.path.join(gen, 'tiles.4bpp'), 'rb').read()
    tiles = [tiles[i:i + 32] for i in range(0, len(tiles), 32)]
    tiles += [bytes(32)] * (512 - len(tiles))
    st = open(os.path.join(lav, 'tiles.4bpp'), 'rb').read()
    tiles += [st[i:i + 32] for i in range(0, len(st), 32)]
    meta = open(os.path.join(lav, 'metatiles.bin'), 'rb').read()
    t8 = struct.unpack_from('<8H', meta, (metatile - 0x200) * 16)
    out = [[None] * 16 for _ in range(16)]
    for layer in layers:
        for k in range(4):
            e = t8[layer * 4 + k]
            ti, hf, vf, pl = e & 0x3FF, e >> 10 & 1, e >> 11 & 1, e >> 12
            tb = tiles[ti]
            for yy in range(8):
                for xx in range(8):
                    sx, sy = (7 - xx if hf else xx), (7 - yy if vf else yy)
                    b = tb[sy * 4 + sx // 2]
                    c = b >> 4 if sx & 1 else b & 15
                    if c:
                        out[(k >> 1) * 8 + yy][(k & 1) * 8 + xx] = (pl, c)
    return out


def main():
    path = sys.argv[1]
    d = bytearray(open(path, 'rb').read())
    assert len(d) == 256 * 256 * 2, 'trees.rgba5551 must be 256x256'
    g2 = pal(os.path.join(ROOT, 'data/tilesets/primary/general/palettes/02.pal'))
    lav10 = pal(os.path.join(ROOT, 'data/tilesets/secondary/lavaridge/palettes/10.pal'))
    lav11 = pal(os.path.join(ROOT, 'data/tilesets/secondary/lavaridge/palettes/11.pal'))
    lav = {10: lav10, 11: lav11}
    by_g2 = {q5(c): i for i, c in enumerate(g2)}

    def get(x, y):
        return unpack(struct.unpack_from('<H', d, 2 * tex(x, y))[0])

    def put(x, y, c):
        struct.pack_into('<H', d, 2 * tex(x, y), pack(c))

    if '--redo' not in sys.argv:
        for (x0, y0, w, h) in ((128, 0, 96, 36), (128, 48, 32, 32)):
            assert all(get(x, y) is None for y in range(y0, y0 + h) for x in range(x0, x0 + w)), 'slots not free'

    # Brennan 2026-11-26 22:59 ("overexposed, glowing white halo/bloom" on the upper ash trees): the two lightest ash
    # colours a little darker, under the bloom's glare in the sun (the 2D tiles keep theirs)
    dim = {1: 0.84, 2: 0.88}

    def recolour(c):
        i = by_g2.get(c)
        if i is None or i not in (1, 2, 3, 4, 6, 8):
            return c
        k = dim.get(i, 1.0)
        return tuple(int(v * k) for v in lav11[i])

    # ash crowns: the General cards recoloured by palette index
    for (sx, sy, dx, dy, w, h) in ((0, 0, 128, 0, 32, 36), (32, 32, 128, 48, 16, 32)):
        for y in range(h):
            for x in range(w):
                c = get(sx + x, sy + y)
                put(dx + x, dy + y, None if c is None else recolour(c))

    rock = metatile_pixels(0x271, (0, 1))           # the plain pink rock
    under = metatile_pixels(0x2CC, (0,))             # the small ash tree's rock, its lower layer
    # The shadow in the rock's own shades, graded as the General card's is over its grass (ring 83%, shadow 62% and
    # 38% of the grass): Brennan 22:59 - the 2D art's purple shadow (palette 11 colours 3, 4), all of it seen under a
    # standing crown, read as a pit round the trunk ("partially buried/clipped inside a raised circular rock/dirt
    # mound") and as cave mouths under the upper trees ("rock tunnel/cave arches")
    shadow_edge, shadow_mid, shadow_dark = lav10[11], lav10[12], (125, 80, 68)
    grass = {q5(g2[i]) for i in (12, 13, 15)} | {(64, 184, 136)}
    ring = {q5(g2[14]), (48, 144, 104)}

    def ground(c, rx, ry, src, small=False):
        if c is None:
            return None
        if c in grass:
            p = src[ry % 16][rx % 16]
            return lav[p[0]][p[1]] if p else None
        if c in ring:
            return shadow_edge
        i = by_g2.get(c)
        if i == 3 and small:
            return shadow_edge   # (the small trunk's square is all shadow: its outer ring the lighter shadow, as 2D)
        if i == 3:
            return shadow_mid
        if i == 4:
            return shadow_mid if small else shadow_dark
        return c  # the roots and trunk, as drawn

    for y in range(32):
        for x in range(32):
            put(160 + x, y, ground(get(32 + x, y), x, y, rock))
    # the ash tree's: its roots and trunk in the 2D ash trunk's dark red-browns (palette 11 13, 6, 8), by brightness
    browns = {(144, 112, 72): lav11[13], (120, 96, 72): lav11[13], (104, 88, 64): lav11[6], (88, 80, 64): lav11[6],
              (80, 72, 64): lav11[8]}
    for y in range(32):
        for x in range(32):
            c = get(160 + x, y)
            put(192 + x, y, browns.get(c, c) if c is not None else None)
    for y in range(16):
        for x in range(16):
            put(144 + x, 48 + y, ground(get(48 + x, 32 + y), x, y, under, True))
    open(path, 'wb').write(d)
    print('Lavaridge tree cards written')


if __name__ == '__main__':
    main()
