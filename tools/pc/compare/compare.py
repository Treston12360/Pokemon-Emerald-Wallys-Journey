#!/usr/bin/env python3
"""GBA vs PC check (PC port plan step 5).

Runs the same save and the same frame-exact inputs on the GBA ROM (mGBA harness) and on the PC build
(test mode, no widescreen), with the same fixed clock and the same random-number state, then compares
the screenshots taken at each `shot`.

usage: compare.py STEPS SAVE OUTDIR [--rom pokeemerald-release.gba] [--pc ./wallys-journey]
STEPS lines (harness format):  N            wait N frames
                                N KEYS       hold KEYS (A B S=start s=select U D L R l r) for N frames
                                shot NAME    screenshot on both
                                sync         same random-number state on both (after loading a save)
Writes OUTDIR/report.txt and OUTDIR/<name>_gba.png, _pc.png, _diff.png for each shot.
"""
import argparse, os, shutil, struct, subprocess, sys
from PIL import Image, ImageChops

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
EPOCH = 1793404800  # fixed clock on both sides (2026-10-31 00:00 UTC, shown in local time)
BURST = 24
RNG = (0x12345678, 0x9ABCDEF0, 0x0F1E2D3C, 0x00000001)

ELF = os.path.join(REPO, 'pokeemerald-release.elf')

def gba_addr(sym):
    out = subprocess.run(['arm-none-eabi-nm', ELF], capture_output=True, text=True).stdout
    for l in out.splitlines():
        p = l.split()
        if len(p) == 3 and p[2] == sym:
            return int(p[0], 16)
    sys.exit('no symbol ' + sym)

