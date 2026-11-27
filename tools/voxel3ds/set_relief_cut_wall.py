#!/usr/bin/env python3
"""Change the wall metatile of a layout's cut records in relief.bin (VXL4), in place.

usage: set_relief_cut_wall.py RELIEF.bin LAYOUT_ID FROM TO     (FROM, TO: metatile ids, hex or decimal)

Session 014PwRDj, 2026-11-27 (Brennan 2026-11-26 22:59, Jagged Pass: "A dark, thin horizontal gap/tear runs right along
the top seam of the cliff ridge"): 56 of Jagged Pass's cut records (layout 292: its ledges' faces) name the General
tileset's grey cliff 0x07C as their wall - drawn in the General's palette, a dark blue-grey band under every red ledge.
The map's own red cliff is the Lavaridge tileset's 0x274 (its other 65 records use it). Used: 292 0x07C 0x274.
"""
import struct
import sys

CUT_VARIANT = 36
CUT_CELL = 14


def main():
    path, lid = sys.argv[1], int(sys.argv[2])
    old, new = int(sys.argv[3], 0), int(sys.argv[4], 0)
    b = bytearray(open(path, 'rb').read())
    assert b[:4] == b'VXL4' and b[-4:] == b'CUTS'
    at = struct.unpack_from('<I', b, len(b) - 8)[0]
    nv, nc = struct.unpack_from('<HH', b, at)
    c0 = at + 4 + CUT_VARIANT * nv
    done = 0
    for i in range(nc):
        o = c0 + CUT_CELL * i
        if struct.unpack_from('<H', b, o)[0] == lid and struct.unpack_from('<H', b, o + 10)[0] == old:
            struct.pack_into('<H', b, o + 10, new)
            done += 1
    open(path, 'wb').write(b)
    print('walls changed:', done)


if __name__ == '__main__':
    main()
