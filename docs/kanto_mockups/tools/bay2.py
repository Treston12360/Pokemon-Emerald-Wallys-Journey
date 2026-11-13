import render as R
from compose import Map
from PIL import Image
B1='AquaHideout_B1F_Layout';B2='AquaHideout_B2F_Layout'
W,H=50,36
m=Map([B1,B2],W,H,0x228)
def hwall(x,y,w,caps=True):
    m.fill(x,y,w,1,0x2e0);m.fill(x,y+1,w,1,0x221);m.fill(x,y+2,w,1,0x229)
    if caps:
        for j,(a) in enumerate((0x211,0x20a,0x20a)): m.put(x,y+j,a);m.put(x+w-1,y+j,a)
def tank(x,y,w,h):
    m.put(x,y,0x286);m.fill(x+1,y,w-2,1,0x269);m.put(x+w-1,y,0x25e)
    for j in range(1,h-1):
        m.put(x,y+j,0x260);m.put(x+w-1,y+j,0x2a6 if j==1 else 0x2a4)
        m.put(x+1,y+j,0x2ae if j==1 else 0x2af);m.fill(x+2,y+j,w-3,1,0x2a7 if j==1 else 0x14f)
    m.put(x,y+h-1,0x268);m.fill(x+1,y+h-1,w-2,1,0x269);m.put(x+w-1,y+h-1,0x2ac)
# shell
hwall(0,0,W)
for y in range(H): m.put(0,y,0x20a);m.put(W-1,y,0x20a)
m.fill(1,H-1,W-2,1,0x259);m.put(0,H-1,0x209);m.put(1,H-1,0x20b);m.put(W-1,H-1,0x209)
# central spine: teal clean-room walkway from the core to the gallery
m.fill(17,3,16,H-4,0x202)
# ---- north cells: DIALGA (west), PALKIA (east)
for px,tx in ((6,1),(40,35)):
    m.fill(tx+1,3,12,4,0x202)            # platform round the pod
    m.stamp(B2,20,0,4,7,px,0)            # pod set in the wall
    tank(tx+1,7,12,9)                    # sealed tank
m.stamp(B1,8,1,2,2,2,1);m.stamp(B1,8,1,2,2,11,1);m.stamp(B1,8,1,2,2,36,1);m.stamp(B1,8,1,2,2,45,1)
# ---- the REQUIEM core, wall-mounted, flanked by monitor banks and analysers
m.stamp(B1,29,6,4,6,23,0)
for x in (17,19,28,30): m.stamp(B1,8,1,2,2,x,1)
m.stamp(B1,5,2,2,4,15,0);m.stamp(B1,5,2,2,4,33,0)
# conduits: charger columns lining the spine
for y in (6,11,23,28):
    m.stamp(B2,21,7,2,4,18,y);m.stamp(B2,21,7,2,4,30,y)
m.stamp(B1,24,18,2,2,22,7);m.stamp(B1,24,18,2,2,26,7)
# ---- divider: inner wall with the south cells' pods, open spine in the middle
hwall(1,17,16,caps=False);hwall(33,17,16,caps=False)
m.put(16,17,0x211);m.put(16,18,0x20a);m.put(16,19,0x20a);m.put(33,17,0x211);m.put(33,18,0x20a);m.put(33,19,0x20a)
m.fill(2,20,12,4,0x202);m.fill(36,20,12,4,0x202)
m.stamp(B2,20,0,4,7,6,17);m.stamp(B2,26,0,4,7,40,17)
m.stamp(B1,8,1,2,2,2,18);m.stamp(B1,8,1,2,2,11,18);m.stamp(B1,8,1,2,2,36,18);m.stamp(B1,8,1,2,2,45,18)
# ---- south cells: GIRATINA (west), HOOPA (east)
for tx in (1,35):
    tank(tx+1,24,12,9)
# ---- control gallery at the foot of the spine
m.stamp(B1,16,18,2,3,20,15);m.stamp(B1,22,18,2,3,28,15)
m.stamp(B1,17,15,2,2,23,16);m.stamp(B1,17,15,2,2,25,16)
m.stamp(B1,16,18,2,3,21,30);m.stamp(B1,22,18,2,3,27,30)
m.stamp(B1,17,15,2,2,24,31)
m.stamp(B1,24,18,2,2,22,21);m.stamp(B1,24,18,2,2,26,21);m.stamp(B1,24,18,2,2,22,25);m.stamp(B1,24,18,2,2,26,25)
m.save('bay2')
# ---- legendaries suspended in their tanks, tinted by the fluid
def sprite(name,size):
    im=Image.open(f'{R.ROOT}/graphics/pokemon/{name}/overworld.png')
    pal=open(f'{R.ROOT}/graphics/pokemon/{name}/overworld_normal.pal').read().split('\n')[3:19]
    cols=[tuple(int(v) for v in l.split()) for l in pal if l.strip()]
    fr=im.crop((0,0,size,size));out=Image.new('RGBA',(size,size));px=fr.load()
    for y in range(size):
        for x in range(size):
            c=px[x,y]&15
            if c:
                r,g,b=cols[c];out.putpixel((x,y),(int(r*.72+40*.28),int(g*.72+90*.28),int(b*.72+200*.28),255))
    return out
base=Image.open('bay2.png').convert('RGBA')
for name,size,cx,cy in [('dialga',64,8,11.5),('palkia',64,42,11.5),('giratina',64,8,28.5),('hoopa',32,42,28.5)]:
    sp=sprite(name,size)
    if size==32: sp=sp.resize((48,48),Image.NEAREST);size=48
    base.alpha_composite(sp,(int(cx*16-size//2),int(cy*16-size//2)))
base.convert('RGB').save('bay2_mons.png')
base.resize((W*24,H*24),Image.NEAREST).convert('RGB').save('bay2_mons_big.png')
