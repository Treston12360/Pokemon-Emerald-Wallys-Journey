#!/usr/bin/env python3
"""POL-7 (Brennan 2026-11-16: flowers, flowerbeds and short grass "same as tall grass"): two cards in the free
bottom row of voxel/trees.rgba5551 (128x128 RGBA5551, PICA 8x8 Morton tiles; rows 112-127 were empty):

  (0, 112)  16x16  the General tileset's flowers (metatile 0x004) - its drawing less every pixel of the plain
                   grass's three colours (metatile 0x001: base 73C5A4, specks 41B483 / A4D5C5)
  (16, 112) 16x8   a short-grass tuft - the top 8 rows (the blades' tips) of the long-grass tuft already in
                   the texture at (80, 32), recoloured to the short grass's colours (dark -> 41B483,
                   mid -> 73C5A4, light -> A4D5C5; its darkest outline -> 41B483)

usage: add_tree_cards.py TREES.rgba5551 (rewritten in place)
Needs the repo's tileset graphics built (data/tilesets/**/tiles.4bpp + palettes/*.gbapal, gbagfx) and
3ds_port/scripts (Emerald 3DS's art reader). Renderer side: voxel_tree.c (VoxelTree_EmitInstance).
"""
import os, struct, sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, os.path.join(ROOT, '3ds_port', 'scripts'))

def tex(x, y, w=128):
    m = (x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3)
    return ((y // 8) * (w // 8) + x // 8) * 64 + m

def pack(c):
    if c is None:
        return 0
    r, g, b = c
    return ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) | 1

def unpack(v):
    return None if not v & 1 else (((v >> 11) & 31) * 8, ((v >> 6) & 31) * 8, ((v >> 1) & 31) * 8)

def main():
    from voxel_building import LayoutArt
    from dump_region_art import metatile_image
    path = sys.argv[1]
    d = bytearray(open(path, 'rb').read())
    assert all(struct.unpack_from('<H', d, 2 * tex(x, y))[0] & 1 == 0 for y in range(112, 128) for x in range(0, 32)), \
        'slots not free'
    a = LayoutArt('LAYOUT_ROUTE101')
    grass = metatile_image(a.entries(0x001), a.ts).convert('RGB')
    flower = metatile_image(a.entries(0x004), a.ts).convert('RGB')
    gcols = {grass.getpixel((x, y)) for y in range(16) for x in range(16)}
    for y in range(16):
        for x in range(16):
            c = flower.getpixel((x, y))
            struct.pack_into('<H', d, 2 * tex(x, 112 + y), pack(None if c in gcols else c))
    # the tuft: long grass (80,32) rows 32..39, recoloured
    remap = {(56, 136, 48): (65, 180, 131), (128, 192, 96): (115, 197, 164), (176, 248, 128): (164, 213, 197),
             (56, 80, 0): (65, 180, 131)}
    for y in range(8):
        for x in range(16):
            c = unpack(struct.unpack_from('<H', d, 2 * tex(80 + x, 32 + y))[0])
            struct.pack_into('<H', d, 2 * tex(16 + x, 112 + y), pack(remap.get(c, c) if c else None))
    open(path, 'wb').write(d)
    print('cards written')

if __name__ == '__main__':
    main()
