# usage: autoclean.py capture.ppm out_400x240.png
# Paints out what the HUD-less 3DS battle still shows: the blank cream text windows
# (exact colour 255,255,222) and the wild Pokemon's drop shadow (fixed spot).
import sys, cv2, numpy as np
from PIL import Image
im = np.asarray(Image.open(sys.argv[1]).convert('RGB').crop((0, 0, 400, 240))).copy()
mask = np.all(im == (255, 255, 222), axis=-1).astype(np.uint8) * 255
# keep only real bars (big blobs), not stray cream pixels in the scenery
n, lab, stats, _ = cv2.connectedComponentsWithStats(mask)
mask[:] = 0
for i in range(1, n):
    if stats[i, cv2.CC_STAT_AREA] >= 200:
        mask[lab == i] = 255
# foe drop shadow. On tall grass: copy the same rows from further left (same depth, so grass lines
# up). On plain ground (env SHADOW=blur): inpaint, which leaves smooth ground.
import os
if os.environ.get('SHADOW') == 'right':
    im[108:134, 234:310] = im[108:134, 234 + 60:310 + 60]
elif os.environ.get('SHADOW') == 'blur':
    cv2.ellipse(mask, (272, 121), (34, 10), 0, 0, 360, 255, -1)
else:
    im[108:134, 234:310] = im[108:134, 234 - 84:310 - 84]
mask = cv2.dilate(mask, np.ones((5, 5), np.uint8))
# Bars in the upper half (not covered by our own health boxes in game): copy the strip just
# above, so leaves / grass continue instead of blurring.
n2, lab2, st2, _ = cv2.connectedComponentsWithStats(np.all(im == (255, 255, 222), axis=-1).astype(np.uint8))
for i in range(1, n2):
    x, y, w, h, area = st2[i]
    if area >= 200 and y < 100:
        x0, y0, x1, y1 = x - 2, y - 2, x + w + 2, y + h + 2
        hh = y1 - y0
        im[y0:y1, x0:x1] = im[y0 - hh:y1 - hh, x0:x1]
        mask[y0:y1, x0:x1] = 0
        # soften the two seams a little
        for yy in (y0, y1 - 1):
            mask[max(0, yy - 1):yy + 2, x0:x1] = 255
bgr = cv2.cvtColor(im, cv2.COLOR_RGB2BGR)
out = cv2.inpaint(bgr, mask, 6, cv2.INPAINT_TELEA)
Image.fromarray(cv2.cvtColor(out, cv2.COLOR_BGR2RGB)).save(sys.argv[2])
Image.fromarray(mask).save(sys.argv[2].replace('.png', '_mask.png'))
