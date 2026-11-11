#!/usr/bin/env python3
"""Turn an mGBA harness script (N | N KEYS | shot NAME) into a WJPC_SCRIPT."""
import sys
outdir = sys.argv[2]
for line in open(sys.argv[1]):
    p = line.split()
    if not p:
        continue
    if p[0] == 'shot':
        print(f'shot {outdir}/{p[1].split("/")[-1]}.ppm')
    elif len(p) == 1 and p[0].isdigit():
        print(f'wait {p[0]}')
    elif len(p) == 2 and p[0].isdigit():
        print(f'press {p[1]} {p[0]}')
print('quit')
