import json,sys,os,re,struct
from PIL import Image
import os; ROOT=os.environ.get('WJ_ROOT', os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..')))
def tsdir(name):
    s=re.sub(r'^gTileset_','',name)
    s=re.sub(r'(?<!^)([A-Z])',r'_\1',s).lower().replace('__','_')
    for k in ('primary','secondary'):
        for cand in (s, s.replace('_',''), s.replace('_frlg','_frlg')):
            p=f'{ROOT}/data/tilesets/{k}/{cand}'
            if os.path.isdir(p): return p
    # fallback search
    for k in ('primary','secondary'):
        for d in os.listdir(f'{ROOT}/data/tilesets/{k}'):
            if d.replace('_','')==s.replace('_',''): return f'{ROOT}/data/tilesets/{k}/{d}'
    return tsdir('gTileset_'+name[9:]+'Frlg') if not name.endswith('Frlg') else (_ for _ in ()).throw(Exception(name))
def pals(d):
    out=[]
    for i in range(16):
        p=f'{d}/palettes/{i:02d}.pal'
        cols=[(0,0,0)]*16
        if os.path.exists(p):
            ln=open(p).read().split('\n')[3:19]
            cols=[tuple(int(x) for x in l.split()) for l in ln if l.strip()]
        out.append(cols)
    return out
def tiles(d):
    im=Image.open(f'{d}/tiles.png'); w,h=im.size
    px=im.load(); t=[]
    for ty in range(h//8):
        for tx in range(w//8):
            t.append([[px[tx*8+x,ty*8+y]&15 for x in range(8)] for y in range(8)])
    return t
import glob
HDR=''.join(open(p).read() for p in glob.glob(ROOT+'/src/data/tilesets/*.h')+[ROOT+'/src/graphics.c'])
def sym(ts,field):
    m=re.search(r'const struct Tileset '+ts+r'\s*=\s*\{(.*?)\};',HDR,re.S).group(1)
    v=re.search(r'\.'+field+r'\s*=\s*(\w+)',m).group(1)
    path=re.search(r'\b'+v+r'\b[^=;]*=\s*\{?\s*(?://[^\n]*\n\s*)?INC\w+\("([^"]+)"',HDR,re.S)
    if not path: raise Exception(v)
    return ROOT+'/'+path.group(1)
class TS:
    def __init__(s,prim,sec):
        fr=prim.endswith('Frlg'); NP=640 if fr else 512; PP=7 if fr else 6
        tp,tq=sym(prim,'tiles'),sym(sec,'tiles')
        pd,sd=os.path.dirname(tp),os.path.dirname(tq)
        P=pals(pd);Q=pals(sd)
        s.pal=[P[i] if i<PP else Q[i] for i in range(16)]
        a=tiles(pd);s.tiles=(a+[None]*NP)[:NP]+tiles(sd)
        s.meta=open(sym(prim,'metatiles'),'rb').read()
        s.meta2=open(sym(sec,'metatiles'),'rb').read()
        s.NP=NP;s.cache={}
    def metatile(s,i):
        if i in s.cache: return s.cache[i]
        if i<s.NP: b=s.meta[i*16:i*16+16]
        else: b=s.meta2[(i-s.NP)*16:(i-s.NP)*16+16]
        im=Image.new('RGBA',(16,16),(0,0,0,255))
        if len(b)<16: s.cache[i]=im; return im
        for layer in range(2):
            for q in range(4):
                v=struct.unpack('<H',b[layer*8+q*2:layer*8+q*2+2])[0]
                ti=v&0x3FF; hf=v>>10&1; vf=v>>11&1; pl=v>>12
                t=s.tiles[ti] if ti<len(s.tiles) else None
                if not t: continue
                ox=(q&1)*8; oy=(q>>1)*8
                for y in range(8):
                    for x in range(8):
                        c=t[7-y if vf else y][7-x if hf else x]
                        if c==0 and not (layer==0): continue
                        if c==0 and layer==0: col=s.pal[pl][0]
                        else: col=s.pal[pl][c]
                        im.putpixel((ox+x,oy+y),col+(255,))
        s.cache[i]=im; return im
def layout(name):
    for l in json.load(open(f'{ROOT}/data/layouts/layouts.json'))['layouts']:
        if l['name']==name: return l
def render_layout(name,out,grid=False):
    l=layout(name);ts=TS(l['primary_tileset'],l['secondary_tileset'])
    b=open(f"{ROOT}/{l['blockdata_filepath']}",'rb').read()
    W,H=l['width'],l['height'];ids=[struct.unpack('<H',b[i*2:i*2+2])[0]&0x3FF for i in range(W*H)]
    return render_ids(ts,ids,W,H,out,grid),ids
def render_ids(ts,ids,W,H,out,grid=False):
    from PIL import ImageDraw
    im=Image.new('RGB',(W*16,H*16))
    for i,m in enumerate(ids): im.paste(ts.metatile(m),((i%W)*16,(i//W)*16))
    if grid:
        im=im.resize((W*48,H*48),Image.NEAREST);d=ImageDraw.Draw(im)
        for i,m in enumerate(ids): d.text(((i%W)*48+1,(i//W)*48+1),f"{m:x}",fill=(255,0,255))
    im.save(out);return im
if __name__=='__main__':
    for n in sys.argv[1:]: render_layout(n,f'/tmp/claude-0/-home-claude/2bb72568-d7ee-56f4-8a6d-fd6081a29497/scratchpad/sim/{n}.png',grid=True)
