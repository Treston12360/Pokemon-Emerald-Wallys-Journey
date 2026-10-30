"""Hilbert holding a prop (fishing rod, watering can, Poke Ball...): his own standing frame,
plus the prop (and the arm holding it) from Brendan's frame wherever it sticks out past
Brendan's standing body."""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
import gen
from PIL import Image
G=gen.G
NAME=os.environ.get('WJ_NAME','hilbert'); TMPL=os.environ.get('WJ_TMPL','brendan')
H=gen.R+'graphics/object_events/pics/people/%s/'%NAME
walk=Image.open(H+'walking.png')
pal=[tuple(walk.getpalette()[i*3:i*3+3]) for i in range(16)]
PROP={1:5,2:5,3:6,4:4,5:13,6:10,7:10,8:15,9:7,10:3,11:10,12:11,13:13,14:14,15:15}
def wf(d,img):
    i={'S':0,'N':1,'W':2,'E':2}[d]; f=img.crop((i*16,0,i*16+16,32))
    return f.transpose(Image.FLIP_LEFT_RIGHT) if d=='E' else f
def top(f):
    for y in range(32):
        if any(f.getpixel((x,y)) for x in range(f.width)): return y
def build(state,w,inside=False,prop=None):
    prop=prop or PROP
    B=G+'graphics/object_events/pics/people/%s/'%TMPL
    bw=Image.open(B+'walking.png'); T=gen.templates(bw)
    src=Image.open(B+state+'.png'); n=src.width//w
    out=Image.new('P',(src.width,32),0); out.putpalette([v for c in pal for v in c])
    for i in range(n):
        f=src.crop((i*w,0,i*w+w,32))
        s,d,ox,oy=gen.match(f,T)
        bs=wf(d,bw); bt=top(bs)
        hs=wf(d,walk); ht=top(hs)
        g=Image.new('P',(w,32),0)
        # Hilbert standing, head top where Brendan's head is
        for y in range(32):
            for x in range(16):
                v=hs.getpixel((x,y)); X=ox+x; Y=y-ht+oy
                if v and 0<=X<w and 0<=Y<32: g.putpixel((X,Y),v)
        # prop: Brendan pixels not covered by Brendan's own standing body
        for y in range(32):
            for x in range(w):
                v=f.getpixel((x,y))
                if not v: continue
                bx=x-ox; by=y-oy+bt
                body=0<=bx<16 and 0<=by<32 and bs.getpixel((bx,by))
                if body: continue
                if g.getpixel((x,y)) and not inside: continue
                if NAME!='hilbert' and y<oy+10: continue   # props are held at hand height
                if NAME=='hilbert': g.putpixel((x,y),prop[v])
                else:
                    c=tuple(f.getpalette()[v*3:v*3+3])
                    g.putpixel((x,y),min(range(1,16),key=lambda j:sum((a-b)**2 for a,b in zip(c,pal[j]))))
        out.paste(g,(i*w,0))
    return out
if __name__=='__main__':
    od=sys.argv[1]
    BALL=dict(PROP); BALL[12]=8; BALL[13]=9   # Poke Ball stays red and white
    for st,w,p in (('fishing',32,None),('watering',32,None),('field_move',32,BALL),('decorating',16,None)):
        build(st,w,prop=p).save(od+'/'+st+'_B.png')
