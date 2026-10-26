#!/usr/bin/env python3
"""Check effective rare-wild percentages by map and encounter method.

Usage: python3 tools/check_wally_wild_rarity.py
Prints violations and exits with code 1 when a starter or base pseudo-legendary
exceeds its 1% target in the normal wild encounter group.

This is an advisory check for DESIGN TARGETS, not a guarantee that every
habitat, special encounter or species evolutionary line is correct.
"""
import json
import sys as _sys
from pathlib import Path as _P
_sys.path.insert(0, str(_P(__file__).resolve().parent))
from wally_encounter_tables import species_rates, is_weighted
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TABLE = ROOT / "src/data/wild_encounters.json"
PSEUDO_BASES = {
    "DRATINI", "LARVITAR", "BAGON", "BELDUM", "GIBLE", "DEINO",
    "GOOMY", "JANGMO_O", "DREEPY", "FRIGIBAX",
}
STARTERS = {
    "BULBASAUR", "CHARMANDER", "SQUIRTLE", "CHIKORITA", "CYNDAQUIL",
    "TOTODILE", "TREECKO", "TORCHIC", "MUDKIP", "TURTWIG",
    "CHIMCHAR", "PIPLUP", "SNIVY", "TEPIG", "OSHAWOTT",
    "CHESPIN", "FENNEKIN", "FROAKIE", "ROWLET", "LITTEN",
    "POPPLIO", "GROOKEY", "SCORBUNNY", "SOBBLE", "SPRIGATITO",
    "FUECOCO", "QUAXLY",
}
RARE = {"SPECIES_" + x for x in PSEUDO_BASES | STARTERS}


def validate(data):
    group = next(g for g in data["wild_encounter_groups"] if g["label"] == "gWildMonHeaders")
    fields = {x["type"]: x for x in group["fields"]}
    violations = []
    checks = []
    for entry in group["encounters"]:
        loc = entry.get("map", entry.get("base_label", "(unknown)"))
        for method, field in fields.items():
            table = entry.get(method)
            if not isinstance(table, dict) or not isinstance(table.get("mons"), list):
                continue
            mons = table["mons"]
            weights = field["encounter_rates"]
            if not is_weighted(table) and len(mons) != len(weights):
                violations.append(f"{loc} {method}: expected {len(weights)} entries, found {len(mons)}")
                continue
            for rod, rarity in species_rates(table, field).items():
                subtotal = sum(rarity.values())
                if abs(subtotal - 100) > 0.01:
                    violations.append(f"{loc} {method}/{rod}: weights total {subtotal}, not 100")
                for name, percentage in sorted(rarity.items()):
                    if name in RARE:
                        checks.append((loc, method, rod, name, percentage))
                        if percentage > 1:
                            violations.append(
                                f"{loc} {method}/{rod}: {name} is {percentage}% (target <=1%)"
                            )
    return violations, checks


if __name__ == "__main__":
    data = json.loads(TABLE.read_text(encoding="utf-8"))
    violations, checks = validate(data)
    print(f"Checked {len(checks)} starter/pseudo-legendary map-method appearances")
    for violation in violations:
        print("FLAG:", violation)
    print(f"Total flags: {len(violations)}")
    if violations:
        print("Existing flags are audit findings, not automatically safe replacements.")
        raise SystemExit(1)
