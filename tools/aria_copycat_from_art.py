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
img.save('copycat_64.png'); print('size',W,H,'colours',len({px[x,y] for x in range(64) for y in range(64) if px[x,y][3]}))
G=(115,197,164,255)
v=Image.new('RGBA',(64*5+20+192+20+64,64*5),(24,24,30,255))
b=Image.new('RGBA',(64,64),G); b.alpha_composite(img); v.paste(b.resize((320,320),Image.NEAREST),(0,0))
v.paste(b.resize((192,192),Image.NEAREST),(340,0)); v.paste(b,(552,0))
v.save('view.png')
