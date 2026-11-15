# Copycat battle picture from Treston's art (2026-11-15): block-mode downscale to Gen 3 size, remove the dark
# background + glow by flood fill from the border, re-draw a 1-px black outline, place in 64x64 (feet at the bottom).
import sys
from PIL import Image
import numpy as np
from collections import Counter
H=int(sys.argv[1]) if len(sys.argv)>1 else 62
a=np.asarray(Image.open(sys.argv[2] if len(sys.argv)>2 else 'docs/kanto_art/copycat_battle_art_owner.png').convert('RGB')).astype(int)
x0,x1,y0,y1=444,1002,14,998
a=a[y0:y1,x0:x1]
f=(y1-y0)/H
W=int(round((x1-x0)/f))
out=np.zeros((H,W,3),int)
for j in range(H):
    for i in range(W):
        blk=a[int(j*f):int((j+1)*f), int(i*f):int((i+1)*f)].reshape(-1,3)
        # mode of 5-bit-quantised colours, then average the members of that bucket (robust to AI grid wobble)
        q=[tuple(c>>3) for c in blk]
        m=Counter(q).most_common(1)[0][0]
        sel=np.array([c for c,k in zip(blk,q) if k==m])
        out[j,i]=sel.mean(0)
mx=out.max(2); r,g,b=out[:,:,0],out[:,:,1],out[:,:,2]
# background = black + the coloured glow around her. Keep the purple-grey darks of her stockings/jacket shadow
# (blue >= red, green below red, not too dark); the glow is either warm (blue < red) or bluish (green >= red).
protect=(b>=r-2)&(g<r)&(mx>=45)
dark=(mx<35)|((mx<110)&~protect)
bg=np.zeros((H,W),bool)
stack=[(j,i) for j in range(H) for i in (0,W-1)]+[(j,i) for i in range(W) for j in (0,H-1)]
while stack:
    j,i=stack.pop()
    if not(0<=j<H and 0<=i<W) or bg[j,i] or not dark[j,i]: continue
    bg[j,i]=True; stack+= [(j+1,i),(j-1,i),(j,i+1),(j,i-1)]
img=Image.new('RGBA',(64,64),(0,0,0,0))
ox=(64-W)//2; oy=64-H-1
for j in range(H):
    for i in range(W):
        if not bg[j,i]: img.putpixel((ox+i,oy+j),tuple(int(v) for v in out[j,i])+(255,))
# outline: transparent pixels touching the figure become black
px=img.load(); add=[]
for y in range(64):
    for x in range(64):
        if px[x,y][3]==0 and any(0<=x+dx<64 and 0<=y+dy<64 and px[x+dx,y+dy][3] and sum(px[x+dx,y+dy][:3])>60 for dx,dy in ((1,0),(-1,0),(0,1),(0,-1))):
            add.append((x,y))
for p in add: px[p]=(0,0,0,255)
img.save('copycat_64_raw.png'); print('size',W,H,'colours',len({px[x,y] for x in range(64) for y in range(64) if px[x,y][3]}))
G=(115,197,164,255)
v=Image.new('RGBA',(64*5+20+192+20+64,64*5),(24,24,30,255))
b=Image.new('RGBA',(64,64),G); b.alpha_composite(img); v.paste(b.resize((320,320),Image.NEAREST),(0,0))
v.paste(b.resize((192,192),Image.NEAREST),(340,0)); v.paste(b,(552,0))
v.save('view.png')

# --- hand clean-up after the downscale (owner 2026-11-15: "her eyes and poke ball look weird") ----------------
# At 62 px the face is ~6 px wide, so the eyes and the ball are re-drawn by hand in the Gen 3 way:
# eyes = dark lash line over a two-tone magenta iris with a white catch-light; ball = outlined 5-px ball.
K=(0,0,0,255); W=(255,255,255,255); IRD=(110,24,66,255); IRL=(222,82,138,255)
RED=(255,82,90,255); RDD=(189,41,57,255); GRY=(189,189,205,255)
px=img.load()
for x in (29,30,33,34): px[x,11]=K                      # lashes
SK=px[32,13]                                              # her skin tone from the face
for (x,y) in [(29,12),(30,12),(29,13),(30,13),(33,12),(34,12),(33,13),(34,13),(35,13)]: px[x,y]=SK
# 2x2 eyes: dark pupil + white shine on top, magenta iris below (looking left, like the art)
px[29,12]=IRD; px[30,12]=W; px[29,13]=IRL; px[30,13]=IRL
px[33,12]=IRD; px[34,12]=W; px[33,13]=IRL; px[34,13]=IRL
# Poke Ball, 6 px wide, held on top of the fist; clear the mushy old ball pixels first
for y in range(9,16):
    for x in range(14,22):
        c=px[x,y]
        if c[3] and not (c[0]>200 and 150<c[1]<215 and c[2]>120): px[x,y]=(0,0,0,0)   # keep skin (fingers)
B=[" KKKK ",
   "KRRRDK",
   "KKKwKK",
   "KWWWGK",
   " KKKK "]
M={'K':K,'R':RED,'D':RDD,'w':W,'W':W,'G':GRY}
for dy,row in enumerate(B):
    for dx,ch in enumerate(row):
        if ch!=' ': px[15+dx,10+dy]=M[ch]
# re-outline anything the clean-up left open
for y in range(8,18):
    for x in range(12,23):
        if px[x,y][3]==0 and any(px[x+a,y+b][3] and px[x+a,y+b][:3]!=(0,0,0) for a,b in ((1,0),(-1,0),(0,1),(0,-1))): px[x,y]=K
img.save('graphics/hicolor/trainers/copycat.png' if len(sys.argv)>3 and sys.argv[3]=='install' else 'copycat_64.png')
