#!/usr/bin/env python3
"""Wally's Journey: which Gen 1-9 species can a single save actually obtain?

Counts as obtainable (static source analysis):
  * a wild slot on an Emerald map (FRLG-only "_Frlg" maps are unreachable here),
  * a gift / static encounter in an Emerald map script (givemon, setwildbattle,
    createmon-style specials are approximated by givemon and setwildbattle),
  * evolving an obtainable species (any EVO_* edge, items assumed purchasable or
    findable; see audit_wally_item_sources.py for item proof),
  * breeding: the lowest pre-evolution of any obtainable species that can breed.

Target set: every species whose base family member is not legendary, mythical,
sub/restricted legendary, Ultra Beast or Paradox (owner requirement), excluding
Mega/Primal/Gigantamax and other battle-only forms.

Run:  python3 tools/audit_wally_obtainability.py [--json]
This is a static check. It does NOT replace emulator verification.
"""
import argparse
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FAMILY_DIR = ROOT / "src/data/pokemon/species_info"
EVO = re.compile(r"\{\s*(EVO_[A-Z0-9_]+)\s*,\s*([^,{}]+?)\s*,\s*(SPECIES_[A-Z0-9_]+)(?:\s*##\s*\w+)?\s*(?=[,}])")
ENTRY = re.compile(r"\[\s*(SPECIES_[A-Z0-9_]+)\s*\]\s*=\s*\n?\s*\{")
EXCLUDE_FLAGS = ("isLegendary", "isMythical", "isSubLegendary", "isRestrictedLegendary",
                 "isUltraBeast", "isParadox")
FORM_FLAGS = ("isMegaEvolution", "isPrimalReversion", "isGigantamax", "isTeraForm",
              "isUltraBurst", "isTotem", "isAlolanTotem", "cannotBeTraded")


def _expand_form_macros(text):
    """Inline evolutions from per-form macros (e.g. FLABEBE_MISC_INFO(Red, RED, ...))."""
    macros = {}
    for m in re.finditer(r"#define (\w+)\(([^)]*)\)((?:[^\n]*\\\n)+[^\n]*)", text):
        if ".evolutions" in m.group(3):
            macros[m.group(1)] = ([x.strip() for x in m.group(2).split(",")], m.group(3))
    def sub(call):
        name, args = call.group(1), [x.strip() for x in call.group(2).split(",")]
        params, body = macros[name]
        evo = re.search(r"\.evolutions\s*=\s*EVOLUTION\((.*?\})\)", body, re.S)
        if not evo:
            return call.group(0)
        seg = evo.group(1)
        for prm, arg in zip(params, args):
            seg = re.sub(rf"\s*##\s*{prm}\b", arg, seg)
        return call.group(0) + "\n        .evolutions = EVOLUTION(" + seg + "),"
    if macros:
        text = re.sub(r"\b(" + "|".join(map(re.escape, macros)) + r")\(([^()]*)\)", sub, text)
    return text


def parse_species():
    info = {}
    for gen in range(1, 10):
        text = _expand_form_macros((FAMILY_DIR / f"gen_{gen}_families.h").read_text(encoding="utf-8"))
        starts = list(ENTRY.finditer(text))
        for i, m in enumerate(starts):
            body = text[m.end(): starts[i + 1].start() if i + 1 < len(starts) else len(text)]
            sp = m.group(1)
            evos = []
            em = re.search(r"\.evolutions\s*=\s*EVOLUTION\(", body)
            if em:
                seg = re.split(r"\n\s*\.[a-zA-Z_]+\s*=", body[em.end():], maxsplit=1)[0]
                evos = [(a, b.strip(), c) for a, b, c in EVO.findall(seg)]
                # Form macros paste the form onto the target (e.g. SPECIES_FLOETTE_ ##FORM).
                if "##" in seg:
                    form = sp.split("_", 2)[-1] if sp.count("_") >= 2 else ""
                    evos = [(a, b, c + form if c.endswith("_") else c) for a, b, c in evos]
            info[sp] = {
                "gen": gen,
                "excluded": any(re.search(rf"\.{f}\s*=\s*TRUE", body) for f in EXCLUDE_FLAGS),
                "battle_form": any(re.search(rf"\.{f}\s*=\s*TRUE", body) for f in FORM_FLAGS),
                "no_egg": "EGG_GROUP_NO_EGGS_DISCOVERED" in body,
                "evos": evos,
            }
    return info


def emerald_maps():
    ids = set()
    for p in (ROOT / "data/maps").glob("*/map.json"):
        if p.parent.name.endswith("_Frlg"):
            continue
        j = json.loads(p.read_text(encoding="utf-8"))
        ids.add(j.get("id"))
    return ids


