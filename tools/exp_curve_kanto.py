#!/usr/bin/env python3
"""Kanto EXP-rate finder. Kanto is modelled on FRLG's real trainer list (every
trainerbattle in the mainland *_Frlg map scripts, parties from trainers_frlg.party),
fought in FRLG level order, with each trainer's levels mapped linearly from FRLG's
range onto the Wally's Journey Kanto band. The Hoenn team comes from exp_curve_full.py.
Finds the Kanto EXP multiplier that ends Kanto at the target team level."""
import re, glob, sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import exp_curve_sim as E, exp_curve_full as F

parties = {}; cur = None; prev = ''
for line in open('src/data/trainers_frlg.party'):
    line = line.rstrip('\n'); m = re.match(r'=== (TRAINER_\w+) ===', line)
    if m: cur = m.group(1); parties[cur] = []; prev = ''; continue
    lv = re.match(r'Level:\s*(\d+)', line)
    if lv and cur and prev and not E.HEAD.match(prev):
        sp = E.resolve(prev)
        parties[cur].append((sp, int(lv.group(1))))
    if line.strip(): prev = line.strip()
SEVII = re.compile(r'(One|Two|Three|Four|Five|Six|Seven)Island|Navel|Birth|Tanoby|Altering|Lost|Icefall|Berry|Bond|Kindle|Cape|Pattern|Water(Path|Labyrinth)|Ruin|Resort|Trainer(Tower)|Sevault|Dotted|Outcast|Green|Memorial|Canyon|Emberspa')
ids = set()
for p in glob.glob('data/maps/*_Frlg/scripts.inc'):
    if SEVII.search(p): continue
    ids |= set(re.findall(r'trainerbattle\w*\s+(TRAINER_\w+)', open(p).read()))
kanto = [t for t in ids if parties.get(t) and not re.search(r'_[2-9]$|RIVAL_OAKS_LAB|RIVAL_ROUTE22_EARLY', t)]
kanto.sort(key=lambda t: (max(l for _, l in parties[t]), t))
LO = min(max(l for _, l in parties[t]) for t in kanto); HI = max(max(l for _, l in parties[t]) for t in kanto)

def kanto_run(mult, start_team, band=(62, 82), wild_per=2):
    team = [m[:] for m in start_team]; a, b = band; out = []
    def lv(): return [E.level_of(r, x) for r, x in team]
    for i, t in enumerate(kanto):
        p = parties[t]; ace = max(l for _, l in p)
        tgt = a + (ace - LO) * (b - a) / (HI - LO); sh = tgt - ace
        for sp, l in p:
            foe = max(2, min(100, round(l + sh)))
            y = E.yields.get(sp, 100) if sp else 100
            base = int(y * foe / 5 * mult); pool = base + base // 2; share = max(1, pool // 6)
            for mon in team: mon[1] += int(share * E.scale(foe, E.level_of(mon[0], mon[1]))) + 1
        if i % 6 == 0:   # wild battles along the way, about 4 under the local trainers
            for _ in range(wild_per):
                foe = round(tgt) - 4; base = int(140 * foe / 5 * mult); share = (base + base // 2) // 6
                for mon in team: mon[1] += int(share * E.scale(foe, E.level_of(mon[0], mon[1]))) + 1
        out.append((t, lv()))
    return out

if __name__ == '__main__':
    print(f"Kanto trainers modelled: {len(kanto)} (FRLG ace levels {LO}-{HI})")
    for post in (False, True):
        log = F.run(3, post=post)
        team0 = None
        # rebuild the team state at the end of Hoenn
        import copy
        rates = ('MS', 'MF', 'MF', 'MF', 'S', 'F')
        start = [[r, E.exp_for(r, l)] for r, l in zip(rates, log[-1][2])]
        print(f"\nHoenn {'+ post-game one-time fights' if post else 'through Champion Wally'}: arrive avg {sum(log[-1][2])/6:.1f}")
        for mult in (1.0, 0.75, 0.6, 0.5, 0.4, 0.33, 0.25):
            r = kanto_run(mult, start)
            mid = r[len(r)//2][1]; end = r[-1][1]
            print(f"  Kanto EXP x{mult:<4}  mid-Kanto avg {sum(mid)/6:5.1f}   end avg {sum(end)/6:5.1f} top {max(end)}")
