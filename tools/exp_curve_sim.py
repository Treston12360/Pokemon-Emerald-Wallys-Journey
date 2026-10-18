#!/usr/bin/env python3
"""Wally's Journey pre-League EXP simulation (all trainers beaten, EXP ALL mode).

Uses the game's real rules (src/battle_script_commands.c):
  base = expYield * foeLevel / 5            (B_SCALED_EXP gen7+, no trainer 1.5x)
  pool = base * 1 participant + base / 2     (EXP ALL: normal total, split evenly)
  each eligible mon gets pool / partySize, then scaled:
     * ((2*foe+10)^2.5 / (foe+own+10)^2.5) + 1
"""
import re, glob, sys, os, json
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)

# ---------- species exp yields (+ display name -> species)
yields = {}; byname = {}
for p in glob.glob('src/data/pokemon/species_info/*.h'):
    s = open(p).read()
    macros = {}
    for d in re.finditer(r'#define (\w+_EXP_YIELD) (\d+)', s):
        macros.setdefault(d.group(1), int(d.group(2)))   # first = updated (P_UPDATED_EXP_YIELDS on)
    for m in re.finditer(r'\[(SPECIES_\w+)\]\s*=\s*\{(.*?)\n    \},', s, re.S):
        body = m.group(2)
        nm = re.search(r'\.speciesName\s*=\s*_\("([^"]+)"\)', body)
        if nm: byname.setdefault(nm.group(1).upper(), m.group(1))
        y = re.search(r'\.expYield\s*=\s*([^,\n]+)', body)
        if not y: continue
        expr = y.group(1).strip()
        t = re.match(r'\(P_UPDATED_EXP_YIELDS\s*>=\s*GEN_\d+\)\s*\?\s*(\d+)', expr)
        if t: yields[m.group(1)] = int(t.group(1)); continue
        if expr in macros: yields[m.group(1)] = macros[expr]; continue
        n = re.match(r'(\d+)', expr)
        if n: yields[m.group(1)] = int(n.group(1))
# ---------- trainer parties
HEAD = re.compile(r'^(Name|Class|Pic|Gender|Music|Items|Double Battle|AI|Mugshot|Starting Status|Back Pic|Party Size|Pool Rules|Copy Pool|Macro)\b')
parties = {}
cur = None; prev = ''
unresolved = set()
def resolve(tok):
    tok = tok.split(' @')[0].strip()
    m = re.search(r'\((SPECIES_\w+|[^()]+)\)\s*(\((M|F)\))?$', tok)
    if re.search(r'\b(SPECIES_\w+)', tok): return re.search(r'\b(SPECIES_\w+)', tok).group(1)
    tok = re.sub(r'\s*\((M|F)\)$', '', tok)
    inner = re.search(r'\(([^()]+)\)$', tok)
    for cand in ([inner.group(1)] if inner else []) + [tok]:
        c = cand.strip().upper()
        if c in byname: return byname[c]
        base = c.split('-')[0]
        if base in byname: return byname[base]
    unresolved.add(tok); return None
for line in open('src/data/trainers.party'):
    line = line.rstrip('\n')
    m = re.match(r'=== (TRAINER_\w+) ===', line)
    if m:
        cur = m.group(1); parties[cur] = []; prev = ''; continue
    lv = re.match(r'Level:\s*(\d+)', line)
    if lv and cur and prev and not HEAD.match(prev):
        sp = resolve(prev)
        if sp: parties[cur].append([sp, int(lv.group(1))])
    if line.strip(): prev = line.strip()
# ---------- trainers per map
mapTrainers = {}
for p in sorted(glob.glob('data/maps/*/scripts.inc')):
    m = p.split('/')[2]
    if 'Frlg' in m: continue
    ids = sorted(set(re.findall(r'trainerbattle\w*\s+(TRAINER_\w+)', open(p).read())))
    if ids: mapTrainers[m] = ids

