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
# Rare visitors (src/data/wally_rare_visitors.h) are real wild encounters too.
import re as _re
_vis = (ROOT / "src/data/wally_rare_visitors.h").read_text(encoding="utf-8")
VISITOR_SPECIES = set(_re.findall(r"VISITOR\(MAP_\w+,\s*\w+,\s*(SPECIES_\w+)", _vis))
species = set(species) | VISITOR_SPECIES
# A form counts for its species (e.g. SPECIES_GIMMIGHOUL_CHEST -> SPECIES_GIMMIGHOUL).
for _s in list(species):
    _parts = _s.split("_")
    for _k in range(2, len(_parts)):
        species.add("_".join(_parts[:_k]))

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

# Catch accidental rollback of the expanded Gen 1-9 habitat tables.\n# 490 is the floor after replacing 22 sole wild evolved placements with
# earlier catchable stages (2026-10-16); species-count changes must be
# reviewed together with the evolution-family accessibility audit.
# Feebas uses the special 10% Route 119 Good Rod hook, not a JSON slot.
MIN_WILD_SPECIES = 490
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
    # Additional habitat-based species introduced by Wally Journey.
    "SPECIES_PACHIRISU",
    "SPECIES_DRIFLOON",
    "SPECIES_HAPPINY",
    "SPECIES_CHATOT",
    "SPECIES_SPIRITOMB",
    "SPECIES_SKORUPI",
    "SPECIES_SNOVER",
    "SPECIES_PATRAT",
    "SPECIES_PURRLOIN",
    "SPECIES_PIDOVE",
    "SPECIES_BLITZLE",
    "SPECIES_AUDINO",
    "SPECIES_VENIPEDE",
    "SPECIES_MARACTUS",
    "SPECIES_SCRAGGY",
    "SPECIES_DARUMAKA",
    "SPECIES_TRUBBISH",
    "SPECIES_EMOLGA",
    "SPECIES_ELGYEM",
    "SPECIES_CUBCHOO",
    "SPECIES_CRYOGONAL",
    "SPECIES_MIENFOO",
    "SPECIES_DRUDDIGON",
    "SPECIES_BOUFFALANT",
    "SPECIES_VULLABY",
    "SPECIES_HEATMOR",
    "SPECIES_DURANT",
    "SPECIES_DRAMPA",
    "SPECIES_SILICOBRA",
)
EXPANSION_PRE_EVOLUTION = {
    "SPECIES_SWANNA": "SPECIES_DUCKLETT",
    "SPECIES_BARRASKEWDA": "SPECIES_ARROKUDA",
    "SPECIES_PALAFIN": "SPECIES_FINIZEN",  # final form isn't wild; Finizen is
    "SPECIES_LUCARIO": "SPECIES_RIOLU",  # 2026-10-23: friendship evolution, Riolu is wild
}
for mon in EXPANSION_SPECIES:
    if mon not in species and EXPANSION_PRE_EVOLUTION.get(mon) not in species:
        errors.append(f"Expanded species family is no longer available wild: {mon}")

