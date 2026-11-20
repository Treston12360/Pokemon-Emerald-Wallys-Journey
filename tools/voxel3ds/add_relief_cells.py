#!/usr/bin/env python3
"""Add flat relief cells (all 25 lattice heights = H px) where a layout has none, in place (relief.bin VXL4, narrow
cells). Cells that already exist are left alone.

usage: add_relief_cells.py RELIEF.bin LAYOUT_ID HEIGHT_PX X0 X1 Y0 Y1   (inclusive)

POL-51 (Brennan 2026-11-19 19:07, stairs carved into the cliff): a stair flight is recessed by laying the ground above
it back (voxel_relief.c LayFlight, its lip); ground with no relief cell can't be laid back. Mt Pyre outside's flight at
x24-26 row 21 has none in rows 19-20 above it (plain ground at the base). Used: 302 0 24 26 19 20.
"""
import struct, sys
sys.path.insert(0, __import__('os').path.dirname(__file__))
from splice_relief import parse, ROW

def main():
    path, lid, h, x0, x1, y0, y1 = sys.argv[1], *map(int, sys.argv[2:8])
    p = parse(path)
    side = p['side']
    out_rows = []
    for l, r, d in p['rows']:
        if l == lid:
            cells, w, hh = struct.unpack_from('<HHH', r, 2)
            assert not (w & 0x4000), 'wide cells not handled'
            unit = 2 if hh & 0x4000 else 1
            stride = 2 + side * side
            recs = [d[k * stride:(k + 1) * stride] for k in range(cells)]
            have = {(c[0], c[1]) for c in recs}
            add = 0
            for y in range(y0, y1 + 1):
                for x in range(x0, x1 + 1):
                    if (x, y) not in have:
                        recs.append(bytes([x, y]) + struct.pack('<%db' % (side * side), *([h // unit] * (side * side))))
                        add += 1
            recs.sort(key=lambda c: (c[1], c[0]))
            d = b''.join(recs)
            r = bytearray(r)
            struct.pack_into('<H', r, 2, len(recs))
            print('added', add, 'cells')
        out_rows.append((l, r, d))
    # write as splice_relief does
    blob = bytearray(p['head'])
    blob += b'\0' * (ROW * len(out_rows))
    for i, (l, r, d) in enumerate(out_rows):
        r = bytearray(r)
        struct.pack_into('<I', r, 8, len(blob))
        blob[8 + ROW * i: 8 + ROW * (i + 1)] = r
        blob += d
    at = len(blob)
    blob += struct.pack('<HH', len(p['variants']), len(p['cells']))
    for v in p['variants']: blob += v
    for c in p['cells']: blob += c
    if p['edges']:
        head = bytearray(p['edge_head'])
        struct.pack_into('<H', head, 4, len(p['edges']))
        blob += head
        for e in p['edges']: blob += e
    blob += struct.pack('<I', at) + b'CUTS'
    open(path, 'wb').write(blob)

if __name__ == '__main__':
    main()
