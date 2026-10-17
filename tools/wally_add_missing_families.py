#!/usr/bin/env python3
"""Place every Gen 1-9 family that had no obtainable form into fitting Hoenn habitats.

Each entry: (species, map, method, tier, reason). Tier picks which slot rates are used:
  common   -> 10% land / 30% surf / 20% Good Rod / 40% Super Rod slots
  uncommon -> 5% land / 5% surf / Old Rod 30% / Super Rod 15% slots
  rare     -> 4% land / 4% surf / Super Rod 4% slots
  elusive  -> 1% land / 1% surf / 1% Super Rod slot
The slot replaced is the one in that tier whose current species has the most
other appearances on Emerald maps, so no species loses its last wild spot.
Re-run tools/audit_wally_obtainability.py afterwards.
"""
import json
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import audit_wally_obtainability as A  # noqa: E402

TIER_SLOTS = {
    "land_mons": {"common": [2, 3, 4, 5], "uncommon": [6, 7], "rare": [8, 9], "elusive": [10, 11]},
    "water_mons": {"common": [1], "uncommon": [2], "rare": [3], "elusive": [4]},
    # fishing: 0-1 Old Rod, 2-4 Good Rod, 5-9 Super Rod
    "fishing_mons": {"common": [3, 4, 5, 6], "uncommon": [1, 7], "rare": [8], "elusive": [9]},
}

