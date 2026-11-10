import sys, json, struct, os, numpy as np
from PIL import Image
sys.argv=['x']
exec(open(__import__('os').path.join(__import__('os').path.dirname(__file__),'maprender.py')).read().split("if __name__")[0])
R=__import__('os').path.abspath(__import__('os').path.join(__import__('os').path.dirname(__file__),'..','..'))+'/'
MAP='Route102'; CX,CY,MW,MH=25,10,49,19
TREE=set(range(0x1ce,0x1e8))-{0x1d0,0x1d1,0x1d2,0x1d8,0x1d9,0x1da,0x1e0,0x1e1,0x1e2}
TRUNK={0x1dc,0x1dd,0x1e4,0x1e5,0x1e6,0x1e7}
GRASS={0xd}
GROUND=0x1
# layout copy with trees removed
lay=[l for l in json.load(open(R+'data/layouts/layouts.json'))['layouts'] if l['id']=='LAYOUT_'+'ROUTE102'][0]
blkpath=R+lay['blockdata_filepath']; orig=open(blkpath,'rb').read()
W=lay['width']
cells=[struct.unpack_from('<H',orig,i*2)[0] for i in range(len(orig)//2)]
ground=bytearray(orig)
for i,v in enumerate(cells):
    if v&0x3ff in TREE or (v&0x3ff in GRASS and os.environ.get('GMODE','old')!='flat'): struct.pack_into('<H',ground,i*2,(v&0xfc00)|GROUND)
full=np.asarray(render(MAP,CX,CY,MW,MH)).astype(np.float32)
open(blkpath,'wb').write(bytes(ground))
try:
    gnd=np.asarray(render(MAP,CX,CY,MW,MH)).astype(np.float32)
finally:
    open(blkpath,'wb').write(orig)
mask=(np.abs(full-gnd).sum(-1)>0)
x0=CX-MW//2; y0=CY-MH//2
# one clean tree, drawn from the tileset on plain grass (top, middle, trunk rows), used for every tree
import struct as _st
_m=json.load(open(R+'data/maps/Route102/map.json'))
_lay=[l for l in json.load(open(R+'data/layouts/layouts.json'))['layouts'] if l['id']==_m['layout']][0]
_pt,_pp,_pm=tsdir(_lay['primary_tileset']); _sT,_sp,_sm=tsdir(_lay['secondary_tileset'])
_pal=[loadpal(_pp[i]) if i<6 else loadpal(_sp[i]) for i in range(13)]
_tp=Image.open(R+_pt); _ts=Image.open(R+_sT); _mp=open(R+_pm,'rb').read(); _ms=open(R+_sm,'rb').read()
def _draw(grid):
    h=len(grid); w=len(grid[0]); out=Image.new('RGB',(w*16,h*16))
    for my in range(h):
        for mx in range(w):
            mt=grid[my][mx]; b,o=(_mp,mt) if mt<512 else (_ms,mt-512)
            e=[_st.unpack_from('<H',b,(o*8+k)*2)[0] for k in range(8)]
            for layer in (0,1):
                for k in range(4):
                    v=e[layer*4+k]; n=v&0x3ff; im,bb=(_tp,n) if n<512 else (_ts,n-512)
                    tw=im.width//8; t=im.crop(((bb%tw)*8,(bb//tw)*8,(bb%tw)*8+8,(bb//tw)*8+8))
                    if (v>>10)&1: t=t.transpose(Image.FLIP_LEFT_RIGHT)
                    if (v>>11)&1: t=t.transpose(Image.FLIP_TOP_BOTTOM)
                    for j in range(8):
                        for i2 in range(8):
                            c=t.getpixel((i2,j))
                            if c==0 and layer==1: continue
                            out.putpixel((mx*16+(k%2)*8+i2,my*16+(k//2)*8+j),_pal[v>>12][c])
    return np.asarray(out).astype(np.float32)
_tree=_draw([[0x1ce,0x1cf],[0x1d6,0x1d7],[0x1e6,0x1e5]]); _gr=_draw([[1,1],[1,1],[1,1]])
TREE_RGB=_tree; TREE_A=np.abs(_tree-_gr).sum(-1)>0
GMODE=os.environ.get('GMODE','old'); GSQ=float(os.environ.get('GSQ','1'))
_tu=_draw([[0xd]]); _g1=_draw([[1]])
_ta=np.abs(_tu-_g1).sum(-1)>0
# keep only the tuft's blades: drop single stray pixels that are just ground speckle
_ys=np.where(_ta.any(1))[0]
_tu=_tu[_ys[0]:]; _ta=_ta[_ys[0]:]
if GSQ!=1:
    _im=Image.fromarray(np.dstack([_tu,_ta*255]).astype(np.uint8),'RGBA').resize((16,max(4,round(_tu.shape[0]*GSQ))),Image.NEAREST)
    _arr=np.asarray(_im).astype(np.float32); _tu=_arr[...,:3]; _ta=_arr[...,3]>127
TUFT_RGB=_tu; TUFT_A=_ta
Image.fromarray(_tree.astype(np.uint8)).resize((96,144),0).save('canon_tree.png')
bbs=[]
for i,v in enumerate(cells):
    if v&0x3ff in (0x1dc,0x1e4,0x1e6):   # left trunk cell = one whole tree (32 wide, 48 tall)
        mx,my=i%W,i//W; px=(mx-x0)*16; py=(my-y0)*16
        bbs.append((py+16,px,TREE_RGB,TREE_A,1,px))
    if v&0x3ff in GRASS and GMODE!='flat':
        mx,my=i%W,i//W; px=(mx-x0)*16; py=(my-y0)*16
        if px<0 or py<0 or px+16>full.shape[1] or py+16>full.shape[0]: continue
        if GMODE=='old':
            rgb=np.zeros((32,16,3),np.float32); a=np.zeros((32,16),bool)
            rgb[16:]=full[py:py+16,px:px+16]; a[16:]=mask[py:py+16,px:px+16]
        else:
            rgb=TUFT_RGB; a=TUFT_A
        bbs.append((py+16,px,rgb,a,2,px))
haze=np.array([190,226,215],np.float32)
def build(camU,camV,near=1.0,far=3.2,out_h=112,ss=4,upright=1.0,haze_amt=0.45,haze_rows=45,spots=(),patch=(40,22)):
    B=out_h*near/(far-near); A=far*B
    ys=np.linspace(0,out_h,out_h*ss+1); scale=A/(ys+B)
    vpos=np.concatenate([[0],np.cumsum((scale[1:]+scale[:-1])/2*(ys[1]-ys[0]))]); vpos=camV-(vpos[-1]-vpos)
    # ground at ss resolution
    H=out_h*ss; Wd=240*ss
    canvas=np.zeros((H,Wd,3),np.float32); lab=np.zeros((H,Wd),np.uint8)
    for j in range(H):
        s=scale[j]; v=vpos[j]
        u=camU+((np.arange(Wd)+.5)/ss-120)*s
        ui=u.astype(int); vi=int(v)
        ok=(ui>=0)&(ui<gnd.shape[1])&(vi>=0)&(vi<gnd.shape[0])
        row=np.tile(haze,(Wd,1)); row[ok]=gnd[vi,ui[ok]]; canvas[j]=row
    # battle spots: world position under each Pokemon's feet
    wspots=[]
    for (sx_,sy_) in spots:
        j=min(int(sy_*ss),len(vpos)-1); wspots.append((camU+(sx_-120)*scale[j], vpos[j]))
    def in_spot(u,v,k=1.0):
        return any(((u-a)/(patch[0]*k))**2+((v-b)/(patch[1]*k))**2<1 for a,b in wspots)
    # drop trees/grass inside the spots, then put a ring of tall grass around each spot
    gone=[b for b in bbs if in_spot(b[1]+b[2].shape[1]/2,b[0]-8,1.3)]
    gk={(b[0],b[5]) for b in gone if b[4]==1}
    keep=[b for b in bbs if not in_spot(b[1]+b[2].shape[1]/2,b[0]-8,1.3) and not (b[4]==1 and (b[0],b[5]) in gk)]
    grass_img=[b for b in bbs if b[4]==2][0] if GMODE!='flat' else None
    extra=[]
    for (a,b) in wspots:
        for gy in range(-3,4):
            for gx in range(-4,5):
                u=a+gx*16-8+ (8 if gy%2 else 0); v=b+gy*10
                d=((u+8-a)/patch[0])**2+((v-b)/patch[1])**2
                if d<1.2 and grass_img is not None: extra.append((v+8,u,grass_img[2],grass_img[3],2,u))
    # billboards far -> near
    for (vb,px,rgb,a,kind,_t) in sorted(keep+extra,key=lambda b:b[0]):
        if vb<vpos[0] or vb>vpos[-1]: continue
        j=int(np.searchsorted(vpos,vb)); j=min(j,H-1); s=scale[j]
        w=rgb.shape[1]/s*ss; h=rgb.shape[0]/s*ss*upright
        if w<1 or h<1: continue
        xs=((px-camU)/s+120)*ss; xe=((px+rgb.shape[1]-camU)/s+120)*ss
        im=Image.fromarray(np.dstack([rgb,a*255]).astype(np.uint8),'RGBA').resize((max(1,round(xe)-round(xs)),max(1,round(h))),Image.NEAREST if s<1.5 else Image.BOX)
        arr=np.asarray(im).astype(np.float32)
        X=int(round(xs)); Y=int(round(j-h))
        if kind==1 and Y<0: continue   # tree would be cut by the top of the screen: leave it out
        x1,y1=max(0,X),max(0,Y); x2,y2=min(Wd,X+arr.shape[1]),min(H,Y+arr.shape[0])
        if x2<=x1 or y2<=y1: continue
        sub=arr[y1-Y:y2-Y,x1-X:x2-X]; al=(sub[...,3:]/255>0.5).astype(np.float32)
        canvas[y1:y2,x1:x2]=canvas[y1:y2,x1:x2]*(1-al)+sub[...,:3]*al
        lab[y1:y2,x1:x2][al[...,0]>0]=kind
    # shadow under each spot (soft dark oval on the ground)
    for (a,b) in wspots:
        for jj in range(H):
            v=vpos[jj]; 
            if abs(v-b)>patch[1]*0.55: continue
            u=camU+((np.arange(Wd)+.5)/ss-120)*scale[jj]
            dd=((u-a)/(patch[0]*0.75))**2+((v-b)/(patch[1]*0.45))**2
            sh=np.clip(1-dd,0,1)*0.35
            canvas[jj]*= (1-sh)[:,None]
    out=Image.fromarray(canvas.clip(0,255).astype(np.uint8)).resize((240,out_h),Image.BOX)
    o=np.asarray(out).astype(np.float32)
    for y in range(out_h):
        al=max(0,1-y/haze_rows)*haze_amt; o[y]=o[y]*(1-al)+haze*al
    labs=np.asarray(Image.fromarray(lab).resize((240,out_h),Image.NEAREST))
    return o.clip(0,255).astype(np.uint8), labs





camU=(12-x0)*16+8+16; camV=(13-y0)*16+8
o,labs=build(camU,camV,near=float(os.environ.get('NEAR','0.7')),far=float(os.environ.get('FAR','2.2')),ss=1,haze_amt=0.15,haze_rows=35,spots=((176,76),(64,108)),patch=(31*float(os.environ.get('NEAR','0.7'))/0.7,17*float(os.environ.get('NEAR','0.7'))/0.7))
tag=os.environ.get('TAG','x'); np.save('g_%s_rgb.npy'%tag,o); np.save('g_%s_lab.npy'%tag,labs)

