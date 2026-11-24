#!/usr/bin/env python3
"""usage: holes.py JPGDIR [MIN_PX]

Region audit 2026-11-23. For the gap scan's 960-wide pictures (F7: magenta wherever nothing is drawn): the magenta
areas that do not touch the picture's edge - holes inside the scene (outdoors: sky through the ground or a wall;
indoors: the black void through a floor or wall, which shows as a black line or hole in play). Magenta touching the
edge is the open sky / the void round a room and is left out. Prints per picture: holes (pixels at 960 wide, box)."""
import glob
import os
import sys

import numpy as np
from PIL import Image
from scipy import ndimage

D = sys.argv[1]
MIN = int(sys.argv[2]) if len(sys.argv) > 2 else 2

for f in sorted(glob.glob(os.path.join(D, '*.jpg'))):
    a = np.asarray(Image.open(f).convert('RGB')).astype(int)
    m = (a[:, :, 0] > 215) & (a[:, :, 1] < 45) & (a[:, :, 2] > 215)
    lab, n = ndimage.label(m)
    if n == 0:
        continue
    h, w = m.shape
    edge = set(np.unique(np.concatenate([lab[0], lab[-1], lab[:, 0], lab[:, -1]]))) - {0}
    out = []
    for i, sl in enumerate(ndimage.find_objects(lab), 1):
        if i in edge:
            continue
        size = int((lab[sl] == i).sum())
        if size >= MIN:
            out.append((size, sl[1].start, sl[0].start, sl[1].stop, sl[0].stop))
    if out:
        out.sort(reverse=True)
        print(os.path.basename(f)[:-4], sum(o[0] for o in out), ' '.join('%d@(%d,%d,%d,%d)' % o for o in out[:4]))
