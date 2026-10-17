#!/usr/bin/env python3
"""Wally's Journey: dialogue lint for every `.string` text in data/**/*.inc.

Checks each text label for problems that break or garble text on hardware:
  BACKSLASH  a doubled backslash (\\\\p, \\\\n) - fails to assemble
  BLANKLINE  a paragraph that starts with \\n or \\l (empty first line)
  OVERFLOW   a paragraph that uses \\n twice (third line drawn outside the box)
  WIDTH      a line wider than the 208px message window (FONT_NORMAL widths,
             {PLAYER} counted as 7 wide letters)
  NOEND      a text with no $ terminator
  KANTO      the player's family is from Johto, not Kanto

Usage: python3 tools/lint_wally_dialogue.py [--base DIR] [--all]
  --base DIR   a vanilla pokeemerald-expansion checkout; only labels whose text
               differs from vanilla (new or rewritten) are reported
  --all        report every label, vanilla ones included
Exit status 1 when a reported label has an error.
"""
import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LIMIT = 208
LABEL = re.compile(r'^([A-Za-z0-9_]+)::?\s*$')
STRING = re.compile(r'^\s*\.string\s+"(.*)"\s*$')
# texts shown in windows that are not the 2-line field message box
MULTILINE_OK = re.compile(r'(Sign|Pokedex|Description|Rules|Info|Explanation|Brain|Menu|List|Card|Ribbon|Record|Results|Scroll|Plaque|Poster|Bulletin|Notice|Board|Diploma|Tv|TV_)', re.I)


def load_widths():
    text = (ROOT / "src/fonts.c").read_text()
    m = re.search(r'gFontNormalLatinGlyphWidths\[\] = \{(.*?)\};', text, re.S)
    widths = [int(x) for x in re.findall(r'\d+', m.group(1))]
    cmap = {}
    for line in (ROOT / "charmap.txt").read_text(encoding="utf-8").splitlines():
        mm = re.match(r"^'(.+)'\s*=\s*([0-9A-F]{2})\s*$", line)
        if mm:
            cmap.setdefault(mm.group(1), int(mm.group(2), 16))
    return widths, cmap


WIDTHS, CMAP = load_widths()
PLACEHOLDER_W = {"PLAYER": 42, "RIVAL": 42, "KUN": 0, "STR_VAR_1": 48, "STR_VAR_2": 48, "STR_VAR_3": 48}


def width(line):
    w = 0
    i = 0
    while i < len(line):
        if line[i] == '{':
            j = line.index('}', i)
            tok = line[i + 1:j].split()[0]
            w += PLACEHOLDER_W.get(tok, 0)
            i = j + 1
            continue
        c = line[i]
        b = CMAP.get(c)
        w += WIDTHS[b] if b is not None and b < len(WIDTHS) else 6
        i += 1
    return w


def parse(root):
    texts = {}
    for p in sorted((root / "data").rglob("*.inc")):
        cur = None
        buf = []
        for raw in p.read_text(encoding="utf-8", errors="replace").splitlines():
            m = LABEL.match(raw)
            if m:
                if cur and buf:
                    texts[cur] = (p.relative_to(root).as_posix(), "".join(buf))
                cur, buf = m.group(1), []
                continue
            s = STRING.match(raw)
            if s and cur:
                buf.append(s.group(1))
            elif raw.strip() and not raw.strip().startswith("@") and buf:
                texts[cur] = (p.relative_to(root).as_posix(), "".join(buf))
                cur, buf = None, []
        if cur and buf:
            texts[cur] = (p.relative_to(root).as_posix(), "".join(buf))
    return texts


def lint(label, s):
    errs = []
    if "\\\\" in s:
        errs.append("BACKSLASH")
    if "$" not in s:
        errs.append("NOEND")
    if re.search(r'kanto', s, re.I) and re.search(r'(PLAYER|NORMAN|DAD|MOM|your family|we came|our old home)', s):
        errs.append("KANTO")
    body = s.split("$")[0]
    multiline_ok = bool(MULTILINE_OK.search(label))
    for para in re.split(r'\\p', body):
        if para.startswith("\\n") or para.startswith("\\l"):
            errs.append("BLANKLINE")
        if not multiline_ok and para.count("\\n") >= 2:
            errs.append("OVERFLOW")
        for line in re.split(r'\\[nl]', para):
            line = line.replace("\\", "")
            if width(line) > LIMIT:
                errs.append(f"WIDTH({width(line)}px:{line[:40]})")
    return sorted(set(errs))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--base")
    ap.add_argument("--all", action="store_true")
    a = ap.parse_args()
    cur = parse(ROOT)
    base = parse(Path(a.base)) if a.base else {}
    bad = 0
    changed = 0
    for label, (path, s) in sorted(cur.items(), key=lambda kv: kv[1][0]):
        if not a.all and base and label in base and base[label][1] == s:
            continue
        changed += 1
        errs = lint(label, s)
        if errs:
            bad += 1
            print(f"{path}: {label}: {', '.join(errs)}")
    print(f"texts checked: {changed}  with problems: {bad}", file=sys.stderr)
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
