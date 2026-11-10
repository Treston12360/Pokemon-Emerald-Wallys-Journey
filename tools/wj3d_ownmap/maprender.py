import struct, json, sys, os, re
from PIL import Image
R=__import__('os').path.abspath(__import__('os').path.join(__import__('os').path.dirname(__file__),'..','..'))+'/'
def tsdir(label):
    import glob
    n=re.sub(r'(?<!^)(?=[A-Z])','_',label.replace('gTileset_','')).lower()
    d=[x for x in glob.glob(R+'data/tilesets/*/'+n) if os.path.isdir(x)][0].replace(R,'')
    return d+'/tiles.png', [d+'/palettes/%02d.pal'%i for i in range(16)], d+'/metatiles.bin'
def loadpal(f):
    L=open(R+f).read().split()[3:]
    return [tuple(int(v) for v in L[j*3:j*3+3]) for j in range(16)]
def render(mapname, cx, cy, w=15, h=7):
    m=json.load(open(R+f'data/maps/{mapname}/map.json'))
    lay=[l for l in json.load(open(R+'data/layouts/layouts.json'))['layouts'] if l['id']==m['layout']][0]
    pt,ppal,pm=tsdir(lay['primary_tileset']); st,spal,sm=tsdir(lay['secondary_tileset'])
    pal=[loadpal(ppal[i]) if i<6 else loadpal(spal[i]) for i in range(13)]
    tp=Image.open(R+pt); ts=Image.open(R+st)
    mp=open(R+pm,'rb').read(); ms=open(R+sm,'rb').read()
    blk=open(R+lay['blockdata_filepath'],'rb').read(); W=lay['width']
    out=Image.new('RGB',(w*16,h*16),(0,0,0))
    def tile(n):
        im,b=(tp,n) if n<512 else (ts,n-512)
        tw=im.width//8; x,y=(b%tw)*8,(b//tw)*8
        return im.crop((x,y,x+8,y+8))
    for my in range(h):
      for mx in range(w):
        X=cx-w//2+mx; Y=cy-h//2+my
        mt=struct.unpack_from('<H',blk,(Y*W+X)*2)[0]&0x3ff
        b,o=(mp,mt) if mt<512 else (ms,mt-512)
        e=[struct.unpack_from('<H',b,(o*8+k)*2)[0] for k in range(8)]
        for layer in (0,1):
          for k in range(4):
            v=e[layer*4+k]; n=v&0x3ff; t=tile(n)
            if (v>>10)&1: t=t.transpose(Image.FLIP_LEFT_RIGHT)
            if (v>>11)&1: t=t.transpose(Image.FLIP_TOP_BOTTOM)
            p=pal[v>>12]
            for j in range(8):
              for i in range(8):
                c=t.getpixel((i,j))
                if c==0 and layer==1: continue
                if c==0 and layer==0: c=0
                out.putpixel((mx*16+(k%2)*8+i,my*16+(k//2)*8+j),p[c])
    return out
if __name__=='__main__':
    im=render(sys.argv[1],int(sys.argv[2]),int(sys.argv[3]))
    im.save(sys.argv[4])
