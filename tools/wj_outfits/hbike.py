import sys, os
sys.path.insert(0, os.path.dirname(__file__))
import gen
from PIL import Image
G=gen.G
H=gen.R+'graphics/object_events/pics/people/hilbert/'
ride=Image.open(H+'riding.png'); walk=Image.open(H+'walking.png')
pal=[tuple(walk.getpalette()[i*3:i*3+3]) for i in range(16)]
# Brendan colours below Hilbert's seated body: bike keeps its red/white/grey, legs become his trousers/shoes
LOW={1:5,2:5,3:6,4:9,5:3,6:10,7:10,8:15,9:14,10:3,11:10,12:8,13:9,14:14,15:15}
def ridef(d):
    i={'S':0,'N':1,'W':2,'E':2}[d]; f=ride.crop((i*16,0,i*16+16,32))
    return f.transpose(Image.FLIP_LEFT_RIGHT) if d=='E' else f
def top(f):
    for y in range(32):
        if any(f.getpixel((x,y)) for x in range(f.width)): return y
def bottom(f):
    for y in range(31,-1,-1):
        if any(f.getpixel((x,y)) for x in range(f.width)): return y
def build(state,tmpl='brendan'):
    B=G+'graphics/object_events/pics/people/%s/'%tmpl
    T=gen.templates(Image.open(B+'walking.png'))
    src=Image.open(B+state+'.png'); n=src.width//32
    out=Image.new('P',(src.width,32),0); out.putpalette([v for c in pal for v in c])
    for i in range(n):
        f=src.crop((i*32,0,i*32+32,32))
        s,d,ox,oy=gen.match(f,T)
        r=ridef(d); rt=top(r); rb=bottom(r)
        dy=oy-rt+ (0)
        g=Image.new('P',(32,32),0)
        cut=rb+dy-1            # rows above this are Hilbert's own seated art
        for y in range(32):
            for x in range(32):
                v=f.getpixel((x,y))
                if v and y>cut-6:
                    nv=LOW[v]
                    if v in (1,2,3) and y>=oy+18: nv=3
                    g.putpixel((x,y),nv)
        # clear anything of Brendan's upper body left beside Hilbert, then draw Hilbert
        for y in range(32):
            for x in range(32):
                if y<=cut and g.getpixel((x,y)) and not (y>cut-6): g.putpixel((x,y),0)
        for y in range(32):
            for x in range(16):
                v=r.getpixel((x,y)); X=ox+x; Y=y+dy
                if v and 0<=X<32 and 0<=Y<32: g.putpixel((X,Y),v)
        out.paste(g,(i*32,0))
    return out
if __name__=='__main__':
    od=sys.argv[1]
    for st in ('mach_bike','acro_bike'): build(st).save(od+'/'+st+'_B.png')
