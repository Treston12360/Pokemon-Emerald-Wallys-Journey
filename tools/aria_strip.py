#!/usr/bin/env python3
"""Project ARIA S0: take FireRed's own story triggers out of a Kanto map.
usage: tools/aria_strip.py MAPDIR SCRIPT [SCRIPT ...]
Removes every coord (trigger) event and every ON_FRAME `map_script_2` entry that runs one of the named
scripts. The scripts themselves stay in scripts.inc (unused) so diffs stay small and they can be restored.
Every removal is listed in docs/KANTO_S0_STRIPPED.md by the caller (see that file)."""
import json, re, sys
d, names = sys.argv[1], set(sys.argv[2:])
p = d + '/map.json'
m = json.load(open(p))
before = len(m.get('coord_events', []))
m['coord_events'] = [c for c in m.get('coord_events', []) if c.get('script') not in names]
removed = before - len(m['coord_events'])
open(p, 'w').write(json.dumps(m, indent=2, ensure_ascii=False) + '\n')
s = open(d + '/scripts.inc').read()
n2 = 0
for nm in names:
    s, k = re.subn(r'^\tmap_script_2 [^\n]*, ' + re.escape(nm) + r'\n', '', s, flags=re.M)
    n2 += k
open(d + '/scripts.inc', 'w').write(s)
print('%s: %d trigger(s), %d frame script(s) removed' % (d, removed, n2))
