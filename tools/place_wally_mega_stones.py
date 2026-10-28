#!/usr/bin/env python3
"""Wally's Journey: put every hidden Mega Stone where its Pokémon lives, on a tile that makes sense.

Owner rule (2026-10-23): Mega Stones "rest where their Pokémon feel at home" (Steven says so in-game),
appear only after the Mega Bracelet, and sparkle on the ground. A good tile is:
  * walkable land (not water, ledge, door, ice slide), or any open tile on an underwater map,
  * reachable from the map's entrances (on foot or by Surf),
  * not on stairs/ramps/bridges (elevation 0 or 15 on maps that have height levels),
  * at least 3 tiles from the map edge and 5 from any warp, not on another event,
  * tucked into a nook (walls/trees/rocks on one or more sides), like the stones in the Hoenn remakes.
Stones already on such a tile in the right map stay put.

Usage: python3 tools/place_wally_mega_stones.py [--apply]
"""
import glob, hashlib, json, re, struct, sys
from collections import deque
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

# stone -> map where its Pokémon lives (map folder name)
HOME = {
    'AUDINITE': 'Route116', 'BARBARACITE': 'SeafloorCavern_Room1', 'CHESNAUGHTITE': 'SafariZone_Northwest',
    'DELPHOXITE': 'Route111', 'DRAGALGITE': 'Underwater_Route126', 'DRAGONINITE': 'SkyPillar_1F',
    'DRAMPANITE': 'MeteorFalls_StevensCave', 'EELEKTROSSITE': 'NewMauville_Inside', 'EMBOARITE': 'FieryPath',
    'FALINKSITE': 'Route114', 'FERALIGITE': 'Route114', 'GLIMMORANITE': 'SeafloorCavern_Room3',
    'GRENINJITE': 'Route120', 'HAWLUCHANITE': 'Route119', 'HERACRONITE': 'SafariZone_North',
    'LOPUNNITE': 'Route115', 'LUCARIONITE': 'VictoryRoad_B2F', 'MALAMARITE': 'SeafloorCavern_Room7',
    'MEGANIUMITE': 'Route104', 'MEOWSTICITE': 'Route121', 'PIDGEOTITE': 'Route102',
    'PINSIRITE': 'SafariZone_Northwest', 'SCOLIPITE': 'PetalburgWoods', 'SCRAFTINITE': 'JaggedPass',
    'STARAPTITE': 'Route104', 'VENUSAURITE': 'SafariZone_South', 'VICTREEBELITE': 'Route119',
    'BLASTOISINITE': 'Route118', 'BAXCALIBRITE': 'ShoalCave_LowTideIceRoom', 'TATSUGIRINITE': 'Underwater_Route128',
    'GALLADITE': 'VictoryRoad_B1F', 'HOUNDOOMINITE': 'SafariZone_Northeast',
    'ALAKAZITE': 'GraniteCave_1F', 'MANECTITE': 'Route110', 'AMPHAROSITE': 'SafariZone_Southeast',
    'RAICHUNITE_Y': 'SafariZone_Southwest',
    'CHARIZARDITE_Y': 'Route112', 'TYRANITARITE': 'MagmaHideout_4F', 'GARCHOMPITE': 'MeteorFalls_B1F_2R',
    'STARMINITE': 'Route109',
}

# Where inside the map the stone should sit: desert sand, tall grass, or a shore beside water.
SAND_STONES = {'DELPHOXITE', 'SCOVILLAINITE'}
SHORE_STONES = {'BLASTOISINITE', 'FERALIGITE', 'GRENINJITE', 'STARMINITE'}
GRASS_STONES = {'AUDINITE', 'MEGANIUMITE', 'STARAPTITE', 'PIDGEOTITE', 'PYROARITE', 'FALINKSITE', 'FLOETTITE',
                'HAWLUCHANITE', 'VICTREEBELITE', 'MEOWSTICITE', 'MANECTITE', 'RAICHUNITE_X', 'RAICHUNITE_Y', 'ABSOLITE',
                'LOPUNNITE', 'SKARMORITE', 'CHESNAUGHTITE', 'PINSIRITE', 'VENUSAURITE', 'HERACRONITE', 'SCIZORITE',
                'KANGASKHANITE', 'AMPHAROSITE', 'HOUNDOOMINITE', 'BEEDRILLITE', 'SCOLIPITE', 'MEDICHAMITE', 'CHIMECHITE'}

