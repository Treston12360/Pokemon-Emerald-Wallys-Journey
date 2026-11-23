#!/usr/bin/env python3
"""Move a column of relief cells' heights one row north, in place in relief.bin.

Brennan 2026-11-22 13:38 (C9, Meteor Falls 1F 1R, lower-left red box: "two thin upright vertical wall panels are
sticking out sideways from the cliff - remove/align them flush with the rock wall"). The drips on the cliff by the
south-west stairs (Meteor Falls 0x24C/254/25C, 0x264-0x277) had their wall face one row late: the drip's own first
row stood flat at the top and its pool row (the last) was the bottom of the face, so each drip stood a row out from the
wall beside it and its sides were the dark panels. Every other drip in the cave has its face start on its first row.
Here each listed run of cells takes the heights of the cell one row south of it (the last takes the floor's).

usage: raise_relief_run.py RELIEF.bin LAYOUT_NAME X:Y0-Y1 [X:Y0-Y1 ...]
Needs 3ds_port/scripts (voxel_relief_fixes).
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(HERE)), '3ds_port', 'scripts'))
import voxel_relief_fixes as vrf  # noqa: E402


def main():
    path, name, runs = sys.argv[1], sys.argv[2], sys.argv[3:]
    relief = vrf.parse(open(path, 'rb').read())
    t = relief['layouts'][vrf.layout_index(name)]
    cells = {c: list(g) for c, g in t['cells'].items()}
    for run in runs:
        x, ys = run.split(':')
        x = int(x)
        y0, y1 = (int(v) for v in ys.split('-'))
        for y in range(y0, y1 + 1):
            below = cells.get((x, y + 1))
            assert below is not None, 'no relief at %d,%d' % (x, y + 1)
            cells[(x, y)] = list(below)
        print('%s: column %d rows %d-%d raised a row' % (name, x, y0, y1))
    t['cells'] = cells
    open(path, 'wb').write(vrf.pack(relief))


if __name__ == '__main__':
    main()
