#!/usr/bin/env python3
"""Set relief cells' 25 lattice heights (relief.bin, VXL4) to one value, in place.

usage: patch_relief_cells.py RELIEF.bin LAYOUT_ID HEIGHT_PX X,Y [X,Y ...]

POL-5 (2026-11-16, session 014PwRDj): the Trainer Hill door model (buildings.bin, cells 30..32 x 112..113
of Route 111 and Route111_NoMirageTower) stands on the lift of the cell under its door's foot
(VoxelBuildings_EmitSome: VoxelRelief_CellLift of its rectangle's bottom-left cell). Those cells' relief
(16 px at their north edge to 0 at their south) lifted it 8 px - a strip of light showed at its foot.
The model owns those cells (the relief is not drawn there), so they are set to 0: it stands on the ground.
"""
import struct, sys

def main():
    path, lid, h = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
    cells = {tuple(map(int, a.split(','))) for a in sys.argv[4:]}
    b = bytearray(open(path, 'rb').read())
    n = struct.unpack_from('<H', b, 4)[0]
    done = 0
    for i in range(n):
        lidr, count, w, hh, off = struct.unpack_from('<HHHHI', b, 8 + 14 * i)
        if lidr != lid:
            continue
        unit = 2 if hh & 0x4000 else 1
        wide = bool(w & 0x4000)
        stride = 52 if wide else 27
        for k in range(count):
            o = off + k * stride
            if (b[o], b[o + 1]) in cells:
                for g in range(25):
                    if wide:
                        struct.pack_into('<h', b, o + 2 + 2 * g, h // unit)
                    else:
                        struct.pack_into('<b', b, o + 2 + g, h // unit)
                done += 1
    assert done == len(cells), 'cells found: %d of %d' % (done, len(cells))
    open(path, 'wb').write(b)
    print('patched', done)

if __name__ == '__main__':
    main()