def behaviors():
    text = (ROOT / 'include/constants/metatile_behaviors.h').read_text()
    body = text[text.index('{', text.index('enum')) + 1:]
    names, val = {}, 0
    for line in body.splitlines():
        m = re.match(r'\s*(MB_\w+)\s*(?:=\s*(\w+))?\s*,', line)
        if m:
            if m[2]:
                val = int(m[2], 0)
            names[m[1]] = val
            val += 1
        if line.strip().startswith('}'):
            break
    return names

MB = behaviors()
def mbset(*pats):
    return {v for k, v in MB.items() if any(re.fullmatch(p, k) for p in pats)}
WATER = mbset(r'MB_POND_WATER', r'MB_INTERIOR_DEEP_WATER', r'MB_DEEP_WATER', r'MB_WATERFALL', r'MB_SOOTOPOLIS_DEEP_WATER',
              r'MB_OCEAN_WATER', r'MB_SEMI_DEEP_WATER', r'MB_NO_SURFACING', r'MB_SEAWEED', r'MB_SEAWEED_NO_SURFACING',
              r'MB_EASTWARD_CURRENT', r'MB_WESTWARD_CURRENT', r'MB_NORTHWARD_CURRENT', r'MB_SOUTHWARD_CURRENT')
SAND = mbset(r'MB_SAND', r'MB_DEEP_SAND')
GRASS = mbset(r'MB_TALL_GRASS', r'MB_LONG_GRASS', r'MB_ASHGRASS')
AVOID = mbset(r'MB_JUMP_\w+', r'MB_\w*_DOOR\w*', r'MB_\w*WARP\w*', r'MB_\w*STAIRS\w*', r'MB_\w*LADDER\w*', r'MB_SLIDE_\w+',
              r'MB_\w*ARROW\w*', r'MB_CRACKED\w*', r'MB_THIN_ICE', r'MB_ICE', r'MB_MUDDY_SLOPE', r'MB_\w*BRIDGE\w*',
              r'MB_\w*_EDGE', r'MB_SECRET_BASE\w*', r'MB_\w*HOLE\w*', r'MB_COUNTER', r'MB_PC', r'MB_\w*SHELF\w*') | WATER

def tileset_attrs(sym):
    src = (ROOT / 'src/data/tilesets/metatiles.h').read_text()
    m = re.search(r'gMetatileAttributes_' + re.escape(sym[len('gTileset_'):]) + r'\[\]\s*=\s*INCBIN_U16\("([^"]+)"\)', src)
    data = (ROOT / m[1]).read_bytes()
    return [struct.unpack_from('<H', data, i)[0] & 0xFF for i in range(0, len(data), 2)]

LAYOUTS = {l['id']: l for l in json.loads((ROOT / 'data/layouts/layouts.json').read_text())['layouts'] if 'id' in l}
MAPS = {Path(p).parent.name: (Path(p), json.loads(Path(p).read_text())) for p in glob.glob(str(ROOT / 'data/maps/*/map.json'))}

class Grid:
    def __init__(self, mapname):
        self.path, self.m = MAPS[mapname]
        L = LAYOUTS[self.m['layout']]
        self.w, self.h = L['width'], L['height']
        raw = (ROOT / L['blockdata_filepath']).read_bytes()
        self.blocks = [struct.unpack_from('<H', raw, i)[0] for i in range(0, len(raw), 2)]
        prim, sec = tileset_attrs(L['primary_tileset']), tileset_attrs(L['secondary_tileset'])
        self.attr = prim[:512] + [0] * max(0, 512 - len(prim)) + sec
        self.underwater = mapname.startswith('Underwater')
        walk = {self.info(x, y)[1] for x in range(self.w) for y in range(self.h) if self.info(x, y)[0] == 0}
        self.multilevel = bool(walk - {0, 15}) and bool(walk & {0, 15})
    def info(self, x, y):
        v = self.blocks[y * self.w + x]
        mt, coll, elev = v & 0x3FF, (v >> 10) & 3, v >> 12
        beh = self.attr[mt] if mt < len(self.attr) else 0
        return coll, elev, beh
    def passable(self, x, y):
        return 0 <= x < self.w and 0 <= y < self.h and self.info(x, y)[0] == 0
    def reachable(self):
        starts = [(w['x'], w['y']) for w in self.m.get('warp_events', [])]
        for x in range(self.w):
            starts += [(x, 0), (x, self.h - 1)]
        for y in range(self.h):
            starts += [(0, y), (self.w - 1, y)]
        if self.underwater:
            return {(x, y) for x in range(self.w) for y in range(self.h) if self.passable(x, y)}
        seen, q = set(), deque()
        for s in starts:
            if self.passable(*s) and s not in seen:
                seen.add(s); q.append(s)
        while q:
            x, y = q.popleft(); _, e1, b1 = self.info(x, y)
            for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
                if (nx, ny) in seen or not self.passable(nx, ny):
                    continue
                _, e2, b2 = self.info(nx, ny)
                if e1 == e2 or e1 in (0, 15) or e2 in (0, 15) or b1 in WATER or b2 in WATER:
                    seen.add((nx, ny)); q.append((nx, ny))
        return seen
    def occupied(self):
        occ = set()
        for key in ('object_events', 'warp_events', 'coord_events', 'bg_events'):
            for e in self.m.get(key, []):
                if 'x' in e:
                    occ.add((e['x'], e['y']))
        return occ

