import render as R,struct
class Map:
    def __init__(s,srcs,W,H,base):
        s.src={}
        for n in srcs:
            l=R.layout(n);b=open(R.ROOT+'/'+l['blockdata_filepath'],'rb').read()
            s.src[n]=(l['width'],[struct.unpack('<H',b[i*2:i*2+2])[0]&0x3FF for i in range(l['width']*l['height'])])
        l=R.layout(srcs[0]);s.ts=R.TS(l['primary_tileset'],l['secondary_tileset'])
        s.W,s.H=W,H;s.g=[[base]*W for _ in range(H)]
    def put(s,x,y,v):
        if 0<=x<s.W and 0<=y<s.H: s.g[y][x]=v
    def fill(s,x,y,w,h,v):
        for j in range(h):
            for i in range(w): s.put(x+i,y+j,v)
    def stamp(s,n,sx,sy,w,h,dx,dy,skip=()):
        SW,ids=s.src[n]
        for j in range(h):
            for i in range(w):
                v=ids[(sy+j)*SW+sx+i]
                if v not in skip: s.put(dx+i,dy+j,v)
    def save(s,name):
        ids=[v for r in s.g for v in r]
        im=R.render_ids(s.ts,ids,s.W,s.H,name+'.png')
        im.resize((s.W*32,s.H*32),R.Image.NEAREST).save(name+'_2x.png')
