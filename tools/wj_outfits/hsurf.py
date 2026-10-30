import sys
from PIL import Image
G='/home/claude/pokemon-emerald-wallys-journey/'
H='/tmp/claude-0/-home-claude/c01f291d-beb5-5ce1-b8e2-6743db5de3e3/scratchpad/rogue/r/graphics/object_events/pics/people/hilbert/'
def bbox_top(f):
    for y in range(f.height):
        xs=[x for x in range(f.width) if f.getpixel((x,y))]
        if xs: return y,(min(xs)+max(xs))/2
def headcx(f,top,rows=10):
    xs=[x for y in range(top,min(top+rows,f.height)) for x in range(f.width) if f.getpixel((x,y))]
    return (min(xs)+max(xs))/2
def place(dst,src,ref,fi):
    # align src frame's head (top row + head centre) with ref frame's head
    rt,_=bbox_top(ref); st,_=bbox_top(src)
    rc=headcx(ref,rt); sc=headcx(src,st)
    dx=round(rc-sc); dy=rt-st
    for y in range(32):
        for x in range(16):
            v=src.getpixel((x,y))
            X=fi*32+x+dx; Y=y+dy
            if v and 0<=X-fi*32<32 and 0<=Y<32: dst.putpixel((X,Y),v)
b=Image.open(G+'graphics/object_events/pics/people/brendan/surfing.png')
ride=Image.open(H+'riding.png'); walk=Image.open(H+'walking.png')
out=Image.new('P',(192,32),0); out.putpalette(walk.getpalette())
R=lambda i: ride.crop((i*16,0,i*16+16,32)); W=lambda i: walk.crop((i*16,0,i*16+16,32)); BF=lambda i: b.crop((i*32,0,i*32+32,32))
for fi,src in ((0,R(0)),(2,R(1)),(4,R(2)),(1,W(0)),(3,W(1)),(5,W(2))):
    place(out,src,BF(fi),fi)
out.save(sys.argv[1])
