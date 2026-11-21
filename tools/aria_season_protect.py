#!/usr/bin/env python3
"""Project ARIA (owner 2026-11-21, option 1): which Kanto tileset colours follow the seasons.
Kanto buildings keep their colours; only nature changes. Python model of the C rule in src/wally_journey.c
(KantoSeasonProtected), used for the before/after pictures:
  * General_Frlg (primary): slots 0, 1, 4, 5 are nature (trees, grass, rocks, sand, paths) -> change;
    slots 2, 3 are buildings (Pokémon Centers, Marts, gates, roofs) -> keep.
  * Kanto secondary tilesets: every slot keeps its colours, except the nature slots listed in NATURE.
  * Anywhere a kept slot has the exact grass/path colour of General_Frlg slots 0/5 (the grass fringe drawn
    round a building), that colour still changes, so the fringe matches the grass next to it.
Usage: python3 tools/aria_season_protect.py OUTDIR Layout [Layout ...]  (4 seasons, kept vs old)"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import aria_season_sim as A
import render as R
from PIL import Image, ImageDraw
PRIMARY_NATURE = {0, 1, 4, 5}
NATURE = {  # secondary tileset -> its nature slots (absolute BG slot numbers 7..12)
    'gTileset_PalletTown': {11, 12},
    'gTileset_FuchsiaCity': {11},          # Safari Zone trees
    'gTileset_ViridianForest': {8, 10},
    'gTileset_BerryForest': {8, 10, 11},
    'gTileset_SeviiIslands123': {12},      # rocks / mountain
    'gTileset_SeviiIslands67': {10},       # rocks / ground
}
def protect_set(prim, sec):
    if prim != 'gTileset_General_Frlg': return frozenset()
    ts = R.TS(prim, sec)
    shared = {A.to5(c) for s in (0, 5) for c in ts.pal[s][1:]}
    keep = set()
    for slot in range(16):
        nature = slot in PRIMARY_NATURE if slot < 7 else slot in NATURE.get(sec, set())
        if nature: continue
        for i, c in enumerate(ts.pal[slot]):
            if i and A.to5(c) not in shared: keep.add((slot, i))
    return frozenset(keep)
if __name__ == '__main__':
    out = sys.argv[1]
    for n in sys.argv[2:]:
        l = R.layout(n); P = protect_set(l['primary_tileset'], l['secondary_tileset'])
        rows = []
        for prot in (frozenset(), P):
            rows.append([A.render_map(n, m, prot) for m in (A.NONE, A.SPRING, A.AUTUMN, A.FROST)])
        w, h = rows[0][0].size; sc = 0.5 if w > 500 else 1.0
        w2, h2 = int(w*sc), int(h*sc)
        o = Image.new('RGB', (4*(w2+6), 2*(h2+20)), (30, 30, 30)); d = ImageDraw.Draw(o)
        for r, row in enumerate(rows):
            for k, im in enumerate(row):
                o.paste(im.resize((w2, h2)), (k*(w2+6), r*(h2+20)+18))
                d.text((k*(w2+6)+4, r*(h2+20)+3), ['before', 'fixed'][r] + ' - ' + ['summer', 'spring', 'autumn', 'winter'][k], fill='white')
        o.save(os.path.join(out, n + '_fix.png'))