def rival(tag):
    return ['TRAINER_MAY_%s_TREECKO' % tag]

ORDER = [
 ('Route102', None), ('Route103', None), ('Route104', None), ('PetalburgWoods', None),
 ('RustboroCity_Gym', None), ('@', rival('RUSTBORO')), ('Route116', None), ('RusturfTunnel', None),
 ('DewfordTown_Gym', None), ('Route109', None), ('Route109_SeashoreHouse', None),
 ('SlateportCity_OceanicMuseum_2F', None), ('Route110', None), ('@', rival('ROUTE_110')),
 ('@', ['TRAINER_WALLY_MAUVILLE']), ('MauvilleCity_Gym', None), ('Route110_TrickHousePuzzle1', None),
 ('Route117', None), ('@', ['TRAINER_WALLY_VERDANTURF']), ('Route111', None), ('Route112', None),
 ('Route113', None), ('Route114', None), ('Route115', None), ('MtChimney', None), ('JaggedPass', None),
 ('LavaridgeTown_Gym_1F', None), ('@', ['TRAINER_WALLY_LAVARIDGE']), ('Route110_TrickHousePuzzle2', None),
 ('PetalburgCity_Gym', None), ('Route105', None), ('Route106', None), ('Route107', None), ('Route108', None),
 ('AbandonedShip_Corridors_1F', None), ('AbandonedShip_Corridors_B1F', None), ('AbandonedShip_Rooms2_1F', None),
 ('AbandonedShip_Rooms_1F', None), ('Route110_TrickHousePuzzle3', None), ('Route118', None),
 ('Route119', None), ('@', rival('ROUTE_119')), ('Route119_WeatherInstitute_1F', None),
 ('Route119_WeatherInstitute_2F', None), ('FortreeCity_Gym', None), ('@', ['TRAINER_WALLY_FORTREE']),
 ('Route110_TrickHousePuzzle4', None), ('Route120', None), ('Route121', None),
 ('MtPyre_2F', None), ('MtPyre_3F', None), ('MtPyre_4F', None), ('MtPyre_5F', None), ('MtPyre_6F', None),
 ('MtPyre_Summit', None), ('@', rival('LILYCOVE')), ('@', ['TRAINER_WALLY_LILYCOVE']),
 ('MagmaHideout_1F', None), ('MagmaHideout_2F_1R', None), ('MagmaHideout_2F_2R', None), ('MagmaHideout_3F_1R', None),
 ('MagmaHideout_3F_2R', None), ('MagmaHideout_4F', None), ('AquaHideout_1F', None), ('AquaHideout_B1F', None),
 ('AquaHideout_B2F', None), ('Route123', None), ('Route124', None), ('SSTidalLowerDeck', None), ('SSTidalRooms', None),
 ('MossdeepCity_SpaceCenter_1F', None), ('MossdeepCity_SpaceCenter_2F', None), ('MossdeepCity_Gym', None),
 ('@', ['TRAINER_WALLY_MOSSDEEP']), ('Route110_TrickHousePuzzle6', None), ('Route125', None), ('Route126', None),
 ('Route127', None), ('Route128', None), ('SeafloorCavern_Room1', None), ('SeafloorCavern_Room3', None),
 ('SeafloorCavern_Room4', None), ('SeafloorCavern_Room9', None), ('@', ['TRAINER_WALLY_SOOTOPOLIS']),
 ('SootopolisCity_Gym_B1F', None), ('SootopolisCity_Gym_1F', None), ('Route110_TrickHousePuzzle7', None),
 ('Route110_TrickHousePuzzle8', None), ('Route129', None), ('Route130', None), ('Route131', None),
 ('Route132', None), ('Route133', None), ('Route134', None), ('MeteorFalls_1F_2R', None),
 ('VictoryRoad_1F', None), ('VictoryRoad_B1F', None), ('VictoryRoad_B2F', None),
]
SKIP = re.compile(r'TRAINER_(MAY|BRENDAN)_|TRAINER_WALLY_(VR_2|CHAMPION)|TRAINER_STEVEN$')
CHECKPOINTS = {'RustboroCity_Gym': 'Roxanne', 'DewfordTown_Gym': 'Brawly', 'MauvilleCity_Gym': 'Wattson',
               'LavaridgeTown_Gym_1F': 'Flannery', 'PetalburgCity_Gym': 'Norman', 'FortreeCity_Gym': 'Winona',
               'MossdeepCity_Gym': 'Tate&Liza', 'SootopolisCity_Gym_1F': 'Juan', 'VictoryRoad_B2F': 'Sidney'}