PLAN = [
    # --- Hoenn natives that had disappeared (canonical RSE spots) ---
    ("SPECIES_SURSKIT", "MAP_ROUTE102", "land_mons", "rare", "Emerald swarm route; pond edges"),
    ("SPECIES_SURSKIT", "MAP_ROUTE120", "land_mons", "uncommon", "rainy ponds"),
    ("SPECIES_ZANGOOSE", "MAP_ROUTE114", "land_mons", "uncommon", "Ruby/Emerald Route 114"),
    ("SPECIES_LUNATONE", "MAP_METEOR_FALLS_1F_1R", "land_mons", "rare", "Sapphire Meteor Falls"),
    ("SPECIES_ROSELIA", "MAP_ROUTE117", "land_mons", "uncommon", "RSE Route 117 flowers"),
    ("SPECIES_MEDITITE", "MAP_MT_PYRE_EXTERIOR", "land_mons", "uncommon", "RSE Mt. Pyre"),
    ("SPECIES_MEDITITE", "MAP_VICTORY_ROAD_1F", "land_mons", "rare", "RSE Victory Road"),
    # --- Kanto / Johto grassland and forest families ---
    ("SPECIES_PIDGEY", "MAP_ROUTE102", "land_mons", "common", "open grassland bird"),
    ("SPECIES_SPEAROW", "MAP_ROUTE113", "land_mons", "uncommon", "dry hills, ash fields"),
    ("SPECIES_RATTATA", "MAP_ROUTE103", "land_mons", "common", "ubiquitous near towns"),
    ("SPECIES_SENTRET", "MAP_ROUTE116", "land_mons", "uncommon", "meadow scout"),
    ("SPECIES_CATERPIE", "MAP_PETALBURG_WOODS", "land_mons", "common", "forest bug"),
    ("SPECIES_WEEDLE", "MAP_PETALBURG_WOODS", "land_mons", "common", "forest bug"),
    ("SPECIES_PARAS", "MAP_PETALBURG_WOODS", "land_mons", "rare", "mushrooms on forest floor"),
    ("SPECIES_VENONAT", "MAP_ROUTE119", "land_mons", "uncommon", "drawn to light in damp forest"),
    ("SPECIES_BELLSPROUT", "MAP_ROUTE119", "land_mons", "uncommon", "humid tall grass"),
    ("SPECIES_EKANS", "MAP_ROUTE116", "land_mons", "uncommon", "grassland snake"),
    ("SPECIES_NIDORAN_F", "MAP_SAFARI_ZONE_SOUTH", "land_mons", "uncommon", "Safari grassland"),
    ("SPECIES_NIDORAN_M", "MAP_SAFARI_ZONE_SOUTH", "land_mons", "uncommon", "Safari grassland"),
    ("SPECIES_MEOWTH", "MAP_ROUTE110", "land_mons", "uncommon", "near Slateport and Mauville"),
    ("SPECIES_MANKEY", "MAP_ROUTE115", "land_mons", "uncommon", "mountain forest"),
    ("SPECIES_GROWLITHE", "MAP_FIERY_PATH", "land_mons", "rare", "warm volcanic ground"),
    ("SPECIES_PONYTA", "MAP_ROUTE112", "land_mons", "uncommon", "open hills below Mt. Chimney"),
    ("SPECIES_DIGLETT", "MAP_RUSTURF_TUNNEL", "land_mons", "uncommon", "tunnels through soil"),
    ("SPECIES_DROWZEE", "MAP_ROUTE114", "land_mons", "rare", "hills near Fallarbor"),
    ("SPECIES_CUBONE", "MAP_MT_PYRE_EXTERIOR", "land_mons", "rare", "graveyard mountain"),
    ("SPECIES_EXEGGCUTE", "MAP_ROUTE123", "land_mons", "uncommon", "warm berry groves"),
    ("SPECIES_LICKITUNG", "MAP_SAFARI_ZONE_NORTHWEST", "land_mons", "rare", "Safari"),
    ("SPECIES_TANGELA", "MAP_ROUTE120", "land_mons", "rare", "jungle vines"),
    ("SPECIES_KANGASKHAN", "MAP_SAFARI_ZONE_SOUTHEAST", "land_mons", "rare", "Safari, protected"),
    ("SPECIES_TAUROS", "MAP_SAFARI_ZONE_SOUTH", "land_mons", "rare", "Safari herds"),
    ("SPECIES_SCYTHER", "MAP_SAFARI_ZONE_NORTHEAST", "land_mons", "rare", "Safari tall grass"),
    ("SPECIES_EEVEE", "MAP_ROUTE121", "land_mons", "elusive", "historically scarce"),
    ("SPECIES_PORYGON", "MAP_NEW_MAUVILLE_INSIDE", "land_mons", "elusive", "abandoned power plant data"),
    ("SPECIES_CLEFFA", "MAP_METEOR_FALLS_1F_2R", "land_mons", "elusive", "meteor lore"),
    ("SPECIES_DUNSPARCE", "MAP_GRANITE_CAVE_B1F", "land_mons", "rare", "dark burrows"),
    ("SPECIES_MURKROW", "MAP_MT_PYRE_EXTERIOR", "land_mons", "uncommon", "omens near graves"),
    ("SPECIES_MISDREAVUS", "MAP_MT_PYRE_5F", "land_mons", "rare", "haunts Mt. Pyre"),
    ("SPECIES_HOPPIP", "MAP_ROUTE119", "land_mons", "uncommon", "drifts on Route 119 winds"),
    ("SPECIES_YANMA", "MAP_ROUTE120", "land_mons", "uncommon", "marshes"),
    ("SPECIES_SNEASEL", "MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM", "land_mons", "uncommon", "ice cave"),
    ("SPECIES_DELIBIRD", "MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM", "land_mons", "rare", "ice cave"),
    ("SPECIES_TYROGUE", "MAP_VICTORY_ROAD_B1F", "land_mons", "rare", "trains among Victory Road fighters"),
    ("SPECIES_MUNCHLAX", "MAP_ROUTE123", "land_mons", "elusive", "berry groves; historically rare"),
    # --- Sinnoh ---
    ("SPECIES_BUDEW", "MAP_ROUTE117", "land_mons", "rare", "flower beds"),
    ("SPECIES_BURMY_PLANT", "MAP_ROUTE119", "land_mons", "rare", "hangs from trees"),
    ("SPECIES_CROAGUNK", "MAP_SAFARI_ZONE_SOUTHWEST", "land_mons", "uncommon", "marsh"),
    ("SPECIES_CARNIVINE", "MAP_ROUTE120", "land_mons", "elusive", "swamp trees; rare in DPPt"),
    # --- Unova ---
    ("SPECIES_PANSAGE", "MAP_PETALBURG_WOODS", "land_mons", "rare", "forest monkeys"),
    ("SPECIES_PANSEAR", "MAP_JAGGED_PASS", "land_mons", "rare", "volcano slopes"),
    ("SPECIES_PANPOUR", "MAP_ROUTE120", "land_mons", "rare", "riverside"),
    ("SPECIES_MUNNA", "MAP_ROUTE114", "land_mons", "rare", "night meadow near Fallarbor"),
    ("SPECIES_THROH", "MAP_ROUTE115", "land_mons", "rare", "mountain trainers' rivals"),
    ("SPECIES_SAWK", "MAP_ROUTE115", "land_mons", "rare", "mountain trainers' rivals"),
    ("SPECIES_COTTONEE", "MAP_ROUTE121", "land_mons", "uncommon", "wind-blown fluff"),
    ("SPECIES_DEERLING_SPRING", "MAP_ROUTE117", "land_mons", "rare", "meadow deer (form follows season)"),
    ("SPECIES_FOONGUS", "MAP_ROUTE120", "land_mons", "rare", "forest floor"),
    ("SPECIES_BASCULIN_RED_STRIPED", "MAP_ROUTE119", "fishing_mons", "common", "river fish"),
    ("SPECIES_BASCULIN_WHITE_STRIPED", "MAP_ROUTE119", "fishing_mons", "elusive", "rare upstream variant"),
    # --- Kalos ---
    ("SPECIES_HELIOPTILE", "MAP_ROUTE111", "land_mons", "uncommon", "desert sun"),
    ("SPECIES_SPRITZEE", "MAP_ROUTE117", "land_mons", "rare", "perfume flowers"),
    ("SPECIES_SWIRLIX", "MAP_ROUTE121", "land_mons", "rare", "near Lilycove sweets"),
    ("SPECIES_KLEFKI", "MAP_NEW_MAUVILLE_INSIDE", "land_mons", "rare", "collects keys"),
    ("SPECIES_PUMPKABOO_AVERAGE", "MAP_MT_PYRE_EXTERIOR", "land_mons", "rare", "graveyard pumpkin"),
    # --- Alola ---
    ("SPECIES_ORICORIO_BAILE", "MAP_ROUTE121", "land_mons", "rare", "flowers; style via nectar"),
    ("SPECIES_MORELULL", "MAP_PETALBURG_WOODS", "land_mons", "rare", "glowing mushrooms"),
    ("SPECIES_MIMIKYU_DISGUISED", "MAP_MT_PYRE_4F", "land_mons", "elusive", "lurks in dark places"),
    ("SPECIES_COMFEY", "MAP_ROUTE117", "land_mons", "elusive", "flower crowns"),
    ("SPECIES_WISHIWASHI_SOLO", "MAP_ROUTE104", "fishing_mons", "uncommon", "coastal school fish"),
    # --- Galar ---
    ("SPECIES_GOSSIFLEUR", "MAP_ROUTE104", "land_mons", "uncommon", "flower meadow"),
    ("SPECIES_MILCERY", "MAP_ROUTE121", "land_mons", "elusive", "sweet scents"),
    ("SPECIES_EISCUE_ICE", "MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM", "land_mons", "elusive", "cold seas"),
    ("SPECIES_INDEEDEE_M", "MAP_ROUTE123", "land_mons", "rare", "attends calm people"),
    ("SPECIES_MORPEKO_FULL_BELLY", "MAP_ROUTE110", "land_mons", "rare", "follows seeds"),
    ("SPECIES_SINISTEA_PHONY", "MAP_MT_PYRE_2F", "land_mons", "rare", "haunted teacups"),
    # --- Paldea ---
    ("SPECIES_SQUAWKABILLY_GREEN", "MAP_ROUTE103", "land_mons", "rare", "flocks near towns"),
    ("SPECIES_TATSUGIRI_CURLY", "MAP_ROUTE128", "fishing_mons", "elusive", "deep sea, rare"),
    ("SPECIES_GIMMIGHOUL_ROAMING", "MAP_MIRAGE_TOWER_2F", "land_mons", "rare", "treasure ruins"),
    ("SPECIES_POLTCHAGEIST_COUNTERFEIT", "MAP_ROUTE123", "land_mons", "rare", "tea leaves"),
    # --- Water ---
    ("SPECIES_POLIWAG", "MAP_ROUTE117", "fishing_mons", "common", "freshwater ponds"),
    ("SPECIES_POLIWAG", "MAP_ROUTE120", "fishing_mons", "common", "freshwater"),
    ("SPECIES_SEEL", "MAP_SHOAL_CAVE_LOW_TIDE_INNER_ROOM", "water_mons", "uncommon", "cold shoals"),
    ("SPECIES_KRABBY", "MAP_ROUTE106", "fishing_mons", "common", "rocky coast"),
    ("SPECIES_LAPRAS", "MAP_ROUTE130", "water_mons", "elusive", "rare open sea"),
    ("SPECIES_SHELLOS_WEST", "MAP_ROUTE109", "water_mons", "uncommon", "warm western beaches"),
    # --- Regional forms needed for their evolutions ---
    ("SPECIES_ZIGZAGOON_GALAR", "MAP_ROUTE113", "land_mons", "elusive", "Obstagoon line"),
    ("SPECIES_MEOWTH_GALAR", "MAP_ROUTE111", "land_mons", "elusive", "Perrserker line"),
    ("SPECIES_FARFETCHD_GALAR", "MAP_ROUTE117", "land_mons", "elusive", "Sirfetch'd line"),
    ("SPECIES_YAMASK_GALAR", "MAP_DESERT_UNDERPASS", "land_mons", "rare", "Runerigus line; ancient sand"),
    ("SPECIES_CORSOLA_GALAR", "MAP_UNDERWATER_ROUTE126", "water_mons", "rare", "Cursola line; bleached reef"),
    ("SPECIES_SNEASEL_HISUI", "MAP_SHOAL_CAVE_LOW_TIDE_STAIRS_ROOM", "land_mons", "rare", "Sneasler line"),
    ("SPECIES_QWILFISH_HISUI", "MAP_ROUTE132", "fishing_mons", "rare", "Overqwil line"),
    ("SPECIES_WOOPER_PALDEA", "MAP_ROUTE114", "land_mons", "rare", "Clodsire line; boggy ground"),
]


