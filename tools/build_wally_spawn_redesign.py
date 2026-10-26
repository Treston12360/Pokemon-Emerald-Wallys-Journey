#!/usr/bin/env python3
"""Build src/data/wild_encounters.json + wally_rare_visitors.h from redesign.py.
--apply writes; otherwise prints a review."""
import json, os, re, sys, collections
S = os.path.dirname(os.path.abspath(__file__))  # tools/
R = os.path.dirname(S)
sys.path.insert(0, R + '/tools')
import audit_wally_obtainability as A

info = A.parse_species()
need = {}
for sp, d in info.items():
    for m, req, tgt in d['evos']:
        if m == 'EVO_LEVEL' and req.isdigit():
            lv = int(req)
            if tgt not in need or lv < need[tgt][1]:
                need[tgt] = (sp, lv)

ns = {}
exec(open(os.path.join(S, 'wally_spawn_redesign.py')).read(), ns)
D = ns['D']

RATES = {'land_mons': [20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1],
         'water_mons': [35, 30, 20, 10, 5],
         'rock_smash_mons': [40, 30, 15, 10, 5]}
FISH = {'old': [70, 30], 'good': [60, 20, 20], 'super': [40, 40, 15, 4, 1]}

P = R + '/src/data/wild_encounters.json'
data = json.load(open(P))
G = [g for g in data['wild_encounter_groups'] if g['label'] == 'gWildMonHeaders'][0]
for f in G['fields']:
    if f['type'] in RATES:
        f['encounter_rates'] = RATES[f['type']]
ROD = [f for f in G['fields'] if f['type'] == 'fishing_mons'][0]['groups']
assert [len(ROD[k]) for k in ('old_rod', 'good_rod', 'super_rod')] == [2, 3, 5], ROD

defined = set(re.findall(r'^\s*(SPECIES_[A-Z0-9_]+)\s*=', open(R + '/include/constants/species.h').read(), re.M))
warn, review = [], []

def assign(weights, rates):
    w = collections.OrderedDict()
    for k, v in weights.items():
        k = k.strip(); w[k] = w.get(k, 0) + v
    tot = sum(w.values())
    tgt = {k: v * 100.0 / tot for k, v in w.items()}
    order = sorted(tgt, key=lambda k: -tgt[k])
    if len(order) > len(rates):
        raise SystemExit(f'too many species {order}')
    got = collections.Counter(); out = [None] * len(rates); free = list(range(len(rates)))
    for sp in sorted(order, key=lambda k: tgt[k]):          # smallest first: rare species get the small slots
        i = min(free, key=lambda j: (abs(rates[j] - tgt[sp]), rates[j]))
        free.remove(i); out[i] = sp; got[sp] += rates[i]
    for i in sorted(free, key=lambda j: -rates[j]):          # leftovers go to whoever is furthest under target
        sp = max(order, key=lambda k: tgt[k] - got[k])
        out[i] = sp; got[sp] += rates[i]
    return out, got

def legal(sp, lo, hi, where):
    full = 'SPECIES_' + sp
    if full not in defined:
        raise SystemExit(f'unknown species {sp} at {where}')
    while full in need and need[full][1] > hi:
        warn.append(f'{where}: {sp} evolves at {need[full][1]} > max {hi}; using pre-evolution')
        full = need[full][0]
    mn = max(lo, need[full][1]) if full in need else lo
    return full, mn

def levels(rate, lo, hi, kind):
    if kind == 'land':
        if rate >= 10: return lo, hi
        if rate >= 4: return (lo + hi + 1) // 2, hi
        return hi, hi
    if rate >= 30: return lo, hi
    if rate >= 15: return min(hi, lo + 2), hi
    return (lo + hi + 1) // 2, hi

def build(spec, rates, kind, where):
    lo, hi, weights = spec
    sps, got = assign(weights, rates)
    mons = []
    for sp, r in zip(sps, rates):
        a, b = levels(r, lo, hi, kind)
        full, mn = legal(sp, a, b, where)
        mons.append({'min_level': mn, 'max_level': max(mn, b), 'species': full})
    agg = collections.Counter()
    for m, r in zip(mons, rates): agg[m['species'][8:]] += r
    review.append(f'  {where}: ' + ', '.join(f'{k} {v}%' for k, v in agg.most_common()))
    return mons

def build_weighted(spec, kind, where, rod=None):
    """Weighted table (no slot limit): each species once, weight = its share in 1/10000."""
    lo, hi, weights = spec
    w = collections.OrderedDict()
    for k, v in weights.items():
        k = k.strip(); w[k] = w.get(k, 0) + v
    # Design weights are percentages. If a table's weights don't add up to 100, the rare ones
    # (under 10%) keep their exact percentage and the common ones share the difference, the
    # same way the old slot builder gave leftover slots to the most common species.
    tot = sum(w.values())
    big = sum(v for v in w.values() if v >= 10) or tot
    scaled = collections.OrderedDict()
    for sp, v in w.items():
        if v >= 10 or big == tot:
            scaled[sp] = v + (100.0 - tot) * v / big
        else:
            scaled[sp] = v
    mons = []
    for sp, v in scaled.items():
        pct = v
        a, b = levels(pct, lo, hi, kind)
        full, mn = legal(sp, a, b, where)
        m = {'min_level': mn, 'max_level': max(mn, b), 'species': full, 'weight': max(1, int(round(v * 100)))}
        if rod:
            m['rod'] = rod
        mons.append(m)
    fix = 10000 - sum(m['weight'] for m in mons)        # rounding: keep each table/rod at exactly 10000
    max(mons, key=lambda m: m['weight'])['weight'] += fix
    review.append(f'  {where}: ' + ', '.join(f"{m['species'][8:]} {m['weight']/100:g}%" for m in mons))
    return mons

