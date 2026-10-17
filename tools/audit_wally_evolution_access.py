#!/usr/bin/env python3
"""Inventory every Gen 1-9 evolution requirement and flag accessibility work.

Run:
  python3 tools/audit_wally_evolution_access.py
  python3 tools/audit_wally_evolution_access.py --json > evolution_report.json

Static source analysis only: it does NOT prove that required items are
available, trades are supported, or every condition is reachable in a ROM.
"""
import argparse
import json
import re
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FAMILY_DIR = ROOT / "src/data/pokemon/species_info"
PATTERN = re.compile(
    r"\{\s*(EVO_[A-Z0-9_]+)\s*,\s*([^,{}]+?)\s*,\s*(SPECIES_[A-Z0-9_]+)\s*(?=[,}])"
)
SOURCE_PATTERN = re.compile(r"\[\s*(SPECIES_[A-Z0-9_]+)\s*\]\s*=")
# Conditions involving interactions, items and complex game state need
# explicit checks; avoid treating presence of an evolution rule as access.
SPECIAL_METHODS = {
    "EVO_TRADE", "EVO_ITEM", "EVO_ITEM_HOLD", "EVO_TRADE_ITEM",
    "EVO_TRADE_SPECIFIC_MON", "EVO_BEAUTY", "EVO_FRIENDSHIP",
}

def run():
    rows = []
    for generation in range(1, 10):
        file = FAMILY_DIR / f"gen_{generation}_families.h"
        data = file.read_text(encoding="utf-8")
        sources = list(SOURCE_PATTERN.finditer(data))
        for index, source in enumerate(sources):
            segment = data[source.end():sources[index+1].start() if index+1 < len(sources) else len(data)]
            # Evolution macro can span multiple lines and contain nested conditions.
            match = re.search(r"\.evolutions\s*=\s*EVOLUTION\(", segment)
            if not match:
                continue
            segment = segment[match.end():]
            # Stop at next struct field, rather than guessing the end from nested parens.
            segment = re.split(r"\n\s*\.[a-zA-Z_]+\s*=", segment, maxsplit=1)[0]
            for evo in PATTERN.finditer(segment):
                method, requirement, target = evo.groups()
                rows.append({"generation": generation, "source": source.group(1), "target": target,
                             "method": method, "requirement": requirement.strip(),
                             "needs_review": method in SPECIAL_METHODS or "TRADE" in method or "ITEM" in method})
    wild = json.loads((ROOT / "src/data/wild_encounters.json").read_text(encoding="utf-8"))
    group = next(x for x in wild["wild_encounter_groups"] if x["label"] == "gWildMonHeaders")
    present = {m["species"] for e in group["encounters"] for p in e.values()
               if isinstance(p, dict) and isinstance(p.get("mons"), list) for m in p["mons"]}
    for row in rows:
        row["source_in_main_wild"] = row["source"] in present
    by_method = Counter(row["method"] for row in rows)
    special = [row for row in rows if row["needs_review"]]
    return {"evolution_edges_parsed": len(rows), "methods": dict(sorted(by_method.items())),
            "review_edges": special, "review_count": len(special),
            "trade_edges": [x for x in rows if "TRADE" in x["method"]],
            "dusk_stone_edges": [x for x in rows if "DUSK_STONE" in x["requirement"]],
            "notes": [
                "Wild source species does not prove evolutionary obtainability.",
                "Check Dusk Stone and all other items against reachable maps, shops, and pickup/event scripts.",
                "Trade evolutions need an explicit single-player alternative or verified working trade path.",
                "Parser inventories conventional EVO_* edges; verify complex conditional/macro rules separately."
            ]}

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    result = run()
    if args.json:
        print(json.dumps(result, indent=2))
    else:
        print("Evolution edges:", result["evolution_edges_parsed"])
        print("Review needed:", result["review_count"])
        print("Trade edges:", len(result["trade_edges"]))
        print("Dusk Stone edges:", len(result["dusk_stone_edges"]))
        print("Methods:", result["methods"])
        print("Note: this audit inventories evolution definitions; it is not a gameplay accessibility certification.")
