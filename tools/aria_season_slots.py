#!/usr/bin/env python3
"""Project ARIA: contact sheet of a Kanto tileset's palette slots — every 8x8 tile its metatiles draw with each
slot, and which colours the season recolour would change (red outline). Used to pick which slots are foliage
(may change with the seasons) and which are buildings (keep their colours). Usage:
  python3 tools/aria_season_slots.py OUT.png gTileset_General_Frlg gTileset_CeladonCity [primary|secondary]"""
import sys, os, struct
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))); sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '../docs/kanto_mockups/tools'))
import render as R, aria_season_sim as A
from PIL import Image, ImageDraw
out, prim, sec, which = sys.argv[1:5]
ts = R.TS(prim, sec)
meta = ts.meta if which == 'primary' else ts.meta2
use = {}
for j in range(len(meta)//16):
    for k in range(8):
        v = struct.unpack('<H', meta[j*16+k*2:j*16+k*2+2])[0]
        use.setdefault(v >> 12, set()).add(v & 0x3FF)
slots = range(0, 7) if which == 'primary' else range(7, 13)
rows = []
for slot in slots:
    tl = sorted(use.get(slot, ()))
    chg = [k for k, c in enumerate(ts.pal[slot]) if k and any(A.seasonal(A.to5(c), m, slot) != A.to5(c) for m in (A.AUTUMN, A.FROST, A.SPRING))]
    n = min(len(tl), 96)
    im = Image.new('RGB', (260 + 24*20, max(40, 30 + ((n+23)//24)*20)), (40, 40, 40)); d = ImageDraw.Draw(im)
    d.text((2, 2), 'slot %d (%d tiles)' % (slot, len(tl)), fill='white')
    for k, c in enumerate(ts.pal[slot]): d.rectangle((2+k*15, 16, 14+k*15, 28), fill=c, outline=(255, 0, 0) if k in chg else (90, 90, 90), width=2)
    for i, t in enumerate(tl[:n]):
        tile = ts.tiles[t] if t < len(ts.tiles) else None
        if not tile: continue
        ti = Image.new('RGB', (8, 8))
        for y in range(8):
            for x in range(8): ti.putpixel((x, y), (60, 0, 60) if tile[y][x] == 0 else ts.pal[slot][tile[y][x]])
        im.paste(ti.resize((16, 16), Image.NEAREST), (260 + (i % 24)*20, 30 + (i//24)*20))
    rows.append(im)
o = Image.new('RGB', (max(r.width for r in rows), sum(r.height + 4 for r in rows)), (0, 0, 0)); y = 0
for r in rows: o.paste(r, (0, y)); y += r.height + 4
o.save(out)