def encounters():
    seen = set()
    for m, ids in ORDER:
        if m == '@':
            lst = ids
        else:
            lst = [t for t in mapTrainers.get(m, []) if not SKIP.search(t)]
        for t in lst:
            if t in seen: continue
            seen.add(t)
            yield m, t

# ---------- growth rates
def exp_for(rate, n):
    if n <= 1: return 0
    if rate == 'MF': return n**3
    if rate == 'F': return 4 * n**3 // 5
    if rate == 'S': return 5 * n**3 // 4
    if rate == 'MS': return int(6/5 * n**3 - 15 * n**2 + 100 * n - 140)
def level_of(rate, e):
    n = 1
    while n < 100 and exp_for(rate, n + 1) <= e: n += 1
    return n

def scale(foe, own):
    return ((2*foe + 10) ** 2.5) / ((foe + own + 10) ** 2.5)

def simulate(mult=1.0, join_stage=(0, 1, 3, 5, 9, 12), rates=('MS', 'MF', 'MF', 'MF', 'S', 'F'), verbose=False, levelfn=None):
    team = [[rates[0], exp_for(rates[0], 5)]]
    log = []
    k = 0
    missing = set()
    for idx, (m, t) in enumerate(encounters()):
        # party grows: a new member joins at about the team's current level - 2
        while len(team) < 6 and idx >= join_stage[len(team)]:
            lv = max(3, min(level_of(r, e) for r, e in team) - 2)
            r = rates[len(team)]; team.append([r, exp_for(r, lv)])
        for sp, lv in parties.get(t, []):
            if levelfn: lv = levelfn(t, lv)
            y = yields.get(sp)
            if y is None or lv is None:
                missing.add(sp); continue
            base = int(y * lv / 5 * mult)
            pool = base + base // 2
            share = max(1, pool // len(team))
            for mon in team:
                own = level_of(mon[0], mon[1])
                mon[1] += int(share * scale(lv, own)) + 1
        if m in CHECKPOINTS and t == [x for x in mapTrainers.get(m, []) if not SKIP.search(x)][-1]:
            pass
        log.append((m, t, [level_of(r, e) for r, e in team]))
    return team, log, missing

if __name__ == '__main__':
    team, log, missing = simulate()
    # checkpoint report: levels right after each Gym leader / at Victory Road end
    leaders = {'TRAINER_ROXANNE_1': 'Roxanne', 'TRAINER_BRAWLY_1': 'Brawly', 'TRAINER_WATTSON_1': 'Wattson',
               'TRAINER_FLANNERY_1': 'Flannery', 'TRAINER_NORMAN_1': 'Norman', 'TRAINER_WINONA_1': 'Winona',
               'TRAINER_TATE_AND_LIZA_1': 'Tate&Liza', 'TRAINER_JUAN_1': 'Juan'}
    for m, t, lvls in log:
        if t in leaders:
            print(f"after {leaders[t]:10s} party {lvls} avg {sum(lvls)/len(lvls):.1f}")
    lvls = log[-1][2]
    print(f"at Sidney        party {lvls} avg {sum(lvls)/len(lvls):.1f}   (trainers beaten: {len(log)})")
    if missing: print('missing yields/levels:', sorted(missing)[:10])
    if unresolved: print('unresolved names:', sorted(unresolved)[:15])
