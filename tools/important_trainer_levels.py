#!/usr/bin/env python3
"""Wally's Journey: expected player level at every important trainer's first fight,
and the minimum ace level used by first-encounter scaling (owner request 2026-11-02).

Builds on tools/exp_curve_sim.py (real EXP rules, EXP ALL even split, every reachable
trainer in story order, rematch tiers skipped) and adds:
  * a little wild training: WILD_PER_MAP battles on each map that has wild Pokémon,
    at that map's average level and average EXP yield, before its trainers;
  * important trainers fought after the ordinary trainers of their map;
  * the Route 103 rival, the Space Center multi battle, the Elite Four and the Champion;
  * important teams raised to their minimum as the game will (so the EXP they give
    is the scaled EXP).

Minimum ace = expected highest party level right before the fight + the group's margin
(owner: Gyms +2, rivals +1, admins +1, Elite Four +3, Wally +4 for his whole team (owner 2026-11-03); Maxie/Archie +2,
Winstrate family +2, Vito in Victory Road +2).

    python3 tools/important_trainer_levels.py            # table
    python3 tools/important_trainer_levels.py --header   # also write src/data/wj_trainer_scaling.h
"""
import json, os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import exp_curve_sim as E

WILD_PER_MAP = 4      # "a little bit of extra exploration and wild training"
GYM, RIVAL, ADMIN, BOSS, E4, WALLY, FAMILY = 2, 1, 1, 2, 3, 4, 2

def rivals(fight):
    return ['TRAINER_%s_%s_%s' % (g, fight, s) for g in ('MAY', 'BRENDAN')
            for s in ('TREECKO', 'TORCHIC', 'MUDKIP')]

# (label, trainer ids sharing one fight/save slot, margin) in story order inside each map.
# Wally's Mauville fight is left out: he is the beginner with one Ralts there (story).
FIGHTS = [
    ('May/Brendan Route 103', rivals('ROUTE_103'), RIVAL),
    ('Roxanne', ['TRAINER_ROXANNE_1'], GYM),
    ('May/Brendan Rustboro', rivals('RUSTBORO'), RIVAL),
    ('Brawly', ['TRAINER_BRAWLY_1'], GYM),
    ('May/Brendan Route 110', rivals('ROUTE_110'), RIVAL),
    ('Wattson', ['TRAINER_WATTSON_1'], GYM),
    ('Victor (Winstrates)', ['TRAINER_VICTOR'], FAMILY),
    ('Victoria (Winstrates)', ['TRAINER_VICTORIA'], FAMILY),
    ('Vivi (Winstrates)', ['TRAINER_VIVI'], FAMILY),
    ('Vicky (Winstrates)', ['TRAINER_VICKY'], FAMILY),
    ('Wally Fiery Path', ['TRAINER_WALLY_VERDANTURF'], WALLY),
    ('Tabitha Mt. Chimney', ['TRAINER_TABITHA_MT_CHIMNEY'], ADMIN),
    ('Maxie Mt. Chimney', ['TRAINER_MAXIE_MT_CHIMNEY'], BOSS),
    ('Flannery', ['TRAINER_FLANNERY_1'], GYM),
    ('Wally Route 112', ['TRAINER_WALLY_LAVARIDGE'], WALLY),
    ('Norman', ['TRAINER_NORMAN_1'], GYM),
    ('May/Brendan Route 119', rivals('ROUTE_119'), RIVAL),
    ('Shelly Weather Institute', ['TRAINER_SHELLY_WEATHER_INSTITUTE'], ADMIN),
    ('Winona', ['TRAINER_WINONA_1'], GYM),
    ('Wally Route 118', ['TRAINER_WALLY_FORTREE'], WALLY),
    ('May/Brendan Lilycove', rivals('LILYCOVE'), RIVAL),
    ('Wally Lilycove', ['TRAINER_WALLY_LILYCOVE'], WALLY),
    ('Tabitha Magma Hideout', ['TRAINER_TABITHA_MAGMA_HIDEOUT'], ADMIN),
    ('Maxie Magma Hideout', ['TRAINER_MAXIE_MAGMA_HIDEOUT'], BOSS),
    ('Matt', ['TRAINER_MATT'], ADMIN),
    ('Tabitha Space Center', ['TRAINER_TABITHA_MOSSDEEP'], ADMIN),
    ('Maxie Space Center', ['TRAINER_MAXIE_MOSSDEEP'], BOSS),
    ('Tate & Liza', ['TRAINER_TATE_AND_LIZA_1'], GYM),
    ('Wally Mossdeep', ['TRAINER_WALLY_MOSSDEEP'], WALLY),
    ('Shelly Seafloor Cavern', ['TRAINER_SHELLY_SEAFLOOR_CAVERN'], ADMIN),
    ('Archie', ['TRAINER_ARCHIE'], BOSS),
    ('Wally Sootopolis', ['TRAINER_WALLY_SOOTOPOLIS'], WALLY),
    ('Juan', ['TRAINER_JUAN_1'], GYM),
    ('Wally Victory Road', ['TRAINER_WALLY_VR_1'], WALLY),
    ('Vito Victory Road', ['TRAINER_VITO'], FAMILY),
    ('Sidney', ['TRAINER_SIDNEY'], E4),
    ('Phoebe', ['TRAINER_PHOEBE'], E4),
    ('Glacia', ['TRAINER_GLACIA'], E4),
    ('Drake', ['TRAINER_DRAKE'], E4),
    ('Wally Champion', ['TRAINER_WALLY_CHAMPION'], WALLY),
]
FIGHT_OF = {t: n for n, f in enumerate(FIGHTS) for t in f[1]}
# Wally (owner 2026-11-03) and the Gym Leaders (owner 2026-11-11): the minimum and margin
# apply to the lowest Pokemon, so the whole team is at least the player's top + margin.
def BY_LOWEST(n): return FIGHTS[n][0].startswith('Wally') or FIGHTS[n][2] == GYM and FIGHTS[n][1][0].endswith('_1')
SIM_ID = {n: f[1][0] for n, f in enumerate(FIGHTS)}          # the team the sim fights
LEAGUE = ['TRAINER_SIDNEY', 'TRAINER_PHOEBE', 'TRAINER_GLACIA', 'TRAINER_DRAKE', 'TRAINER_WALLY_CHAMPION']

