#!/usr/bin/env python3
"""Audit selected late-evolving wild forms against standard level thresholds.

Run: python3 tools/check_wally_wild_evolution_levels.py

These are targeted assertions, not a complete evolution-chain parser. Every
exception requires documentation and a conscious game-design decision.
"""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
# Only exact, ordinary level-up evolutions with well-established thresholds.
MIN_EVOLUTION_LEVEL = {
    "SPECIES_SALAMENCE": 50,
    "SPECIES_METAGROSS": 45,
    "SPECIES_HAXORUS": 48,
    "SPECIES_KOMMO_O": 45,
    "SPECIES_SHELGON": 30,
    "SPECIES_METANG": 20,
    "SPECIES_FRAXURE": 38,
    "SPECIES_HAKAMO_O": 35,
    "SPECIES_ZWEILOUS": 50,
    "SPECIES_HYDREIGON": 64,
    "SPECIES_GARCHOMP": 48,
    "SPECIES_GABITE": 24,
    "SPECIES_DRAGONAIR": 30,
    "SPECIES_DRAGONITE": 55,
    "SPECIES_TYRANITAR": 55,
    "SPECIES_PUPITAR": 30,
    "SPECIES_DRAKLOAK": 50,
    "SPECIES_DRAGAPULT": 60,
    "SPECIES_ARCTIBAX": 35,
    "SPECIES_BAXCALIBUR": 54,
}

data = json.loads((ROOT / "src/data/wild_encounters.json").read_text(encoding="utf-8"))
group = next(g for g in data["wild_encounter_groups"] if g["label"] == "gWildMonHeaders")
errors = []
checked = 0
for e in group["encounters"]:
    for method, payload in e.items():
        if not isinstance(payload, dict) or not isinstance(payload.get("mons"), list):
            continue
        for slot, mon in enumerate(payload["mons"]):
            floor = MIN_EVOLUTION_LEVEL.get(mon["species"])
            if floor is None:
                continue
            checked += 1
            if min(mon["min_level"], mon["max_level"]) < floor:
                errors.append(f'{e.get("map", e.get("base_label"))} {method}[{slot}]: {mon["species"]} level {mon["min_level"]}-{mon["max_level"]} below evolution level {floor}')
for error in errors:
    print("FLAG:", error)
print(f"Audited {checked} slots. Flagged {len(errors)} underleveled evolved encounters.")
if errors:
    raise SystemExit(1)