def main():
    data = json.loads((ROOT / "src/data/wild_encounters.json").read_text(encoding="utf-8"))
    group = next(g for g in data["wild_encounter_groups"] if g["label"] == "gWildMonHeaders")
    maps = A.emerald_maps()
    tables = {e["map"]: e for e in group["encounters"] if e["map"] in maps}
    info = A.parse_species()
    pre = {}
    for sp, d in info.items():
        for _, _, t in d["evos"]:
            pre.setdefault(t, sp)

    def family(sp):
        while sp in pre:
            sp = pre[sp]
        return sp

    def counts():
        c = Counter()
        for e in tables.values():
            for k, v in e.items():
                if k.endswith("_mons"):
                    for m in v["mons"]:
                        c[m["species"]] += 1
        return c

    placed = set()
    log = []
    for sp, mp, method, tier, why in PLAN:
        assert sp in info, f"unknown species {sp}"
        e = tables.get(mp)
        assert e and method in e, f"{mp} has no {method}"
        mons = e[method]["mons"]
        c = counts()
        order = {"common": ["common", "uncommon"], "uncommon": ["uncommon", "rare", "common"],
                 "rare": ["rare", "uncommon", "elusive"], "elusive": ["elusive", "rare"]}[tier]
        cands = []
        for tr in order:
            cands = [i for i in TIER_SLOTS[method][tr]
                     if (mp, method, i) not in placed and c[mons[i]["species"]] > 1
                     and family(mons[i]["species"]) != family(sp)]
            if cands:
                break
        if not cands:
            log.append(f"SKIPPED {sp} on {mp} {method}: every slot holds a family's only spot")
            continue
        i = max(cands, key=lambda k: c[mons[k]["species"]])
        old = mons[i]["species"]
        mons[i]["species"] = sp
        placed.add((mp, method, i))
        log.append(f"{sp} -> {mp} {method}[{i}] ({tier}) replacing {old}: {why}")
    (ROOT / "src/data/wild_encounters.json").write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n",
                                                         encoding="utf-8")
    print("\n".join(log))
    print(len(log), "placements")


if __name__ == "__main__":
    main()
