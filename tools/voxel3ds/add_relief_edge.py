#!/usr/bin/env python3
"""Add (or replace) an edge fill in relief.bin, in place.

usage: add_relief_edge.py RELIEF.bin LAYOUT_ID EDGE METATILE [FIRST LAST] [--top PX]
  LAYOUT_ID: the layout's number in data/layouts/layouts.json, counted from 1 (Rustboro = 4).
  EDGE: W, E, N or S.  METATILE: hex or decimal; one id, or four comma-separated (the 2x2 block).
  FIRST LAST: the cells along the edge it covers (default: all of it).
  --flat: its top stays level out to the horizon (not rounded off); --sparse: a wood of it thinned.
  --top PX: where the fill's top stands, pixels over the map's base (default: water at the map's own sea level,
            anything else as the border is).

An edge fill is what the 3D view shows past a map's edge instead of the map's border block (voxel_relief.c
VoxelRelief_EdgeFill; Emerald 3DS's relief_fixes.json "edges"). Written by the generator; this adds one without
regenerating relief.bin (which needs the splices in README.md). Water metatiles (General 0x170) are flagged water.

Session 014PwRDj, 2026-11-19 (Brennan 14:23, POL-57: Route 121's south coast showed its border's trees standing in
the sea; and 13:30: "Sea all round" Mt Pyre).
"""
import struct, sys

WATER = {0x170, 0x0A1}   # 0x0A1: Mt Pyre's own sea (its border block)
EDGES = {'W': 1, 'E': 2, 'N': 4, 'S': 8}


def main():
    args = sys.argv[1:]
    top = None
    extra = 0
    if '--flat' in args:      # flags 8: its top level out to the horizon (Brennan 2026-11-22 21:54, Fallarbor's woods)
        extra |= 8
        args.remove('--flat')
    if '--sparse' in args:    # flags 4: a wood of it thinned, open patches between the trees (VoxelMesh_WoodGap)
        extra |= 4
        args.remove('--sparse')
    if '--top' in args:
        i = args.index('--top')
        top = int(args[i + 1])
        del args[i:i + 2]
    path, lid, edge, mts = args[0], int(args[1]), EDGES[args[2].upper()], args[3]
    mt = [int(m, 0) for m in mts.split(',')]
    if len(mt) == 1:
        mt *= 4
    first, last = (int(args[4]), int(args[5])) if len(args) >= 6 else (-1, -1)
    b = bytearray(open(path, 'rb').read())
    assert b[-4:] == b'CUTS'
    tail = len(b) - 8
    i = b.rfind(b'EDGE', 0, tail)
    assert i >= 0, 'no EDGE block'
    count = struct.unpack_from('<H', b, i + 4)[0]
    end = i + 8 + 18 * count
    assert end == tail, 'EDGE block is not last before the trailer'
    flags = (2 if top is not None else 0) | extra
    for k in range(4):
        if mt[k] in WATER:
            flags |= 1 << (4 + k)
    rec = struct.pack('<HBBhhh4H', lid, edge, flags, first, last, top or 0, *mt)
    for k in range(count):
        o = i + 8 + 18 * k
        l, e = struct.unpack_from('<HB', b, o)
        f, la = struct.unpack_from('<hh', b, o + 4)
        if l == lid and e == edge and f == first and la == last:
            b[o:o + 18] = rec
            break
    else:
        b[end:end] = rec
        struct.pack_into('<H', b, i + 4, count + 1)
    open(path, 'wb').write(b)
    print('ok', lid, args[2], [hex(m) for m in mt], first, last, top)


if __name__ == '__main__':
    main()
