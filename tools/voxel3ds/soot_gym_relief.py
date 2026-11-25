#!/usr/bin/env python3
"""Sootopolis Gym 1F: the wall blocks either side of the lower ice room's front stand flush (relief.bin, in place).

Brennan 2026-11-24 18:23 ("line here remaining, needs fixed"): row 15's wall cells 3-4 and 12-13 (the blocks
between the fountains and the room's front wall) lay at the lower tier (16) under the walls model, the ice room's
row 14 at 32 - the riser between them, ice-coloured, showed as a thin line along the wall's top from the fountain
to the ice. They lie at 32, the wall's top: no riser.
usage: soot_gym_relief.py RELIEF.bin
"""
import os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(HERE)), "3ds_port", "scripts"))
import voxel_relief_fixes as vrf  # noqa: E402

LAYOUT = 109                      # LAYOUT_SOOTOPOLIS_CITY_GYM_1F
CELLS = [(3, 15), (4, 15), (12, 15), (13, 15)]


def main():
    relief = vrf.parse(open(sys.argv[1], "rb").read())
    cells = relief["layouts"][LAYOUT]["cells"]
    for c in CELLS:
        cells[c] = [32] * 25
    open(sys.argv[1], "wb").write(vrf.pack(relief))
    print("Sootopolis Gym 1F: %d cells flush" % len(CELLS))


if __name__ == "__main__":
    main()
