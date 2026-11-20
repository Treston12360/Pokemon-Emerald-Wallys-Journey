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
  - (Brennan 23:28: the east rock formations got sky tears and dark cracks - lowered against the unchanged ground
    north of row 15 there. East of x32 the lowering fades out over x32..36 (lattice by lattice) and is none from x36:
    the east terraces stay as the generator made them.)
  - (Brennan 23:41: dark lines round the foot of every rock in the lowered part - the cut records' feet (the rock's
    foot height, px) were left at the old level. Each cut cell's foot moves with its cell: the same offset as the
    lattice's centre point.)
"""
import struct, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from splice_relief import parse, ROW

LID, W, H = 303, 50, 37
OFFSET = {}
WATER_CELLS = set()
ONE_FACE = set()

def load_water():
    import json
    root = os.path.join(os.path.dirname(__file__), '..', '..')
    L = {e['id']: e for e in json.load(open(os.path.join(root, 'data/layouts/layouts.json')))['layouts'] if 'id' in e}
    l = L['LAYOUT_MT_PYRE_SUMMIT']
    v = struct.unpack('<%dH' % (W * H), open(os.path.join(root, l['blockdata_filepath']), 'rb').read())
    a1 = open(os.path.join(root, 'data/tilesets/primary/general/metatile_attributes.bin'), 'rb').read()
    a2 = open(os.path.join(root, 'data/tilesets/secondary/facility/metatile_attributes.bin'), 'rb').read()
    for y in range(H):
        for x in range(W):
            m = v[y * W + x] & 0x3FF
            a, i = (a1, m) if m < 512 else (a2, m - 512)
            if 0x10 <= (struct.unpack_from('<H', a, 2 * i)[0] & 0xFF) <= 0x1A:
                WATER_CELLS.add((x, y))

def main():
    path = sys.argv[1]
    load_water()
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
                return 15 <= y <= 16 and 20 <= x <= 26   # (the drawing: rock ridges at x<=19 and x>=27; Brennan 00:43, the chunk right of the stairs was x27-28 left at 0)
            def fade(x, i):
                wx = x + i / 4.0
                return 1.0 if wx <= 32 else 0.0 if wx >= 36 else (36 - wx) / 4.0
            for (x, y), v in g.items():
                # (Brennan 23:56: a second rock ledge part-way down the cliff line north of row 15 - the row-14 face
                # ended at the old ground and a further 16 px step stood at the row 14/15 seam. Row 14's face now
                # runs on down to the lowered ground: its lattice rows stepped down by up to 16 px, south edge
                # -16 - except above the landing and the flight at x18-28, and faded out east as the rest)
                if y == 14 and not (18 <= x <= 28):
                    orig = list(v)
                    for j in range(5):
                        for i in range(5):
                            v[j * 5 + i] = int(round(orig[j * 5 + i] - 16 * j / 4.0 * fade(x, i)))
                    continue
                if y < 15 or landing(x, y):
                    continue
                orig = list(v)
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
                for j in range(5):
                    for i in range(5):
                        q = j * 5 + i
                        v[q] = int(round(orig[q] + (v[q] - orig[q]) * fade(x, i)))
                OFFSET[(x, y)] = v[12] - orig[12] if y != 17 and y != 31 else v[22] - orig[22]
            # (Brennan 23:56: west and east of the landing the cliff line north of row 18 is two rock bands with a
            # shelf between - row 14's face, then the ridge of rows 15-16 and its face at row 17. One wall instead:
            # rows 14-17 of those columns laid as one straight face from row 14's top to row 17's foot, lattice row
            # by lattice row. Cut records of those cells dropped below (no rock layer / walls of the old shelf).)
            ONE_FACE.clear()
            for (x0f, x1f) in ((13, 20), (27, 33)):
              for x in range(x0f, x1f):
                if not all((x, y) in g for y in (14, 15, 16, 17)):
                    continue
                for i in range(5):
                    # (faded to the cells beside the range over its outer column, so its edges meet them exactly:
                    # a seam of sky stood at the range's end)
                    wx = x + i / 4.0
                    wgt = max(0.0, min(1.0, wx - x0f, x1f - wx))
                    A, B = g[(x, 14)][i], g[(x, 17)][20 + i]
                    for y in (14, 15, 16, 17):
                        for j in range(5):
                            t = ((y - 14) + j / 4.0) / 4.0
                            old = g[(x, y)][j * 5 + i]
                            g[(x, y)][j * 5 + i] = int(round(old + (A + (B - A) * t - old) * wgt))
                for y in (14, 15, 16, 17):
                    ONE_FACE.add((x, y))
            # (Brennan 00:43 / POL-60: a flat teal square of sea at the summit's north-east - the map's own water
            # tiles along its north, drawn at the map's base, 64 px over the sea round it. Laid at the sea's -64.)
            for (x, y) in WATER_CELLS:
                g[(x, y)] = [-64] * 25
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
    cells_out = []
    for c in p['cells']:
        lid, x, y = struct.unpack_from('<HBB', c, 0)
        if lid == LID and (x, y) in ONE_FACE:
            continue
        if lid == LID and (x, y) in OFFSET:
            c = bytearray(c)
            foot = struct.unpack_from('<h', c, 6)[0]
            struct.pack_into('<h', c, 6, foot + OFFSET[(x, y)])
            c = bytes(c)
        cells_out.append(c)
    blob += struct.pack('<HH', len(p['variants']), len(cells_out))
    for v in p['variants']: blob += v
    for c in cells_out: blob += c
    if p['edges']:
        head = bytearray(p['edge_head'])
        struct.pack_into('<H', head, 4, len(p['edges']))
        blob += head
        for e in p['edges']: blob += e
    blob += struct.pack('<I', at) + b'CUTS'
    open(path, 'wb').write(blob)

if __name__ == '__main__':
    main()
