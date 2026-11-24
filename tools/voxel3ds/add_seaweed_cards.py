#!/usr/bin/env python3
"""Brennan 2026-11-23 21:03 (audit item 31, the dive areas' seaweed lying flat on the trench floor: "stand them up
like tall grass"): two 16x16 cards in voxel/trees.rgba5551 - the Underwater tileset's seaweed (metatile 0x201,
Route 126's trenches, at (96, 48); 0x281, the same drawing in Route 124's darker palette, at (112, 48)) less every
pixel of the trench floor's colours round it (0x216 and 0x2A9, the floors next to the patches). Renderer:
voxel_tree.c VoxelTree_EmitInstance (EmitSeaweedCard); the floor under them is VoxelTree_GroundMetatile's.

usage: add_seaweed_cards.py TREES.rgba5551 (rewritten in place)
"""
import os, struct, sys
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, os.path.join(ROOT, '3ds_port', 'scripts'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from add_tree_cards import tex, pack

CARDS = ((0x201, 0x216, 96), (0x281, 0x2A9, 112))


def main():
    from voxel_building import LayoutArt
    from dump_region_art import metatile_image
    path = sys.argv[1]
    d = bytearray(open(path, 'rb').read())
    a = LayoutArt('LAYOUT_UNDERWATER_ROUTE124')
    for weed, floor, sx in CARDS:
        free = all(struct.unpack_from('<H', d, 2 * tex(x, y))[0] & 1 == 0 for y in range(48, 64) for x in range(sx, sx + 16))
        if not free:
            print('slot %d,48 already written: skipped' % sx)
            continue
        f = metatile_image(a.entries(floor), a.ts).convert('RGB')
        fcols = {f.getpixel((x, y)) for y in range(16) for x in range(16)}
        w = metatile_image(a.entries(weed), a.ts).convert('RGB')
        fcols |= {w.getpixel((x, y)) for y in range(16) for x in range(16)
                  if w.getpixel((x, y))[2] > 150 and w.getpixel((x, y))[0] < 130 and w.getpixel((x, y))[1] < 90}  # its own purple floor
        for y in range(16):
            for x in range(16):
                c = w.getpixel((x, y))
                struct.pack_into('<H', d, 2 * tex(sx + x, 48 + y), pack(None if c in fcols else c))
    open(path, 'wb').write(d)
    print('cards written')


if __name__ == '__main__':
    main()
