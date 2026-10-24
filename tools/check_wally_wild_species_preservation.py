#!/usr/bin/env python3
"""Guard against accidental loss of normal wild Pokémon species.

Run from the repository root:
    python3 tools/check_wally_wild_species_preservation.py

Adding species is allowed. Removing a previously present species fails the
check until the change is reviewed and the baseline is intentionally updated.
This is NOT a full obtainable-dex test: it does not inspect gift Pokémon,
breeding, evolutions, fossils or events.
"""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
data = json.loads((ROOT / "src/data/wild_encounters.json").read_text(encoding="utf-8"))
group = next(g for g in data["wild_encounter_groups"] if g["label"] == "gWildMonHeaders")
existing = {
    mon["species"]
    for encounter in group["encounters"]
    for payload in encounter.values()
    if isinstance(payload, dict) and isinstance(payload.get("mons"), list)
    for mon in payload["mons"]
}
# Rare visitors (src/data/wally_rare_visitors.h) are real wild encounters too.
import re as _re
_vis = (ROOT / "src/data/wally_rare_visitors.h").read_text(encoding="utf-8")
VISITOR_SPECIES = set(_re.findall(r"VISITOR\(MAP_\w+,\s*\w+,\s*(SPECIES_\w+)", _vis))
existing |= VISITOR_SPECIES
baseline = {
    line.strip()
    for line in (ROOT / "tools/wally_wild_species_baseline.txt").read_text(encoding="utf-8").splitlines()
    if line.strip() and not line.startswith("#")
}
missing = sorted(baseline - existing)
added = sorted(existing - baseline)
print(f"Wild species: {len(existing)}; baseline: {len(baseline)}; new: {len(added)}; missing: {len(missing)}")
for species in missing:
    print("MISSING:", species)
for species in added:
    print("NEW:", species)
if missing:
    raise SystemExit("Wild species coverage regressed; review and update placements or baseline.")
