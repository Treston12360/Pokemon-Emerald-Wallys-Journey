#!/usr/bin/env python3
"""ENG-NAMEBOX (Project ARIA, 2026-11-14): turn "Name: " speaker prefixes in Hoenn texts into the green name box.

Emerald writes who is talking as a prefix ("Steven: Hi!"). The owner wants the green name box above the text box
for every important character instead. This tool replaces a prefix at the start of a page (start of a text, or
right after \\p) with {SPEAKER SPK_X}. The code only names that one message (see src/field_name_box.c), so a
following text without a code goes back to whatever speaker was set before.

Not touched:
  * names not in NAMES below (ordinary NPCs keep their "Name:" prefix);
  * texts used as a trainer's defeat/after text on a trainerbattle line (they are shown inside the battle,
    where there is no name box, so the prefix must stay readable there);
  * Kanto (_Frlg) maps and FireRed scripts. (match_call.inc is converted: on the PokéNav screen the code is
    simply skipped by text.c, in a call on the map it shows the green box.)

Usage: python3 tools/namebox_prefixes.py [--write]
"""
import re, glob, sys, os
NAMES = {
    'Tate&Liza': 'TATE_LIZA', 'Tate': 'TATE', 'Liza': 'LIZA', 'Wally': 'WALLY', 'May': 'MAY', 'Brendan': 'BRENDAN',
    'Steven': 'STEVEN', 'Dad': 'DAD', 'Norman': 'NORMAN', 'Scott': 'SCOTT', 'Prof. Birch': 'BIRCH', 'Mr. Briney': 'BRINEY',
    'Briney': 'BRINEY', 'Mom': 'MOM', 'Archie': 'ARCHIE', 'Maxie': 'MAXIE', 'Wattson': 'WATTSON', 'Wallace': 'WALLACE',
    'Juan': 'JUAN', 'Roxanne': 'ROXANNE', 'Capt. Stern': 'STERN', 'Winona': 'WINONA', 'Flannery': 'FLANNERY',
    'Brawly': 'BRAWLY', 'Mr. Stone': 'MR_STONE', 'Prof. Cozmo': 'COZMO', 'Sidney': 'SIDNEY', 'Phoebe': 'PHOEBE',
    'Glacia': 'GLACIA', 'Drake': 'DRAKE', 'Tabitha': 'TABITHA', 'Matt': 'MATT', 'Courtney': 'COURTNEY', 'Shelly': 'SHELLY',
}
ALT = '|'.join(re.escape(n) for n in sorted(NAMES, key=len, reverse=True))
PAGE_START = re.compile(r'^(\s*\.string ")(' + ALT + r'): ')
AFTER_P = re.compile(r'(\\p)(' + ALT + r'): ')

def files():
    out = [f for f in glob.glob('data/maps/*/scripts.inc') + glob.glob('data/maps/*/text.inc') if '_Frlg/' not in f]
    out += [f for f in glob.glob('data/scripts/*.inc') if 'frlg' not in f and 'aria_' not in f]
    out += ['data/event_scripts.s', 'data/text/berries.inc', 'data/text/pokedex_rating.inc', 'data/text/event_ticket_2.inc', 'data/text/match_call.inc']
    return [f for f in out if os.path.exists(f)]

def battle_texts(all_files):
    skip = set()
    for f in glob.glob('data/maps/*/scripts.inc') + glob.glob('data/scripts/*.inc'):
        for ln in open(f, encoding='utf-8'):
            if re.match(r'\s*trainerbattle', ln):
                labs = [a.strip() for a in ln.split(',') if re.fullmatch(r'\s*\w*Text\w*\s*', a)]
                skip.update(labs[1:2])  # [1] = defeat text (shown in battle); intro and "not enough Pokémon" are on the map
    return skip

def main(write):
    skip = battle_texts(files())
    total = 0; per = {}
    for f in files():
        lines = open(f, encoding='utf-8').read().split('\n')
        label = None; prev_ended_p = True; prev_ended_n = False; changed = False
        for i, ln in enumerate(lines):
            m = re.match(r'(\w+)::?\s*$', ln)
            if m:
                label = m.group(1); prev_ended_p = True; prev_ended_n = False; continue
            if '.string' not in ln:
                continue
            new = ln
            if label not in skip:
                if not prev_ended_p and prev_ended_n and PAGE_START.match(new):
                    # second speaker on line 2 of a page ("Tate: ...\n" / "Liza: ..."): one box shows one name,
                    # so line 2 becomes its own page.
                    lines[i - 1] = lines[i - 1].rstrip()[:-3] + '\\p"'
                    prev_ended_p = True; changed = True
                if prev_ended_p:
                    new = PAGE_START.sub(lambda m: m.group(1) + '{SPEAKER SPK_' + NAMES[m.group(2)] + '}', new)
                new = AFTER_P.sub(lambda m: m.group(1) + '{SPEAKER SPK_' + NAMES[m.group(2)] + '}', new)
            if new != ln:
                n = new.count('{SPEAKER') - ln.count('{SPEAKER'); total += n; per[f] = per.get(f, 0) + n
                lines[i] = new; changed = True
            body = ln.rstrip()
            prev_ended_p = body.endswith('\\p"') or body.endswith('$"')
            prev_ended_n = body.endswith('\\n"')
        if changed and write:
            open(f, 'w', encoding='utf-8').write('\n'.join(lines))
    for f, n in sorted(per.items(), key=lambda x: -x[1]):
        print(n, f)
    print('total', total, '(written)' if write else '(dry run)')

if __name__ == '__main__':
    main('--write' in sys.argv)
