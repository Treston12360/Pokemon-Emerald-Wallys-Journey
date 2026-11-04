# Build an Emerald 3DS data pack whose battle sprites/HUD graphics are blank,
# so a battle shows only the 3D stage (for capturing backdrops).
import sys, re, struct, os
from pathlib import Path
PY = Path(sys.argv[1]); ROM = Path(sys.argv[2]); OUT = Path(sys.argv[3])
sys.path.insert(0, str(PY))
from emerald3ds_builder import build, pak
from emerald3ds_builder.recipe import lz77_decompress

BLANK = re.compile(os.environ.get('BLANK_RE', r'^graphics/pokemon/[^/]+/(front|back|anim_front)\.4bpp\.lz$'))


def lz_blank(n):
    # GBA LZ77 of n zero bytes: two literals, then 18-byte copies at distance 2 (VRAM-safe)
    out = bytearray([0x10, n & 255, (n >> 8) & 255, (n >> 16) & 255])
    toks = []; left = n
    lits = [1, 0, 0]  # one opaque pixel: all-empty pictures hang the 3DS port at boot
    for _ in range(min(3, left)): toks.append(('L',)); left -= 1
    while left > 0:
        k = min(18, left)
        if k >= 3: toks.append(('B', k)); left -= k
        else: toks.append(('L',)); left -= 1
    for i in range(0, len(toks), 8):
        grp = toks[i:i+8]; flags = 0; body = bytearray()
        for j, t in enumerate(grp):
            if t[0] == 'B':
                flags |= 0x80 >> j; body += bytes([((t[1] - 3) << 4) | 0, 1])
            else:
                body.append(lits[i + j] if i + j < 3 else 0)
        out.append(flags); out += body
    while len(out) % 4: out.append(0)
    return bytes(out)

hits = []
real = build.build_entry
def patched(entry, rom, literals, bitmaps, sources=None):
    data = real(entry, rom, literals, bitmaps) if sources is None else real(entry, rom, literals, bitmaps, sources)
    p = entry['path']
    if BLANK.match(p):
        hits.append(p)
        if p.endswith('.lz'):
            n = struct.unpack('<I', data[:4])[0] >> 8
            return lz_blank(n)
        return b'\x01' + bytes(len(data) - 1)
    return data
build.build_entry = patched
orig_abi = pak.engine_abi
want = {}
def abi(items):
    return want['abi']
pak.engine_abi = abi
from emerald3ds_builder.recipe import Recipe
pl = build.Payload(PY.parent / 'payload')
want['abi'] = Recipe.load(pl.recipe).engine_abi
info = build.build_pack(ROM, pl, OUT, follower_fetcher=True)
print('blanked', len(hits), 'files;', info['entries'], 'entries')
print('\n'.join(sorted(h for h in hits if 'pokemon/' not in h)))
