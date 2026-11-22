#!/usr/bin/env python3
"""Brennan 2026-11-22 13:37 list B1 (Rusturf Tunnel: "The small rocks scattered across the tunnel floor ... are still
flat 2D ground textures - convert them into proper 3D rock models"): the drawn rocks' own pixels as cards in
voxel/trees.rgba5551, for voxel_tree.c's rock mounds (VoxelTree_EmitInstance: a mound of the card's columns, as the
Rock Smash rocks are - voxel_entities.c EmitMound - on the plain ground VoxelTree_GroundMetatile names).

Cards, 16x16 each, row 112: x 64 Rusturf 0x203 (an olive rock on the floor), 80 0x204 (olive rocks), 96 0x205 (a grey
rock on the rock tops), 112 0x206 (grey rocks). A rock pixel is one where the metatile's top layer is drawn over its
ground (the whole drawing differs from its ground layer alone). Rusturf's colours toned as voxel_gl.c CaveToneDown
tones its atlas (45% to grey, 88% bright).

usage: add_rock_cards.py TREES.rgba5551 (rewritten in place; needs 3ds_port/scripts)
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


def tone(c):
    r, g, b = c
    y = 0.299 * r + 0.587 * g + 0.114 * b
    return tuple(max(0, min(255, int(round((v + (y - v) * 0.45) * 0.88)))) for v in (r, g, b))


def main():
    from voxel_building import LayoutArt
    path = sys.argv[1]
    d = bytearray(open(path, 'rb').read())
    for layout, m, x0, toned in CARDS:
        assert all(struct.unpack_from('<H', d, 2 * tex(x, y))[0] & 1 == 0 for y in range(112, 128)
                   for x in range(x0, x0 + 16)), 'slot %d,112 not free' % x0
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
                struct.pack_into('<H', d, 2 * tex(x0 + x, 112 + y), pack((tone(c) if toned else c) if rock else None))
    open(path, 'wb').write(d)
    print('%d rock cards written' % len(CARDS))


if __name__ == '__main__':
    main()
