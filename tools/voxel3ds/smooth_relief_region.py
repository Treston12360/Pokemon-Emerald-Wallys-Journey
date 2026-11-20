#!/usr/bin/env python3
"""Re-lay a block of relief cells as one smooth surface between the ground round it, in place.

usage: smooth_relief_region.py RELIEF.bin LAYOUT_ID X0 X1 Y0 Y1 [--keep-walls]

The block's cells x0..x1, y0..y1 (inclusive) get heights that run smoothly (a harmonic fill, solved on the relief's
lattice - five samples a side, shared edges) between the cells round the block: their edges are kept fixed and the
block meets them exactly. A side of the block on the map's own edge has no cells beyond it: that side is held at the
straight line between its two corners' outside values. Then each cut record in the block loses its walls (the
dark cliff sides a jagged cell stood up with: wall 0xFFFF, no sides, no flags) and its foot is set 16 px under the
cell's lowest point, as the generator writes them on an even slope (Ever Grande's rows 14-32). --keep-walls: leave
the cut records as they are.

Session 014PwRDj, 2026-11-20 (Brennan 12:27, Q6 Ever Grande: "A, Rebuild the slopes as smooth steps down to the sea,
area by area, like the Mt Pyre corner"): the generator gave the east rock slope (rows 33-53) a jagged mix of heights,
single cells standing 50-90 px above the rock round them, each with its own dark cliff walls.
blend_relief_region.py does the same along rows only (one row at a time between its two side cells).
"""
import struct, sys

S = 5  # lattice samples a side


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    keep_walls = '--keep-walls' in sys.argv
    path, lid, x0, x1, y0, y1 = args[0], *map(int, args[1:6])
    b = bytearray(open(path, 'rb').read())
    n = struct.unpack_from('<H', b, 4)[0]
    for j in range(n):
        l, cells, w, h, off = struct.unpack_from('<HHHHI', b, 8 + 14 * j)
        if l == lid:
            break
    else:
        sys.exit('layout not in relief')
    assert not (w & 0x4000), 'a layout with depths (stairs): not handled'
    unit = 2 if h & 0x4000 else 1
    W, H = w & 0x3FFF, h & 0x3FFF
    at = {(b[off + k * 27], b[off + k * 27 + 1]): off + k * 27 + 2 for k in range(cells)}

    def sample(x, y, r, c):
        return struct.unpack_from('b', b, at[(x, y)] + r * S + c)[0] * unit

    # lattice point (X, Y) = (4x + c, 4y + r)
    fixed, val = {}, {}
    X0, X1, Y0, Y1 = 4 * x0, 4 * (x1 + 1), 4 * y0, 4 * (y1 + 1)
    for (X, Y) in [(X, Y) for X in range(X0, X1 + 1) for Y in range(Y0, Y1 + 1)]:
        if X0 < X < X1 and Y0 < Y < Y1:
            continue
        # a boundary point: the cell just outside the block that holds it
        if X == X0 and x0 > 0:
            fixed[(X, Y)] = sample(x0 - 1, min(Y // 4, y1) if Y < Y1 else y1, Y - 4 * (min(Y // 4, y1)), 4)
        elif X == X1 and x1 < W - 1:
            cy = min(Y // 4, y1)
            fixed[(X, Y)] = sample(x1 + 1, cy, Y - 4 * cy, 0)
        elif Y == Y0 and y0 > 0:
            cx = min(X // 4, x1)
            fixed[(X, Y)] = sample(cx, y0 - 1, 4, X - 4 * cx)
        elif Y == Y1 and y1 < H - 1:
            cx = min(X // 4, x1)
            fixed[(X, Y)] = sample(cx, y1 + 1, 0, X - 4 * cx)
    # a side on the map's edge: the straight line between its corners
    for side in ('W', 'E', 'N', 'S'):
        if side == 'E' and x1 == W - 1:
            pts = [(X1, Y) for Y in range(Y0, Y1 + 1)]
        elif side == 'W' and x0 == 0:
            pts = [(X0, Y) for Y in range(Y0, Y1 + 1)]
        elif side == 'N' and y0 == 0:
            pts = [(X, Y0) for X in range(X0, X1 + 1)]
        elif side == 'S' and y1 == H - 1:
            pts = [(X, Y1) for X in range(X0, X1 + 1)]
        else:
            continue
        a, z = fixed[pts[0]], fixed[pts[-1]]
        for i, p in enumerate(pts):
            fixed[p] = a + (z - a) * i / (len(pts) - 1)
    for X in range(X0 + 1, X1):
        for Y in range(Y0 + 1, Y1):
            val[(X, Y)] = sum(fixed.values()) / len(fixed)
    val.update(fixed)
    for _ in range(4000):
        for X in range(X0 + 1, X1):
            for Y in range(Y0 + 1, Y1):
                val[(X, Y)] = (val[(X - 1, Y)] + val[(X + 1, Y)] + val[(X, Y - 1)] + val[(X, Y + 1)]) / 4.0
    lows = {}
    for x in range(x0, x1 + 1):
        for y in range(y0, y1 + 1):
            if (x, y) not in at:
                print('no cell', x, y)
                continue
            lo = 1e9
            for r in range(S):
                for c in range(S):
                    q = int(round(val[(4 * x + c, 4 * y + r)] / unit))
                    struct.pack_into('b', b, at[(x, y)] + r * S + c, max(-128, min(127, q)))
                    lo = min(lo, q * unit)
            lows[(x, y)] = lo
    # cut records: 14 bytes each after the variants (trailer: u32 offset of the counts, "CUTS")
    cut_at = struct.unpack_from('<I', b, len(b) - 8)[0]
    nv, nc = struct.unpack_from('<HH', b, cut_at)
    o = cut_at + 4 + 36 * nv   # cut variants are 36 bytes (splice_relief.py CUT_VARIANT)
    changed = 0
    for k in range(nc):
        r = o + 14 * k
        l, x, y = struct.unpack_from('<HBB', b, r)
        if l == lid and (x, y) in lows and not keep_walls:
            struct.pack_into('<h', b, r + 6, lows[(x, y)] - 16)
            struct.pack_into('<H', b, r + 10, 0xFFFF)
            b[r + 12] = 0
            b[r + 13] = 0
            changed += 1
    open(path, 'wb').write(b)
    print('smoothed', len(lows), 'cells;', changed, 'cut records without walls')


if __name__ == '__main__':
    main()
