#!/usr/bin/env python3
"""Add (or replace) one cell's cut record in relief.bin (VXL4), in place.

usage: set_relief_cut.py RELIEF.bin LAYOUT_ID X,Y VARIANT FOOT_PX GROUND WALL SIDES
  VARIANT: the cut variant's index, or none (0xFFFF: the cell's drawing whole, on its relief)
  FOOT_PX: where `ground` runs on under the cell, pixels over the map's base (16 = a level)
  GROUND, WALL: metatile ids (hex or decimal), or none (0xFFFF)
  SIDES: the `sides` bits (voxel_relief.h: 0x0400 VOXEL_RELIEF_BLOCK_WALLS, 0x0200 VOXEL_RELIEF_GOES_ON, ...)
The records stay sorted by (layout, x, y) - the game finds them by halving (voxel_relief.c VoxelRelief_Cut).

Session 014PwRDj, 2026-11-27 (Jagged Pass, N79): the mound's two hip cells (15,16 and 17,16), re-laid without their
cut records, were left with no ground run on under them, so where their north edge stands over the ground north of
them a slit opened (seen from the north, a dark speck over the safety floor). Each gets the record its peak (16,16)
has: no variant, the ground 0x271 run on under it at the ground's height (56 px). Used:
  set_relief_cut.py relief.bin 292 15,16 none 56 0x271 none 0
  set_relief_cut.py relief.bin 292 17,16 none 56 0x271 none 0
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(__file__))
from splice_relief import parse, ROW


def num(s):
    return 0xFFFF if s.lower() == 'none' else int(s, 0)


def main():
    path, lid = sys.argv[1], int(sys.argv[2])
    x, y = (int(v) for v in sys.argv[3].split(','))
    variant, foot = num(sys.argv[4]), int(sys.argv[5])
    ground, wall, sides = num(sys.argv[6]), num(sys.argv[7]), int(sys.argv[8], 0)
    assert foot % 1 == 0 and -32768 <= foot <= 32767
    rec = struct.pack('<HBBHhHHH', lid, x, y, variant, foot, ground, wall, sides)
    p = parse(path)
    key = lambda c: (struct.unpack_from('<H', c, 0)[0], c[2], c[3])
    cells = [c for c in p['cells'] if key(c) != (lid, x, y)]
    replaced = len(cells) != len(p['cells'])
    cells.append(rec)
    cells.sort(key=key)
    blob = bytearray(p['head'])
    blob += b'\0' * (ROW * len(p['rows']))
    for i, (l, r, d) in enumerate(p['rows']):
        r = bytearray(r)
        struct.pack_into('<I', r, 8, len(blob))
        blob[8 + ROW * i: 8 + ROW * (i + 1)] = r
        blob += d
    at = len(blob)
    blob += struct.pack('<HH', len(p['variants']), len(cells))
    for v in p['variants']:
        blob += v
    for c in cells:
        blob += c
    if p['edges']:
        head = bytearray(p['edge_head'])
        struct.pack_into('<H', head, 4, len(p['edges']))
        blob += head
        for e in p['edges']:
            blob += e
    blob += struct.pack('<I', at) + b'CUTS'
    open(path, 'wb').write(blob)
    print('replaced' if replaced else 'added', 'cut record', x, y)


if __name__ == '__main__':
    main()
