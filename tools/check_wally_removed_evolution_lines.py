#!/usr/bin/env python3
"""Audit the three evolution lines recently removed from ordinary wild encounters.

Reads the actual expansion species-info definitions, not assumed evolution rules.
Run from repo root: python3 tools/check_wally_removed_evolution_lines.py

This proves that evolution *definitions* exist, not that the necessary stone
is reachable in gameplay or that a species is actually catchable in a compiled ROM.
"""
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPECIES_ROOT = ROOT / "src/data/pokemon/species_info"
EXPECTED = {
    "gen_2_families.h": {
        "SPECIES_MAREEP": ("EVO_LEVEL", "15", "SPECIES_FLAAFFY"),
        "SPECIES_FLAAFFY": ("EVO_LEVEL", "30", "SPECIES_AMPHAROS"),
    },
    "gen_6_families.h": {
        "SPECIES_HONEDGE": ("EVO_LEVEL", "35", "SPECIES_DOUBLADE"),
        "SPECIES_DOUBLADE": ("EVO_ITEM", "ITEM_DUSK_STONE", "SPECIES_AEGISLASH_SHIELD"),
    },
    "gen_8_families.h": {
        "SPECIES_TOXEL": ("EVO_LEVEL", "30", "SPECIES_TOXTRICITY_AMPED"),
    },
}
WILD_BASES = {"SPECIES_MAREEP", "SPECIES_HONEDGE", "SPECIES_TOXEL"}


def species_section(text, species):
    start = text.find(f"[{species}] =")
    if start == -1:
        raise AssertionError(f"Missing species definition: {species}")
    match = re.search(r"\n\s*\[SPECIES_[A-Z0-9_]+\]\s*=", text[start + 1:])
    return text[start: start + 1 + match.start()] if match else text[start:]


def main():
    errors = []
    for filename, definitions in EXPECTED.items():
        text = (SPECIES_ROOT / filename).read_text(encoding="utf-8")
        for species, (method, requirement, target) in definitions.items():
            section = species_section(text, species)
            pattern = r"\{" + re.escape(method) + r"\s*,\s*" + re.escape(requirement) + r"\s*,\s*" + re.escape(target) + r"(?=[,}\s])"
            if not re.search(pattern, section):
                errors.append(f"{species} -> {target} ({method} {requirement}) not found")
    gen8 = (SPECIES_ROOT / "gen_8_families.h").read_text(encoding="utf-8")
    toxel = species_section(gen8, "SPECIES_TOXEL")
    if "{EVO_LEVEL, 30, SPECIES_TOXTRICITY_LOW_KEY" not in toxel:
        errors.append("Toxel low-key nature evolution missing")
    data = json.loads((ROOT / "src/data/wild_encounters.json").read_text(encoding="utf-8"))
    group = next(g for g in data["wild_encounter_groups"] if g["label"] == "gWildMonHeaders")
    wild = {mon["species"] for e in group["encounters"] for v in e.values()
            if isinstance(v, dict) and isinstance(v.get("mons"), list) for mon in v["mons"]}
    for base in sorted(WILD_BASES):
        if base not in wild:
            errors.append(f"Base species not present in normal wild tables: {base}")
    for error in errors:
        print("FLAG:", error)
    print(f"Evolution definitions reviewed: {sum(map(len, EXPECTED.values())) + 1}, base wild species: {len(WILD_BASES)}, flags: {len(errors)}")
    print("ITEM_DUSK_STONE must separately be tested for in-game accessibility.")
    if errors:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
