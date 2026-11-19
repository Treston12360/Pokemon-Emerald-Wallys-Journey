#!/usr/bin/env python3
"""Copy relief cells' 25 lattice heights from one column to another, mirrored east-west, in place (relief.bin VXL4,
narrow cells only).

usage: mirror_relief_cells.py RELIEF.bin LAYOUT_ID SRC_X DST_X Y0 Y1   (rows Y0..Y1 inclusive)

POL-51 (Brennan 2026-11-19 16:30, Mt Pyre outside, top path to the summit: "the brown rock cliff wall on the left
side is present, but the matching brown rock cliff wall on the right side disappeared into a flat/hollow strip ...
mirror"): the generator (gen_voxel_relief.py, VOXEL_SPREAD_OK) left the path's east rock (x 21-22, metatiles 33B and
072) at 0 px while the west rock (x 17-18, 070 and 33D) rises to 48. Used: 302 17 22 0 9 and 302 18 21 0 9.
"""
import struct, sys

def main():
    path, lid, sx, dx, y0, y1 = sys.argv[1], *map(int, sys.argv[2:7])
    b = bytearray(open(path, 'rb').read())
    n = struct.unpack_from('<H', b, 4)[0]
    for i in range(n):
        l, cells, w, h, off = struct.unpack_from('<HHHHI', b, 8 + 14 * i)
        if l != lid:
            continue
        assert not (w & 0x4000), 'wide cells not handled'
        at = {}
        for k in range(cells):
            o = off + k * 27
            at[(b[o], b[o + 1])] = o + 2
        done = 0
        for y in range(y0, y1 + 1):
            if (sx, y) not in at or (dx, y) not in at:
                print('missing cell', (sx, y), (dx, y)); continue
            src = bytes(b[at[(sx, y)]: at[(sx, y)] + 25])
            for j in range(5):
                for k in range(5):
                    b[at[(dx, y)] + j * 5 + k] = src[j * 5 + (4 - k)]
            done += 1
        print('mirrored', done, 'cells')
    open(path, 'wb').write(b)

if __name__ == '__main__':
    main()
