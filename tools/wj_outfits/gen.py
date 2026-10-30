#!/usr/bin/env python3
"""Build full Emerald player sheets for every outfit.
Sheets Rogue already has are copied; the rest are made by taking Brendan's/May's sheet,
swapping in the outfit's head (from its walking frame facing the same way) and recolouring
the body into the outfit's palette (skin -> skin, shirt -> its 'secondary' layer colours,
everything else -> nearest colour)."""
import os, sys
from PIL import Image
R='/tmp/claude-0/-home-claude/c01f291d-beb5-5ce1-b8e2-6743db5de3e3/scratchpad/rogue/r/'
G='/home/claude/pokemon-emerald-wallys-journey/'
OUT=sys.argv[1] if len(sys.argv)>1 else G+'graphics/object_events/pics/people/wj_outfits/'
OUTFITS=[('brendan','M'),('may','F'),('red','M'),('leaf','F'),('hilbert','M')]
PALDIR={'red':'red_leaf','leaf':'red_leaf'}
STATES={'walking':(16,9),'running':(16,9),'mach_bike':(32,9),'acro_bike':(32,27),'surfing':(32,6),
        'field_move':(32,5),'fishing':(32,12),'watering':(32,6),'decorating':(16,1)}
def readpal(p):
    L=open(p).read().split('\n')[3:19]; return [tuple(int(v) for v in l.split()) for l in L]
