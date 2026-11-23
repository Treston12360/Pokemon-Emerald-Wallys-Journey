#!/usr/bin/env python3
"""Weld small height mismatches between neighbouring relief cells, in place in relief.bin.

Brennan 2026-11-22 13:38 (C9, Meteor Falls: "multiple cliff ledges have sharp triangular spikes, broken jagged
corners ... Smooth and weld those cliff ledges cleanly"). The drawn-relief solver gives each cell along a diagonal
ledge its own east-west ramp (96 -> 80 px across the cell); the next row's ramp starts a cell further west, so where
two rows meet their edges differ by 2..8 px point by point - the renderer stands a face in every such seam, a thin
triangle (8, 6, 4, 2, 0 px) : the "spikes". Here every lattice point shared by two or four cells takes one height when
the cells disagree by at most --max px there (the highest, so nothing sinks under its neighbour); a real step (a
ledge's back, a cliff - more than that) is kept.

usage: weld_relief_seams.py RELIEF.bin LAYOUT_NAME [LAYOUT_NAME...] [--max 8] [--tiles 212,213,...]
  --tiles: only points a cell of one of these metatiles (hex) shares.
Needs 3ds_port/scripts (voxel_relief_fixes).
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(HERE)), '3ds_port', 'scripts'))
import voxel_relief_fixes as vrf  # noqa: E402

N = 5


def weld(t, limit, tiles=None, mt=None):
    cells = t['cells']
    W, H = t['w'], t['h']
    # world lattice point (X, Y) in 4ths of a cell: X = 4 * x + s
    points = {}
    for (x, y), g in cells.items():
        for r in range(N):
            for s in range(N):
                points.setdefault((4 * x + s, 4 * y + r), []).append(((x, y), r * N + s))
    # cells with no relief entry are level 0 there
    changed = 0
    for (X, Y), refs in points.items():
        owners = {c for c, _ in refs}
        # the cells sharing this point that have no entry (flat 0) count too
        xs = {X // 4} | ({X // 4 - 1} if X % 4 == 0 else set())
        ys = {Y // 4} | ({Y // 4 - 1} if Y % 4 == 0 else set())
        vals = [cells[c][k] for c, k in refs]
        for cx in xs:
            for cy in ys:
                if 0 <= cx < W and 0 <= cy < H and (cx, cy) not in owners:
                    vals.append(0)
        lo, hi = min(vals), max(vals)
        if lo == hi or hi - lo > limit:
            continue
        # only round a diagonal ledge's cells (--tiles): elsewhere (a cliff's foot by Meteor Falls' south-west drips)
        # a raised point stood a new thin fin (tried 2026-11-22)
        if tiles is not None and not any(mt.get(c) in tiles for c in owners):
            continue
        for c, k in refs:
            if cells[c][k] != hi:
                cells[c][k] = hi
                changed += 1
    return changed


def main():
    args = sys.argv[1:]
    limit = 8
    if '--max' in args:
        i = args.index('--max')
        limit = int(args[i + 1])
        del args[i:i + 2]
    tiles = None
    if '--tiles' in args:
        i = args.index('--tiles')
        tiles = {int(v, 16) for v in args[i + 1].split(',')}
        del args[i:i + 2]
    path, names = args[0], args[1:]
    relief = vrf.parse(open(path, 'rb').read())
    for name in names:
        t = relief['layouts'][vrf.layout_index(name)]
        t['cells'] = {c: list(g) for c, g in t['cells'].items()}
        mt = None
        if tiles is not None:
            import struct
            e = vrf.layout_entry(name)
            raw = open(os.path.join(os.path.dirname(os.path.dirname(HERE)), e['blockdata_filepath']), 'rb').read()
            ids = struct.unpack('<%dH' % (len(raw) // 2), raw)
            mt = {(i % e['width'], i // e['width']): v & 0x3FF for i, v in enumerate(ids)}
        n = weld(t, limit, tiles, mt)
        print('%s: %d lattice points raised' % (name, n))
    open(path, 'wb').write(vrf.pack(relief))


if __name__ == '__main__':
    main()
