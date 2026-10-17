#!/usr/bin/env python3
"""Regression guard: important solo evolution items remain purchasable.

This checks source definitions only; it does not compile or playtest the ROM.
Run: python3 tools/check_wally_slateport_evolution_shop.py
"""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SHOP = ROOT / "data/maps/SlateportCity_Mart/scripts.inc"
REQUIRED = {
    "ITEM_LINKING_CORD",
    "ITEM_METAL_COAT",
    "ITEM_DRAGON_SCALE",
    "ITEM_KINGS_ROCK",
}
source = SHOP.read_text(encoding="utf-8")
match = re.search(
    r"(?m)^SlateportCity_Mart_Pokemart:\s*\n(.*?)^\s*pokemartlistend",
    source,
    re.DOTALL | re.MULTILINE,
)
if not match:
    raise SystemExit("FAIL: Slateport Pokemart inventory missing")
entries = set(re.findall(r"(?m)^\s*\.2byte\s+(ITEM_[A-Z0-9_]+)\s*$", match.group(1)))
missing = sorted(REQUIRED - entries)
if missing:
    raise SystemExit("FAIL: Missing shop items: " + ", ".join(missing))
print(f"PASS: Slateport Pokemart stocks {len(REQUIRED)} required evolution items")
