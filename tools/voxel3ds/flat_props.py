#!/usr/bin/env python3
"""usage: flat_props.py DUMPDIR MAPS.json OUT.json   (run from the repo root)

Region audit 2026-11-23 (Brennan 18:34: "check every single map for flat 2D props, rocks, boulders, craters, stools,
potted plants, signs, or laid-down flat 2D ground sprites that should be 3D models").
For every map dumped by WJPC_DUMP_TERRAIN_ALL=DIR (voxel_gl.c): a cell the player cannot walk on (map.bin collision),
not water, whose 3D has nothing standing on it - no model or tree vertex more than 0.2 tile over the cell's ground,
the cell's ground no higher than its lowest neighbour's + 0.1, and no upright terrain face inside it - is drawn flat.
Such cells are counted per metatile (tileset and id) with the maps they are on. MAPS.json: [[group, num, name, type,
w, h, spots], ...] (the audit's list). Writes OUT.json {metatile: {count, maps: {name: [cells]}}}."""
import collections
import glob
import json
import os
import re
import struct
import sys

DUMP, MAPS, OUT = sys.argv[1:4]
maps = json.load(open(MAPS))
layouts = {l['id']: l for l in json.load(open('data/layouts/layouts.json'))['layouts'] if 'id' in l}
mt_h = open('src/data/tilesets/metatiles.h').read()
beh_names = re.findall(r'^\s*(MB_[A-Z0-9_]+)', open('include/constants/metatile_behaviors.h').read(), re.M)
WATERISH = re.compile(r'WATER|POND|OCEAN|WATERFALL|SEAWEED|DIVE|CURRENT|PUDDLE|SOOTOPOLIS_DEEP|NO_SURFACING')
attr_cache = {}


def attrs(tileset):
    if tileset not in attr_cache:
        m = re.search(r'gMetatileAttributes_%s\[\] = INCBIN_U16\("([^"]+)"' % tileset.replace('gTileset_', ''), mt_h)
        d = open(m.group(1), 'rb').read() if m else b''
        attr_cache[tileset] = struct.unpack('<%dH' % (len(d) // 2), d)
    return attr_cache[tileset]


def load(path):
    d = open(path, 'rb').read()
    n = len(d) // 24
    v = struct.unpack('<%df' % (n * 6), d[:n * 24])
    return [v[i * 6:i * 6 + 3] for i in range(n)]


def main():
    found = collections.defaultdict(lambda: {'count': 0, 'maps': collections.defaultdict(list)})
    for g, n, name, typ, w, h, spots in maps:
        pre = os.path.join(DUMP, '%d_%d' % (g, n))
        pages = [f for f in glob.glob(pre + '.*') if f.rsplit('.', 1)[1].isdigit()]
        if not pages:
            continue
        m = json.load(open('data/maps/%s/map.json' % name))
        L = layouts[m['layout']]
        W, H = L['width'], L['height']
        mb = open(L['blockdata_filepath'], 'rb').read()
        pa, sa = attrs(L['primary_tileset']), attrs(L['secondary_tileset'])
        top = collections.defaultdict(lambda: -99.0)   # cell -> highest flat terrain at its centre (approx: any flat tri vertex in cell)
        upright = set()
        for f in pages:
            vs = load(f)
            for t in range(len(vs) // 3):
                a, b, c = vs[3 * t:3 * t + 3]
                ys = (a[1], b[1], c[1])
                cx = (a[0] + b[0] + c[0]) / 3; cz = (a[2] + b[2] + c[2]) / 3
                cell = (int(cx // 1), int(cz // 1))
                if max(ys) - min(ys) < 1e-4:
                    top[cell] = max(top[cell], ys[0])
                elif max(ys) - min(ys) > 0.2:
                    upright.add(cell)
        tall = set()
        for layer in ('models', 'trees'):
            f = pre + '.' + layer
            if not os.path.exists(f):
                continue
            vs = load(f)
            for v in vs:
                cell = (int(v[0] // 1), int(v[2] // 1))
                if v[1] > top[cell] + 0.2:
                    tall.add(cell)
        for y in range(H):
            for x in range(W):
                v = struct.unpack_from('<H', mb, 2 * (y * W + x))[0]
                if (v >> 10) & 3 == 0:
                    continue
                mid = v & 0x3FF
                at = pa if mid < 512 else sa
                k = mid if mid < 512 else mid - 512
                beh = at[k] & 0xFF if k < len(at) else 0
                bname = beh_names[beh] if beh < len(beh_names) else ''
                if WATERISH.search(bname):
                    continue
                if (x, y) in tall or (x, y) in upright:
                    continue
                t0 = top[(x, y)]
                if t0 < -90:
                    continue
                nb = [top[(x + dx, y + dy)] for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)) if top[(x + dx, y + dy)] > -90]
                if nb and t0 > min(nb) + 0.1:
                    continue
                ts = L['primary_tileset'] if mid < 512 else L['secondary_tileset']
                key = '%s:0x%03X' % (ts.replace('gTileset_', ''), mid)
                found[key]['count'] += 1
                found[key]['maps'][name].append((x, y))
    out = {k: {'count': v['count'], 'maps': dict(v['maps'])} for k, v in found.items()}
    old = json.load(open(OUT)) if os.path.exists(OUT) else {}
    for k, v in out.items():
        o = old.setdefault(k, {'count': 0, 'maps': {}})
        o['count'] += v['count']
        o['maps'].update(v['maps'])
    json.dump(old, open(OUT, 'w'))


if __name__ == '__main__':
    main()
