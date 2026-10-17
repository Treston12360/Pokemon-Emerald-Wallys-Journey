#!/usr/bin/env python3
"""Audit every ordinary level-evolved species in Hoenn wild tables.

Uses THIS project's Gen 1-9 evolution definitions, including custom levels.
Only enforce a level floor when *all* parsed ways into a species are level-based
and no non-level method exists. Stone, friendship and special evolution species
need deliberate per-species design review, not an invented minimum level.

Run: python3 tools/check_wally_all_wild_evolution_levels.py
     python3 tools/check_wally_all_wild_evolution_levels.py --json
"""
import argparse
import json
import re
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
INFO = ROOT / "src/data/pokemon/species_info"
TABLE = ROOT / "src/data/wild_encounters.json"
ENTRY = re.compile(r"\[\s*(SPECIES_[A-Z0-9_]+)\s*\]\s*=")
EDGE = re.compile(r"\{\s*(EVO_[A-Z0-9_]+)\s*,\s*([^,{}]+)\s*,\s*(SPECIES_[A-Z0-9_]+)\s*(?=[,}])")
# Restrict automatic level thresholds to explicit level-up methods.
LEVEL_METHODS = {
    "EVO_LEVEL", "EVO_LEVEL_MALE", "EVO_LEVEL_FEMALE",
    "EVO_LEVEL_DAY", "EVO_LEVEL_NIGHT",
    "EVO_LEVEL_RAIN", "EVO_LEVEL_DARK_TYPE_MON_IN_PARTY",
    "EVO_LEVEL_NATURE_AMPED", "EVO_LEVEL_NATURE_LOW_KEY",
}
def evolution_rules():
    incoming = defaultdict(list)
    for generation in range(1, 10):
        data = (INFO / f"gen_{generation}_families.h").read_text(encoding="utf-8")
        entries = list(ENTRY.finditer(data))
        for i, entry in enumerate(entries):
            segment = data[entry.end():entries[i + 1].start() if i + 1 < len(entries) else len(data)]
            match = re.search(r"\.evolutions\s*=\s*EVOLUTION\(", segment)
            if not match:
                continue
            segment = re.split(r"\n\s*\.[a-zA-Z_]+\s*=", segment[match.end():], maxsplit=1)[0]
            for method, requirement, target in EDGE.findall(segment):
                incoming[target].append((entry.group(1), method, requirement.strip()))
    return incoming

def audit():
    incoming = evolution_rules()
    floors = {}
    needs_review = {}
    for target, edges in incoming.items():
        if all(method in LEVEL_METHODS and value.isdigit() for _, method, value in edges):
            # An alternative lower-level path makes that level the earliest ordinary evolution.
            floors[target] = min(int(value) for _, _, value in edges)
        else:
            needs_review[target] = edges
    data = json.loads(TABLE.read_text(encoding="utf-8"))
    main = next(group for group in data["wild_encounter_groups"] if group["label"] == "gWildMonHeaders")
    failures = []
    checked = 0
    special_present = set()
    for entry in main["encounters"]:
        location = entry.get("map", entry.get("base_label", "unknown"))
        for method, payload in entry.items():
            if not isinstance(payload, dict) or not isinstance(payload.get("mons"), list):
                continue
            for index, mon in enumerate(payload["mons"]):
                species = mon["species"]
                if species in needs_review:
                    special_present.add(species)
                if species not in floors:
                    continue
                checked += 1
                floor = floors[species]
                if mon["min_level"] < floor:
                    failures.append({
                        "map": location, "method": method, "slot": index,
                        "species": species, "actual": [mon["min_level"], mon["max_level"]],
                        "minimum_evolution_level": floor,
                    })
    return {
        "level_evolved_species_defined": len(floors),
        "level_evolved_wild_slots_checked": checked,
        "underleveled_wild_slots": failures,
        "wild_special_evolution_species_requiring_manual_review": sorted(special_present),
        "limitations": [
            "A minimum level is applied only to species with exclusively numeric level-evolution paths.",
            "Alternate item, trade, friendship, move, location, or conditional evolutions need manual design decisions.",
            "Evolution method variants missing from LEVEL_METHODS are intentionally not assumed to be numeric.",
            "This checks ordinary main wild tables, not gifts, static events or roaming encounters.",
        ],
    }

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    report = audit()
    if args.json:
        print(json.dumps(report, indent=2))
    else:
        print("Defined exclusively level-evolved targets:", report["level_evolved_species_defined"])
        print("Wild slots checked:", report["level_evolved_wild_slots_checked"])
        print("Underleveled slots:", len(report["underleveled_wild_slots"]))
        for problem in report["underleveled_wild_slots"]:
            print("FLAG:", json.dumps(problem, sort_keys=True))
        print("Special-evolution wild species needing review:",
              len(report["wild_special_evolution_species_requiring_manual_review"]))
    if report["underleveled_wild_slots"]:
        raise SystemExit(1)