# ---------- wild tables per map folder
mapId = {}
for p in E.glob.glob('data/maps/*/map.json'):
    try: mapId[p.split('/')[2]] = json.load(open(p))['id']
    except Exception: pass
wild = {}
for g in json.load(open('src/data/wild_encounters.json'))['wild_encounter_groups']:
    for e in g.get('encounters', []):
        if 'map' not in e or e.get('base_label', '').endswith(('_Morning', '_Evening', '_Night')): continue
        mons = []
        for key in ('land_mons', 'water_mons'):
            if key in e: mons += e[key]['mons']
        if mons: wild.setdefault(e['map'], mons)

def sequence():
    """Story-order encounters: ('wild', map, mons) or ('trainer', map, id)."""
    seenmap = set(); out = []
    for m, t in E.encounters():
        if t in LEAGUE: continue
        if m != '@' and m not in seenmap:
            seenmap.add(m)
            mons = wild.get(mapId.get(m, ''))
            if mons: out += [('wild', m, mons)] * WILD_PER_MAP
        out.append(('trainer', m, t))
    res = []; i = 0
    while i < len(out):                 # important fights last inside their map
        j = i + 1
        if out[i][1] != '@':
            while j < len(out) and out[j][1] == out[i][1]: j += 1
        chunk = out[i:j]
        imp = lambda c: c[0] == 'trainer' and c[2] in FIGHT_OF
        res += [c for c in chunk if not imp(c)] + sorted(filter(imp, chunk), key=lambda c: FIGHT_OF[c[2]])
        i = j
    def after(mapname, ids):
        k = max(n for n, c in enumerate(res) if c[1] == mapname)
        res[k + 1:k + 1] = [('trainer', '@', t) for t in ids]
    after('Route103', ['TRAINER_MAY_ROUTE_103_TREECKO'])
    after('MossdeepCity_SpaceCenter_2F', ['TRAINER_TABITHA_MOSSDEEP', 'TRAINER_MAXIE_MOSSDEEP'])
    res += [('trainer', 'League', t) for t in LEAGUE]
    # the Route 103 rival comes before Route 102's trainers
    k = next(n for n, c in enumerate(res) if c[2:] == ('TRAINER_MAY_ROUTE_103_TREECKO',))
    res.insert(0, res.pop(k))
    return res

