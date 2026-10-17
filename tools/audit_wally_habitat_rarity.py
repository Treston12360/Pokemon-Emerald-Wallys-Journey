#!/usr/bin/env python3
"""Produce a prioritized, evidence-limited Hoenn encounter habitat/rarity review.

This reports potentially implausible placements for HUMAN review, not automatic
deletions. Canonical game-specific locations and Pokédex habitat text still need
independent verification. JSON: --json
"""
import argparse
import json
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / "src/data/wild_encounters.json"
PSEUDO_FAMILIES = {
    "DRATINI", "DRAGONAIR", "DRAGONITE", "LARVITAR", "PUPITAR", "TYRANITAR",
    "BAGON", "SHELGON", "SALAMENCE", "BELDUM", "METANG", "METAGROSS",
    "GIBLE", "GABITE", "GARCHOMP", "DEINO", "ZWEILOUS", "HYDREIGON",
    "GOOMY", "SLIGGOO", "GOODRA", "JANGMO_O", "HAKAMO_O", "KOMMO_O",
    "DREEPY", "DRAKLOAK", "DRAGAPULT", "FRIGIBAX", "ARCTIBAX", "BAXCALIBUR",
}
STARTER_BASES = {
    "BULBASAUR", "CHARMANDER", "SQUIRTLE", "CHIKORITA", "CYNDAQUIL",
    "TOTODILE", "TREECKO", "TORCHIC", "MUDKIP", "TURTWIG", "CHIMCHAR",
    "PIPLUP", "SNIVY", "TEPIG", "OSHAWOTT", "CHESPIN", "FENNEKIN",
    "FROAKIE", "ROWLET", "LITTEN", "POPPLIO", "GROOKEY", "SCORBUNNY",
    "SOBBLE", "SPRIGATITO", "FUECOCO", "QUAXLY",
}
# Never infer a real habitat from a species name alone. These categories are
# deliberate review prompts, not comprehensive ecological facts.
ENVIRONMENTS = {
    "desert": ("DESERT", "MIRAGE"),
    "underwater": ("UNDERWATER",),
    "sea": ("SHOAL", "SEAFLOOR", "ABANDONEDSHIP"),
    "volcanic": ("FIERY", "JAGGED", "MAGMA", "MTCHIMNEY"),
    "cold": ("SHOAL",),
    "forest": ("PETALBURGWOODS",),
}
HABITAT_CANDIDATES = {
    "desert": {"SNOVER", "ABOMASNOW", "CUBCHOO", "BEARTIC", "CRYOGONAL",
               "VANILLITE", "VANILLISH", "VANILLUXE", "SPHEAL", "SEALEO", "WALREIN"},
    "volcanic": {"SPHEAL", "SEALEO", "WALREIN", "CUBCHOO", "BEARTIC",
                 "SNOVER", "ABOMASNOW", "CRYOGONAL"},
    "cold": {"NUMEL", "CAMERUPT", "SLUGMA", "MAGCARGO", "TORKOAL"},
}
def audit():
    groups = json.loads(DATA.read_text(encoding="utf-8"))["wild_encounter_groups"]
    main = next(g for g in groups if g["label"] == "gWildMonHeaders")
    fields = {field["type"]: field for field in main["fields"]}
    flags = []
    rare_placements = []
    maps_by_species = defaultdict(set)
    checked = 0
    for entry in main["encounters"]:
        location = entry.get("map", entry.get("base_label", "unknown"))
        upper = location.upper().replace("_", "")
        for method, field in fields.items():
            table = entry.get(method)
            if not isinstance(table, dict) or not isinstance(table.get("mons"), list):
                continue
            mons, weights = table["mons"], field["encounter_rates"]
            if len(mons) != len(weights):
                flags.append({"type": "invalid_slots", "map": location, "method": method})
                continue
            groups_for_method = field.get("groups") or {"all": list(range(len(weights)))}
            for subgroup, indices in groups_for_method.items():
                chances = defaultdict(int)
                for index in indices:
                    mon = mons[index]
                    species = mon["species"]
                    maps_by_species[species].add(location)
                    chances[species] += weights[index]
                    checked += 1
                    short = species.removeprefix("SPECIES_")
                    for habitat, keywords in ENVIRONMENTS.items():
                        if short in HABITAT_CANDIDATES.get(habitat, ()) and any(k in upper for k in keywords):
                            flags.append({"type": "habitat_conflict_candidate", "map": location,
                                          "method": method, "subgroup": subgroup, "species": species,
                                          "reason": f"{habitat} location conflicts with likely climate"})
                for species, percent in chances.items():
                    short = species.removeprefix("SPECIES_")
                    group = ("starter" if short in STARTER_BASES else
                             "pseudo_legendary_family" if short in PSEUDO_FAMILIES else None)
                    if group:
                        rare_placements.append({"map": location, "method": method,
                                                "subgroup": subgroup, "species": species,
                                                "rarity_class": group, "conditional_percent": percent})
                        if percent > 1:
                            flags.append({"type": "rare_above_1_percent", "map": location,
                                          "method": method, "subgroup": subgroup,
                                          "species": species, "conditional_percent": percent})
    return {"audited_slots_across_subgroups": checked,
            "distinct_species": len(maps_by_species),
            "rare_placements": rare_placements,
            "review_flags": flags,
            "notes": [
                "Percentages are conditional on a wild encounter and method/subgroup, not the chance per step.",
                "A rare species on many maps may be common region-wide despite 1% on each map.",
                "Map-name climate flags are heuristics; caves, microclimates and intentional exceptions exist.",
                "Real multi-game Pokédex and encounter-location comparisons remain a separate research task.",
                "Legendary, mythical, gifts, evolutions, and scripted encounters require separate coverage audits."
            ]}
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    result = audit()
    if args.json:
        print(json.dumps(result, indent=2))
    else:
        print(f"Audited {result['audited_slots_across_subgroups']} subgroup slots; "
              f"{result['distinct_species']} wild species")
        print(f"Rare family placements: {len(result['rare_placements'])}")
        print(f"Habitat/rarity review flags: {len(result['review_flags'])}")
        for flag in result["review_flags"]:
            print("REVIEW:", json.dumps(flag, sort_keys=True))
        print("No placements were changed automatically.")