def legacy_rod_entries(fm, rod):
    """Entries for one rod from an existing table (classic slots or already weighted)."""
    if fm.get('weighted'):
        return [dict(m) for m in fm['mons'] if m.get('rod') == rod]
    out = collections.OrderedDict()
    for slot, r in zip(ROD[rod + '_rod'], FISH[rod]):
        m = fm['mons'][slot]
        key = (m['species'], m['min_level'], m['max_level'])
        out[key] = out.get(key, 0) + r
    return [{'min_level': k[1], 'max_level': k[2], 'species': k[0], 'weight': v * 100, 'rod': rod} for k, v in out.items()]

byMap = {e['map']: e for e in G['encounters'] if 'map' in e}
for name, spec in D.items():
    mp = 'MAP_' + name
    e = byMap.get(mp)
    if e is None:
        raise SystemExit('no encounter entry ' + mp)
    review.append(name)
    for key, field in (('land', 'land_mons'), ('water', 'water_mons'), ('rock', 'rock_smash_mons')):
        if key in spec:
            e.setdefault(field, {'encounter_rate': {'land_mons': 20, 'water_mons': 4, 'rock_smash_mons': 20}[field]})
            e[field]['mons'] = build_weighted(spec[key], 'land' if key == 'land' else 'other', f'{name} {key}')
            e[field]['weighted'] = True
    if any(k in spec for k in ('old', 'good', 'super')):
        fm = e.get('fishing_mons')
        if fm is None:
            raise SystemExit('no fishing table ' + mp)
        mons = []
        for rod in ('old', 'good', 'super'):
            if rod in spec:
                mons += build_weighted(spec[rod], 'other', f'{name} {rod}', rod)
            else:
                mons += legacy_rod_entries(fm, rod)
        fm['mons'] = mons
        fm['weighted'] = True

# Hoenn water / rock tables that the new global rates touch but the design does not cover
emerald = A.emerald_maps()
for e in G['encounters']:
    mp = e.get('map')
    if mp in emerald and mp[4:] not in D:
        for f in ('water_mons', 'rock_smash_mons', 'fishing_mons'):
            if f in e:
                warn.append(f'UNDESIGNED {mp} {f}')

# ---------------- visitors
H = R + '/src/data/wally_rare_visitors.h'
h = open(H).read()
head = h[:h.index('static const struct WallyRareVisitor sWallyRareVisitors[] =')]
old_rows = re.findall(r'^\s*VISITOR\((MAP_\w+),\s*(\w+),\s*(SPECIES_\w+),\s*(\d+)\),?\s*(//.*)?$', h, re.M)
AREA = {'v_land': 'WILD_AREA_LAND', 'v_water': 'WILD_AREA_WATER', 'v_rock': 'WILD_AREA_ROCKS',
        'v_old': 'WJ_VISITOR_OLD_ROD', 'v_good': 'WJ_VISITOR_GOOD_ROD', 'v_super': 'WJ_VISITOR_SUPER_ROD'}
rows = []
for name, spec in D.items():
    for k, areaName in AREA.items():
        for sp, pct in spec.get(k, {}).items():
            if 'SPECIES_' + sp not in defined:
                raise SystemExit('unknown visitor ' + sp)
            rows.append(f'    VISITOR(MAP_{name}, {areaName}, SPECIES_{sp}, {int(round(pct * 100))}), // {pct:g}%')
kept = [r for r in old_rows if r[0][4:] not in D]
for mp, ar, sp, ch, cm in kept:
    rows.append(f'    VISITOR({mp}, {ar}, {sp}, {ch}), {cm}'.rstrip())
body = 'static const struct WallyRareVisitor sWallyRareVisitors[] =\n{\n' + '\n'.join(rows) + '\n};\n'

print('\n'.join(review))
print('\n'.join(warn))
print('visitor rows', len(rows), 'kept-from-old', len(kept))
if '--apply' in sys.argv:
    json.dump(data, open(P, 'w'), indent=2, ensure_ascii=False); open(P, 'a').write('\n')
    if 'WJ_VISITOR_OLD_ROD' not in head:
        head = head.replace('#define VISITOR(', '// Fishing visitors use these area ids (rod-specific).\n'
                            '#define WJ_VISITOR_OLD_ROD   (WILD_AREA_HIDDEN + 1 + OLD_ROD)\n'
                            '#define WJ_VISITOR_GOOD_ROD  (WILD_AREA_HIDDEN + 1 + GOOD_ROD)\n'
                            '#define WJ_VISITOR_SUPER_ROD (WILD_AREA_HIDDEN + 1 + SUPER_ROD)\n\n#define VISITOR(', 1)
    open(H, 'w').write(head + body)
    print('written')
