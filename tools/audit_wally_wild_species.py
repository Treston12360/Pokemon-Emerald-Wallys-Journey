#!/usr/bin/env python3
"""Audit species appearing in ordinary Hoenn wild encounter tables.

Run from repository root:
  python3 tools/audit_wally_wild_species.py
  python3 tools/audit_wally_wild_species.py --json

This is an encounter-table coverage audit, NOT a full catchability audit.
Evolutions, breeding, gifts, fossils, scripted events, and version-dependent
conditions need separate checks before a species is deemed unobtainable.
"""
import argparse
import json
import re
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ENCOUNTERS = ROOT / "src/data/wild_encounters.json"
SPECIES = ROOT / "include/constants/species.h"
SPECIES_RE = re.compile(r"^\s*(SPECIES_[A-Z0-9_]+)\s*=\s*(\d+)\s*,", re.M)


def encounter_species(entry):
    """Yield species and context from every encounter method in a map record."""
    for method, content in entry.items():
        if not isinstance(content, dict) or not isinstance(content.get("mons"), list):
            continue
        for mon in content["mons"]:
            if isinstance(mon, dict) and isinstance(mon.get("species"), str):
                yield method, mon["species"]


def audit():
    data = json.loads(ENCOUNTERS.read_text(encoding="utf-8"))
    defined = {name: int(num) for name, num in SPECIES_RE.findall(SPECIES.read_text(encoding="utf-8"))}
    groups = data["wild_encounter_groups"]
    main = next((g for g in groups if g.get("label") == "gWildMonHeaders"), None)
    if main is None:
        raise ValueError("Main gWildMonHeaders encounter group not found")
    methods = Counter()
    locations = {}
    for record in main["encounters"]:
        location = record.get("map", record.get("base_label", "unknown"))
        for method, species in encounter_species(record):
            methods[method] += 1
            locations.setdefault(species, set()).add(location)

    # Species IDs 1..905 cover the original Gen 1-8/Legends base species in
    # this expansion version. Gen 9 is interspersed later with alternate forms.
    # Audit all defined species and leave form/legendary filtering explicit.
    all_defined = {n for n, i in defined.items() if n != "SPECIES_NONE" and not n.startswith("SPECIES_CUSTOM_")}
    main_set = set(locations)
    other_groups = {}
    for group in groups:
        if group is main:
            continue
        other_groups[group["label"]] = sorted({
            species for e in group.get("encounters", [])
            for _, species in encounter_species(e)
        })
    missing = sorted(all_defined - main_set, key=lambda n: (defined[n], n))
    unknown = sorted(main_set - all_defined)
    return {
        "main_encounter_records": len(main["encounters"]),
        "main_wild_slots": sum(methods.values()),
        "distinct_main_wild_species": len(main_set),
        "main_method_slots": dict(sorted(methods.items())),
        "total_defined_species_and_forms": len(all_defined),
        "missing_from_main_wild_tables_count": len(missing),
        "missing_from_main_wild_tables": [{"id": defined[n], "species": n} for n in missing],
        "species_unknown_to_enum": unknown,
        "other_encounter_groups": other_groups,
        "locations_by_species": {n: sorted(v) for n, v in sorted(locations.items())},
        "caveat": "Absent from a wild table does not mean unobtainable. Species list includes forms, Mega/Primal, and event-only species. Check evolution, breeding, gifts, and events separately."
    }


if __name__ == "__main__":
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--json", action="store_true", help="Print complete machine-readable coverage report")
    args = p.parse_args()
    report = audit()
    if args.json:
        print(json.dumps(report, indent=2))
    else:
        for field in ("main_encounter_records", "main_wild_slots", "distinct_main_wild_species",
                      "total_defined_species_and_forms", "missing_from_main_wild_tables_count"):
            print(f"{field}: {report[field]}")
        print("Main wild encounter slots by method:", report["main_method_slots"])
        print("\nFirst 100 missing species/forms (not necessarily unobtainable):")
        for item in report["missing_from_main_wild_tables"][:100]:
            print(f'  {item["id"]:4} {item["species"]}')
        print("\n" + report["caveat"])
