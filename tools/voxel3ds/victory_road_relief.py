#!/usr/bin/env python3
"""Victory Road B2F: the rock south of the bridge's east end ramps up from the bridge (relief.bin, in place).

Brennan 2026-11-24 21:12 ("the bridge is meshed into the rock"): the cells under the bridge's east half (row 20,
columns 36-39), the top of the rock wall that runs down to the floor, stood 54-107 px high right along the bridge's
south edge (48) - a ridge rising in front of it, and on (39, 20) a spike over the 96 px ledge with a sheer drop to
the next cell. Each column of them now ramps from the bridge's height at the cell's north edge to its own south edge
(the row below's north edge, as before): the bridge's edge clear all along, the drop gone.
usage: victory_road_relief.py RELIEF.bin
"""
import os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(HERE)), "3ds_port", "scripts"))
import voxel_relief_fixes as vrf  # noqa: E402

LAYOUT = 286                      # LAYOUT_VICTORY_ROAD_B2F
ROW, COLUMNS, BRIDGE = 20, range(36, 40), 48


def main():
    relief = vrf.parse(open(sys.argv[1], "rb").read())
    cells = relief["layouts"][LAYOUT]["cells"]
    for x in COLUMNS:
        g = cells[(x, ROW)]
        south = g[20:25]
        cells[(x, ROW)] = [round(BRIDGE + (south[i] - BRIDGE) * j / 4) for j in range(5) for i in range(5)]
    open(sys.argv[1], "wb").write(vrf.pack(relief))
    print("Victory Road B2F: %d cells ramp from the bridge" % len(COLUMNS))


if __name__ == "__main__":
    main()
