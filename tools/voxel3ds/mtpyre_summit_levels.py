#!/usr/bin/env python3
"""Mt Pyre summit (layout 303): give the lower summit its two terraces, in place (relief.bin VXL4).

usage: mtpyre_summit_levels.py RELIEF.bin

POL-51 (Brennan 2026-11-19, summit's "bottom stairs" and upper flight still flat; 15:37: "manually or
programmatically reconstruct the elevation heightmap"). Emerald 3DS's generator (even with VOXEL_SPREAD_OK) lays
everything south of row 14 at one level, so the flights drawn at rows 17 and 31 join equal ground and stay flat. The
map's own elevation bits are all 3 (no help). Read from the drawing instead:
  - rows <= 14: as they are;
  - the landing between the flights at rows 14 and 17 (rows 15-16, x18-28, closed by rock on both sides): as is (0);
  - the rest of rows 15-30: one level down (-16 px); every cell of them, missing cells added flat;
  - row 17 x19-27 (the face with the flight at x22-24) and row 31 (the face along the whole width, a ramp at x6-10):
    each lattice row stepped from the level above to the level below, so the stair finder lays its steps;
  - rows 32-36: two levels down (-32 px).
"""
import struct, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from splice_relief import parse, ROW

LID, W, H = 303, 50, 37

def main():
    path = sys.argv[1]
    p = parse(path)
    side = p['side']
    rows = []
    for l, r, d in p['rows']:
        if l == LID:
            cells, w, hh = struct.unpack_from('<HHH', r, 2)
            assert not (w & 0x4000)
            unit = 2 if hh & 0x4000 else 1
            st = 2 + side * side
            g = {}
            for k in range(cells):
                c = d[k * st:(k + 1) * st]
                g[(c[0], c[1])] = [unit * q for q in struct.unpack_from('<25b', c, 2)]
            for y in range(15, H):
                for x in range(W):
                    g.setdefault((x, y), [0] * 25)
            def landing(x, y):
                return 15 <= y <= 16 and 18 <= x <= 28
            for (x, y), v in g.items():
                if y < 15 or landing(x, y):
                    continue
                if y == 17 and 19 <= x <= 27:
                    for j in range(5):
                        for i in range(5):
                            v[j * 5 + i] += -16 * j // 4
                elif y == 31:
                    for j in range(5):
                        for i in range(5):
                            v[j * 5 + i] += -16 - 16 * j // 4
                elif y < 31:
                    for q in range(25):
                        v[q] -= 16
                else:
                    for q in range(25):
                        v[q] -= 32
            recs = sorted(g.items(), key=lambda kv: (kv[0][1], kv[0][0]))
            d = b''.join(bytes([x, y]) + struct.pack('<25b', *[max(-128, min(127, q // unit)) for q in v])
                         for (x, y), v in recs)
            r = bytearray(r)
            struct.pack_into('<H', r, 2, len(recs))
            print('summit cells', cells, '->', len(recs))
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
