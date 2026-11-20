#!/bin/bash
# Rebuild Mt Pyre's relief edits (summit levels, sea fills, NE corner) from relief.bin as it was before them
# (commit 9474b4a1~1), all steps in order. Run from the repo root. TESTDIR: a test folder to copy it into (optional).
set -e
S=$(mktemp -d)
git show 9474b4a1~1:pc_data/voxel/relief.bin > pc_data/voxel/relief.bin
python3 tools/voxel3ds/mtpyre_summit_levels.py pc_data/voxel/relief.bin
for e in N S W E; do python3 tools/voxel3ds/add_relief_edge.py pc_data/voxel/relief.bin 303 $e 0x0A1 --top -64 >/dev/null; python3 tools/voxel3ds/add_relief_edge.py pc_data/voxel/relief.bin 302 $e 0x0A1 --top -256 >/dev/null; done
python3 tools/voxel3ds/drop_relief_cuts.py pc_data/voxel/relief.bin 303 48,8
python3 tools/voxel3ds/blend_relief_region.py pc_data/voxel/relief.bin 303 48 48 8 8
python3 - <<'PY'
import struct
p='pc_data/voxel/relief.bin'
b=bytearray(open(p,'rb').read());n=struct.unpack_from('<H',b,4)[0]
for i in range(n):
    lid,cells,ww,hh,off=struct.unpack_from('<HHHHI',b,8+14*i)
    if lid!=303: continue
    at={(b[off+k*27],b[off+k*27+1]):off+k*27+2 for k in range(cells)}
    west=[struct.unpack_from('<b',b,at[(48,9)]+j*5+4)[0]*2 for j in range(5)]
    for j in range(5):
        for k in range(5):
            struct.pack_into('<b',b,at[(49,9)]+j*5+k,int(round((west[j]+(-48-west[j])*k/4.0)/2)))
open(p,'wb').write(b)
PY
[ -n "$TESTDIR" ] && cp pc_data/voxel/relief.bin "$TESTDIR/voxel/" || true
