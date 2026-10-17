#!/usr/bin/env python3
"""Inventory GBA script dialogue for story-continuity review.

This is a source-level candidate finder, not a guarantee that an NPC or
dialogue branch is reachable at a particular story stage.
"""
from __future__ import annotations

from pathlib import Path
import re

LABEL = re.compile(r"^([A-Za-z_]\w*):{1,2}\s*$")
STRING = re.compile(r'^\s*\.string\s+"(.*)"\s*$')
PAGE = re.compile(r"\\[npl]")
TITLE_WORDS = ("WALLY", "WALLACE", "CHAMPION")
GIFT_PHRASES = ("HIDDEN MACHINE", "HM", "GIVE YOU", "PRESENT FOR YOU")

def collect():
    files = sorted(Path("data/maps").rglob("scripts.inc"))
    files += sorted(Path("data/scripts").rglob("*.inc"))
    blocks = []
    for path in files:
        label = None
        fragments = []
        def finish():
            if label and fragments:
                blocks.append((str(path), label, " ".join(fragments)))
        for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
            match = LABEL.match(line)
            if match:
                finish()
                label, fragments = match.group(1), []
                continue
            match = STRING.match(line)
            if match and label:
                fragments.append(match.group(1))
        finish()
    return files, blocks

def clean(s):
    return PAGE.sub(" / ", s).replace(r"\$", "").replace("$", "").strip()

def main():
    files, blocks = collect()
    print(f"DIALOGUE INVENTORY: {len(files)} script files, {len(blocks)} labeled text blocks")
    title = [(path, label, msg) for path, label, msg in blocks
             if "CHAMPION" in msg.upper() and
             ("WALLY" in msg.upper() or "WALLACE" in msg.upper())]
    gifts = [(path, label, msg) for path, label, msg in blocks
             if ("HIDDEN MACHINE" in msg.upper() or re.search(r"\bHM\d*\b", msg.upper()))
             and any(t in msg.upper() for t in ("GIVE", "GIFT", "PRESENT", "TAKE", "RECEIVED", "OBTAINED"))]
    print(f"TITLE-CONTINUITY CANDIDATES: {len(title)}")
    for path, label, msg in title[:200]:
        print(f"TITLE: {path} :: {label} :: {clean(msg)[:280]}")
    if len(title) > 200:
        print(f"TITLE: {len(title) - 200} additional candidates not displayed")
    print(f"HM-GIFT WORDING CANDIDATES: {len(gifts)}")
    for path, label, msg in gifts[:75]:
        print(f"HM: {path} :: {label} :: {clean(msg)[:250]}")
    if len(gifts) > 75:
        print(f"HM: {len(gifts) - 75} additional candidates not displayed")
    print("NOTE: Candidate lines may be unused, pre-broadcast, or valid historical references.")
    print("Every potentially problematic result requires event-gate and emulator verification.")

if __name__ == "__main__":
    main()
