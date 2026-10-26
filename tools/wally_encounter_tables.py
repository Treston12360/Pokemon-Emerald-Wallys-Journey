"""Shared reader for src/data/wild_encounters.json tables (Wally's Journey).

A table is either classic (fixed slots; the field's "encounter_rates" give each slot's %)
or weighted ("weighted": true; each mon has its own "weight" in 1/10000 and, for fishing,
a "rod" of old/good/super). table_rates() returns {group: [(species, percent, mon), ...]}
for either kind, with fishing grouped per rod like the classic "groups" names.
"""

ROD_GROUPS = {"old": "old_rod", "good": "good_rod", "super": "super_rod"}


def is_weighted(table):
    return isinstance(table, dict) and bool(table.get("weighted"))


def table_rates(table, field):
    mons = table["mons"]
    if is_weighted(table):
        groups = {}
        for mon in mons:
            g = ROD_GROUPS.get(mon.get("rod"), "all")
            groups.setdefault(g, []).append(mon)
        out = {}
        for g, members in groups.items():
            total = sum(int(m["weight"]) for m in members) or 1
            out[g] = [(m["species"], int(m["weight"]) * 100.0 / total, m) for m in members]
        return out
    weights = field["encounter_rates"]
    groups = field.get("groups") or {"all": list(range(len(weights)))}
    return {g: [(mons[i]["species"], weights[i], mons[i]) for i in idx] for g, idx in groups.items()}


def species_rates(table, field):
    """{group: {species: total percent}}"""
    out = {}
    for g, rows in table_rates(table, field).items():
        acc = {}
        for sp, pct, _ in rows:
            acc[sp] = acc.get(sp, 0) + pct
        out[g] = acc
    return out
