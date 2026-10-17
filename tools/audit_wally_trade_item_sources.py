#!/usr/bin/env python3
"""Find candidate in-game sources for items required by solo trade evolutions.

This is an inventory, NOT proof that a source is reachable or repeatable.
Run: python3 tools/audit_wally_trade_item_sources.py [--json]
"""
import argparse
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FAMILIES = ROOT / "src/data/pokemon/species_info"
TRADE = re.compile(r"\{\s*EVO_TRADE\s*,\s*[^,{}]+\s*,\s*(SPECIES_[A-Z0-9_]+)\b")
ITEM = re.compile(r"\{\s*EVO_ITEM\s*,\s*(ITEM_[A-Z0-9_]+)\s*,\s*(SPECIES_[A-Z0-9_]+)\b")
SPECIES = re.compile(r"\[\s*(SPECIES_[A-Z0-9_]+)\s*\]\s*=")
# Runtime-facing map scripts, shop inventories, and item-ball definitions.
# Exclude species definitions: those only prove the item is consumed.
SEARCH_ROOTS = [ROOT / "data/maps", ROOT / "data/scripts", ROOT / "src/data"]
SEARCH_SUFFIXES = {".inc", ".s", ".c", ".h", ".json"}
SOURCE_HINTS = ("shop", "mart", "item", "script", "map", "pickup")
SKIP_PARTS = {"pokemon", "battle_frontier", "trainers", "graphics"}
MAX_EXAMPLES = 30


def required_items():
    result = {}
    for generation in range(1, 10):
        path = FAMILIES / f"gen_{generation}_families.h"
        data = path.read_text(encoding="utf-8")
        entries = list(SPECIES.finditer(data))
        for i, entry in enumerate(entries):
            section = data[entry.end():entries[i + 1].start() if i + 1 < len(entries) else len(data)]
            if ".evolutions" not in section:
                continue
            alternatives = {}
            for item, target in ITEM.findall(section):
                alternatives.setdefault(target, set()).add(item)
            for target in set(TRADE.findall(section)):
                for item in alternatives.get(target, ()):
                    result.setdefault(item, set()).add(f"{entry.group(1)} -> {target}")
    return result


def candidate_files():
    seen = set()
    for root in SEARCH_ROOTS:
        if not root.exists():
            continue
        for path in root.rglob("*"):
            if not path.is_file() or path.suffix not in SEARCH_SUFFIXES:
                continue
            relative = path.relative_to(ROOT)
            if relative in seen or any(part in SKIP_PARTS for part in relative.parts):
                continue
            if root.name == "data" and not any(hint in path.name.lower() for hint in SOURCE_HINTS):
                continue
            seen.add(relative)
            yield path, relative


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    requirements = required_items()
    patterns = {item: re.compile(r"(?<![A-Z0-9_])" + re.escape(item) + r"(?![A-Z0-9_])")
                for item in requirements}
    results = {item: {"evolutions": sorted(evos), "candidate_sources": [], "total_matches": 0}
               for item, evos in sorted(requirements.items())}
    for path, relative in candidate_files():
        try:
            lines = path.read_text(encoding="utf-8").splitlines()
        except (UnicodeError, OSError):
            continue
        for lineno, line in enumerate(lines, 1):
            for item, pattern in patterns.items():
                if pattern.search(line):
                    results[item]["total_matches"] += 1
                    if len(results[item]["candidate_sources"]) < MAX_EXAMPLES:
                        results[item]["candidate_sources"].append({
                            "file": str(relative), "line": lineno, "text": line.strip()[:180]
                        })
    report = {
        "scope": "Static candidate references; not verified reachable, purchasable, or renewable",
        "items": results,
        "items_without_candidate_sources": [
            item for item, info in results.items() if info["total_matches"] == 0
        ],
    }
    if args.json:
        print(json.dumps(report, indent=2))
    else:
        for item, info in results.items():
            print(f"{item}: {info['total_matches']} candidate references; "
                  f"{len(info['evolutions'])} trade evolution(s)")
            for hit in info["candidate_sources"][:5]:
                print(f"  {hit['file']}:{hit['line']}: {hit['text']}")
        print("No candidate reference:", ", ".join(report["items_without_candidate_sources"]) or "none")
        print("NOTE: Every candidate requires a manual progression/repeatability check.")


if __name__ == "__main__":
    main()
