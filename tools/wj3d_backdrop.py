#!/usr/bin/env python3
"""Wally's Journey: turn an Emerald 3DS 3D-battle capture into a GBA battle background.

    python3 tools/wj3d_backdrop.py capture.ppm out_dir [--rows 14]

capture.ppm is the 3DS top screen (400x240, or a 400x480 top+bottom frame) of a
battle with the battlers and HUD blanked. Emerald 3DS draws the GBA battle picture
at 1.4x with GBA (120,112) on screen (200,192) (3ds_video.h CTR_BATTLE_ZOOM), so the
GBA screen is the 336x205 box from (32,35); it is scaled to 240 wide.

Writes out_dir/tiles.png (indexed, 16 tiles wide; pixel values are the 4bpp indices),
tilemap.bin (32x32 entries, the BG3 screen) and palette.pal (3 palettes, loaded to
BG palettes 2-4 like the stock backgrounds). Only the top `rows` tile rows are
drawn; the battle text box covers the rest. At most 511 unique tiles (char block 2).
"""
import sys, os, struct
from PIL import Image
import numpy as np

NPAL = 3
PAL_BASE = 2          # stock battle backgrounds use BG palettes 2..4

def load_capture(path):
    im = Image.open(path).convert('RGB')
    if im.height == 480:
        im = im.crop((0, 0, 400, 240))
    if im.size == (400, 240):
        box = (32, 35.2, 32 + 240 * 1.4, 240)
        im = im.resize((240, round((240 - 35.2) / 1.4)), Image.LANCZOS, box=box)
        full = Image.new('RGB', (240, 160))
        full.paste(im, (0, 0))
        im = full
    if im.size != (240, 160):
        sys.exit('expected a 400x240 3DS frame or a 240x160 image')
    return np.asarray(im).astype(np.float32)

def to5(c):
    return np.clip(np.round(c / 255 * 31), 0, 31)

def kmeans(points, k, iters=20, seed=1):
    rng = np.random.default_rng(seed)
    uniq = np.unique(points, axis=0)
    if len(uniq) <= k:
        return uniq
    cent = uniq[rng.choice(len(uniq), k, replace=False)]
    for _ in range(iters):
        d = ((points[:, None, :] - cent[None]) ** 2).sum(-1)
        lab = d.argmin(1)
        for j in range(k):
            m = points[lab == j]
            if len(m):
                cent[j] = m.mean(0)
    return np.round(cent)

def main():
    src, out = sys.argv[1], sys.argv[2]
    rows = int(sys.argv[sys.argv.index('--rows') + 1]) if '--rows' in sys.argv else 14
    os.makedirs(out, exist_ok=True)
    img = to5(load_capture(src))[: rows * 8]          # 5-bit colour, only the visible rows
    tiles = [img[ty*8:(ty+1)*8, tx*8:(tx+1)*8].reshape(-1, 3) for ty in range(rows) for tx in range(30)]
    # group tiles by their average colour, then give each group its own 15 colours
    means = np.array([t.mean(0) for t in tiles])
    group = kmeans(means, NPAL)
    lab = ((means[:, None] - group[None]) ** 2).sum(-1).argmin(1)
    for _ in range(3):
        pals = []
        for g in range(NPAL):
            px = np.concatenate([tiles[i] for i in range(len(tiles)) if lab[i] == g] or [np.zeros((1, 3))])
            pals.append(kmeans(px, 15))
        # each tile takes the palette that draws it best
        err = np.array([[((t[:, None] - p[None]) ** 2).sum(-1).min(1).sum() for p in pals] for t in tiles])
        lab = err.argmin(1)
    pals = [np.pad(p, ((0, 15 - len(p)), (0, 0))) for p in pals]
    # index every pixel, de-duplicate tiles (tile 0 = blank, under the text box)
    uniq = {bytes(64): 0}
    tilelist = [np.zeros(64, np.uint8)]
    tmap = np.zeros((32, 32), np.uint16)
    for i, t in enumerate(tiles):
        p = pals[lab[i]]
        idx = (((t[:, None] - p[None]) ** 2).sum(-1).argmin(1) + 1).astype(np.uint8)
        key = idx.tobytes()
        if key not in uniq:
            uniq[key] = len(tilelist)
            tilelist.append(idx)
        ty, tx = divmod(i, 30)
        tmap[ty, tx] = uniq[key] | ((PAL_BASE + lab[i]) << 12)
    for ty in range(rows, 32):
        for tx in range(32):
            tmap[ty, tx] = PAL_BASE << 12
    if len(tilelist) > 511:
        sys.exit('too many tiles: %d' % len(tilelist))
    n = len(tilelist)
    w = 16
    sheet = np.zeros((((n + w - 1) // w) * 8, w * 8), np.uint8)
    for i, t in enumerate(tilelist):
        r, c = divmod(i, w)
        sheet[r*8:(r+1)*8, c*8:(c+1)*8] = t.reshape(8, 8)
    im = Image.fromarray(sheet, 'P')
    flat = [0, 0, 0] + [int(v) * 255 // 31 for v in pals[0].reshape(-1)]
    im.putpalette(flat + [0] * (768 - len(flat)))
    im.save(os.path.join(out, 'tiles.png'))
    open(os.path.join(out, 'tilemap.bin'), 'wb').write(tmap.astype('<u2').tobytes())
    with open(os.path.join(out, 'palette.pal'), 'w') as f:
        f.write('JASC-PAL\r\n0100\r\n%d\r\n' % (16 * NPAL))
        for p in pals:
            f.write('0 0 0\r\n')
            for c in p:
                f.write('%d %d %d\r\n' % tuple(int(v) * 255 // 31 for v in c))
    # a preview of exactly what the GBA will draw
    prev = np.zeros((160, 240, 3), np.uint8)
    for i in range(len(tiles)):
        ty, tx = divmod(i, 30)
        e = tmap[ty, tx]; t = tilelist[e & 0x3FF].reshape(8, 8); p = pals[(e >> 12) - PAL_BASE]
        prev[ty*8:(ty+1)*8, tx*8:(tx+1)*8] = (p[t - 1] * 255 // 31).astype(np.uint8)
    Image.fromarray(prev).save(os.path.join(out, 'preview.png'))
    print('%s: %d tiles, %d palettes' % (out, n, NPAL))

if __name__ == '__main__':
    main()
