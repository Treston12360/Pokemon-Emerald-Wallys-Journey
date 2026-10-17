#!/usr/bin/env python3
"""Check that explicitly elusive wild species stay exceptional per encounter method.

Run: python3 tools/check_wally_elusive_encounters.py
Species not listed here need a separate lore and historical encounter review.
"""
import json
from pathlib import Path
from collections import defaultdict

ROOT = Path(__file__).resolve().parents[1]
CAPS = {
    "SPECIES_SPIRITOMB": 1,
    "SPECIES_ROTOM": 1,
    "SPECIES_MIMIKYU": 1,
    "SPECIES_POLTEAGEIST": 1,
}
groups = json.loads((ROOT / "src/data/wild_encounters.json").read_text(encoding="utf-8"))["wild_encounter_groups"]
main = next(group for group in groups if group["label"] == "gWildMonHeaders")
problems = []
found = defaultdict(int)
for entry in main["encounters"]:
    for field in main["fields"]:
        method = field["type"]
        table = entry.get(method)
        if not isinstance(table, dict) or "mons" not in table:
            continue
        weights = field["encounter_rates"]
        for group_name, indices in (field.get("groups") or {"all": list(range(len(weights)))}).items():
            rates = defaultdict(int)
            for i in indices:
                species = table["mons"][i]["species"]
                rates[species] += weights[i]
            for species, cap in CAPS.items():
                if species in rates:
                    found[species] += 1
                    if rates[species] > cap:
                        problems.append(f'{entry.get("map", entry.get("base_label"))} {method}/{group_name}: {species} {rates[species]}% > {cap}%')
for species in CAPS:
    if not found[species]:
        problems.append(f"{species}: no main wild placement (review gift/event availability)")
for item in problems:
    print("FLAG:", item)
print(f"Elusive species checked: {len(CAPS)}, problems: {len(problems)}")
if problems:
    raise SystemExit(1)
