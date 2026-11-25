#!/usr/bin/env python3
"""Victory Road B2F: the rock south of the bridge's east end ramps up from the bridge (relief.bin, in place).

Brennan 2026-11-24 21:12 ("the bridge is meshed into the rock"): the cells under the bridge's east half (row 20,
columns 36-39), the top of the rock wall that runs down to the floor, stood 54-107 px high right along the bridge's
south edge (48) - a ridge rising in front of it, and on (39, 20) a spike over the 96 px ledge with a sheer drop to
the next cell. Each column of them now ramps from the bridge's height at the cell's north edge to its own south edge
(the row below's north edge, as before): the bridge's edge clear all along, the drop gone.

Victory Road 1F (2026-11-24, Brennan 22:09 "continue", the same stairs and ladders brought to 1F and B1F): the rock
face under its two north bridges (row 19, columns 17-38) began 15-29 px under the bridges' edge (48) - an open seam
along the bridges' south side, the sky showing through it (gap test). Those cells ramp from the bridges' height the
same way.
usage: victory_road_relief.py RELIEF.bin
"""
import os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(HERE)), "3ds_port", "scripts"))
import voxel_relief_fixes as vrf  # noqa: E402

# (layout, row, columns, the bridge's height)
RAMPS = [(286, 20, range(36, 40), 48),   # LAYOUT_VICTORY_ROAD_B2F
         (163, 19, range(17, 39), 48)]   # LAYOUT_VICTORY_ROAD_1F


def main():
    relief = vrf.parse(open(sys.argv[1], "rb").read())
    n = 0
    for layout, row, columns, bridge in RAMPS:
        cells = relief["layouts"][layout]["cells"]
        for x in columns:
            g = cells[(x, row)]
            if all(v == bridge for v in g[0:5]):
                continue   # (already level with the bridge at its north edge: as it is)
            south = g[20:25]
            cells[(x, row)] = [round(bridge + (south[i] - bridge) * j / 4) for j in range(5) for i in range(5)]
            n += 1
    open(sys.argv[1], "wb").write(vrf.pack(relief))
    print("Victory Road: %d cells ramp from their bridges" % n)


if __name__ == "__main__":
    main()