def main():
    global ELF
    ap = argparse.ArgumentParser()
    ap.add_argument('steps'); ap.add_argument('save'); ap.add_argument('out')
    ap.add_argument('--rom', default=os.path.join(REPO, 'pokeemerald-release.gba'))
    ap.add_argument('--pc', default=os.path.join(REPO, 'wallys-journey'))
    ap.add_argument('--harness', default='/home/claude/tools-src/harness_cmp')
    ap.add_argument('--epoch', type=int, default=EPOCH, help='clock (Unix seconds, shown in local time) on both sides')
    ap.add_argument('--pc-only', action='store_true', help='only run the PC side and report errors the guard skipped (fast sweep)')
    ap.add_argument('--vblank-lock', dest='lock', action='store_true', help='reseed RNG every VBlank (patched ROM)')
    ap.add_argument('--elf', default=ELF, help='symbols of --rom (a frozen copy lets test ROMs be built meanwhile)')
    a = ap.parse_args()
    ELF = a.elf
    out = os.path.abspath(a.out)
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out + '/gba'); os.makedirs(out + '/pc')
    rng, rng2 = gba_addr('gRngValue'), gba_addr('gRng2Value')

    hs, ps, shots, rnglabels = [], [], [], []
    for line in open(a.steps):
        p = line.split()
        if not p or p[0].startswith('#'):
            continue
        if p[0] == 'shot':
            # A burst of BURST frames on each side; the closest pair is compared, so the same picture
            # reached a few frames earlier or later (lag, animation phase) isn't reported.
            burst = int(p[2]) if len(p) > 2 else BURST  # "shot NAME 120": wider window for animations
            shots.append((p[1], burst))
            for i in range(burst):
                hs.append(f'shot gba/{p[1]}_{i:02d}')  # harness paths must be short
                ps.append(f'shot {out}/pc/{p[1]}_{i:02d}.ppm')
                if i + 1 < burst:
                    hs.append('1'); ps.append('wait 1')
        elif p[0] == 'rng':
            # "rng LABEL": random-number state on both sides (gba: stderr PEEK, pc: rng.log)
            hs.append(f'peek {rng:08X}'); ps.append(f'peekrng {p[1]}')
            rnglabels.append(p[1])
        elif p[0] == 'lock':
            # "lock on" / "lock off": same random state at the start of every frame on both sides
            # optional "lock on A B C CTR" (hex) picks the state
            on = p[1] == 'on'
            val = ' ' + ' '.join(p[2:6]) if on and len(p) >= 6 else ''
            hs.append(f'lockrng {rng:08X}{val}' if on else 'lockrng 0'); ps.append(f'lockrng {1 if on else 0}{val}')
        elif p[0] == 'sync':
            for base in (rng, rng2):
                for i, v in enumerate(RNG):
                    hs.append(f'poke32 {base + 4 * i:08X} {v:08X}')
            vals = ' '.join(f'{v:X}' for v in RNG)
            ps.append(f'setrng {vals}'); ps.append(f'setrng2 {vals}')
        elif len(p) == 1:
            hs.append(p[0]); ps.append(f'wait {p[0]}')
        else:
            hs.append(f'{p[0]} {p[1]}'); ps.append(f'press {p[1]} {p[0]}')
    ps.append('quit')
    open(out + '/gba.txt', 'w').write('\n'.join(hs) + '\n')
    open(out + '/pc.txt', 'w').write('\n'.join(ps) + '\n')
    shutil.copy(a.save, out + '/gba/g.sav'); shutil.copy(a.save, out + '/pc/g.sav')

    env = dict(os.environ)  # both sides read the fixed clock in the local time zone
    # Exact logic comparison: no once-per-VBlank random roll on either side (the GBA gets extra rolls
    # on frames where it lags, the PC never lags). GBA: a test copy of the ROM with that call removed.
    rom = os.path.abspath(a.rom)
    dis = subprocess.run(['arm-none-eabi-objdump', '-d', ELF,
                          '--start-address=0x%x' % gba_addr('VBlankIntr'), '--stop-address=0x%x' % (gba_addr('VBlankIntr') + 0x400)],
                         capture_output=True, text=True).stdout
    calls = [l for l in dis.splitlines() if '<Random32>' in l and '\tbl\t' in l]
    if len(calls) != 1:
        sys.exit('expected one Random32 call in VBlankIntr, found %d' % len(calls))
    off = int(calls[0].split(':')[0], 16) - 0x08000000
    data = bytearray(open(rom, 'rb').read())
    if a.lock:
        # Instead of the roll: SeedRng(0x55) every VBlank (PC: WJPC_RNG_LOCK does the same), so the same
        # game action gets the same random numbers on both. The 8 bytes before the call are the
        # "if (sRngLoopUnlocked)" check: movs r0,#0x55; nop; nop; bl SeedRng; nop.
        start = off - 8
        addr = 0x08000000 + start + 6           # where the BL goes
        rel = (gba_addr('SeedRng') & ~1) - (addr + 4)
        bl = struct.pack('<HH', 0xF000 | ((rel >> 12) & 0x7FF), 0xF800 | ((rel >> 1) & 0x7FF))
        data[start:start + 12] = b'\x55\x20' + b'\xc0\x46' * 2 + bl + b'\xc0\x46'  # 12 bytes, keeps the branch back
    else:
        data[off:off + 4] = b'\xc0\x46\xc0\x46'  # two Thumb NOPs
    rom = out + '/test_no_vblank_rng.gba'
    open(rom, 'wb').write(data)
    if a.pc_only:
        penv = dict(env, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy', WJPC_SCRIPT=out + '/pc.txt', WJPC_SAVE='g.sav',
                    WJPC_NOWIDE='1', WJPC_RTC_EPOCH=str(a.epoch), WJPC_NO_VBLANK_RNG='1')
        r = subprocess.run(['timeout', '-s', 'KILL', '1800', a.pc], cwd=out + '/pc', env=penv, capture_output=True, text=True, errors='replace')
        errs = open(out + '/pc/WallysJourney_errors.txt').read() if os.path.exists(out + '/pc/WallysJourney_errors.txt') else ''
        print('pc exit', r.returncode, '| errors:', errs.strip() or 'none')
        return
    hr = subprocess.run([a.harness, rom, 'gba.txt', str(a.epoch), 'gba/g.sav'], cwd=out, env=env,
                        stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True, errors='replace', timeout=1800)
    gpeeks = [l for l in hr.stderr.splitlines() if l.startswith('PEEK')]
    penv = dict(env, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy', WJPC_SCRIPT=out + '/pc.txt', WJPC_SAVE='g.sav',
                WJPC_NOWIDE='1', WJPC_RTC_EPOCH=str(a.epoch), WJPC_NO_VBLANK_RNG='1', **({'WJPC_RNG_LOCK': '1'} if a.lock else {}))
    r = subprocess.run(['timeout', '-s', 'KILL', '1800', a.pc], cwd=out + '/pc', env=penv, capture_output=True, text=True, errors='replace')
    skipped = sorted(set(l for l in (r.stdout + r.stderr).splitlines() if 'WJPC: skipped' in l or 'crash' in l))

    import numpy as np
    def load(path):
        return (np.asarray(Image.open(path).convert('RGB'), dtype=np.uint8) >> 3)  # 5-bit colour
    report = []
    for s, burst in shots:
        g = [out + f'/gba/{s}_{i:02d}.ppm' for i in range(burst)]
        pc = [out + f'/pc/{s}_{i:02d}.ppm' for i in range(burst)]
        if not all(os.path.exists(x) for x in g + pc):
            report.append(f'{s}: MISSING')
            continue
        G = [load(x) for x in g]; P = [load(x) for x in pc]
        # mGBA (32-bit colour build) blends and fades with 8-bit channels; the GBA and the PC use 5-bit
        # maths, so blended pixels can be one step apart. Those are counted separately ("rounding").
        best = None
        for i, gi in enumerate(G):
            for j, pj in enumerate(P):
                dd = np.abs(gi.astype(int) - pj.astype(int)).max(axis=2)
                n = int(np.count_nonzero(dd > 1)); rnd = int(np.count_nonzero(dd == 1))
                if best is None or (n, rnd) < best[:2]:
                    best = (n, rnd, i, j)
        n, rnd, i, j = best
        report.append(f'{s}: ' + ('same' if n == 0 else f'DIFFERENT ({n} pixels)')
                      + (f' +{rnd} rounding' if rnd else '') + f' (gba frame +{i}, pc frame +{j})')
        Image.open(g[i]).save(out + f'/{s}_gba.png'); Image.open(pc[j]).save(out + f'/{s}_pc.png')
        if n:
            mask = np.abs(G[i].astype(int) - P[j].astype(int)).max(axis=2) > 1
            Image.fromarray((mask * 255).astype(np.uint8)).save(out + f'/{s}_diff.png')
    if rnglabels:
        plog = open(out + '/pc/rng.log').read().splitlines() if os.path.exists(out + '/pc/rng.log') else []
        for k, lab in enumerate(rnglabels):
            gb = gpeeks[k].split(':')[1].split()[:12] if k < len(gpeeks) else []
            gwords = ['%02X%02X%02X%02X' % tuple(int(x, 16) for x in gb[i*4:i*4+4][::-1]) for i in range(3)] if len(gb) == 12 else []
            pw = plog[k].split()[1:4] if k < len(plog) else []
            report.append(f'rng {lab}: gba {" ".join(gwords)}  pc {" ".join(pw)}  ' + ('same' if gwords == pw else 'DIFFERENT'))
    report += ['pc exit ' + str(r.returncode)] + skipped
    open(out + '/report.txt', 'w').write('\n'.join(report) + '\n')
    print('\n'.join(report))

main()
