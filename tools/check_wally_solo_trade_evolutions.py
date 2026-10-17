#!/usr/bin/env python3
"""Ensure all standard trade-evolution rules have a non-trade item alternative.

Static check on Gen 1-9 species_info family files. This verifies definitions,
NOT item accessibility. Run: python3 tools/check_wally_solo_trade_evolutions.py
"""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
FAMILY_DIR = ROOT / "src/data/pokemon/species_info"
SOURCE = re.compile(r"\[\s*(SPECIES_[A-Z0-9_]+)\s*\]\s*=")
TRADE = re.compile(r"\{\s*EVO_TRADE\s*,\s*[^,{}]+\s*,\s*(SPECIES_[A-Z0-9_]+)\b")
ITEM = re.compile(r"\{\s*EVO_ITEM\s*,\s*(ITEM_[A-Z0-9_]+)\s*,\s*(SPECIES_[A-Z0-9_]+)\b")

errors = []
checked = 0
requirements = set()
for generation in range(1, 10):
    file = FAMILY_DIR / f"gen_{generation}_families.h"
    data = file.read_text(encoding="utf-8")
    source_matches = list(SOURCE.finditer(data))
    for index, source in enumerate(source_matches):
        section = data[source.end():source_matches[index + 1].start() if index + 1 < len(source_matches) else len(data)]
        if ".evolutions" not in section:
            continue
        trade_targets = set(TRADE.findall(section))
        alternatives = {target: item for item, target in ITEM.findall(section)}
        for target in trade_targets:
            checked += 1
            if target not in alternatives:
                errors.append(f"Gen {generation} {source.group(1)} -> {target}: no EVO_ITEM alternative")
            else:
                requirements.add(alternatives[target])

for error in errors:
    print("FLAG:", error)
print(f"Trade targets checked: {checked}, missing solo alternatives: {len(errors)}")
print("Required items (accessibility not verified):", ", ".join(sorted(requirements)))
if errors:
    raise SystemExit(1)
