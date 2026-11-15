# Mimi (the Copycat) battle picture from Treston's art (2026-11-15).
# python3 tools/aria_copycat_from_art.py [H=88] [ART] [install]
# Block-mode downscale of the art to H px tall (full figure), dark background + coloured glow removed by flood fill
# from the border (her purple-grey stockings/jacket shadows protected), 1-px black outline re-drawn, then the top 64
# rows go into the 64x64 picture -> cut at the thighs, like Janine's and Varnell's (owner-approved precedent).
# History: full body at 62 px made her eyes a dark streak and the ball mush ("look weird"); redrawing them by hand
# at that size was worse ("You made it worse"). At 88 px the eyes and ball come straight from the art.
import sys
from PIL import Image
import numpy as np
from collections import Counter
H=int(sys.argv[1]) if len(sys.argv)>1 else 88
ART=sys.argv[2] if len(sys.argv)>2 else 'docs/kanto_art/copycat_battle_art_owner.png'
INSTALL=len(sys.argv)>3 and sys.argv[3]=='install'
a=np.asarray(Image.open(ART).convert('RGB')).astype(int)
x0,x1,y0,y1=444,1002,14,998
a=a[y0:y1,x0:x1]
f=(y1-y0)/H; W=int(round((x1-x0)/f))
out=np.zeros((H,W,3),int)
for j in range(H):
    for i in range(W):
        blk=a[int(j*f):int((j+1)*f), int(i*f):int((i+1)*f)].reshape(-1,3)
        q=[tuple(c>>3) for c in blk]; m=Counter(q).most_common(1)[0][0]
        out[j,i]=np.array([c for c,k in zip(blk,q) if k==m]).mean(0)
mx=out.max(2); r,g,b=out[:,:,0],out[:,:,1],out[:,:,2]
protect=(b>=r-2)&(g<r)&(mx>=45)
dark=(mx<35)|((mx<110)&~protect)
bg=np.zeros((H,W),bool); st=[(j,i) for j in range(H) for i in (0,W-1)]+[(j,i) for i in range(W) for j in (0,H-1)]
while st:
    j,i=st.pop()
    if not(0<=j<H and 0<=i<W) or bg[j,i] or not dark[j,i]: continue
    bg[j,i]=True; st+=[(j+1,i),(j-1,i),(j,i+1),(j,i-1)]
fig=Image.new('RGBA',(W,H),(0,0,0,0))
for j in range(H):
    for i in range(W):
        if not bg[j,i]: fig.putpixel((i,j),tuple(int(v) for v in out[j,i])+(255,))
px=fig.load(); add=[]
for y in range(H):
    for x in range(W):
        if px[x,y][3]==0 and any(0<=x+dx<W and 0<=y+dy<H and px[x+dx,y+dy][3] and sum(px[x+dx,y+dy][:3])>60 for dx,dy in ((1,0),(-1,0),(0,1),(0,-1))): add.append((x,y))
for p in add: px[p]=(0,0,0,255)
img=Image.new('RGBA',(64,64),(0,0,0,0))
w=min(64,W); img.alpha_composite(fig.crop((0,0,w,64)),((64-w)//2,1))
img.save('graphics/hicolor/trainers/copycat.png' if INSTALL else 'copycat_preview.png')
print('figure',W,'x',H)
