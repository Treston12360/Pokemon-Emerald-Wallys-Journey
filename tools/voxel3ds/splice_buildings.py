#!/usr/bin/env python3
"""Copy one building placement (its model, page, heights...) from one buildings.bin (VXB9) into another.

usage: splice_buildings.py OURS.bin DONOR.bin OUT.bin LAYOUT_ID X Y
       splice_buildings.py --layout OURS.bin DONOR.bin OUT.bin LAYOUT_ID   (a whole room: all its pieces)

Wally's Journey (POL-36, 2026-11-15): the Weather Institute's model on Route 119 was made with
3ds_port/scripts (spec "weather_institute_exterior"), but a whole buildings.bin made here does not
match the one in pc_data/voxel (that one was made in another session with more models). So the
generator's output is used only as the DONOR of this one placement: its model, vertices, heights,
footprints + masks, quarters, its page (copied whole as a new page at the end) and the ground
variants of its layout that OURS lacks. Everything in OURS stays byte-for-byte; the page offsets
and the animation table's offset move with the bytes added. With no match it stops.
Format: src/platform/voxel3d/voxel_building.c (header comment).
"""
import struct, sys


def u16(b, o): return struct.unpack_from('<H', b, o)[0]
def u32(b, o): return struct.unpack_from('<I', b, o)[0]


def parse(path):
    b = open(path, 'rb').read()
    assert b[:4] == b'VXB9'
    pages, models, pms, places, hb, masks = struct.unpack_from('<6H', b, 4)
    verts = u32(b, 16)
    variants, decks, conds = struct.unpack_from('<3H', b, 20)
    o = 28
    def rows(n, size):
        nonlocal o
        out = [bytearray(b[o + size * i: o + size * (i + 1)]) for i in range(n)]
        o += size * n
        return out
    f = {}
    f['pages'] = rows(pages, 8)
    f['models'] = rows(models, 16)
    f['pms'] = rows(pms, 8)
    f['places'] = rows(places, 16)
    f['heights'] = bytearray(b[o:o + hb]); o += hb
    o += hb & 1
    f['foot'] = list(struct.unpack_from('<%dH' % hb, b, o)); o += 2 * hb
    f['masks'] = rows(masks, 32)
    f['quarters'] = bytearray(b[o:o + hb]); o += hb
    o += hb & 1
    f['variants'] = rows(variants, 6)
    f['decks'] = rows(decks, 8)
    f['conds'] = rows(conds, 6)
    o += (4 - (o & 3)) & 3
    f['verts'] = rows(verts, 24)
    f['vert_end'] = o
    # page texels (+ glass masks)
    f['texels'] = []
    for p in f['pages']:
        w, h, off = u16(p, 0), u16(p, 2), u32(p, 4)
        n = w * (h & 0x7FFF) * 2 + (w * (h & 0x7FFF) if h & 0x8000 else 0)
        f['texels'].append(b[off:off + n])
    # what lies between the vertices and the end: the texels, then maybe the ANIM table
    f['anim'] = None
    if b[-4:] == b'ANIM':
        at = u32(b, len(b) - 8)
        f['anim'] = b[at:len(b) - 8]
    f['raw'] = b
    return f


def texel(f, page, x, y):
    w = u16(f['pages'][page], 0)
    i = ((y // 8) * (w // 8) + x // 8) * 64 + ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1)
                                               | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3))
    return u16(f['texels'][page], 2 * i)


def check_same(a, pa, pma, b, pb, pmb):
    """The model's drawing on page pa at pma's (ox, oy) is the one on pb at pmb's."""
    ma, mb = a['models'][u16(pma, 0)], b['models'][u16(pmb, 0)]
    assert (ma[0], ma[1]) == (mb[0], mb[1])
    first, count = u32(ma, 4), u32(ma, 8) & 0x7FFFFFFF
    us = [struct.unpack_from('<f', a['verts'][k], 12)[0] for k in range(first, first + count)]
    vs = [struct.unpack_from('<f', a['verts'][k], 16)[0] for k in range(first, first + count)]
    ax, ay = struct.unpack_from('<hh', pma, 4)
    bx, by = struct.unpack_from('<hh', pmb, 4)
    bad = 0
    for v in range(int(min(vs)), int(max(vs))):
        for u in range(int(min(us)), int(max(us))):
            if texel(a, pa, ax + u, ay + v) != texel(b, pb, bx + u, by + v):
                bad += 1
    assert bad == 0, 'a drawing differs between the pages (%d texels)' % bad


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
    # texels: each page's in order, offsets fixed up
    page_at = 28
    for i, t in enumerate(f['texels']):
        struct.pack_into('<I', blob, page_at + 8 * i + 4, len(blob))
        blob += t
    if f['anim'] is not None:
        at = len(blob)
        blob += f['anim'] + struct.pack('<I', at) + b'ANIM'
    open(path, 'wb').write(blob)
    return len(blob)


