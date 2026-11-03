#!/usr/bin/env python3
"""Read a Wally's Journey .sav and show which important trainers have locked a scaled level.

    python3 tools/check_scaling_save.py path/to/game.sav

The first-encounter scaling (0.8.9+) saves each important fight's level in SaveBlock3
(`wjScaleMagic` 0xB1 + `wjScaleLock[]`, slot numbers from src/data/wj_trainer_scaling.h).
A save made only on 0.8.8 or earlier has no marker. The value stored is the ace's level
(for Wally: his lowest Pokemon's level; from 0.8.11 the whole team is at least that).
Offsets are for SaveBlock3 as of 0.8.11; re-check with offsetof() if SaveBlock3 changes.
"""
import os, re, struct, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SB3_MAGIC_OFF, SB3_LOCK_OFF, SB3_LOCKS = 0x5F3, 0x5F4, 48
CHUNK, DATA = 116, 3968

def slots(data):
    res = {}
    for off in range(0, len(data) - 4095, 4096):
        sid, chk, sig, cnt = struct.unpack_from('<HHII', data, off + 4084)
        if sig == 0x08012025 and sid < 14:
            res.setdefault(cnt, {})[sid] = data[off:off + 4096]
    return res

def labels():
    out = {}
    h = open(os.path.join(ROOT, 'src/data/wj_trainer_scaling.h')).read()
    for m in re.finditer(r'\{(TRAINER_\w+), (\d+), (\d+), (\d+)(?:, (\d+))?\}, // (.+)', h):
        out.setdefault(int(m.group(4)), (m.group(6).strip(), int(m.group(2)), int(m.group(3))))
    return out

def main(path):
    d = open(path, 'rb').read()
    S = slots(d)
    if not S:
        print('no save slots found'); return
    names = labels()
    for cnt in sorted(S, reverse=True):
        s = S[cnt]
        if not all(i in s for i in range(14)):
            continue
        sb3 = b''.join(s[i][DATA:DATA + CHUNK] for i in range(14))
        magic = sb3[SB3_MAGIC_OFF]
        print(f'save #{cnt} (newest)' if cnt == max(S) else f'save #{cnt} (backup)')
        if magic != 0xB1:
            print('  no scaling marker: this save has never started an important fight on 0.8.9 or newer')
            continue
        locks = sb3[SB3_LOCK_OFF:SB3_LOCK_OFF + SB3_LOCKS]
        any_ = False
        for slot, (label, minimum, margin) in sorted(names.items()):
            if locks[slot]:
                any_ = True
                what = 'lowest' if label.startswith('Wally') else 'ace'
                print(f'  {label:28s} locked: {what} Lv{locks[slot]:3d}   (minimum {minimum}, margin +{margin})')
        if not any_:
            print('  scaling marker set, but no fight locked yet')

if __name__ == '__main__':
    for p in sys.argv[1:]:
        print('==', os.path.basename(p)); main(p)
