# Copycat battle picture v3: Emerald Lass body + FRLG Erika head (blunt bangs, long sides; headband removed),
# hair extended Hinata-style: strands in front of the shoulders, long hair behind the back. Pink hair.
from PIL import Image
R='/home/claude/pokemon-emerald-wallys-journey/graphics/trainers/front_pics/'
lass=Image.open(R+'lass.png'); lp=lass.getpalette()
eri=Image.open(R+'leader_erika_frlg.png'); ep=eri.getpalette()
W=64
C={}   # (x,y) -> rgb
for y in range(64):
    for x in range(64):
        i=lass.getpixel((x,y))
        if i: C[(x,y)]=tuple(lp[i*3:i*3+3])
# 1. remove the Lass's head and hair
for y in range(7,24):
    for x in range(27,50): C.pop((x,y),None)
for x in range(41,50):
    if lass.getpixel((x,24)) in (6,8,15): C.pop((x,24),None)
# 2. paste Erika's head, shifted
HAIR=dict(base=(214,96,146),light=(244,152,186),shine=(244,152,186),shadow=(166,62,114),line=(120,36,78))
DX,DY=4,2
hairpx=set()
for y in range(6,22):
    for x in range(22,44):
        i=eri.getpixel((x,y))
        if i==0 or i in (5,6,10): continue
        X,Y=x+DX,y+DY
        if i in (11,12,13): i=9                      # headband -> hair
        if i==9:   c=HAIR['shadow'] if X>=41 else HAIR['base']; hairpx.add((X,Y))
        elif i==8 and (x<29 or x>38 or y<14): c=HAIR['light']; hairpx.add((X,Y))
        elif i==7: c=HAIR['shine']; hairpx.add((X,Y))
        elif i==4 and (x<29 or x>38 or y<14): c=HAIR['line']; hairpx.add((X,Y))
        else:
            c=tuple(ep[i*3:i*3+3])
            LASS=[tuple(lp[j*3:j*3+3]) for j in range(1,16)]
            c=min(LASS,key=lambda L:sum((p-q)**2 for p,q in zip(L,c)))   # use the body's own skin/eye colours
        C[(X,Y)]=c
# headband rows become bangs: give the top of the bangs a highlight row (light from top-left)
for x in range(30,40):
    if C.get((x,12))==HAIR['base']: C[(x,12)]=HAIR['light']
K=(0,0,0)
# 3a. long back hair (behind everything): one straight mass from the sides of the head down to a blunt cut
back=set()
for y in range(13,37):
    for x in range(25,50):
        left_ok  = (x<=29 and (y<=23 or y>=29)) or (29<x<=32 and y>=29)
        right_ok = x>=45
        if not (left_ok or right_ok): continue
        if (x,y) in C: continue
        # silhouette: hair hangs straight, slightly wider than the head
        if x<25+ (0 if y>16 else 2) or x>48 - (0 if y>16 else 1): continue
        if y>=29 and x<28: continue
        back.add((x,y))
# outline = mass pixels touching transparency
def nb4(p): x,y=p; return [(x+1,y),(x-1,y),(x,y+1),(x,y-1)]
for p in back:
    if any((q not in back and q not in C) for q in nb4(p)): C[p]=K
    else:
        x,y=p
        if x>=45: c=HAIR['shadow'] if x>=47 else HAIR['base']
        else:      c=HAIR['light'] if x<=26 else HAIR['base']
        pass
        C[p]=c
# 3b. front locks framing the face (in front of the shoulders), tapering
def lock(x0,y0,y1,lightside):
    for y in range(y0,y1+1):
        w=2 if y<y1 else 1
        for k in range(w):
            x=x0+k
            C[(x,y)]= HAIR['light'] if (k==0)==lightside else HAIR['shadow']
            if w==1: C[(x,y)]=HAIR['base']
        C[(x0-1,y)]=K; C[(x0+w,y)]=K
    C[(x0,y1+1)]=K
lock(30,22,27,True)
lock(43,22,27,False)
# black lines that only separate hair from hair become a dark pink line (outline only against other things)
HS=set(HAIR.values())
for (x,y),c in list(C.items()):
    if c==K and C.get((x-1,y)) in HS and C.get((x+1,y)) in HS: C[(x,y)]=HAIR['line']
# fit 15 colours: merge the closest pair until it fits (hair ramp is protected)
def reduce(C,n=15):
    import itertools
    while True:
        cols=sorted(set(C.values()))
        if len(cols)<=n: return
        best=None
        for a,b in itertools.combinations(cols,2):
            if a in HS and b in HS: continue
            d=sum((p-q)**2 for p,q in zip(a,b))
            if best is None or d<best[0]: best=(d,a,b)
        _,a,b=best
        cnt={c:0 for c in (a,b)}
        for v in C.values():
            if v in cnt: cnt[v]+=1
        keep,drop=(a,b) if cnt[a]>=cnt[b] else (b,a)
        for k,v in C.items():
            if v==drop: C[k]=keep
from collections import Counter
print(Counter(C.values()))
reduce(C)
out=Image.new('RGBA',(64,64),(0,0,0,0))
for (x,y),c in C.items():
    if 0<=x<64 and 0<=y<64: out.putpixel((x,y),c+(255,))
cols={c for c in C.values()}
print('colours',len(cols))
out.save('ccD_battle.png')
G=(115,197,164,255)
v=Image.new('RGBA',(64*6*2+20,64*6),(24,24,30,255))
for i,(im,crop) in enumerate([(out,None),(out,(18,4,58,44))]):
    c=im.crop(crop) if crop else im
    b=Image.new('RGBA',c.size,G); b.alpha_composite(c); v.paste(b.resize((384,384),Image.NEAREST),(i*404,0))
v.save('ccD_view.png')