def main():
    ours, donor = parse(sys.argv[1]), parse(sys.argv[2])
    lid, px, py = int(sys.argv[4]), int(sys.argv[5]), int(sys.argv[6])
    # sanity: OURS rewrites byte-for-byte when nothing is added (checked by --roundtrip)
    if len(sys.argv) > 7 and sys.argv[7] == '--roundtrip':
        n = write(ours, sys.argv[3]); print('round trip', n); return
    hit = [i for i, p in enumerate(donor['places']) if (u16(p, 0), u16(p, 4), u16(p, 6)) == (lid, px, py)]
    assert len(hit) == 1, 'donor placements at that spot: %d' % len(hit)
    for p in ours['places']:
        assert (u16(p, 0), u16(p, 4), u16(p, 6)) != (lid, px, py), 'OURS already has a placement there'
    dp = donor['places'][hit[0]]
    dpm = donor['pms'][u16(dp, 2)]
    dm = donor['models'][u16(dpm, 0)]
    w, h = dm[0], dm[1]
    first, count, hts = u32(dm, 4), u32(dm, 8), u32(dm, 12)
    static = count & 0x80000000
    count &= 0x7FFFFFFF
    # The renderer binds ONE page per map (VoxelBuildings_PageOf: its first placement's), so the
    # layout's page in OURS is replaced by the donor's page of that layout, which holds the same
    # drawings as ours plus the new one (checked texel by texel below); the pages' other models
    # move to where the donor's page has them.
    dpage = u16(dpm, 2)
    ours_on = [p for p in ours['places'] if u16(p, 0) == lid]
    opages = {u16(ours['pms'][u16(p, 2)], 2) for p in ours_on}
    assert len(opages) == 1, 'OURS layout pages: %r' % opages
    new_page = opages.pop()
    if ours['anim'] is not None:
        runs = u16(ours['anim'], 0)
        assert all(u16(ours['anim'], 12 + 20 * r) != new_page for r in range(runs)), 'page has animated runs'
    dmoved = {}
    for p in donor['places']:
        if u16(p, 0) == lid and u16(p, 2) != u16(dp, 2):
            dmoved[(u16(p, 4), u16(p, 6))] = donor['pms'][u16(p, 2)]
    fixed = set()
    for p in ours_on:
        pmi = u16(p, 2)
        d = dmoved[(u16(p, 4), u16(p, 6))]
        assert u16(d, 2) == dpage
        old = ours['pms'][pmi]
        if pmi in fixed:
            continue
        check_same(ours, new_page, old, donor, dpage, d)
        struct.pack_into('<hh', old, 4, *struct.unpack_from('<hh', d, 4))
        fixed.add(pmi)
    for k, pm in enumerate(ours['pms']):
        assert u16(pm, 2) != new_page or k in fixed, 'a page model of that page with no placement there'
    ours['pages'][new_page] = bytearray(donor['pages'][dpage])
    ours['texels'][new_page] = donor['texels'][dpage]
    page = ours['pages'][new_page]
    # vertices of the model
    new_first = len(ours['verts'])
    ours['verts'] += [bytearray(v) for v in donor['verts'][first:first + count]]
    # heights, footprints (masks copied), quarters
    new_hts = len(ours['heights'])
    ours['heights'] += donor['heights'][hts:hts + w * h]
    ours['quarters'] += donor['quarters'][hts:hts + w * h]
    for k in range(w * h):
        m = donor['foot'][hts + k]
        if m != 0xFFFF:
            ours['masks'].append(bytearray(donor['masks'][m]))
            m = len(ours['masks']) - 1
        ours['foot'].append(m)
    model = bytearray(dm)
    struct.pack_into('<III', model, 4, new_first, count | static, new_hts)
    new_model = len(ours['models'])
    ours['models'].append(model)
    pm = bytearray(dpm)
    struct.pack_into('<HH', pm, 0, new_model, new_page)
    new_pm = len(ours['pms'])
    ours['pms'].append(pm)
    # the placement's ground patches (extra vertices, uv in its page's pixels)
    place = bytearray(dp)
    extra_n, extra_first = u16(dp, 10), u32(dp, 12)
    new_extra = len(ours['verts'])
    ours['verts'] += [bytearray(v) for v in donor['verts'][extra_first:extra_first + extra_n]]
    struct.pack_into('<H', place, 2, new_pm)
    struct.pack_into('<I', place, 12, new_extra if extra_n else extra_first)
    # placements stay sorted by layout; conditions name placements by index
    at = len(ours['places'])
    for i, p in enumerate(ours['places']):
        if u16(p, 0) > lid:
            at = i
            break
    ours['places'].insert(at, place)
    for c in ours['conds']:
        if u16(c, 0) >= at:
            struct.pack_into('<H', c, 0, u16(c, 0) + 1)
    # ground variants of that layout OURS lacks
    have = {bytes(v) for v in ours['variants']}
    added = 0
    for v in donor['variants']:
        if u16(v, 0) == lid and bytes(v) not in have:
            ours['variants'].append(bytearray(v)); added += 1
    assert len(ours['heights']) < 65536
    n = write(ours, sys.argv[3])
    print('model %dx%d, %d vertices, %d extra, page %dx%d, %d variants added, %d bytes'
          % (w, h, count, extra_n, u16(page, 0), u16(page, 2) & 0x7FFF, added, n))



