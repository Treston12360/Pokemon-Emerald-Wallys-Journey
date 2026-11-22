#!/usr/bin/env python3
"""Brennan 2026-11-21 17:25 (image 8, Verdanturf: "All the red/yellow flower patches in Verdanturf Town are still flat
2D ground tiles - convert them to 3D flower models (matching the 3D flowers used elsewhere)"): a card at (48, 112),
16x16, in voxel/trees.rgba5551 - the Mauville tileset's flower (metatiles 0x211-0x216, one drawing; Verdanturf,
Mauville, Route 117) less every pixel of its grass's colours (0x204 and the General's 0x001), as add_tree_cards.py's
General flower at (0, 112). Renderer: voxel_tree.c VoxelTree_EmitInstance (CARD_FLOWER_MAUVILLE).

usage: add_mauville_flower_card.py TREES.rgba5551 (rewritten in place)
"""
import os, struct, sys
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, os.path.join(ROOT, '3ds_port', 'scripts'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from add_tree_cards import tex, pack

def main():
    from voxel_building import LayoutArt
    from dump_region_art import metatile_image
    path = sys.argv[1]
    d = bytearray(open(path, 'rb').read())
    assert all(struct.unpack_from('<H', d, 2 * tex(x, y))[0] & 1 == 0 for y in range(112, 128) for x in range(48, 64)), \
        'slot not free'
    a = LayoutArt('LAYOUT_VERDANTURF_TOWN')
    gcols = set()
    for m in (0x204, 0x205, 0x001):
        g = metatile_image(a.entries(m), a.ts).convert('RGB')
        gcols |= {g.getpixel((x, y)) for y in range(16) for x in range(16)}
    flower = metatile_image(a.entries(0x211), a.ts).convert('RGB')
    for y in range(16):
        for x in range(16):
            c = flower.getpixel((x, y))
            struct.pack_into('<H', d, 2 * tex(48 + x, 112 + y), pack(None if c in gcols else c))
    open(path, 'wb').write(d)
    print('card written')

if __name__ == '__main__':
    main()
