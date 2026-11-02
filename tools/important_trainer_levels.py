#!/usr/bin/env python3
"""Wally's Journey: expected player level at every important trainer's first fight.

Builds on tools/exp_curve_sim.py (real EXP rules, EXP ALL even split, every reachable
trainer in story order, rematch tiers skipped) and adds:
  * a little wild training: WILD_PER_MAP battles on each map that has wild Pokémon,
    at that map's average level and average EXP yield, before its trainers;
  * important trainers fought after the ordinary trainers of their map;
  * the Elite Four and the Champion fight after Victory Road.

Output: for each important trainer, the party's expected HIGHEST level right before the
fight, and the trainer's current lowest/highest level. The minimum level for the
first-encounter scaling is derived from the expected highest level.

    python3 tools/important_trainer_levels.py            # table
    python3 tools/important_trainer_levels.py --header   # also write the C table
"""
import json, os, re, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import exp_curve_sim as E

WILD_PER_MAP = 4      # "a little bit of extra exploration and wild training"
MARGIN = 1            # minimum ace level = expected highest level + MARGIN

# Important trainers, in the order they're fought inside their map.
IMPORTANT = [
    'TRAINER_ROXANNE_1', 'TRAINER_MAY_RUSTBORO_TREECKO', 'TRAINER_BRAWLY_1',
    'TRAINER_MAY_ROUTE_110_TREECKO', 'TRAINER_WALLY_MAUVILLE', 'TRAINER_WATTSON_1',
    'TRAINER_WALLY_VERDANTURF', 'TRAINER_TABITHA_MT_CHIMNEY', 'TRAINER_MAXIE_MT_CHIMNEY',
    'TRAINER_FLANNERY_1', 'TRAINER_WALLY_LAVARIDGE', 'TRAINER_NORMAN_1',
    'TRAINER_MAY_ROUTE_119_TREECKO', 'TRAINER_SHELLY_WEATHER_INSTITUTE', 'TRAINER_WINONA_1',
    'TRAINER_WALLY_FORTREE', 'TRAINER_MAY_LILYCOVE_TREECKO', 'TRAINER_WALLY_LILYCOVE',
    'TRAINER_TABITHA_MAGMA_HIDEOUT', 'TRAINER_MAXIE_MAGMA_HIDEOUT', 'TRAINER_MATT',
    'TRAINER_TABITHA_MOSSDEEP', 'TRAINER_MAXIE_MOSSDEEP', 'TRAINER_TATE_AND_LIZA_1',
    'TRAINER_WALLY_MOSSDEEP', 'TRAINER_SHELLY_SEAFLOOR_CAVERN', 'TRAINER_ARCHIE',
    'TRAINER_WALLY_SOOTOPOLIS', 'TRAINER_JUAN_1', 'TRAINER_WALLY_VR_1',
    'TRAINER_SIDNEY', 'TRAINER_PHOEBE', 'TRAINER_GLACIA', 'TRAINER_DRAKE', 'TRAINER_WALLY_CHAMPION',
]
LEAGUE = ['TRAINER_SIDNEY', 'TRAINER_PHOEBE', 'TRAINER_GLACIA', 'TRAINER_DRAKE', 'TRAINER_WALLY_CHAMPION']

# ---------- wild tables per map folder
mapId = {}
for p in E.glob.glob('data/maps/*/map.json'):
    try: mapId[p.split('/')[2]] = json.load(open(p))['id']
    except Exception: pass
wild = {}
for g in json.load(open('src/data/wild_encounters.json'))['wild_encounter_groups']:
    for e in g.get('encounters', []):
        if e.get('base_label', '').endswith(('_Morning', '_Evening', '_Night')): continue
        mons = []
        for key in ('land_mons', 'water_mons'):
            if key in e: mons += e[key]['mons']
        if mons and 'map' in e: wild.setdefault(e['map'], mons)

def sequence():
    """Story-order encounters: ('wild', map, [(sp, lv)]) or ('trainer', map, id)."""
    seenmap = set(); out = []; pend = []
    for m, t in E.encounters():
        if t in LEAGUE: continue
        if m != '@' and m not in seenmap:
            seenmap.add(m)
            mons = wild.get(mapId.get(m, ''))
            if mons:
                out += [('wild', m, mons)] * WILD_PER_MAP
        out.append(('trainer', m, t))
    # important trainers go after the ordinary trainers of their map, in IMPORTANT order
    res = []
    i = 0
    while i < len(out):
        j = i
        while j < len(out) and out[j][1] == out[i][1] and out[i][1] != '@': j += 1
        if j == i: j = i + 1
        chunk = out[i:j]
        normal = [c for c in chunk if not (c[0] == 'trainer' and c[2] in IMPORTANT)]
        imp = sorted([c for c in chunk if c[0] == 'trainer' and c[2] in IMPORTANT],
                     key=lambda c: IMPORTANT.index(c[2]))
        res += normal + imp
        i = j
    # The Space Center multi battle is started with multi_2_vs_2, not trainerbattle.
    k = max(n for n, c in enumerate(res) if c[1] == 'MossdeepCity_SpaceCenter_2F')
    res[k + 1:k + 1] = [('trainer', '@', 'TRAINER_TABITHA_MOSSDEEP'),
                        ('trainer', '@', 'TRAINER_MAXIE_MOSSDEEP')]
    res += [('trainer', 'League', t) for t in LEAGUE]
    return res

def lv_of(mon): return E.level_of(mon[0], mon[1])

def gain(team, sp, lv):
    y = E.yields.get(sp)
    if y is None: return
    base = int(y * lv / 5)
    pool = base + base // 2
    share = max(1, pool // len(team))
    for mon in team:
        mon[1] += int(share * E.scale(lv, lv_of(mon))) + 1

def run():
    rates = ('MS', 'MF', 'MF', 'MF', 'S', 'F')
    join = (0, 1, 3, 5, 9, 12)
    team = [[rates[0], E.exp_for(rates[0], 5)]]
    rows = []; ntr = 0
    for kind, m, x in sequence():
        if kind == 'trainer':
            while len(team) < 6 and ntr >= join[len(team)]:
                l = max(3, min(lv_of(t) for t in team) - 2)
                r = rates[len(team)]; team.append([r, E.exp_for(r, l)])
            ntr += 1
            if x in IMPORTANT:
                lv = [lv_of(t) for t in team]
                p = [l for _, l in E.parties.get(x, [])]
                rows.append((x, max(lv), sum(lv) / len(lv), min(p), max(p)))
            for sp, l in E.parties.get(x, []): gain(team, sp, l)
        else:
            avg_lv = round(sum((w['min_level'] + w['max_level']) / 2 for w in x) / len(x))
            sps = [w['species'] for w in x if w['species'] in E.yields]
            if not sps: continue
            y = sum(E.yields[s] for s in sps) / len(sps)
            E.yields['_WILD'] = round(y)
            gain(team, '_WILD', avg_lv)
    return rows

if __name__ == '__main__':
    rows = run()
    print(f"{'trainer':34s}{'you top':>8s}{'you avg':>8s}{'foe low':>8s}{'foe ace':>8s}{'min ace':>8s}")
    for t, top, avg, lo, hi in rows:
        print(f"{t[8:]:34s}{top:8d}{avg:8.1f}{lo:8d}{hi:8d}{top + MARGIN:8d}")
