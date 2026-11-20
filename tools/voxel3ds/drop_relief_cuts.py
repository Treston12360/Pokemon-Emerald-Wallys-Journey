#!/usr/bin/env python3
"""Remove cut records (cliff walls / rock layer of a cell) from relief.bin, in place.

usage: drop_relief_cuts.py RELIEF.bin LAYOUT_ID X,Y [X,Y ...]

Mt Pyre outside (Brennan 2026-11-19 22:30, "fix the dark line" at the small round rock's right side): cells x30 rows
25-26 and x31 rows 24-25 kept the generator's cut records (cliff face 0x0A9 on every side) after the corner was
re-laid as one surface (blend_relief_region.py) - their walls stood up as a thin dark slit. Used: 302 30,25 30,26
31,24 31,25.
"""
import struct, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from splice_relief import parse, ROW

def main():
    path, lid = sys.argv[1], int(sys.argv[2])
    drop = {tuple(map(int, a.split(','))) for a in sys.argv[3:]}
    p = parse(path)
    before = len(p['cells'])
    cells = [c for c in p['cells'] if not (struct.unpack_from('<H', c, 0)[0] == lid and (c[2], c[3]) in drop)]
    blob = bytearray(p['head'])
    blob += b'\0' * (ROW * len(p['rows']))
    for i, (l, r, d) in enumerate(p['rows']):
        r = bytearray(r)
        struct.pack_into('<I', r, 8, len(blob))
        blob[8 + ROW * i: 8 + ROW * (i + 1)] = r
        blob += d
    at = len(blob)
    blob += struct.pack('<HH', len(p['variants']), len(cells))
    for v in p['variants']: blob += v
    for c in cells: blob += c
    if p['edges']:
        head = bytearray(p['edge_head'])
        struct.pack_into('<H', head, 4, len(p['edges']))
        blob += head
        for e in p['edges']: blob += e
    blob += struct.pack('<I', at) + b'CUTS'
    open(path, 'wb').write(blob)
    print('dropped', before - len(cells), 'cut records')

if __name__ == '__main__':
    main()
