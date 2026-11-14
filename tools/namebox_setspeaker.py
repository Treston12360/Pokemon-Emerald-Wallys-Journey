#!/usr/bin/env python3
"""ENG-NAMEBOX (Project ARIA): put `setspeaker SP_NAME_X` in front of reviewed msgbox lines.

Input: a list of "Map:line SPEAKER" (or "@path:line SPEAKER") lines, each checked by hand against the script
(who is really talking). Line numbers are the msgbox lines in the file as it was when the list was written;
the tool works bottom-up so they stay valid, and refuses a line that is not a msgbox/message.
Usage: python3 tools/namebox_setspeaker.py LIST
"""
import sys, collections
todo = collections.defaultdict(list)
for ln in open(sys.argv[1]):
    ln = ln.strip()
    if not ln: continue
    where, sp = ln.split()
    path, line = where.rsplit(':', 1)
    path = path[1:] if path.startswith('@') else f'data/maps/{path}/scripts.inc'
    todo[path].append((int(line), sp))
n = 0
for path, items in todo.items():
    lines = open(path, encoding='utf-8').read().split('\n')
    for line, sp in sorted(items, reverse=True):
        cur = lines[line - 1]
        assert cur.strip().startswith(('msgbox', 'message')), (path, line, cur)
        indent = cur[:len(cur) - len(cur.lstrip())]
        if lines[line - 2].strip().startswith('setspeaker'):
            continue
        lines.insert(line - 1, f'{indent}setspeaker SP_NAME_{sp}')
        n += 1
    open(path, 'w', encoding='utf-8').write('\n'.join(lines))
print(n, 'setspeaker lines added')