def lum(c): return 0.3*c[0]+0.59*c[1]+0.11*c[2]
def frames(im,w): return [im.crop((i*w,0,i*w+w,32)) for i in range(im.width//w)]
def px(im): return [[im.getpixel((x,y)) for x in range(im.width)] for y in range(im.height)]
def head_rows(f):
    rows=[y for y in range(32) if any(f.getpixel((x,y)) for x in range(f.width))]
    return rows[0] if rows else 0
HEADH=12
HEADCOL=set()
def templates(tmplwalk):
    fr=frames(tmplwalk,16)
    T={}
    for name,i,flip in (('S',0,False),('N',1,False),('W',2,False),('E',2,True)):
        f=fr[i].transpose(Image.FLIP_LEFT_RIGHT) if flip else fr[i]
        t=head_rows(f); T[name]=(f.crop((0,t,16,t+HEADH)),flip)
    return T
def match(frame,T):
    best=None
    W=frame.width
    P=px(frame)
    for name,(t,flip) in T.items():
        tp=px(t)
        for oy in range(0,32-HEADH+1):
            for ox in range(-2,W-14):
                s=0
                for y in range(HEADH):
                    for x in range(16):
                        v=tp[y][x]
                        if not v: continue
                        X=ox+x
                        if 0<=X<W:
                            s+= 2 if P[oy+y][X]==v else (-1)
                        else: s-=1
                if best is None or s>best[0]: best=(s,name,ox,oy)
    return best
def outfit_head(walk,dirname):
    fr=frames(walk,16)
    i={'S':0,'N':1,'W':2,'E':2}[dirname]
    f=fr[i].transpose(Image.FLIP_LEFT_RIGHT) if dirname=='E' else fr[i]
    t=head_rows(f)
    return f.crop((0,t,16,t+HEADH+1))
def colour_map(srcpal,dstpal,dstlayers):
    # semantic: Brendan/May skin 1,2,3 ; shirt 12,13,4 (Brendan) - pick by hue
    m={}
    sec=[i for i in range(1,16) if dstlayers[i]==(0,0,255)]
    skin=[i for i in range(1,16) if dstlayers[i]==(255,0,0) and dstpal[i][0]>150 and dstpal[i][0]>dstpal[i][1]>dstpal[i][2]]
    sec.sort(key=lambda i:-lum(dstpal[i])); skin.sort(key=lambda i:-lum(dstpal[i]))
    srcskin=sorted([i for i in (1,2,3)],key=lambda i:-lum(srcpal[i]))
    srcshirt=sorted([i for i in range(1,16) if srcpal[i][0]>150 and srcpal[i][1]<120 and srcpal[i][2]<120],key=lambda i:-lum(srcpal[i]))
    for k,i in enumerate(srcskin):
        if skin: m[i]=skin[min(k,len(skin)-1)]
    for k,i in enumerate(srcshirt):
        if sec: m[i]=sec[min(k,len(sec)-1)]
    for i in range(1,16):
        if i in m: continue
        c=srcpal[i]
        m[i]=min(range(1,16),key=lambda j:sum((a-b)**2 for a,b in zip(c,dstpal[j])))
    m[0]=0
    return m
def build(name,gender):
    pd=R+'graphics/rogue_palettes/%s/'%PALDIR.get(name,name)
    pal=readpal(pd+'object_event_base.pal'); layers=readpal(pd+'object_event_layers.pal')
    src=R+'graphics/object_events/pics/people/%s/'%name
    tdir=G+'graphics/object_events/pics/people/%s/'%('brendan' if gender=='M' else 'may')
    walk=Image.open(src+'walking.png')
    tw=Image.open(tdir+'walking.png'); T=templates(tw)
    HEADCOL.clear(); [HEADCOL.update(v for row in px(T[k][0]) for v in row if v and v!=15) for k in T]
    srcpal=[tuple(tw.getpalette()[i*3:i*3+3]) for i in range(16)]
    cm=colour_map(srcpal,pal,layers)
    od=OUT+name+'/'; os.makedirs(od,exist_ok=True)
    flat=[v for c in pal for v in c]
    made=[]
    for st,(w,n) in STATES.items():
        sp=src+st+'.png'
        ok=False
        if os.path.exists(sp):
            _p=Image.open(sp).getpalette(); ok=sum(1 for i in range(1,16) if tuple(_p[i*3:i*3+3])!=pal[i])<=1
        if ok:
            im=Image.open(sp)
            assert im.size==(w*n,32),(name,st,im.size)
            ip=[tuple(im.getpalette()[i*3:i*3+3]) for i in range(16)]
            remap=[0]+[min(range(1,16),key=lambda j:sum((a-b)**2 for a,b in zip(ip[i],pal[j]))) for i in range(1,16)]
            out=im.point(lambda v:remap[v] if v<16 else 0); out.putpalette(flat)
        elif st=='running':
            out=walk.copy(); out.putpalette(flat)
        else:
            tm=Image.open(tdir+st+'.png'); out=Image.new('P',tm.size,0); out.putpalette(flat)
            for i,f in enumerate(frames(tm,w)):
                s,d,ox,oy=match(f,T)
                tp=px(T[d][0])
                g=Image.new('P',(w,32),0)
                for y in range(32):
                    for x in range(w):
                        v=f.getpixel((x,y))
                        if not v: continue
                        # erase the template head (and anything above it in its columns)
                        hx=x-ox; hy=y-oy
                        if 0<=hx<16 and ((0<=hy<HEADH and tp[hy][hx]) or (hy<0 and v in HEADCOL)): continue
                        g.putpixel((x,y),cm[v])
                h=outfit_head(walk,d)
                for y in range(h.height):
                    for x in range(16):
                        v=h.getpixel((x,y)); X=ox+x; Y=oy+y
                        if v and 0<=X<w and 0<=Y<32: g.putpixel((X,Y),v)
                out.paste(g,(i*w,0))
            made.append(st)
        out.save(od+st+'.png')
    # palettes
    open(od+'ow.pal','w').write(open(pd+'object_event_base.pal').read())
    open(od+'ow_layers.pal','w').write(open(pd+'object_event_layers.pal').read())
    return made
if __name__=='__main__':
    only=sys.argv[2:] or [n for n,_ in OUTFITS]
    for n,g in OUTFITS:
        if n in only: print(n,'generated:',build(n,g))
