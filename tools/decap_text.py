#!/usr/bin/env python3
"""Wally's Journey: turn the game's ALL-CAPS words into normal capitalisation
(POKéMON -> Pokémon, PROF. BIRCH -> Prof. Birch, GYM LEADER -> Gym Leader).

Only text is touched:
  * `.string "..."` lines in data/**/*.inc and data/**/*.s
  * `_("...")` and `COMPOUND_STRING("...")` strings in src/**/*.c and src/**/*.h
  * `Name:` lines in src/data/trainers.party
Anything inside {braces} (placeholders, colour codes) is left alone, as are acronyms.

    python3 tools/decap_text.py            # rewrite in place
    python3 tools/decap_text.py --dry      # print what would change, write nothing
    python3 tools/decap_text.py --sample N # print N random changed lines
"""
import glob, os, random, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

KEEP = {  # acronyms and symbols that stay upper case
    'HP', 'PP', 'TM', 'TMS', 'HM', 'HMS', 'TV', 'PC', 'OK', 'EXP', 'IV', 'IVS', 'EV', 'EVS', 'ID', 'IDNO',
    'HQ', 'KO', 'PKMN', 'DNA', 'UFO', 'SS', 'TMHM', 'NPC', 'AM', 'PM', 'GBA', 'EX', 'VS', 'LV', 'CD', 'DS',
    'OT', 'BP', 'SE', 'NE', 'NW', 'SW', 'II', 'III', 'IV', 'VI', 'VII', 'VIII', 'IX', 'XI', 'XII',
    'TRAINERCARD',
}
SPECIAL = {
    'POKéMON': 'Pokémon', 'POKEMON': 'Pokémon', 'POKéDEX': 'Pokédex', 'POKéNAV': 'PokéNav',
    'POKéBLOCK': 'Pokéblock', 'POKEBLOCK': 'Pokéblock', 'POKéBLOCKS': 'Pokéblocks', 'POKEBLOCKS': 'Pokéblocks',
    'POKéMART': 'Poké Mart', 'POKé': 'Poké', 'POKéDOLL': 'Poké Doll',
    'DEXNAV': 'DexNav', 'TRAINERHILL': 'Trainer Hill', 'TVS': 'TVs', 'PCS': 'PCs', 'HMS': 'HMs', 'TMS': 'TMs',
    'IVS': 'IVs', 'EVS': 'EVs', 'MCPHEE': 'McPhee', 'MACHOP': 'Machop',
}

LETTERS = 'A-Za-zé'
TOKEN = re.compile(r"(?<![%s])([A-Zé][A-Zé]+)('S)?(?![a-zé])" % LETTERS)


def fix_word(m):
    w, poss = m.group(1), m.group(2)
    # a word that is really lower-case-ish (contains no capitals besides é) never matches TOKEN
    if w in SPECIAL:
        out = SPECIAL[w]
    elif w in KEEP or w.rstrip('S') in KEEP and w.endswith('S') and len(w) <= 4:
        out = w
    else:
        out = w[0] + w[1:].lower()
    if poss:
        out += "'s"
    return out


def fix_text(t):
    parts = re.split(r'(\{[^}]*\})', t)
    for i in range(0, len(parts), 2):  # even parts are outside braces
        parts[i] = TOKEN.sub(fix_word, parts[i])
    return ''.join(parts)


STRING_LINE = re.compile(r'^(\s*\.string\s+")(.*)("\s*)$')
C_STRING = re.compile(r'(_\(|COMPOUND_STRING\()(\s*)"((?:[^"\\]|\\.)*)"')
PARTY_NAME = re.compile(r'^(Name:\s*)(.*)$')


def process_file(path, changes):
    try:
        src = open(path, encoding='utf-8').read()
    except UnicodeDecodeError:
        return
    out = []
    changed = False
    for line in src.split('\n'):
        new = line
        if path.endswith(('.inc', '.s')):
            m = STRING_LINE.match(line)
            if m:
                new = m.group(1) + fix_text(m.group(2)) + m.group(3)
        elif path.endswith('trainers.party'):
            m = PARTY_NAME.match(line)
            if m:
                new = m.group(1) + fix_text(m.group(2))
        else:
            new = C_STRING.sub(lambda m: m.group(1) + m.group(2) + '"' + fix_text(m.group(3)) + '"', line)
        if new != line:
            changed = True
            changes.append((path, line, new))
        out.append(new)
    if changed and '--dry' not in sys.argv and '--sample' not in sys.argv:
        open(path, 'w', encoding='utf-8').write('\n'.join(out))


def main():
    os.chdir(ROOT)
    files = glob.glob('data/**/*.inc', recursive=True) + glob.glob('data/**/*.s', recursive=True)
    files += glob.glob('src/**/*.c', recursive=True) + glob.glob('src/**/*.h', recursive=True)
    files += ['src/data/trainers.party']
    files = [f for f in files if os.path.isfile(f) and '/test/' not in f]
    changes = []
    for f in sorted(set(files)):
        process_file(f, changes)
    print('%d lines in %d files' % (len(changes), len({c[0] for c in changes})))
    if '--sample' in sys.argv:
        n = int(sys.argv[sys.argv.index('--sample') + 1])
        random.seed(7)
        for path, a, b in random.sample(changes, min(n, len(changes))):
            print('--', path)
            print('   ' + a.strip())
            print('   ' + b.strip())
    elif '--dry' in sys.argv:
        for path, a, b in changes:
            print(path, '|', b.strip())


if __name__ == '__main__':
    main()
