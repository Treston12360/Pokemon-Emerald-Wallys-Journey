# Copycat battle picture v4. Lessons from the research applied:
#  - never splice a front-facing head onto a 3/4 body (v3's mistake): keep the Emerald Lass's OWN head & face
#    (same angle, her lively open eyes) and re-draw only the hair, by hand, in the same 1-px style;
#  - light from the top-left; outlines = dark shade of the hair on the lit side, black only on the shadow side;
#  - shade hair in strand clusters of uneven length (no tubes, no banding, no pillow shading);
#  - bangs cast a shadow on the forehead; hair behind the body only shows where nothing is in front.
from PIL import Image
R='/home/claude/pokemon-emerald-wallys-journey/graphics/trainers/front_pics/'
src=Image.open(R+'lass.png'); P=src.getpalette()
col=lambda i: tuple(P[i*3:i*3+3])
A=(255,198,214); B=(242,140,174); Cc=(208,86,140); D=(150,50,106); O=(102,30,70); K=(0,0,0)
SK1,SK2,SK3,SKO=col(1),col(2),col(3),col(4)
C={}
for y in range(64):
    for x in range(64):
        i=src.getpixel((x,y))
        if i: C[(x,y)]=col(i)
# --- 1. clear the old hair (keep the face: skin, eyes, mouth) -------------------------------------------
face_keep=set()
for y in range(17,25):
    for x in range(32,43):
        i=src.getpixel((x,y))
        if i in (1,2,3,4,13,14,15) or (i==6 and y in (18,19)): face_keep.add((x,y))
for y in range(8,25):
    for x in range(28,52):
        if (x,y) in face_keep: continue
        i=src.getpixel((x,y))
        if y<17 or i in (5,6,8) or (i==15 and (x>=40 or x<=33)): C.pop((x,y),None)
# --- 2. silhouette of the head hair (crown + blunt bangs) --------------------------------------------------
rows={8:(37,41),9:(35,43),10:(33,44),11:(32,45),12:(31,46),13:(31,46),14:(30,46),15:(30,46),16:(30,46)}
hair={}
for y,(l,r) in rows.items():
    for x in range(l,r+1):
        t=(x-l)/(r-l)                      # 0 = lit (left) side, 1 = shadow (right) side
        c=Cc
        if t<0.35: c=B
        if t>0.72: c=D
        hair[(x,y)]=c
# highlight band on the crown, curving with the head (top-left light)
for (x,y) in [(36,9),(37,9),(38,9),(34,10),(35,10),(36,10),(39,10),(33,11),(34,11),(37,11),(38,11),(32,12),(33,12),(36,12)]:
    hair[(x,y)]=A
# strand partings in the bangs (uneven lengths), bangs end in one straight line at row 16
for (x,y) in [(35,13),(35,14),(35,15),(39,14),(39,15),(42,12),(42,13),(42,14),(42,15),(42,16),(44,13),(44,14)]:
    hair[(x,y)]=D if x>40 else Cc
for x in range(31,42): hair[(x,16)]= D if x in (35,39) else Cc     # bottom row of the bangs, slightly darker
# --- 3. side locks framing the face (near side, in front of the shoulder), and the far side -------------------
near=[(30,32,17,22),(30,32,23,25),(30,31,26,27),(30,30,28,28)]    # x0,x1,y0,y1  tapering
for x0,x1,y0,y1 in near:
    for y in range(y0,y1+1):
        for x in range(x0,x1+1):
            hair[(x,y)] = B if x==x0 else (D if x==x1 and y>19 else Cc)
far=[(43,46,17,21),(43,47,22,24)]
for x0,x1,y0,y1 in far:
    for y in range(y0,y1+1):
        for x in range(x0,x1+1):
            hair[(x,y)] = Cc if x==x0 else D
# --- 4. long straight back hair behind the body (only visible where nothing is in front) -------------------
back={}
for y in range(24,39):
    for x in range(44,51):
        if x>=50 and y<27: continue
        back[(x,y)] = D if x in (46,49) else Cc
        if x==44 or x==47: back[(x,y)]=B if y<30 else Cc
# blunt, slightly uneven ends (Hinata's cut)
for x,yend in [(44,38),(45,38),(46,37),(47,38),(48,38),(49,37),(50,37)]:
    for y in range(yend+1,40): back.pop((x,y),None)
for p,c in back.items():
    if p not in C: hair.setdefault(p,c)
for p,c in hair.items():
    if p not in face_keep: C[p]=c
# --- 5. forehead shadow under the bangs ----------------------------------------------------------------
for x in range(33,40):
    if C.get((x,17)) in (SK1,SK2): C[(x,17)]=SK2 if x>34 else SK3
# --- 6. outlines: dark pink on the lit (top/left) side, black on the shadow (right/bottom) side ------------
HS={A,B,Cc,D}
outline={}
for (x,y),c in C.items():
    if c not in HS: continue
    for dx,dy in ((1,0),(-1,0),(0,1),(0,-1)):
        q=(x+dx,y+dy)
        if q not in C and q not in outline:
            outline[q]= O if (dx<0 or dy<0) and not (x>=43) else K
for q,c in outline.items(): C[q]=c
# fit 15 colours the way official sprites do: hair shine reuses the light skin tone, lit-side outline reuses
# the hair shadow, the two near-identical navies merge
SUB={A:SK1, O:D, col(8):col(7)}
for k,v in list(C.items()): C[k]=SUB.get(v,v)
from collections import Counter
cnt=Counter(C.values()); print('colours',len(cnt)); print(sorted(cnt.items(),key=lambda kv:kv[1]))
out=Image.new('RGBA',(64,64),(0,0,0,0))
for (x,y),c in C.items():
    if 0<=x<64 and 0<=y<64: out.putpixel((x,y),c+(255,))
out.save('ccE_battle.png')
G=(115,197,164,255)
v=Image.new('RGBA',(64*5+20+300,320),(24,24,30,255))
b=Image.new('RGBA',(64,64),G); b.alpha_composite(out); v.paste(b.resize((320,320),Image.NEAREST),(0,0))
c=out.crop((22,4,56,44)); b=Image.new('RGBA',c.size,G); b.alpha_composite(c); v.paste(b.resize((255,300),Image.NEAREST),(340,0))
b=Image.new('RGBA',(64,64),G); b.alpha_composite(out); v.paste(b,(600,250))
v.save('ccE_view.png')
