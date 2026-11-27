#!/usr/bin/env python3
"""Brennan 2026-11-22 13:37 list B1 (Rusturf Tunnel: "The small rocks scattered across the tunnel floor ... are still
flat 2D ground textures - convert them into proper 3D rock models"): the drawn rocks' own pixels as cards in
voxel/trees.rgba5551, for voxel_tree.c's rock mounds (VoxelTree_EmitInstance: a mound of the card's columns, as the
Rock Smash rocks are - voxel_entities.c EmitMound - on the plain ground VoxelTree_GroundMetatile names).

Cards, 16x16 each, row 112: x 64 Rusturf 0x203 (an olive rock on the floor), 80 0x204 (olive rocks), 96 0x205 (a grey
rock on the rock tops), 112 0x206 (grey rocks). A rock pixel is one where the metatile's top layer is drawn over its
ground (the whole drawing differs from its ground layer alone). Rusturf's colours toned as voxel_gl.c CaveToneDown
tones its atlas (45% to grey, 88% bright).

usage: add_rock_cards.py TREES.rgba5551 [--only-new] (rewritten in place; needs 3ds_port/scripts)
"""
import os
import struct
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, os.path.join(ROOT, '3ds_port', 'scripts'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from add_tree_cards import tex, pack  # noqa: E402

CARDS = [('LAYOUT_RUSTURF_TUNNEL', 0x203, 64, True), ('LAYOUT_RUSTURF_TUNNEL', 0x204, 80, True),
         ('LAYOUT_RUSTURF_TUNNEL', 0x205, 96, True), ('LAYOUT_RUSTURF_TUNNEL', 0x206, 112, True)]
# Brennan 2026-11-22 (C7): the four rocks by Fallarbor's Contest Hall (Fallarbor 0x223, a mossy boulder on the soil),
# card at (16, 48), not toned. Written with --only-new (the Rusturf cards already there).
# (2026-11-26: it was at (0, 32), and 0x20E below at (16, 32) - rows 32-35 there are the bottom of the large tree's
# crown card, which must stay clear: the rocks' tops showed under every large crown where the ground dips. Moved.)
CARDS_ROW = {('LAYOUT_FALLARBOR_TOWN', 0x223): 48}
CARDS += [('LAYOUT_FALLARBOR_TOWN', 0x223, 16, False)]
# C9: Meteor Falls' floor rocks 0x20E at (64, 48), 0x20F at (0, 48) (0x21F, the same rocks on the darker floor, uses
# 0x20F's card)
CARDS_ROW.update({('LAYOUT_METEOR_FALLS_1F_1R', 0x20E): 48, ('LAYOUT_METEOR_FALLS_1F_1R', 0x20F): 48})
CARDS += [('LAYOUT_METEOR_FALLS_1F_1R', 0x20E, 64, False), ('LAYOUT_METEOR_FALLS_1F_1R', 0x20F, 0, False)]


def slot_free(d, x0, y0):
    return all(struct.unpack_from('<H', d, 2 * tex(x, y))[0] & 1 == 0 for y in range(y0, y0 + 16)
               for x in range(x0, x0 + 16))


# Brennan 2026-11-23 06:28 (Meteor Falls' 3D rocks came out darker than the drawing): "brighter them to darker grey"
# - its cards' colours lifted BRIGHT x (still a grey darker than the floor). Applied as the cards are written.
BRIGHT = {('LAYOUT_METEOR_FALLS_1F_1R', 0x20E): 1.3, ('LAYOUT_METEOR_FALLS_1F_1R', 0x20F): 1.3}


def bright(c, k):
    return tuple(min(255, int(round(v * k))) for v in c)


def tone(c):
    r, g, b = c
    y = 0.299 * r + 0.587 * g + 0.114 * b
    return tuple(max(0, min(255, int(round((v + (y - v) * 0.45) * 0.88)))) for v in (r, g, b))


def main():
    from voxel_building import LayoutArt
    path = sys.argv[1]
    d = bytearray(open(path, 'rb').read())
    only_new = '--only-new' in sys.argv
    # --redo LAYOUT:HEX[,LAYOUT:HEX]: write just these cards again over their slots
    only = None
    if '--redo' in sys.argv:
        only = {(a.split(':')[0], int(a.split(':')[1], 16)) for a in sys.argv[sys.argv.index('--redo') + 1].split(',')}
        for (lay, mm) in only:
            x0 = [c[2] for c in CARDS if c[0] == lay and c[1] == mm][0]
            y0 = CARDS_ROW.get((lay, mm), 112)
            for yy in range(y0, y0 + 16):
                for xx in range(x0, x0 + 16):
                    struct.pack_into('<H', d, 2 * tex(xx, yy), 0)
    for layout, m, x0, toned in CARDS:
        y0 = CARDS_ROW.get((layout, m), 112)
        if only and (layout, m) not in only:
            continue
        if only_new and ((layout, m) not in CARDS_ROW or not slot_free(d, x0, CARDS_ROW[(layout, m)])):
            continue
        assert all(struct.unpack_from('<H', d, 2 * tex(x, y))[0] & 1 == 0 for y in range(y0, y0 + 16)
                   for x in range(x0, x0 + 16)), 'slot %d,%d not free' % (x0, y0)
        a = LayoutArt(layout)
        base = a.cell_image(m, layers=(0,)).convert('RGB')
        full = a.cell_image(m).convert('RGB')
        drawn = {(x, y) for y in range(16) for x in range(16) if full.getpixel((x, y)) != base.getpixel((x, y))}
        # the rock's own holes (its highlights are the ground's colour, drawn through it) are rock too: only what the
        # card's edge reaches without crossing the rock is ground
        out, todo = set(), [(x, y) for x in range(-1, 17) for y in (-1, 16)] + [(x, y) for y in range(16) for x in (-1, 16)]
        while todo:
            q = todo.pop()
            if q in out or q in drawn or not (-1 <= q[0] <= 16 and -1 <= q[1] <= 16):
                continue
            out.add(q)
            todo += [(q[0] + 1, q[1]), (q[0] - 1, q[1]), (q[0], q[1] + 1), (q[0], q[1] - 1)]
        for y in range(16):
            for x in range(16):
                c = full.getpixel((x, y))
                rock = (x, y) not in out
                c = tone(c) if toned else bright(c, BRIGHT.get((layout, m), 1.0))
                struct.pack_into('<H', d, 2 * tex(x0 + x, y0 + y), pack(c if rock else None))
    open(path, 'wb').write(d)
    print('%d rock cards written' % len(CARDS))


if __name__ == '__main__':
    main()
