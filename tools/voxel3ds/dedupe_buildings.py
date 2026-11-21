#!/usr/bin/env python3
"""Make buildings.bin (VXB9) smaller without changing what it draws: identical vertex runs and identical pages stored once.

usage: dedupe_buildings.py IN.bin OUT.bin

Session 014PwRDj, Brennan 2026-11-20 20:15 ("buildings.bin (53 MB): deduplicate"): of its 4244 models only about half
have vertices of their own - the rest are copies of another model's run (the same tree, shrub, fence or house placed
under another layout's id), 8 MB of the file, all read into memory when the game starts. Each model now points at the
first run with the same bytes; a placement's ground patches (extraFirst/extraCount) are kept as they are (moved with
the rest). Pages whose texels are byte-for-byte the same share one copy (their offset). Everything else is unchanged:
models, placements and pages keep their numbers. Check: --verify reads both files back and compares every model's
and placement's vertices and every page's texels.
Format: src/platform/voxel3d/voxel_building.c (header comment); the parse comes from splice_buildings.py.
"""
import struct, sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from splice_buildings import parse


def model_run(m):
    first, count = struct.unpack_from('<II', m, 4)
    return first, count & 0x7FFFFFFF


def place_run(p):
    count = struct.unpack_from('<H', p, 10)[0]
    first = struct.unpack_from('<I', p, 12)[0]
    return first, count


def dedupe(f):
    verts = f['verts']
    new = []
    seen = {}
    def take(first, count):
        key = b''.join(bytes(v) for v in verts[first:first + count])
        if count and key in seen:
            return seen[key]
        at = len(new)
        new.extend(verts[first:first + count])
        if count:
            seen[key] = at
        return at
    for m in f['models']:
        first, count = model_run(m)
        struct.pack_into('<I', m, 4, take(first, count) if count else 0)
    for p in f['places']:
        first, count = place_run(p)
        if count:
            struct.pack_into('<I', p, 12, take(first, count))
    f['verts'] = new


def write(f, path):
    head = bytearray(b'VXB9')
    head += struct.pack('<6HI4H', len(f['pages']), len(f['models']), len(f['pms']), len(f['places']),
                        len(f['heights']), len(f['masks']), len(f['verts']), len(f['variants']),
                        len(f['decks']), len(f['conds']), 0)
    body = bytearray()
    for name in ('pages', 'models', 'pms', 'places'):
        for r in f[name]:
            body += r
    body += f['heights']
    if len(f['heights']) & 1: body += b'\0'
    body += struct.pack('<%dH' % len(f['foot']), *f['foot'])
    for r in f['masks']: body += r
    body += f['quarters']
    if len(f['heights']) & 1: body += b'\0'
    for name in ('variants', 'decks', 'conds'):
        for r in f[name]:
            body += r
    blob = head + body
    blob += b'\0' * ((4 - (len(blob) & 3)) & 3)
    for r in f['verts']: blob += r
    page_at = 28
    where = {}
    for i, t in enumerate(f['texels']):
        key = bytes(t)
        if key not in where:
            where[key] = len(blob)
            blob += t
        struct.pack_into('<I', blob, page_at + 8 * i + 4, where[key])
    if f['anim'] is not None:
        at = len(blob)
        blob += f['anim'] + struct.pack('<I', at) + b'ANIM'
    open(path, 'wb').write(blob)
    return len(blob)


def verify(a_path, b_path):
    a, b = parse(a_path), parse(b_path)
    va = lambda f, first, count: b''.join(bytes(v) for v in f['verts'][first:first + count])
    for ma, mb in zip(a['models'], b['models']):
        (fa, ca), (fb, cb) = model_run(ma), model_run(mb)
        assert ca == cb and va(a, fa, ca) == va(b, fb, cb)
        assert ma[:4] == mb[:4] and ma[8:] == mb[8:]
    for pa, pb in zip(a['places'], b['places']):
        (fa, ca), (fb, cb) = place_run(pa), place_run(pb)
        assert ca == cb and va(a, fa, ca) == va(b, fb, cb) and pa[:12] == pb[:12]
    for i, (ta, tb) in enumerate(zip(a['texels'], b['texels'])):
        assert bytes(ta) == bytes(tb), 'page %d' % i
    for k in ('pms', 'heights', 'foot', 'masks', 'quarters', 'variants', 'decks', 'conds', 'anim'):
        assert (list(map(bytes, a[k])) if isinstance(a[k], list) and a[k] and isinstance(a[k][0], (bytes, bytearray)) else a[k]) == \
               (list(map(bytes, b[k])) if isinstance(b[k], list) and b[k] and isinstance(b[k][0], (bytes, bytearray)) else b[k]), k
    print('verified: same models, placements, pages')


def main():
    if sys.argv[1] == '--verify':
        verify(sys.argv[2], sys.argv[3])
        return
    f = parse(sys.argv[1])
    before = os.path.getsize(sys.argv[1])
    n0 = len(f['verts'])
    dedupe(f)
    size = write(f, sys.argv[2])
    print('vertices %d -> %d; file %.2f MB -> %.2f MB' % (n0, len(f['verts']), before / 1e6, size / 1e6))


if __name__ == '__main__':
    main()
