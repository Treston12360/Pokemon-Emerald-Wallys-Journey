#!/usr/bin/env python3
"""Hilbert's missing Emerald action sheets, redrawn frame by frame over Brendan's poses:
Hilbert's head from his own walking frame facing the same way, and the body repainted in
Hilbert's clothes (blue jacket, black bag strap, dark trousers down to the shoes, red/white
shoes) instead of Brendan's colours. Output is reviewed sheet by sheet before it's used."""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
import gen
from PIL import Image
B=gen.G+'graphics/object_events/pics/people/brendan/'
H=gen.R+'graphics/object_events/pics/people/hilbert/'
pal=gen.readpal(gen.R+'graphics/rogue_palettes/hilbert/object_event_base.pal')
flat=[v for c in pal for v in c]
# Brendan index -> Hilbert index (body below the head)
BODY={1:5,2:5,3:6,4:4,5:3,6:10,7:10,8:15,9:7,10:3,11:10,12:11,13:13,14:14,15:15}
SKIN={1,2,3}
def frame(sheet,w,f): return sheet.crop((f*w,0,f*w+w,32))
def redraw(bframe,w,T,walk):
    s,d,ox,oy=gen.match(bframe,T)
    tp=gen.px(T[d][0])
    g=Image.new('P',(w,32),0); g.putpalette(flat)
    # where Brendan's legs start: 15 rows under the top of his head
    legs=oy+16
    for y in range(32):
        for x in range(w):
            v=bframe.getpixel((x,y))
            if not v: continue
            hx=x-ox; hy=y-oy
            if 0<=hx<16 and ((0<=hy<gen.HEADH and tp[hy][hx]) or (hy<0 and v in gen.HEADCOL)): continue
            nv=BODY[v]
            if v in SKIN and y>=legs: nv=3      # long trousers, not bare legs
            g.putpixel((x,y),nv)
    h=gen.outfit_head(walk,d)
    for y in range(h.height):
        for x in range(16):
            v=h.getpixel((x,y)); X=ox+x; Y=oy+y
            if v and 0<=X<w and 0<=Y<32: g.putpixel((X,Y),v)
    return g
def build(state,w):
    tw=Image.open(B+'walking.png'); T=gen.templates(tw)
    gen.HEADCOL.clear(); [gen.HEADCOL.update(v for row in gen.px(T[k][0]) for v in row if v and v!=15) for k in T]
    walk=Image.open(H+'walking.png')
    src=Image.open(B+state+'.png'); n=src.width//w
    out=Image.new('P',(src.width,32),0); out.putpalette(flat)
    for i in range(n): out.paste(redraw(frame(src,w,i),w,T,walk),(i*w,0))
    return out
if __name__=='__main__':
    od=sys.argv[1]; os.makedirs(od,exist_ok=True)
    for st,w in (('running',16),('mach_bike',32),('acro_bike',32),('surfing',32),('field_move',32),('fishing',32),('watering',32),('decorating',16)):
        build(st,w).save(od+'/'+st+'.png')
