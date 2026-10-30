#!/usr/bin/env python3
"""Make every Emerald action sheet Emerald Rogue doesn't have for a protagonist, the way the
owner approved for Hilbert:
  running   - their walking frames leaned 1px forward
  surfing   - their seated (riding) art; their standing frame for hopping on/off
  bikes     - their seated art on the bike, legs pedalling in their own leg colour
  fishing / field move / watering / decorating - their standing art holding the item
Usage: make_outfit.py <name> <M|F> <outdir>"""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
import gen
from PIL import Image
from collections import Counter
NAME, GENDER, OUT = sys.argv[1], sys.argv[2], sys.argv[3]
TM = 'brendan' if GENDER == 'M' else 'may'
G = gen.G; B = G+'graphics/object_events/pics/people/%s/' % TM
SRC = gen.R+'graphics/object_events/pics/people/%s/' % NAME
pd = gen.R+'graphics/rogue_palettes/%s/' % gen.PALDIR.get(NAME, NAME)
pal = gen.readpal(pd+'object_event_base.pal'); flat = [v for c in pal for v in c]
def load(p):
    im = Image.open(p); ip = [tuple(im.getpalette()[i*3:i*3+3]) for i in range(16)]
    rm = [0]+[min(range(1, 16), key=lambda j: sum((a-b)**2 for a, b in zip(ip[i], pal[j]))) for i in range(1, 16)]
    out = im.point(lambda v: rm[v] if v < 16 else 0); out.putpalette(flat); return out
walk = load(SRC+'walking.png'); ride = load(SRC+'riding.png')
def near(c): return min(range(1, 16), key=lambda j: sum((a-b)**2 for a, b in zip(c, pal[j])))
def fr(img, i, w=16): return img.crop((i*w, 0, i*w+w, 32))
def dirf(img, d):
    f = fr(img, {'S': 0, 'N': 1, 'W': 2, 'E': 2}[d])
    return f.transpose(Image.FLIP_LEFT_RIGHT) if d == 'E' else f
def top(f):
    for y in range(32):
        if any(f.getpixel((x, y)) for x in range(f.width)): return y
def bottom(f):
    for y in range(31, -1, -1):
        if any(f.getpixel((x, y)) for x in range(f.width)): return y
def new(w): g = Image.new('P', (w, 32), 0); g.putpalette(flat); return g
# their leg colour: most common colour in the lower legs of the front standing frame
f0 = fr(walk, 0); b0 = bottom(f0)
cnt = Counter(f0.getpixel((x, y)) for y in range(b0-4, b0-1) for x in range(4, 12) if f0.getpixel((x, y)) not in (0, 15))
LEG = cnt.most_common(1)[0][0] if cnt else near((60, 60, 70))
tw = Image.open(B+'walking.png'); T = gen.templates(tw)
gen.HEADCOL.clear(); [gen.HEADCOL.update(v for row in gen.px(T[k][0]) for v in row if v and v != 15) for k in T]
tpal = [tuple(tw.getpalette()[i*3:i*3+3]) for i in range(16)]
TSKIN = {1, 2, 3}
def running():
    g = new(144)
    for i in range(9): g.paste(fr(walk, i), (i*16, 1))
    return g
def headcx(f, t, rows=10):
    xs = [x for y in range(t, min(t+rows, 32)) for x in range(f.width) if f.getpixel((x, y))]
    return (min(xs)+max(xs))/2
def surfing():
    src = Image.open(B+'surfing.png'); g = new(192)
    for fi, s in ((0, dirf(ride, 'S')), (2, dirf(ride, 'N')), (4, dirf(ride, 'W')), (1, dirf(walk, 'S')), (3, dirf(walk, 'N')), (5, dirf(walk, 'W'))):
        ref = fr(src, fi, 32); rt = top(ref); st = top(s)
        dx = round(headcx(ref, rt)-headcx(s, st)); dy = rt-st
        for y in range(32):
            for x in range(16):
                v = s.getpixel((x, y)); X = x+dx; Y = y+dy
                if v and 0 <= X < 32 and 0 <= Y < 32: g.putpixel((fi*32+X, Y), v)
    return g
def bike(state):
    src = Image.open(B+state+'.png'); n = src.width//32; g = new(src.width)
    for i in range(n):
        f = fr(src, i, 32); s, d, ox, oy = gen.match(f, T)
        r = dirf(ride, d); rt = top(r); rb = bottom(r); dy = oy-rt; cut = rb+dy-1
        h = new(32)
        for y in range(32):
            for x in range(32):
                v = f.getpixel((x, y))
                if v and y > cut-6:
                    nv = near(tpal[v])
                    if v in TSKIN and y >= oy+18: nv = LEG
                    h.putpixel((x, y), nv)
        for y in range(32):
            for x in range(16):
                v = r.getpixel((x, y)); X = ox+x; Y = y+dy
                if v and 0 <= X < 32 and 0 <= Y < 32: h.putpixel((X, Y), v)
        g.paste(h, (i*32, 0))
    return g
def prop(state, w, rod=False):
    src = Image.open(B+state+'.png'); n = src.width//w; g = new(src.width)
    for i in range(n):
        f = fr(src, i, w); s, d, ox, oy = gen.match(f, T)
        bs = dirf(tw, d); bt = top(bs); hs = dirf(walk, d); ht = top(hs)
        h = new(w)
        for y in range(32):
            for x in range(16):
                v = hs.getpixel((x, y)); X = ox+x; Y = y-ht+oy
                if v and 0 <= X < w and 0 <= Y < 32: h.putpixel((X, Y), v)
        for y in range(32):
            for x in range(w):
                v = f.getpixel((x, y))
                if not v: continue
                bx = x-ox; by = y-oy+bt
                if 0 <= bx < 16 and 0 <= by < 32 and bs.getpixel((bx, by)): continue
                if h.getpixel((x, y)): continue
                if not rod and y < oy+10: continue
                h.putpixel((x, y), near(tpal[v]))
        g.paste(h, (i*w, 0))
    return g
os.makedirs(OUT, exist_ok=True)
running().save(OUT+'/running_B.png'); surfing().save(OUT+'/surfing_B.png')
bike('mach_bike').save(OUT+'/mach_bike_B.png'); bike('acro_bike').save(OUT+'/acro_bike_B.png')
prop('fishing', 32, rod=True).save(OUT+'/fishing_B.png'); prop('field_move', 32).save(OUT+'/field_move_B.png')
prop('watering', 32).save(OUT+'/watering_B.png'); prop('decorating', 16).save(OUT+'/decorating_B.png')
print(NAME, 'leg colour', LEG, pal[LEG])
