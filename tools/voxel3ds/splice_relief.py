#!/usr/bin/env python3
"""Splice chosen layouts' relief from one relief.bin (VXL4) into another (POL-36).

usage: splice_relief.py OURS.bin DONOR.bin OUT.bin LAYOUT_ID [LAYOUT_ID ...]

Every record of the named layouts (its row + cells, its cut variants and cut cells, its edge
fills) is taken from DONOR; everything else stays as in OURS. Used for Route 119 (+120, 121,
Lilycove): their maps are the same as vanilla Emerald's, and relief.bin generated against our
tree (2026-11-14) lifted Route 119's ground into walls; the vanilla-generated one is right.
"""
import struct, sys

ROW = 14
CUT_VARIANT = 36
CUT_CELL = 14
EDGE = 18

def parse(path):
    b = open(path, 'rb').read()
    assert b[:4] == b'VXL4' and b[-4:] == b'CUTS'
    n, side = struct.unpack_from('<HH', b, 4)
    rows = []
    for i in range(n):
        r = bytearray(b[8 + ROW * i: 8 + ROW * (i + 1)])
        lid, cells, w, h, off = struct.unpack_from('<HHHHI', r, 0)
        stride = 2 + 2 * side * side if (w & 0x4000) else 2 + side * side
        rows.append((lid, r, b[off: off + cells * stride]))
    at = struct.unpack_from('<I', b, len(b) - 8)[0]
    nv, nc = struct.unpack_from('<HH', b, at)
    v0 = at + 4
    variants = [b[v0 + CUT_VARIANT * i: v0 + CUT_VARIANT * (i + 1)] for i in range(nv)]
    c0 = v0 + CUT_VARIANT * nv
    cells = [b[c0 + CUT_CELL * i: c0 + CUT_CELL * (i + 1)] for i in range(nc)]
    e = c0 + CUT_CELL * nc
    edges, edge_head = [], None
    if e + 8 <= len(b) - 8 and b[e:e + 4] == b'EDGE':
        ne = struct.unpack_from('<H', b, e + 4)[0]
        edge_head = b[e:e + 8]
        edges = [b[e + 8 + EDGE * i: e + 8 + EDGE * (i + 1)] for i in range(ne)]
        assert e + 8 + EDGE * ne == len(b) - 8, 'unexpected bytes after the edges'
    else:
        assert e == len(b) - 8, 'unexpected bytes after the cut cells'
    return dict(side=side, head=b[:8], rows=rows, variants=variants, cells=cells, edges=edges, edge_head=edge_head)

def lid_of(rec):
    return struct.unpack_from('<H', rec, 0)[0]

def main():
    ours, donor, out = parse(sys.argv[1]), parse(sys.argv[2]), sys.argv[3]
    take = set(int(x) for x in sys.argv[4:])
    assert ours['side'] == donor['side']
    drows = {lid: (r, d) for lid, r, d in donor['rows']}
    # rows + cells
    rows = []
    for lid, r, d in ours['rows']:
        if lid in take:
            r, d = drows[lid]
            r = bytearray(r)
        rows.append((lid, r, d))
    # cut variants: all of ours stay where they are (cells of other maps may use them), then the
    # donor's that the taken layouts' cut cells use, appended (indices remapped)
    variants = list(ours['variants'])
    remap_ours = {i: i for i in range(len(variants))}
    remap_donor = {}
    for c in donor['cells']:
        if lid_of(c) in take:
            vi = struct.unpack_from('<H', c, 4)[0]
            if vi != 0xFFFF and vi not in remap_donor:
                v = donor['variants'][vi]
                remap_donor[vi] = variants.index(v) if v in variants else len(variants)
                if remap_donor[vi] == len(variants):
                    variants.append(v)
    def fix(c, remap):
        c = bytearray(c); vi = struct.unpack_from('<H', c, 4)[0]
        if vi != 0xFFFF:
            struct.pack_into("<H", c, 4, remap[vi])
        return bytes(c)
    cells = [fix(c, remap_ours) for c in ours['cells'] if lid_of(c) not in take]
    cells += [fix(c, remap_donor) for c in donor['cells'] if lid_of(c) in take]
    cells.sort(key=lambda c: (lid_of(c), c[2], c[3]))  # the reader binary-searches (layout, x, y)
    edges = [e for e in ours['edges'] if lid_of(e) not in take] + [e for e in donor['edges'] if lid_of(e) in take]
    # write
    blob = bytearray(ours['head'])
    data_at = 8 + ROW * len(rows)
    blob += b'\0' * (ROW * len(rows))
    for i, (lid, r, d) in enumerate(rows):
        struct.pack_into('<I', r, 8, len(blob))
        blob[8 + ROW * i: 8 + ROW * (i + 1)] = r
        blob += d
    at = len(blob)
    blob += struct.pack('<HH', len(variants), len(cells))
    for v in variants: blob += v
    for c in cells: blob += c
    if edges:
        head = bytearray(ours['edge_head'] or donor['edge_head'])
        struct.pack_into('<H', head, 4, len(edges))
        blob += head
        for e in edges: blob += e
    blob += struct.pack('<I', at) + b'CUTS'
    open(out, 'wb').write(blob)
    print('rows', len(rows), 'variants', len(variants), 'cut cells', len(cells), 'edges', len(edges), 'bytes', len(blob))

if __name__ == '__main__':
    main()
