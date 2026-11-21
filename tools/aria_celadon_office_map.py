#!/usr/bin/env python3
"""Project ARIA ch. 5 — Varnell's Celadon office (owner-approved building + 13x11 layout, 2026-11-21).

Builds the real map data (not just a picture):
  * data/layouts/CeladonCity_VarnellOffice/map.bin + border.bin — the 13x11 interior, stamped from Game Freak's
    own Celadon Condominiums 1F/2F/3F maps (full 16-bit metatile values, so collision/elevation stay official).
  * data/layouts/CeladonCity_Frlg/map.bin — gives the doorless house at (44..47, 17..21) a door (same metatiles as
    Celadon's other houses) and opens the fence under it so the player can walk straight in from the street
    (owner 2026-11-21: the sign moves one tile beside the path, to (46,23)).
Run from the repo root:  python3 tools/aria_celadon_office_map.py   (idempotent; safe to re-run)
  --preview OUT.png   also writes a picture of the interior with the people placed (needs docs/kanto_mockups/tools).

Interior layout (x right, y down; 13 wide, 11 tall):
  top wall: bookshelves 0-3, window 4, staff-only doorway 5-7 (blocked), the child's drawing 8-9, window 10, phone 11
  (8,2) / (9,2): clear floor right under the drawing — stand there, face up, press A (owner 2026-11-21)
  Varnell's desk 10-12 x 2-4; staff desk 0-2 x 3-5; reception desk 2-4 x 6-8; meeting table 8-11 x 5-8,
  stools row 9; plant (12,8); boxes (0,8); door mat 5-7 on row 10 (warps), clear walkway columns 5-7.
"""
import json, struct, sys, os
from collections import Counter

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LAYOUTS = {l['name']: l for l in json.load(open(os.path.join(ROOT, 'data/layouts/layouts.json')))['layouts'] if 'name' in l}
W, H = 13, 11

def load(name):
    l = LAYOUTS[name]
    b = open(os.path.join(ROOT, l['blockdata_filepath']), 'rb').read()
    return l['width'], [struct.unpack('<H', b[i*2:i*2+2])[0] for i in range(l['width'] * l['height'])]

F1, F2, F3 = 'CeladonCity_Condominiums_1F_Layout', 'CeladonCity_Condominiums_2F_Layout', 'CeladonCity_Condominiums_3F_Layout'
SRC = {n: load(n) for n in (F1, F2, F3)}
# full value (with collision + elevation) for a plain metatile id = the value Game Freak used most for it
_full = Counter()
for _, ids in SRC.values():
    for v in ids: _full[v] += 1
def full(mid):
    c = [(n, v) for v, n in _full.items() if v & 0x3FF == mid]
    if not c: raise SystemExit('metatile %03x not used in the source maps' % mid)
    return max(c)[1]

g = [[full(0x281)] * W for _ in range(H)]
def put(x, y, v): g[y][x] = v
def fill(x, y, w, h, mid):
    for j in range(h):
        for i in range(w): put(x+i, y+j, full(mid))
def stamp(n, sx, sy, w, h, dx, dy):
    SW, ids = SRC[n]
    for j in range(h):
        for i in range(w): put(dx+i, dy+j, ids[(sy+j)*SW + sx+i])

fill(0, 0, W, 1, 0x289); fill(0, 1, W, 1, 0x291); fill(0, 2, W, 1, 0x282); fill(0, 3, 1, H-4, 0x282)
stamp(F3, 0, 5, 2, 3, 0, 0); stamp(F3, 0, 5, 2, 3, 2, 0)      # two bookshelves
stamp(F3, 2, 0, 1, 2, 4, 0)                                    # window
stamp(F1, 1, 0, 3, 2, 5, 0)                                    # staff-only doorway
put(6, 1, g[1][6] | 0x0400)                                    #   ...locked: its walkable tile made solid
stamp(F3, 4, 5, 2, 2, 8, 0)                                    # the drawing (a picture frame on the wall)
stamp(F3, 10, 0, 1, 2, 10, 0)                                  # window
stamp(F3, 6, 5, 1, 2, 11, 0)                                   # phone
put(12, 0, full(0x289)); put(12, 1, full(0x291))
stamp(F3, 0, 8, 3, 3, 10, 2)                                   # Varnell's desk
stamp(F3, 0, 8, 3, 3, 0, 3)                                    # staff desk
stamp(F3, 0, 11, 3, 3, 2, 6)                                   # reception desk
stamp(F3, 3, 12, 1, 2, 0, 8)                                   # boxes
stamp(F1, 1, 9, 4, 4, 8, 5); stamp(F1, 1, 13, 4, 1, 8, 9)      # meeting table + stools
stamp(F1, 14, 8, 1, 2, 12, 8)                                  # plant
fill(0, 10, W, 1, 0x008)                                       # exit row
stamp(F1, 11, 18, 3, 2, 5, 9)                                  # door mat (upper half + the 3 exit tiles)

def write_interior():
    d = os.path.join(ROOT, 'data/layouts/CeladonCity_VarnellOffice')
    os.makedirs(d, exist_ok=True)
    open(os.path.join(d, 'map.bin'), 'wb').write(b''.join(struct.pack('<H', v) for r in g for v in r))
    border = open(os.path.join(ROOT, LAYOUTS[F1]['border_filepath']), 'rb').read()
    open(os.path.join(d, 'border.bin'), 'wb').write(border)

# ---- Celadon exterior: door + an open path through the fence (values copied from Celadon's own doored houses)
DOOR = {(44, 20): 0x714, (45, 20): 0x715, (46, 20): 0x716,
        (44, 21): 0x6e5, (45, 21): 0x43d, (46, 21): 0x6e7,
        (44, 22): 0x330e, (45, 22): 0x316e, (46, 22): 0x330f,
        (45, 23): 0x316e,          # was the sign (0x403): now path, straight from the street to the door
        (46, 23): 0x403}           # the sign, one tile beside the path (was fence 0x4e7)
def write_exterior():
    p = os.path.join(ROOT, LAYOUTS['CeladonCity_Layout']['blockdata_filepath'])
    b = bytearray(open(p, 'rb').read()); CW = LAYOUTS['CeladonCity_Layout']['width']
    for (x, y), v in DOOR.items(): struct.pack_into('<H', b, (y*CW + x)*2, v)
    open(p, 'wb').write(b)

if __name__ == '__main__':
    write_interior(); write_exterior()
    if '--preview' in sys.argv:
        out = sys.argv[sys.argv.index('--preview') + 1]
        sys.path.insert(0, os.path.join(ROOT, 'docs/kanto_mockups/tools'))
        import render as R
        l = LAYOUTS[F1]; ts = R.TS(l['primary_tileset'], l['secondary_tileset'])
        R.render_ids(ts, [v & 0x3FF for r in g for v in r], W, H, out)
    print('ok')
