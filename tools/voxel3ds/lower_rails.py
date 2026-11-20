#!/usr/bin/env python3
"""Lay Route 119's Acro Bike rails just over the river, in place.

usage: lower_rails.py RELIEF.bin [LAYOUT_ID=35] [OVER_PX=4]

Brennan 2026-11-19 (POL-53): the river stands on Route 119 "look like thick raised blocks sitting high out of the
water with exposed boxy side walls. Lower their height profile closer to the water surface"; 2026-11-20 16:11: they
are "the long thin grey/white rails over the river". In relief.bin they were cells at the bank's height (16 px over
the water) with walled cut records. Each rail cell (the General's 0x0EC-0x0FD) now stands OVER_PX over the water
nearest it (the lowest water cell - 0x206, 0x207, 0x170 - within 3 cells), the cells drawn under a rail (its side
and shadow: 0x0F5, 0x0FA, 0x0FC) lie at the water's level, and their cut records' feet go down to the water, so
their walls are only the rail's thin edge.
"""
import json, os, struct, sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
RAIL = set(range(0x0EC, 0x0FE))
UNDER = {0x0F5, 0x0FA, 0x0FC}
WATER = {0x206, 0x207, 0x170}


def main():
    path = sys.argv[1]
    lid = int(sys.argv[2]) if len(sys.argv) > 2 else 35
    over = int(sys.argv[3]) if len(sys.argv) > 3 else 4
    layouts = json.load(open(os.path.join(ROOT, 'data/layouts/layouts.json')))['layouts']
    lay = layouts[lid - 1]
    W, H = lay['width'], lay['height']
    blk = open(os.path.join(ROOT, lay['blockdata_filepath']), 'rb').read()
    mt = lambda x, y: struct.unpack_from('<H', blk, 2 * (y * W + x))[0] & 0x3FF
    b = bytearray(open(path, 'rb').read())
    n = struct.unpack_from('<H', b, 4)[0]
    for j in range(n):
        l, cells, w, h, off = struct.unpack_from('<HHHHI', b, 8 + 14 * j)
        if l == lid:
            break
    assert not (w & 0x4000)
    unit = 2 if h & 0x4000 else 1
    at = {(b[off + k * 27], b[off + k * 27 + 1]): off + k * 27 + 2 for k in range(cells)}
    get = lambda x, y: [v * unit for v in struct.unpack_from('25b', b, at[(x, y)])]
    level = {}
    for (x, y) in at:
        if mt(x, y) not in RAIL:
            continue
        near = [min(get(i, j)) for i in range(x - 3, x + 4) for j in range(y - 3, y + 4)
                if 0 <= i < W and 0 <= j < H and (i, j) in at and mt(i, j) in WATER]
        if not near:
            print('no water near', x, y)
            continue
        wl = min(near)
        level[(x, y)] = wl
        px = wl if mt(x, y) in UNDER else wl + over
        struct.pack_into('25b', b, at[(x, y)], *([px // unit] * 25))
    cut_at = struct.unpack_from('<I', b, len(b) - 8)[0]
    nv, nc = struct.unpack_from('<HH', b, cut_at)
    o = cut_at + 4 + 36 * nv
    feet = 0
    for k in range(nc):
        r = o + 14 * k
        l, x, y = struct.unpack_from('<HBB', b, r)
        if l == lid and (x, y) in level:
            struct.pack_into('<h', b, r + 6, level[(x, y)])
            feet += 1
    open(path, 'wb').write(b)
    print('rails laid:', len(level), 'cells;', feet, 'cut feet at the water')


if __name__ == '__main__':
    main()