def splice_layout(ours, donor, lid):
    """Every placement of layout `lid` from DONOR (a room's pieces, rebuilt): OURS's placements of it
    go, DONOR's come with their models, vertices, heights, masks, ground patches and a page of their
    own (appended; the renderer binds one page per map, and all of them are on it). Conditions of the
    removed placements go too (none expected)."""
    dpl = [p for p in donor['places'] if u16(p, 0) == lid]
    pages = {u16(donor['pms'][u16(p, 2)], 2) for p in dpl}
    assert len(pages) == 1, pages
    dpage = pages.pop()
    keep = [i for i, p in enumerate(ours['places']) if u16(p, 0) != lid]
    gone = [i for i, p in enumerate(ours['places']) if u16(p, 0) == lid]
    assert not [c for c in ours['conds'] if u16(c, 0) in gone], 'conditions on removed placements'
    new_page = len(ours['pages'])
    ours['pages'].append(bytearray(donor['pages'][dpage]))
    ours['texels'].append(donor['texels'][dpage])
    model_map, pm_map = {}, {}
    added = []
    for p in dpl:
        dpi = u16(p, 2)
        if dpi not in pm_map:
            dpm = donor['pms'][dpi]
            dmi = u16(dpm, 0)
            if dmi not in model_map:
                dm = donor['models'][dmi]
                w, h = dm[0], dm[1]
                first, count, hts = u32(dm, 4), u32(dm, 8), u32(dm, 12)
                static = count & 0x80000000
                count &= 0x7FFFFFFF
                nf = len(ours['verts'])
                ours['verts'] += [bytearray(v) for v in donor['verts'][first:first + count]]
                nh = len(ours['heights'])
                ours['heights'] += donor['heights'][hts:hts + w * h]
                ours['quarters'] += donor['quarters'][hts:hts + w * h]
                for k in range(w * h):
                    m = donor['foot'][hts + k]
                    if m != 0xFFFF:
                        ours['masks'].append(bytearray(donor['masks'][m]))
                        m = len(ours['masks']) - 1
                    ours['foot'].append(m)
                model = bytearray(dm)
                struct.pack_into('<III', model, 4, nf, count | static, nh)
                model_map[dmi] = len(ours['models'])
                ours['models'].append(model)
            pm = bytearray(dpm)
            struct.pack_into('<HH', pm, 0, model_map[dmi], new_page)
            pm_map[dpi] = len(ours['pms'])
            ours['pms'].append(pm)
        place = bytearray(p)
        extra_n, extra_first = u16(p, 10), u32(p, 12)
        ne = len(ours['verts'])
        ours['verts'] += [bytearray(v) for v in donor['verts'][extra_first:extra_first + extra_n]]
        struct.pack_into('<H', place, 2, pm_map[dpi])
        struct.pack_into('<I', place, 12, ne if extra_n else extra_first)
        added.append(place)
    # rebuild the placement list: OURS's others in order, the layout's new ones where its old ones were
    old_index = {}
    places = []
    at = None
    for i, p in enumerate(ours['places']):
        if u16(p, 0) == lid:
            if at is None:
                at = len(places)
            continue
        old_index[i] = len(places)
        places.append(p)
    if at is None:
        at = next((k for k, p in enumerate(places) if u16(p, 0) > lid), len(places))
    places[at:at] = added
    for c in ours['conds']:
        oi = u16(c, 0)
        ni = old_index[oi]
        if ni >= at:
            ni += len(added)
        struct.pack_into('<H', c, 0, ni)
    ours['places'] = places
    have = {bytes(v) for v in ours['variants']}
    for v in donor['variants']:
        if u16(v, 0) == lid and bytes(v) not in have:
            ours['variants'].append(bytearray(v))
    # decks of the layout (none expected for a room)
    return len(added), len(gone)


if __name__ == '__main__':
    if len(sys.argv) > 1 and sys.argv[1] == '--layout':
        o, d = parse(sys.argv[2]), parse(sys.argv[3])
        print('placements in %d, out %d' % splice_layout(o, d, int(sys.argv[5])))
        print(write(o, sys.argv[4]), 'bytes')
    else:
        main()
