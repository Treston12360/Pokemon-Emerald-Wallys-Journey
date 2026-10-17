#!/usr/bin/env python3
"""Second pass: each remaining family gets several fitting habitats; the first map
with a safe slot (a slot whose species appears elsewhere) is used. See pass 1."""
import json, sys
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import audit_wally_obtainability as A
import wally_add_missing_families as P1

FOREST = ["MAP_PETALBURG_WOODS", "MAP_ROUTE119", "MAP_ROUTE120", "MAP_ROUTE123", "MAP_SAFARI_ZONE_NORTHWEST", "MAP_SAFARI_ZONE_SOUTHWEST", "MAP_ROUTE116", "MAP_ROUTE104"]
MEADOW = ["MAP_ROUTE117", "MAP_ROUTE121", "MAP_ROUTE104", "MAP_ROUTE110", "MAP_ROUTE123", "MAP_ROUTE102", "MAP_ROUTE103", "MAP_SAFARI_ZONE_SOUTH", "MAP_SAFARI_ZONE_SOUTHEAST"]
MOUNTAIN = ["MAP_ROUTE115", "MAP_ROUTE114", "MAP_ROUTE112", "MAP_ROUTE113", "MAP_JAGGED_PASS", "MAP_VICTORY_ROAD_1F", "MAP_VICTORY_ROAD_B1F"]
CAVE = ["MAP_GRANITE_CAVE_1F", "MAP_GRANITE_CAVE_B1F", "MAP_GRANITE_CAVE_B2F", "MAP_RUSTURF_TUNNEL", "MAP_VICTORY_ROAD_1F", "MAP_VICTORY_ROAD_B1F", "MAP_VICTORY_ROAD_B2F", "MAP_METEOR_FALLS_1F_1R", "MAP_ARTISAN_CAVE_1F", "MAP_ARTISAN_CAVE_B1F"]
HAUNT = ["MAP_MT_PYRE_1F", "MAP_MT_PYRE_2F", "MAP_MT_PYRE_3F", "MAP_MT_PYRE_4F", "MAP_MT_PYRE_5F", "MAP_MT_PYRE_6F", "MAP_MT_PYRE_EXTERIOR", "MAP_MT_PYRE_SUMMIT"]
DRY = ["MAP_ROUTE111", "MAP_ROUTE113", "MAP_ROUTE114", "MAP_DESERT_UNDERPASS", "MAP_MIRAGE_TOWER_1F"]
PLAN = [
    ("SPECIES_GASTLY", HAUNT, "uncommon", "ghosts of Mt. Pyre"),
    ("SPECIES_ONIX", CAVE, "uncommon", "rock tunnels"),
    ("SPECIES_SCYTHER", ["MAP_SAFARI_ZONE_NORTHEAST", "MAP_SAFARI_ZONE_NORTH", "MAP_SAFARI_ZONE_NORTHWEST", "MAP_ROUTE119", "MAP_ROUTE120"], "rare", "tall grass"),
    ("SPECIES_BURMY_PLANT", FOREST, "rare", "hangs from trees"),
    ("SPECIES_PANSAGE", FOREST, "rare", "forest monkey"),
    ("SPECIES_PANPOUR", ["MAP_ROUTE120", "MAP_ROUTE119", "MAP_ROUTE123", "MAP_ROUTE117", "MAP_SAFARI_ZONE_SOUTHWEST"], "rare", "riverside monkey"),
    ("SPECIES_MUNNA", MOUNTAIN + MEADOW, "rare", "dreams at night"),
    ("SPECIES_THROH", MOUNTAIN, "rare", "martial mountain"),
    ("SPECIES_SAWK", MOUNTAIN, "rare", "martial mountain"),
    ("SPECIES_DEERLING_SPRING", MEADOW + FOREST, "rare", "meadow deer"),
    ("SPECIES_FOONGUS", FOREST, "rare", "forest floor"),
    ("SPECIES_SPRITZEE", MEADOW, "rare", "perfume flowers"),
    ("SPECIES_PUMPKABOO_AVERAGE", HAUNT + FOREST, "rare", "night pumpkin"),
    ("SPECIES_ORICORIO_BAILE", MEADOW, "rare", "nectar flowers"),
    ("SPECIES_MORELULL", FOREST, "rare", "glowing mushrooms"),
    ("SPECIES_COMFEY", MEADOW, "elusive", "flower crowns"),
    ("SPECIES_MILCERY", MEADOW, "elusive", "sweet scents"),
    ("SPECIES_MORPEKO_FULL_BELLY", MEADOW, "rare", "follows seeds"),
    ("SPECIES_POLTCHAGEIST_COUNTERFEIT", FOREST + MEADOW, "rare", "tea leaves"),
    ("SPECIES_MEOWTH_GALAR", DRY + MOUNTAIN, "elusive", "Perrserker line"),
    ("SPECIES_FARFETCHD_GALAR", MEADOW, "elusive", "Sirfetch'd line"),
    ("SPECIES_WOOPER_PALDEA", ["MAP_ROUTE114", "MAP_ROUTE113", "MAP_ROUTE111", "MAP_ROUTE120", "MAP_SAFARI_ZONE_SOUTHWEST"], "rare", "Clodsire line; bog"),
    ("SPECIES_FLABEBE_RED", MEADOW, "rare", "flower fairy"),
]

def main():
    data = json.loads((ROOT / "src/data/wild_encounters.json").read_text(encoding="utf-8"))
    group = next(g for g in data["wild_encounter_groups"] if g["label"] == "gWildMonHeaders")
    tables = {e["map"]: e for e in group["encounters"] if e["map"] in A.emerald_maps()}
    info = A.parse_species(); pre = {}
    for sp, d in info.items():
        for _, _, t in d["evos"]: pre.setdefault(t, sp)
    def fam(sp):
        while sp in pre: sp = pre[sp]
        return sp
    from collections import Counter
    def counts():
        c = Counter()
        for e in tables.values():
            for k, v in e.items():
                if k.endswith("_mons"):
                    for m in v["mons"]: c[m["species"]] += 1
        return c
    order = {"common": ["common", "uncommon"], "uncommon": ["uncommon", "rare", "common"],
             "rare": ["rare", "uncommon", "elusive"], "elusive": ["elusive", "rare"]}
    for sp, maps, tier, why in PLAN:
        assert sp in info, sp
        done = False
        for mp in maps:
            e = tables.get(mp)
            if not e or "land_mons" not in e: continue
            mons = e["land_mons"]["mons"]; c = counts()
            for tr in order[tier]:
                cands = [i for i in P1.TIER_SLOTS["land_mons"][tr] if c[mons[i]["species"]] > 1 and fam(mons[i]["species"]) != fam(sp)]
                if cands:
                    i = max(cands, key=lambda k: c[mons[k]["species"]])
                    print(f"{sp} -> {mp} land[{i}] ({tr}) replacing {mons[i]['species']}: {why}")
                    mons[i]["species"] = sp; done = True; break
            if done: break
        if not done: print("STILL SKIPPED", sp)
    (ROOT / "src/data/wild_encounters.json").write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

if __name__ == "__main__":
    main()
