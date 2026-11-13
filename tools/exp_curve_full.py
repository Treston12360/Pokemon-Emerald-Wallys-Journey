#!/usr/bin/env python3
"""Wally's Journey full-Hoenn EXP curve: every reachable trainer through the first
Champion Wally fight, optional post-Hall-of-Fame one-time fights, wild battles,
with the real first-encounter scaling (src/wally_journey.c WallyJourney_ScaleImportantParty).
Answers: what level is a thorough player's team when Kanto starts?"""
import re, json, sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import exp_curve_sim as E

# scaling table: trainer -> (minAce, margin, slot, byLowest)
SC = {m.group(1): tuple(int(x) for x in m.groups()[1:]) for m in re.finditer(
    r'\{(TRAINER_\w+), (\d+), (\d+), (\d+), (\d+)\}', open('src/data/wj_trainer_scaling.h').read())}

def scaled(t, lv_list, top, locks):
    if t not in SC: return lv_list
    mn, mg, slot, low = SC[t]
    if slot not in locks: locks[slot] = min(100, max(mn, top + mg))
    ref = min(lv_list) if low else max(lv_list)
    sh = locks[slot] - ref
    return [min(100, l + sh) for l in lv_list] if sh > 0 else lv_list

# wild: average level and EXP yield of each map's grass/cave table
W = json.load(open('src/data/wild_encounters.json'))['wild_encounter_groups'][0]['encounters']
wild = {}
for e in W:
    mons = e.get('land_mons', {}).get('mons') or e.get('water_mons', {}).get('mons')
    if not mons: continue
    name = re.sub(r'^MAP_', '', e['map']).replace('_', '').lower()
    lv = sum((m['min_level'] + m['max_level']) / 2 for m in mons) / len(mons)
    ys = [E.yields.get(m['species'], 60) for m in mons]
    wild[name] = (lv, sum(ys) / len(ys))

POST = ['TRAINER_ROXANNE_3', 'TRAINER_BRAWLY_3', 'TRAINER_WATTSON_3', 'TRAINER_FLANNERY_3',
        'TRAINER_NORMAN_3', 'TRAINER_WINONA_3', 'TRAINER_TATE_AND_LIZA_3', 'TRAINER_JUAN_3',
        'TRAINER_MAY_FINAL_MUDKIP', 'TRAINER_STEVEN']
LEAGUE = ['TRAINER_SIDNEY', 'TRAINER_PHOEBE', 'TRAINER_GLACIA', 'TRAINER_DRAKE', 'TRAINER_WALLY_CHAMPION']

def run(wild_per_map=3, post=True, rates=('MS', 'MF', 'MF', 'MF', 'S', 'F'), join=(0, 1, 3, 5, 9, 12)):
    team = [[rates[0], E.exp_for(rates[0], 5)]]
    locks = {}; log = []
    def lv(): return [E.level_of(r, x) for r, x in team]
    def gain(foe, y):
        base = int(y * foe / 5); pool = base + base // 2; share = max(1, pool // len(team))
        for mon in team:
            mon[1] += int(share * E.scale(foe, E.level_of(mon[0], mon[1]))) + 1
    def fight(t):
        p = E.parties.get(t, [])
        lvls = scaled(t, [l for _, l in p], max(lv()), locks)
        for (sp, _), l in zip(p, lvls): gain(l, E.yields.get(sp, 100))
        return lvls
    seq = list(E.encounters()) + [('League', t) for t in LEAGUE] + ([('Postgame', t) for t in POST] if post else [])
    lastmap = None
    for idx, (m, t) in enumerate(seq):
        while len(team) < 6 and idx >= join[len(team)]:
            l = max(3, min(lv()) - 2); r = rates[len(team)]; team.append([r, E.exp_for(r, l)])
        if m != lastmap and m.lower() in wild:
            wl, wy = wild[m.lower()]
            for _ in range(wild_per_map): gain(round(wl), wy)
        lastmap = m
        foe = fight(t)
        log.append((m, t, lv(), foe))
    return log

def report(log, marks):
    for m, t, me, foe in log:
        if t in marks:
            print(f"  {marks[t]:22s} you avg {sum(me)/6:5.1f} top {max(me):3d} | foe {min(foe)}-{max(foe)}")

if __name__ == '__main__':
    marks = {'TRAINER_JUAN_1': 'after Juan', 'TRAINER_SIDNEY': 'Sidney', 'TRAINER_DRAKE': 'Drake',
             'TRAINER_WALLY_CHAMPION': 'Champion Wally', 'TRAINER_JUAN_3': 'Leader rematches done',
             'TRAINER_STEVEN': 'Steven (Kanto start)'}
    for w in (0, 3, 6):
        print(f"wild battles per area: {w}")
        report(run(w), marks)