def lv_of(mon): return E.level_of(mon[0], mon[1])

def gain(team, y, lv):
    base = int(y * lv / 5)
    pool = base + base // 2
    share = max(1, pool // len(team))
    for mon in team:
        mon[1] += int(share * E.scale(lv, lv_of(mon))) + 1

def run():
    rates = ('MS', 'MF', 'MF', 'MF', 'S', 'F')
    join = (0, 1, 3, 5, 9, 12)
    team = [[rates[0], E.exp_for(rates[0], 5)]]
    rows = {}; ntr = 0
    for kind, m, x in sequence():
        if kind == 'wild':
            sps = [w['species'] for w in x if w['species'] in E.yields]
            if sps:
                lv = round(sum((w['min_level'] + w['max_level']) / 2 for w in x) / len(x))
                gain(team, sum(E.yields[s] for s in sps) / len(sps), lv)
            continue
        while len(team) < 6 and ntr >= join[len(team)]:
            l = max(3, min(lv_of(t) for t in team) - 2)
            r = rates[len(team)]; team.append([r, E.exp_for(r, l)])
        ntr += 1
        party = E.parties.get(x, [])
        if x in FIGHT_OF and party:
            n = FIGHT_OF[x]
            lv = [lv_of(t) for t in team]
            ace = max(l for _, l in party)
            # Wally (owner 2026-11-03): his WHOLE team is at least top + margin, so the
            # minimum is for his lowest Pokemon; the ace keeps its lead above the rest.
            base = min(l for _, l in party) if BY_LOWEST(n) else ace
            minace = max(lv) + FIGHTS[n][2]
            shift = max(0, minace - base)
            rows[n] = (max(lv), sum(lv) / len(lv), ace, minace, shift)
            party = [(sp, min(100, l + shift)) for sp, l in party]
        for sp, l in party:
            if sp in E.yields: gain(team, E.yields[sp], l)
    return rows

def write_header(rows):
    out = ['// Generated by tools/important_trainer_levels.py. Do not edit by hand.',
           '// Wally\'s Journey first-encounter scaling (owner request 2026-11-02):',
           '// {trainer, minimum level, margin over the player\'s highest level, save slot, byLowest}',
           '// byLowest = 1 (Wally): the minimum and margin apply to the lowest Pokemon, not the ace.', '']
    out.append('static const struct WJScaledTrainer sWJScaledTrainers[] = {')
    for n, (label, ids, margin) in enumerate(FIGHTS):
        for t in ids:
            if t not in E.parties: continue
            out.append(f'    {{{t}, {rows[n][3]}, {margin}, {n}, {int(BY_LOWEST(n))}}}, // {label}')
    out += ['};', f'#define WJ_SCALED_SLOTS {len(FIGHTS)}', '']
    open('src/data/wj_trainer_scaling.h', 'w').write('\n'.join(out))

if __name__ == '__main__':
    rows = run()
    print(f"{'fight':28s}{'you top':>8s}{'you avg':>8s}{'ace now':>8s}{'min ace':>8s}{'raise':>7s}")
    for n, (label, ids, margin) in enumerate(FIGHTS):
        top, avg, ace, minace, shift = rows[n]
        print(f"{label:28s}{top:8d}{avg:8.1f}{ace:8d}{minace:8d}{('+%d' % shift) if shift else '':>7s}")
    if '--header' in sys.argv:
        write_header(rows)
        print('wrote src/data/wj_trainer_scaling.h')