# Protect evolutionary FAMILY availability, not mandatory wild evolved forms.
# A species may be absent from wild tables if a catchable prior stage can
# evolve into it. Full evolution/item accessibility still needs separate proof.
EVOLVED_RARE_SPECIES = (
    "SPECIES_TREVENANT", "SPECIES_GOURGEIST", "SPECIES_POLTEAGEIST",
    "SPECIES_DOUBLADE", "SPECIES_AEGISLASH", "SPECIES_ZOROARK",
    "SPECIES_GALLADE", "SPECIES_MANDIBUZZ", "SPECIES_TOGETIC",
    "SPECIES_RUFFLET", "SPECIES_TOXTRICITY", "SPECIES_MAGNEZONE",
    "SPECIES_MAGMORTAR", "SPECIES_CENTISKORCH", "SPECIES_VOLCARONA",
    "SPECIES_PUPITAR", "SPECIES_GOLURK", "SPECIES_SANDACONDA",
    "SPECIES_FRAXURE", "SPECIES_HAXORUS", "SPECIES_SHELGON",
    "SPECIES_GABITE", "SPECIES_ZWEILOUS", "SPECIES_METANG",
    "SPECIES_CROBAT", "SPECIES_FROSLASS", "SPECIES_SLOWKING",
    "SPECIES_ABOMASNOW", "SPECIES_NOIVERN", "SPECIES_BRAVIARY",
    "SPECIES_TOGEKISS", "SPECIES_PALOSSAND", "SPECIES_KINGAMBIT",
    "SPECIES_DRAKLOAK", "SPECIES_DUDUNSPARCE", "SPECIES_LILLIPUP",
    "SPECIES_HERDIER", "SPECIES_AMPHAROS", "SPECIES_ELECTIVIRE",
    "SPECIES_EXCADRILL", "SPECIES_KOMMO_O", "SPECIES_GARCHOMP",
    "SPECIES_SALAMENCE", "SPECIES_METAGROSS", "SPECIES_DRAGONITE",
    "SPECIES_DRAGAPULT", "SPECIES_BAXCALIBUR", "SPECIES_TYRANITAR",
    "SPECIES_FERROTHORN", "SPECIES_HYDREIGON",
)
# Explicitly permitted wild-to-pre-evolution replacements from the level audit.
# This only validates family presence; evolution reachability is a separate audit.
WILD_PRE_EVOLUTION = {
    "SPECIES_DOUBLADE": "SPECIES_HONEDGE",
    "SPECIES_AEGISLASH": "SPECIES_HONEDGE",
    "SPECIES_TOXTRICITY": "SPECIES_TOXEL",
    "SPECIES_HAXORUS": ("SPECIES_AXEW", "SPECIES_FRAXURE"),
    "SPECIES_SHELGON": ("SPECIES_BAGON",),
    "SPECIES_MAGNEZONE": ("SPECIES_MAGNEMITE", "SPECIES_MAGNETON"),
    "SPECIES_TREVENANT": ("SPECIES_PHANTUMP",),  # Trevenant now evolves at Lv 38  # owner rule: evolved stage needs no wild slot
    "SPECIES_ZWEILOUS": "SPECIES_DEINO",
    "SPECIES_DRAKLOAK": "SPECIES_DREEPY",
    "SPECIES_HERDIER": "SPECIES_LILLIPUP",
    "SPECIES_AMPHAROS": "SPECIES_MAREEP",
    "SPECIES_KOMMO_O": "SPECIES_JANGMO_O",
    "SPECIES_GARCHOMP": "SPECIES_GIBLE",
    "SPECIES_SALAMENCE": "SPECIES_BAGON",
    "SPECIES_METAGROSS": "SPECIES_BELDUM",
    "SPECIES_DRAGONITE": "SPECIES_DRATINI",
    "SPECIES_DRAGAPULT": "SPECIES_DREEPY",
    "SPECIES_BAXCALIBUR": "SPECIES_FRIGIBAX",
    "SPECIES_TYRANITAR": "SPECIES_LARVITAR",
    "SPECIES_HYDREIGON": "SPECIES_DEINO",
    "SPECIES_ZOROARK": "SPECIES_ZORUA",
    "SPECIES_MANDIBUZZ": "SPECIES_VULLABY",
    "SPECIES_PALOSSAND": "SPECIES_SANDYGAST",
    "SPECIES_ABOMASNOW": "SPECIES_SNOVER",
    "SPECIES_NOIVERN": "SPECIES_NOIBAT",
    "SPECIES_BRAVIARY": "SPECIES_RUFFLET",
    "SPECIES_VOLCARONA": "SPECIES_LARVESTA",
    "SPECIES_FERROTHORN": "SPECIES_FERROSEED",
    "SPECIES_GOLURK": "SPECIES_GOLETT",
    "SPECIES_SANDACONDA": "SPECIES_SILICOBRA",
    "SPECIES_KINGAMBIT": ("SPECIES_BISHARP", "SPECIES_PAWNIARD"),  # owner: Kingambit isn't wild; Bisharp holds Leader's Crest
    "SPECIES_CENTISKORCH": "SPECIES_SIZZLIPEDE",
    "SPECIES_POLTEAGEIST": "SPECIES_SINISTEA",
    # 2026-10-23 full spawn pass: stone/trade/friendship final forms are no longer wild;
    # their families stay catchable at an earlier stage.
    "SPECIES_GOURGEIST": ("SPECIES_PUMPKABOO", "SPECIES_PUMPKABOO_AVERAGE"),
    "SPECIES_GALLADE": "SPECIES_RALTS",
    "SPECIES_TOGETIC": "SPECIES_TOGEPI",
    "SPECIES_TOGEKISS": "SPECIES_TOGEPI",
    "SPECIES_MAGMORTAR": ("SPECIES_MAGBY", "SPECIES_MAGMAR"),
    "SPECIES_CROBAT": ("SPECIES_ZUBAT", "SPECIES_GOLBAT"),
    "SPECIES_FROSLASS": "SPECIES_SNORUNT",
    "SPECIES_SLOWKING": ("SPECIES_SLOWPOKE", "SPECIES_SLOWPOKE_GALAR"),
    "SPECIES_ELECTIVIRE": "SPECIES_ELEKID",
}
for mon in EVOLVED_RARE_SPECIES:
    alts = WILD_PRE_EVOLUTION.get(mon, ())
    alts = (alts,) if isinstance(alts, str) else alts
    if mon not in species and not any(a in species for a in alts):
        errors.append(f"Missing rare evolution family from wild habitats: {mon}")

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
