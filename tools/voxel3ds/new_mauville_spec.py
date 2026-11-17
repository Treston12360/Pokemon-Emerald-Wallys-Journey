"""POL-31: New Mauville's 3D models (Wally's Journey). This is a copy of the code added to
3ds_port/scripts/voxel_building_specs.py (Emerald 3DS's generator, not in git - see README.md / docs/POL-37 section 6),
just before "# Fortree's gym", plus its entry in the interiors list:

    {
        "name": "new_mauville_inside",
        "interior": {"layout": "LAYOUT_NEW_MAUVILLE_INSIDE", "ground": [0x210],
                     "pieces": own_shell(new_mauville_inside())},
    },

and gen_voxel_buildings.py: MAX_TEXTURE = (512, 1024) (was the 3DS's 512x512; New Mauville's page is 512x536; our
voxel_building.c takes pages up to 1024x1024). Spliced into pc_data/voxel/buildings.bin with
tools/voxel3ds/splice_buildings.py --layout OURS DONOR OUT 185.
"""
# Wally's Journey (POL-31, Brennan 2026-11-15: New Mauville "the models inside are still 2D whenever in 3D";
# 2026-11-17, asked what: the machines and computers, the electric barriers, the floor switches, and the walls -
# "Too short ... They just look flat like 2D and not 3D"). Emerald 3DS never modelled New Mauville (VOXEL_STATUS:
# not started). Its walls are Mossdeep's gym's kind (mossdeep_gym): every blocked cell of the walls - their cream
# tops, their olive faces, the long strips running back - a run down its column, a piece to each rectangle
# (run_rects), 24 px: the face drawn from the top cell's 9th row down the face cell (21f over 227). The props
# standing in the rooms and against the walls are left out of the walls and stand as their own boxes, one to
# each group of their cells; the barriers and switches are not in this first pass.

NEW_MAUVILLE_FLOOR = ("cdcdcd", "acbdac", "d5ded5", "83948b", "000000")
NEW_MAUVILLE_VOID = {0x208}
# the barriers' cells across the corridors (their tops in the walls go with the walls for now)
NEW_MAUVILLE_BARRIER = {0x2c6, 0x2c7, 0x2ce, 0x2cf}
# props, by kind: (metatiles, face height px)
NEW_MAUVILLE_PROPS = [
    ({0x2a1, 0x2a2, 0x2a9, 0x2aa}, 20),                      # parts shelves
    ({0x2d6, 0x2d7, 0x2de, 0x2df}, 16),                      # computers
    ({0x2b0, 0x2b1, 0x2b8, 0x2b9}, 14),                      # cabinets
    ({0x2c0, 0x2c1, 0x2c2, 0x2c3, 0x2b2, 0x2b3, 0x2ba, 0x2bb, 0x2a3, 0x2a4}, 10),  # crates
    ({0x2d4, 0x2d5, 0x2dc, 0x2dd, 0x2e4, 0x2e5}, 24),        # tank machines
    ({0x2d0, 0x2d1, 0x2d2, 0x2d3, 0x2d8, 0x2d9, 0x2da, 0x2db, 0x2e0, 0x2e1, 0x2e2, 0x2e3,
      0x2e8, 0x2e9, 0x2ea, 0x2eb}, 24),                      # the generator
    ({0x2ab, 0x2ac}, 8),                                     # bench
]
NEW_MAUVILLE_SIDE = (64, 24, 80, 32)  # a stretch of a wall's face (cell (4, 1), 227)


def new_mauville_inside():
    lay = LayoutArt("LAYOUT_NEW_MAUVILLE_INSIDE")
    blocked = lambda x, y: (lay.blocks[y * lay.w + x] >> 10) & 3
    kind = {}
    for k, (ms, h) in enumerate(NEW_MAUVILLE_PROPS):
        for m in ms:
            kind[m] = k
    walls = [(x, y) for y in range(lay.h) for x in range(lay.w)
             if blocked(x, y) and lay.metatile(x, y) not in NEW_MAUVILLE_VOID
             and lay.metatile(x, y) not in NEW_MAUVILLE_BARRIER and lay.metatile(x, y) not in kind]
    pieces = []
    # props first (front first): groups of neighbouring prop cells
    seen = set()
    for y in range(lay.h):
        for x in range(lay.w):
            m = lay.metatile(x, y)
            if m not in kind or (x, y) in seen:
                continue
            group, todo = [], [(x, y)]
            seen.add((x, y))
            while todo:
                cx, cy = todo.pop()
                group.append((cx, cy))
                for nx, ny in ((cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1)):
                    if 0 <= nx < lay.w and 0 <= ny < lay.h and (nx, ny) not in seen \
                       and lay.metatile(nx, ny) in kind:
                        seen.add((nx, ny))
                        todo.append((nx, ny))
            x0 = min(c[0] for c in group); x1 = max(c[0] for c in group) + 1
            y0 = min(c[1] for c in group); y1 = max(c[1] for c in group) + 1
            h = max(NEW_MAUVILLE_PROPS[kind[lay.metatile(*c)]][1] for c in group)
            pieces.append(piece("prop_%d_%d" % (x0, y0), [(x0 * 16, y0 * 16, x1 * 16, y1 * 16)],
                                min(h, (y1 - y0) * 16 - 4), leave=NEW_MAUVILLE_FLOOR))
    pieces += rect_pieces("block", runs_of(walls), 24, side=lambda *r: NEW_MAUVILLE_SIDE, solid=True)
    return pieces

