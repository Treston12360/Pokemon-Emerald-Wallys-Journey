#!/usr/bin/env python3
"""Lay every relief cell of one layout flat at one height, in place.

usage: flat_relief_layout.py RELIEF.bin LAYOUT_ID PX
  LAYOUT_ID: 1-based index in data/layouts/layouts.json (Route 108 = 24).  PX: height in pixels over the map's base.

Session 014PwRDj, 2026-11-20 (Brennan 12:27, Q9: "(a) Make Route 108's sea flat, at Route 109's level"). Route 108's
relief rose in equal steps from -16 px (west, beside Route 107) to 0 (east, beside Route 109) over the whole map, sea and
land alike; the flat background sea round it took -15 px and met Route 109's at 0 in a diagonal dark line.
"""
import struct, sys


def main():
    path, lid, px = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
    b = bytearray(open(path, 'rb').read())
    n = struct.unpack_from('<H', b, 4)[0]
    for j in range(n):
        l, cells, w, h, off = struct.unpack_from('<HHHHI', b, 8 + 14 * j)
        if l != lid:
            continue
        unit = 2 if h & 0x4000 else 1
        assert px % unit == 0
        for k in range(cells):
            o = off + k * 27 + 2
            struct.pack_into('25b', b, o, *([px // unit] * 25))
        open(path, 'wb').write(b)
        print('flat', cells, 'cells at', px)
        return
    sys.exit('layout not in relief')


if __name__ == '__main__':
    main()