def habitat(g, stone, x, y):
    beh = g.info(x, y)[2]
    if stone in SAND_STONES:
        return beh in SAND
    if stone in GRASS_STONES:
        return beh in GRASS
    if stone in SHORE_STONES:
        return any(g.passable(nx, ny) and g.info(nx, ny)[2] in WATER or (0 <= nx < g.w and 0 <= ny < g.h and g.info(nx, ny)[2] in WATER)
                   for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)))
    return True

def good(g, x, y, reach, occ, warps, ignore=None):
    coll, elev, beh = g.info(x, y)
    if coll or (x, y) not in reach:
        return None
    if elev in (0, 15) and g.multilevel:
        return None  # on multi-level maps these are stairs, ramps and bridges; a stone there looks wrong and may not respond
    if not g.underwater and beh in AVOID:
        return None
    if g.underwater and beh in AVOID - WATER:
        return None
    if x < 3 or y < 3 or x > g.w - 4 or y > g.h - 4:
        return None
    if any(abs(x - wx) + abs(y - wy) < 5 for wx, wy in warps):
        return None
    if any(abs(x - ox) <= 1 and abs(y - oy) <= 1 for ox, oy in occ if (ox, oy) != ignore):
        return None
    nook = sum(1 for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1))
               if not g.passable(nx, ny) or (not g.underwater and g.info(nx, ny)[2] in WATER))
    if nook == 0 or nook >= 4:
        return None
    dist = min([abs(x - wx) + abs(y - wy) for wx, wy in warps] or [20])
    return nook, min(dist, 20)

def main(apply):
    stones = []
    for name, (p, m) in MAPS.items():
        for b in m.get('bg_events', []):
            it = b.get('item', '')
            if b.get('type') == 'hidden_item' and '_WJ_' in b.get('flag', '') and re.search(r'ITE(_X|_Y)?$', it) and it != 'ITEM_BLACK_AUGURITE':
                stones.append((it[5:], name, b))
    moved, kept, failed = [], [], []
    for stone, cur, b in sorted(stones):
        target = HOME.get(stone, cur)
        g = Grid(target)
        reach, occ = g.reachable(), g.occupied()
        warps = [(w['x'], w['y']) for w in g.m.get('warp_events', [])]
        if target == cur and good(g, b['x'], b['y'], reach, occ, warps, ignore=(b['x'], b['y'])) and habitat(g, stone, b['x'], b['y']):
            kept.append((stone, cur, b['x'], b['y'])); continue
        cands = []
        for (x, y) in reach:
            s = good(g, x, y, reach, occ, warps)
            if s:
                h = int(hashlib.md5(f'{stone}{x},{y}'.encode()).hexdigest()[:6], 16)
                cands.append((habitat(g, stone, x, y), s[0] >= 2, s[1] >= 8, h, x, y))
        if not cands:
            failed.append((stone, target)); continue
        best = max(cands)
        if not best[0]:
            failed.append((stone, target, 'no habitat tile')); continue
        x, y = best[-2], best[-1]
        moved.append((stone, cur, (b['x'], b['y']), target, (x, y)))
        if apply:
            MAPS[cur][1]['bg_events'].remove(b)
            nb = dict(b); nb['x'], nb['y'] = x, y
            nb['elevation'] = g.info(x, y)[1]
            MAPS[target][1]['bg_events'].append(nb)
    for r in moved: print('MOVE', *r)
    print('kept', len(kept), 'moved', len(moved), 'failed', failed)
    if apply and not failed:
        for name in {r[1] for r in moved} | {r[3] for r in moved}:
            p, m = MAPS[name]
            p.write_text(json.dumps(m, indent=2, ensure_ascii=False) + '\n')
        print('written')
    return kept, moved, failed

if __name__ == '__main__':
    main('--apply' in sys.argv)
