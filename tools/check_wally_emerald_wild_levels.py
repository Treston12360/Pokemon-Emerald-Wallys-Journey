#!/usr/bin/env python3
"""Wally's Journey: wild Pokemon on reachable Emerald maps must never appear below
the level at which they evolve under this project's level-evolution rules.
--fix replaces an underleveled evolved Pokemon with its pre-evolution
(owner rule: keep the earlier stage; the evolved stage needs no wild slot)."""
import json, sys
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import audit_wally_obtainability as A

def main(fix):
    info = A.parse_species()
    need = {}  # target -> (pre, level) for plain level evolutions
    for sp, d in info.items():
        for m, req, tgt in d["evos"]:
            if m == "EVO_LEVEL" and req.isdigit():
                lv = int(req)
                if tgt not in need or lv < need[tgt][1]:
                    need[tgt] = (sp, lv)
    data = json.loads((ROOT / "src/data/wild_encounters.json").read_text(encoding="utf-8"))
    group = next(g for g in data["wild_encounter_groups"] if g["label"] == "gWildMonHeaders")
    maps = A.emerald_maps(); bad = 0
    for e in group["encounters"]:
        if e["map"] not in maps: continue
        for k, v in e.items():
            if not k.endswith("_mons"): continue
            for i, mon in enumerate(v["mons"]):
                sp = mon["species"]
                # walk down until the slot's level is legal
                while sp in need and mon["min_level"] < need[sp][1]:
                    bad += 1
                    print(f"{e['map']} {k}[{i}] {sp} Lv{mon['min_level']}-{mon['max_level']} < evolves at {need[sp][1]}")
                    if not fix: break
                    sp = need[sp][0]
                if fix: mon["species"] = sp
    if fix:
        (ROOT / "src/data/wild_encounters.json").write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print("underleveled slots:", bad)
    return bad

if __name__ == "__main__":
    sys.exit(1 if main("--fix" in sys.argv) and "--fix" not in sys.argv else 0)
