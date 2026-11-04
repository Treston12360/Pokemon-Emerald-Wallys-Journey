import json,struct,re,glob,sys,os
def load(root,layout):
    L=json.load(open(f'{root}/data/layouts/layouts.json'))['layouts']
    l=[x for x in L if x.get('id')==layout][0]
    def attrs(ts):
        name=ts.replace('gTileset_',''); snake=re.sub(r'(?<!^)(?=[A-Z])','_',name).lower()
        for p in glob.glob(f'{root}/data/tilesets/*/*/metatile_attributes.bin'):
            if p.split('/')[-2]==snake: d=open(p,'rb').read(); return [struct.unpack_from('<H',d,i)[0] for i in range(0,len(d),2)]
    pa=attrs(l['primary_tileset']); sa=attrs(l['secondary_tileset'])
    d=open(f"{root}/{l['blockdata_filepath']}",'rb').read(); W=l['width']; H=l['height']
    def g(x,y):
        if not(0<=x<W and 0<=y<H): return False
        v=struct.unpack_from('<H',d,(y*W+x)*2)[0]; m=v&0x3ff
        a=pa[m] if m<512 else sa[m-512]
        return (a&0xff)==0x02 and ((v>>10)&3)==0
    return g,W,H
layout=sys.argv[1]
gv,W,H=load('/home/claude/tools-src/pret',layout)
gh,_,_=load('/home/claude/pokemon-emerald-wallys-journey',layout)
c=[(x,y) for y in range(H) for x in range(W) if all(gv(x,y+k) and gh(x,y+k) for k in (-1,0,1)) and gv(x-1,y) and gv(x+1,y)]
# prefer the middle of the biggest patch: most grass neighbours
c.sort(key=lambda p:-sum(gv(p[0]+i,p[1]+j) for i in range(-3,4) for j in range(-3,4)))
print(layout, len(c), c[:3])
