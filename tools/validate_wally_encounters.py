#!/usr/bin/env python3
"""Validate Hoenn encounter tables before spending time on a full GBA build.

Run with: python3 tools/validate_wally_encounters.py
This intentionally validates availability and shape, not Pokédex completion.
"""

import json
import re
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
encounters = json.loads((ROOT / "src/data/wild_encounters.json").read_text())
species_text = (ROOT / "include/constants/species.h").read_text()
defined = set(re.findall(r"^\s*(SPECIES_[A-Z0-9_]+)\s*=", species_text, re.MULTILINE))

errors = []
species = set()
maps = set()
slots = 0
methods = Counter()

for group in encounters["wild_encounter_groups"]:
    if not group.get("for_maps"):
        continue
    formats = {field["type"]: field["encounter_rates"] for field in group["fields"]}
    for entry in group["encounters"]:
        location = entry["map"]
        # A map can deliberately have several tables for alternate encounter sets.
        key = (location, entry["base_label"])
        if key in maps:
            errors.append(f"Duplicate encounter table for {location}")
        maps.add(key)
        for field, rates in formats.items():
            data = entry.get(field)
            if data is None:
                continue
            mons = data["mons"]
            if len(mons) != len(rates):
                errors.append(f"{location}: {field} has {len(mons)} slots, expected {len(rates)}")
            for mon in mons:
                sid = mon["species"]
                if sid not in defined:
                    errors.append(f"{location}: unknown species {sid}")
                low, high = mon["min_level"], mon["max_level"]
                if not (1 <= low <= high <= 100):
                    errors.append(f"{location}: invalid level range for {sid}: {low}-{high}")
                species.add(sid)
                slots += 1
                methods[field] += 1

# Starter Pokémon from all nine generations must be catchable without trading.
# Keep them uncommon in habitat-appropriate tables; all 27 species are required.
STARTER_SPECIES = (
    "SPECIES_BULBASAUR", "SPECIES_CHARMANDER", "SPECIES_SQUIRTLE",
    "SPECIES_CHIKORITA", "SPECIES_CYNDAQUIL", "SPECIES_TOTODILE",
    "SPECIES_TREECKO", "SPECIES_TORCHIC", "SPECIES_MUDKIP",
    "SPECIES_TURTWIG", "SPECIES_CHIMCHAR", "SPECIES_PIPLUP",
    "SPECIES_SNIVY", "SPECIES_TEPIG", "SPECIES_OSHAWOTT",
    "SPECIES_CHESPIN", "SPECIES_FENNEKIN", "SPECIES_FROAKIE",
    "SPECIES_ROWLET", "SPECIES_LITTEN", "SPECIES_POPPLIO",
    "SPECIES_GROOKEY", "SPECIES_SCORBUNNY", "SPECIES_SOBBLE",
    "SPECIES_SPRIGATITO", "SPECIES_FUECOCO", "SPECIES_QUAXLY",
)
for starter in STARTER_SPECIES:
    if starter not in species:
        errors.append(f"Starter species has no Hoenn wild encounter: {starter}")

print(f"Wild encounter audit: {len(maps)} tables, {slots} slots, {len(species)} unique species")
for method, count in sorted(methods.items()):
    print(f"  {method}: {count} slots")
if errors:
    print("\n".join("ERROR: " + e for e in errors))
    raise SystemExit(1)
print("Encounter table validation passed; this does NOT prove all Pokémon are obtainable.")
