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

# Catch accidental rollback of the expanded Gen 1-9 habitat tables.
# Feebas uses the special 10% Route 119 Good Rod hook, not a JSON slot.
MIN_WILD_SPECIES = 439
if len(species) < MIN_WILD_SPECIES:
    errors.append(f"Wild roster fell from at least {MIN_WILD_SPECIES} species to {len(species)}")

EXPANSION_SPECIES = (
    "SPECIES_BIDOOF", "SPECIES_TAROUNTULA", "SPECIES_NYMBLE",
    "SPECIES_FLETCHLING", "SPECIES_PIKIPEK", "SPECIES_JOLTIK",
    "SPECIES_KLAWF", "SPECIES_CYCLIZAR", "SPECIES_MAGBY",
    "SPECIES_GLAMEOW", "SPECIES_KRICKETOT", "SPECIES_MINCCINO",
    "SPECIES_TANDEMAUS", "SPECIES_FIDOUGH", "SPECIES_BLIPBUG",
    "SPECIES_COMBEE", "SPECIES_BURMY", "SPECIES_KLINK",
    "SPECIES_BRONZOR", "SPECIES_CUFANT", "SPECIES_ORTHWORM",
    "SPECIES_TURTONATOR", "SPECIES_TOEDSCOOL", "SPECIES_GIMMIGHOUL",
    "SPECIES_ARCHEN", "SPECIES_TIRTOUGA", "SPECIES_LUCARIO",
    "SPECIES_ORANGURU", "SPECIES_PASSIMIAN", "SPECIES_WOOLOO",
    "SPECIES_SMOOCHUM", "SPECIES_VANILLITE", "SPECIES_FALINKS",
    "SPECIES_FINNEON", "SPECIES_SHELLOS", "SPECIES_BUIZEL",
    "SPECIES_ALOMOMOLA", "SPECIES_HUNTAIL", "SPECIES_GOREBYSS",
    "SPECIES_SEALEO", "SPECIES_MILOTIC", "SPECIES_DREDNAW",
    "SPECIES_CARRACOSTA", "SPECIES_WUGTRIO", "SPECIES_PALAFIN",
    "SPECIES_SWANNA", "SPECIES_GRAPPLOCT", "SPECIES_BARRASKEWDA",
    "SPECIES_STARMIE",
)
for mon in EXPANSION_SPECIES:
    if mon not in species:
        errors.append(f"Expanded species is no longer available wild: {mon}")

# Story-only encounters must not accidentally enter normal grass/surf/fishing.
STORY_ONLY_SPECIES = (
    "SPECIES_MEW", "SPECIES_MEWTWO", "SPECIES_CELEBI",
    "SPECIES_GROUDON", "SPECIES_KYOGRE", "SPECIES_RAYQUAZA",
    "SPECIES_JIRACHI", "SPECIES_DEOXYS", "SPECIES_ARCEUS",
    "SPECIES_DIALGA", "SPECIES_PALKIA", "SPECIES_GIRATINA",
    "SPECIES_RESHIRAM", "SPECIES_ZEKROM", "SPECIES_KYUREM",
    "SPECIES_XERNEAS", "SPECIES_YVELTAL", "SPECIES_ZYGARDE",
    "SPECIES_SOLGALEO", "SPECIES_LUNALA", "SPECIES_NECROZMA",
    "SPECIES_ZACIAN", "SPECIES_ZAMAZENTA", "SPECIES_ETERNATUS",
    "SPECIES_KORAIDON", "SPECIES_MIRAIDON", "SPECIES_TERAPAGOS",
)
for mon in STORY_ONLY_SPECIES:
    if mon in species:
        errors.append(f"Story-only legendary/mythical in ordinary wild table: {mon}")

# The original route-specific Feebas hook is deliberately outside the JSON data.
encounter_code = (ROOT / "src/wild_encounter.c").read_text()
if not (
    "gWildFeebas" in encounter_code
    and "rod == GOOD_ROD" in encounter_code
    and "Random() % 10" in encounter_code
    and "MAP_ROUTE119" in encounter_code
):
    errors.append("Route 119's special 10% Good Rod Feebas encounter was removed")

print(f"Wild encounter audit: {len(maps)} tables, {slots} slots, {len(species)} unique species")
for method, count in sorted(methods.items()):
    print(f"  {method}: {count} slots")
if errors:
    print("\n".join("ERROR: " + e for e in errors))
    raise SystemExit(1)
print("Encounter table validation passed; this does NOT prove all Pokémon are obtainable.")
