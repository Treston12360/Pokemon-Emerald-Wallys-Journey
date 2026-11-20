#!/usr/bin/env python3
"""Re-lay a block of relief cells as one surface blended between the columns beside it, in place (relief.bin VXL4,
narrow cells). Every lattice point of cells X0..X1 x Y0..Y1 takes the height interpolated, along its row, between
the east edge of cell X0-1 and the west edge of cell X1+1 at the same lattice row - so the block's edges weld to
both neighbours exactly and the region steps down with them, row by row (a contour pass, not per-square values).

usage: blend_relief_region.py RELIEF.bin LAYOUT_ID X0 X1 Y0 Y1 [--cols]
  --cols: blend down each column instead (between the south edge of row Y0-1 and the north edge of row Y1+1), for a
          block on a map's east or west edge, where the row has no neighbour on one side.

POL (Brennan 2026-11-19 22:18, Mt Pyre outside, right of the small round rock): "rebuild that entire eastern cliff
corner as a continuous stepped elevation contour" - the generator left a spike of rock (-16..0 px) standing in a
cliff that falls from -16 to -64 on both sides of it. Used: 302 30 32 21 25.
"""
import struct, sys

def main():
    cols = '--cols' in sys.argv
    args = [a for a in sys.argv if a != '--cols']
    path, lid, x0, x1, y0, y1 = args[1], *map(int, args[2:7])
    b = bytearray(open(path, 'rb').read())
    n = struct.unpack_from('<H', b, 4)[0]
    for i in range(n):
        l, cells, w, h, off = struct.unpack_from('<HHHHI', b, 8 + 14 * i)
        if l != lid:
            continue
        assert not (w & 0x4000), 'wide cells not handled'
        unit = 2 if h & 0x4000 else 1
        at = {(b[off + k * 27], b[off + k * 27 + 1]): off + k * 27 + 2 for k in range(cells)}
        def get(x, y, j, k):
            return struct.unpack_from('<b', b, at[(x, y)] + j * 5 + k)[0] * unit
        done = 0
        if cols:
            span = (y1 + 1) - y0
            for x in range(x0, x1 + 1):
                assert (x, y0 - 1) in at and (x, y1 + 1) in at, 'needs both neighbours in column %d' % x
                for y in range(y0, y1 + 1):
                    if (x, y) not in at:
                        print('no cell', x, y); continue
                    for k in range(5):
                        north, south = get(x, y0 - 1, 4, k), get(x, y1 + 1, 0, k)
                        for j in range(5):
                            t = ((y - y0) + j / 4.0) / span
                            q = int(round((north + (south - north) * t) / unit))
                            struct.pack_into('<b', b, at[(x, y)] + j * 5 + k, max(-128, min(127, q)))
                    done += 1
            print('blended', done, 'cells (columns)')
            continue
        span = (x1 + 1) - x0
        for y in range(y0, y1 + 1):
            assert (x0 - 1, y) in at and (x1 + 1, y) in at, 'needs both neighbours in row %d' % y
            for x in range(x0, x1 + 1):
                if (x, y) not in at:
                    print('no cell', x, y); continue
                for j in range(5):
                    west, east = get(x0 - 1, y, j, 4), get(x1 + 1, y, j, 0)
                    for k in range(5):
                        t = ((x - x0) + k / 4.0) / span
                        v = west + (east - west) * t
                        q = int(round(v / unit))
                        struct.pack_into('<b', b, at[(x, y)] + j * 5 + k, max(-128, min(127, q)))
                done += 1
        print('blended', done, 'cells')
    open(path, 'wb').write(b)

if __name__ == '__main__':
    main()
