#!/usr/bin/env python3
"""Set one relief cell's 25 lattice heights (relief.bin, VXL4) to given values, in place.

usage: set_relief_cell.py RELIEF.bin LAYOUT_ID X,Y H0,H1,...,H24
  LAYOUT_ID: as relief.bin keys it (data/layouts/layouts.json position + 1; Jagged Pass = 292).
  H: pixels over the map's base (16 = a level), row by row from the cell's north edge, west to east in each row -
  the order tools/wjwork relief_cells.py prints them in.

Session 014PwRDj, 2026-11-27 (Brennan 2026-11-26 22:59, Jagged Pass: "dark gaps/missing faces at the top of the
ridge"): the north ends of the rock ridges by the tall grass sloped from their eaves up to their ridge line, open
under the plateau's flat edge beside them (a triangle of nothing on each side of the ridge). Their first row is made a
hip instead: its north edge the plateau's height all along, its south edge the ridge as it was.
Also used (N79): the mound by the cobbles (15,16 / 17,16), the far-right triangle (29,16: one slope plane meeting
29,17's edge) and the bottom-right tear (22,28: its rock rising from the ground at 8 px) - every command in
tools/voxel3ds/README.md, "Jagged Pass relief hand edits".
"""
import struct
import sys


def main():
    path, lid = sys.argv[1], int(sys.argv[2])
    x, y = (int(v) for v in sys.argv[3].split(','))
    hs = [int(v) for v in sys.argv[4].split(',')]
    assert len(hs) == 25, '25 heights'
    b = bytearray(open(path, 'rb').read())
    n = struct.unpack_from('<H', b, 4)[0]
    for i in range(n):
        lidr, count, w, hh, off = struct.unpack_from('<HHHHI', b, 8 + 14 * i)
        if lidr != lid:
            continue
        unit = 2 if hh & 0x4000 else 1
        wide = bool(w & 0x4000)
        stride = 52 if wide else 27
        for k in range(count):
            o = off + k * stride
            if (b[o], b[o + 1]) == (x, y):
                for g, h in enumerate(hs):
                    assert h % unit == 0, 'height %d not a multiple of %d' % (h, unit)
                    struct.pack_into('<h' if wide else '<b', b, o + 2 + (2 if wide else 1) * g, h // unit)
                open(path, 'wb').write(b)
                print('set', x, y)
                return
    sys.exit('cell not in the layout\'s relief')


if __name__ == '__main__':
    main()