def wild_species(maps):
    j = json.loads((ROOT / "src/data/wild_encounters.json").read_text(encoding="utf-8"))
    group = next(g for g in j["wild_encounter_groups"] if g["label"] == "gWildMonHeaders")
    found = {}
    for e in group["encounters"]:
        if e.get("map") not in maps:
            continue
        for k, v in e.items():
            if isinstance(v, dict) and isinstance(v.get("mons"), list):
                for mon in v["mons"]:
                    found.setdefault(mon["species"], set()).add(e["map"])
    return found


def visitor_species():
    """Species from the rare-visitor table (src/data/wally_rare_visitors.h)."""
    f = ROOT / "src/data/wally_rare_visitors.h"
    if not f.exists():
        return set()
    return set(re.findall(r"VISITOR\(\s*MAP_\w+\s*,\s*WILD_AREA_\w+\s*,\s*(SPECIES_\w+)", f.read_text(encoding="utf-8")))


def script_species():
    out = set()
    for p in list((ROOT / "data/maps").glob("*/scripts.inc")) + list((ROOT / "data/scripts").glob("*.inc")):
        if "_Frlg" in p.parent.name or "frlg" in p.name:
            continue
        t = p.read_text(encoding="utf-8")
        out |= set(re.findall(r"(?:givemon|setwildbattle|giveegg)\s+(SPECIES_[A-Z0-9_]+)", t))
    return out


def run():
    info = parse_species()
    maps = emerald_maps()
    wild = wild_species(maps)
    gifts = script_species()
    pre = {}
    for sp, d in info.items():
        for _, _, tgt in d["evos"]:
            pre.setdefault(tgt, sp)

    def root(sp):
        seen = set()
        while sp in pre and sp not in seen:
            seen.add(sp)
            sp = pre[sp]
        return sp

    visitors = visitor_species()
    have = set(wild) | gifts | visitors
    changed = True
    while changed:
        changed = False
        for sp in list(have):
            d = info.get(sp)
            if not d:
                continue
            for _, _, tgt in d["evos"]:
                if tgt not in have:
                    have.add(tgt); changed = True
            # Breeding: a parent that can breed hatches its family's lowest stage
            # (babies themselves are in the No Eggs group, so check the parent).
            r = root(sp)
            if r not in have and not d["no_egg"]:
                have.add(r); changed = True

    nat = {}
    for gen in range(1, 10):
        text = (FAMILY_DIR / f"gen_{gen}_families.h").read_text(encoding="utf-8")
        starts = list(ENTRY.finditer(text))
        for i, m in enumerate(starts):
            body = text[m.end(): starts[i + 1].start() if i + 1 < len(starts) else len(text)]
            n = re.search(r"\.natDexNum\s*=\s*(NATIONAL_DEX_\w+)", body)
            if n:
                nat[m.group(1)] = n.group(1)
    # Forms built from macros may hide natDexNum; derive it from the species name.
    known = set(nat.values())
    for sp in info:
        if sp not in nat:
            parts = sp[len("SPECIES_"):].split("_")
            for k in range(len(parts), 0, -1):
                cand = "NATIONAL_DEX_" + "_".join(parts[:k])
                if cand in known:
                    nat[sp] = cand
                    break
    target = sorted(sp for sp, d in info.items()
                    if not d["battle_form"] and not info.get(root(sp), d)["excluded"] and not d["excluded"]
                    and sp != "SPECIES_NONE" and not sp.startswith("SPECIES_EGG"))
    missing = [sp for sp in target if sp not in have]
    # Pokedex view: a dex number counts once any of its forms is obtainable.
    dex = {}
    for sp in target:
        if sp in nat:
            dex.setdefault(nat[sp], []).append(sp)
    missing_dex = sorted(n for n, forms in dex.items() if all(f not in have for f in forms))
    families = {}
    for sp in missing:
        families.setdefault(root(sp), []).append(sp)
    return {
        "target_species": len(target), "obtainable": len(target) - len(missing),
        "missing_count": len(missing), "missing_families": families,
        "dex_numbers": len(dex), "missing_dex": missing_dex,
        "wild_species_on_emerald_maps": len(wild), "gift_or_static_species": sorted(gifts),
    }


if __name__ == "__main__":
    ap = argparse.ArgumentParser(); ap.add_argument("--json", action="store_true")
    a = ap.parse_args(); r = run()
    if a.json:
        print(json.dumps(r, indent=2, default=list))
    else:
        print(f"Target species: {r['target_species']}  obtainable: {r['obtainable']}  missing: {r['missing_count']}")
        print(f"Wild species on Emerald maps: {r['wild_species_on_emerald_maps']}")
        print(f"Pokedex numbers: {r['dex_numbers']}  with no obtainable form: {len(r['missing_dex'])}")
        print("  " + " ".join(n.replace("NATIONAL_DEX_", "") for n in r["missing_dex"]))
        print("Species/forms still missing (includes cosmetic and regional forms):")
        for fam, sps in sorted(r["missing_families"].items()):
            print(f"  {fam}: {' '.join(s.replace('SPECIES_', '') for s in sps)}")
