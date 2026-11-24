#!/usr/bin/env python3
"""Audit #20 (Brennan 2026-11-23 21:03, Route 131 / Pacifidlog seam: "fix this issue, use the sea around it"): after
Sky Pillar opens, Route 131 is drawn with LAYOUT_ROUTE131_SKY_PILLAR (relief.bin layout 319). Its metatiles are
Route 131's (layout 47) everywhere but the Sky Pillar (x 23-51, rows 0-22), but its relief was generated apart: the
rock shoal by Pacifidlog (rows 32-39) lay flat at sea level, while Route 131's and Pacifidlog's own rise 1-2 levels
in terraces - a cliff straight down the map seam, with sky through it. This copies layout SRC's relief cells and cut
records to DST wherever the two layouts' metatiles are the same, so the place looks the same before and after.

usage: relief_copy_same_cells.py RELIEF.bin SRC DST [X0 Y0 X1 Y1]   (layout indexes as in relief.bin: list position + 1;
       the box, inclusive, limits the copy to those cells)
"""
import json, os, struct, sys
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, os.path.join(ROOT, '3ds_port', 'scripts'))
import voxel_relief_fixes as vrf


def main():
    path, src, dst = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
    box = [int(v) for v in sys.argv[4:8]] if len(sys.argv) >= 8 else None
    L = json.load(open(os.path.join(ROOT, 'data/layouts/layouts.json')))['layouts']
    ls, ld = L[src - 1], L[dst - 1]
    assert (ls['width'], ls['height']) == (ld['width'], ld['height'])
    assert (ls['primary_tileset'], ls['secondary_tileset']) == (ld['primary_tileset'], ld['secondary_tileset'])
    a = open(os.path.join(ROOT, ls['blockdata_filepath']), 'rb').read()
    b = open(os.path.join(ROOT, ld['blockdata_filepath']), 'rb').read()
    w, h = ls['width'], ls['height']
    r = vrf.parse(open(path, 'rb').read())
    A, B = r['layouts'][src]['cells'], r['layouts'][dst]['cells']
    changed = 0
    for y in range(h):
        for x in range(w):
            i = 2 * (y * w + x)
            if a[i:i + 2] != b[i:i + 2]:
                continue
            if box and not (box[0] <= x <= box[2] and box[1] <= y <= box[3]):
                continue
            before = (B.get((x, y)), r['cuts'].get((dst, x, y)))
            if (x, y) in A:
                B[(x, y)] = list(A[(x, y)])
            else:
                B.pop((x, y), None)
            if (src, x, y) in r['cuts']:
                r['cuts'][(dst, x, y)] = list(r['cuts'][(src, x, y)])
            else:
                r['cuts'].pop((dst, x, y), None)
            changed += before != (B.get((x, y)), r['cuts'].get((dst, x, y)))
    open(path, 'wb').write(vrf.pack(r))
    print('cells changed', changed)


if __name__ == '__main__':
    main()
