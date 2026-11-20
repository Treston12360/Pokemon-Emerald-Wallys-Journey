#!/usr/bin/env python3
"""Keep only the relief cells of one layout inside a rectangle (drop the rest of its row's cells), in place.

usage: keep_relief_cells.py RELIEF.bin LAYOUT_ID X0 X1 Y0 Y1

Session 014PwRDj, 2026-11-20 (Route 122, Brennan 12:30 "work on route 122 and slateport"): the generator's record for
Route 122 also wrote level cells for the ring of sea rocks round Mt Pyre's island; with them the rocks' models no
longer showed. Kept: the island's cells only.
"""
import struct, sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from splice_relief import parse, ROW


def main():
    path, lid, x0, x1, y0, y1 = sys.argv[1], *map(int, sys.argv[2:7])
    p = parse(path)
    rows = []
    for l, r, d in p['rows']:
        if l == lid:
            r = bytearray(r)
            _, cells, w, h, _ = struct.unpack_from('<HHHHI', r, 0)
            stride = len(d) // cells
            keep = [d[k * stride:(k + 1) * stride] for k in range(cells)
                    if x0 <= d[k * stride] <= x1 and y0 <= d[k * stride + 1] <= y1]
            struct.pack_into('<H', r, 2, len(keep))
            print('kept', len(keep), 'of', cells)
            d = b''.join(keep)
        rows.append((l, r, d))
    blob = bytearray(p['head'])
    blob += b'\0' * (ROW * len(rows))
    for i, (l, r, d) in enumerate(rows):
        r = bytearray(r)
        struct.pack_into('<I', r, 8, len(blob))
        blob[8 + ROW * i: 8 + ROW * (i + 1)] = r
        blob += d
    at = len(blob)
    blob += struct.pack('<HH', len(p['variants']), len(p['cells']))
    for v in p['variants']:
        blob += v
    for c in p['cells']:
        blob += c
    if p['edges']:
        head = bytearray(p['edge_head'])
        struct.pack_into('<H', head, 4, len(p['edges']))
        blob += head
        for e in p['edges']:
            blob += e
    blob += struct.pack('<I', at) + b'CUTS'
    open(path, 'wb').write(blob)


if __name__ == '__main__':
    main()
