# usage: fix_white.py in400.png out400.png [ymin]
# The 3DS draws the space past the map's edge white. Fill white blobs in the lower part of the
# frame by copying the scenery straight above them (floor patterns continue), then soften seams.
import sys, cv2, numpy as np
from PIL import Image
im = np.asarray(Image.open(sys.argv[1]).convert('RGB')).copy()
ymin = int(sys.argv[3]) if len(sys.argv) > 3 else 120
white = (np.all(im >= 225, axis=-1)).astype(np.uint8)
white[:ymin] = 0
n, lab, st, _ = cv2.connectedComponentsWithStats(white)
seam = np.zeros(white.shape, np.uint8)
for i in range(1, n):
    x, y, w, h, a = st[i]
    if a < 60: continue
    m = (lab == i)
    m = cv2.dilate(m.astype(np.uint8), np.ones((5, 5), np.uint8)).astype(bool)
    ys, xs = np.nonzero(m)
    y0, y1 = ys.min(), ys.max() + 1
    d = (y1 - y0) + 2
    for yy, xx in zip(ys, xs):
        sy = yy - d
        while sy >= 0 and m[sy, xx]: sy -= d
        if sy >= 0: im[yy, xx] = im[sy, xx]
    seam[y0 - 1:y0 + 2, max(0, xs.min() - 2):xs.max() + 3] = 255
bgr = cv2.cvtColor(im, cv2.COLOR_RGB2BGR)
out = cv2.inpaint(bgr, seam, 3, cv2.INPAINT_TELEA)
Image.fromarray(cv2.cvtColor(out, cv2.COLOR_BGR2RGB)).save(sys.argv[2])
