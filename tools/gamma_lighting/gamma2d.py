import numpy as np,sys
from PIL import Image
pud=np.load('/home/claude/ue/tex/puddles.npy')/255.0; rough=np.load('/home/claude/ue/tex/rough.npy')/255.0
def samp(t,u,v):
    h,w=t.shape; x=(u*w)%w; y=(v*h)%h; x0=np.floor(x).astype(int); y0=np.floor(y).astype(int); fx=x-x0; fy=y-y0
    x1=(x0+1)%w; y1=(y0+1)%h
    return (t[y0,x0]*(1-fx)+t[y0,x1]*fx)*(1-fy)+(t[y1,x0]*(1-fx)+t[y1,x1]*fx)*fy
def lin(c): return np.where(c<=0.04045,c/12.92,((c+0.055)/1.055)**2.4)
def srgb(c): c=np.clip(c,0,1); return np.where(c<=0.0031308,c*12.92,1.055*c**(1/2.4)-0.055)
M=lambda *v: np.array(v).reshape(3,3)
sRGB_2_AP1=M(0.61319,0.33951,0.04737,0.07021,0.91634,0.01345,0.02062,0.10957,0.86961)
AP1_2_sRGB=M(1.70505,-0.62179,-0.08326,-0.13026,1.14080,-0.01055,-0.02400,-0.12897,1.15297)
sRGB_2_AP0=M(0.4397010,0.3829780,0.1773350,0.0897923,0.8134230,0.0967616,0.0175440,0.1115440,0.8707040)
AP0_2_AP1=M(1.4514393161,-0.2365107469,-0.2149285693,-0.0765537734,1.1762296998,-0.0996759264,0.0083161484,-0.0060324498,0.9977163014)
Y=np.array([0.2722287168,0.6740817658,0.0536895174])
def film(lin_):
    slope,toe,sh,bk,wh=1.0,0.3,1.0,0.0,0.0
    a=lin_@sRGB_2_AP0.T
    mi=a.min(-1);ma=a.max(-1); s=(np.maximum(ma,1e-10)-np.maximum(mi,1e-10))/np.maximum(ma,1e-2)
    r,g,b=a[...,0],a[...,1],a[...,2]; ch=np.sqrt(np.maximum(b*(b-g)+g*(g-r)+r*(r-b),0)); yc=(b+g+r+1.75*ch)/3
    x=(s-0.4)/0.2; t=np.maximum(1-abs(0.5*x),0); sig=(1+np.sign(x)*(1-t*t))/2; gg=0.05*sig
    gl=np.where(yc<=2/3*0.08,gg,np.where(yc>=0.16,0,gg*(0.08/np.maximum(yc,1e-9)-0.5)))
    a=a*(1+gl)[...,None]
    h=np.degrees(np.arctan2(np.sqrt(3)*(a[...,1]-a[...,2]),2*a[...,0]-a[...,1]-a[...,2])); h=np.where(h>180,h-360,h)
    hw=np.clip(1-abs(2*h/135),0,1); hw=hw*hw*(3-2*hw); hw=hw*hw
    a[...,0]+=hw*s*(0.03-a[...,0])*(1-0.82)
    w=np.clip(a@AP0_2_AP1.T,0,65504); w=(w@Y)[...,None]*(1-0.96)+w*0.96
    ts=1+bk-toe; ss=1+wh-sh; bt=(0.18+bk)/ts-1
    tm=np.log10(0.18)-0.5*np.log((1+bt)/(1-bt))*(ts/slope); sm=(1-toe)/slope-tm; shm=sh/slope-sm
    L=np.log10(np.maximum(w,1e-10)); st=slope*(L+sm)
    tc=-bk+2*ts/(1+np.exp((-2*slope/ts)*(L-tm))); sc=(1+wh)-2*ss/(1+np.exp((2*slope/max(ss,1e-6))*(L-shm)))
    tc=np.where(L<tm,tc,st); sc=np.where(L>shm,sc,st)
    t=np.clip((L-tm)/(shm-tm),0,1); t=(3-2*t)*t*t
    tone=tc+(sc-tc)*t; tone=(tone@Y)[...,None]*(1-0.93)+tone*0.93
    return np.maximum(tone,0)
img=np.asarray(Image.open(sys.argv[1]).convert('RGB'))/255.0
H,W,_=img.shape; pxPerTile=float(sys.argv[3])
yy,xx=np.mgrid[0:H,0:W]
# Gamma axes: X = north (screen up), Y = east (screen right); 64 cm per tile
PX=-(yy/pxPerTile)*64.0; PY=(xx/pxPerTile)*64.0; T=float(sys.argv[4]) if len(sys.argv)>4 else 0.0
n=samp(rough,PX/888.25 - 0.0010*T, PY/888.25 - 0.0007*T)
c=samp(pud,(PX+n*40)/5000 - 0.0021*T,(PY+n*40)/5000 - 0.0030*T)
cloud=1-np.clip((c-0.164)*3.8417,0,1)*0.75
sun=np.array([0.788,0.738,0.700])*3/np.pi*0.6428
sky=np.array([1.0,0.701,0.271])*0.32*np.array([0.1305,0.3063,0.5345])
light=sun*cloud[...,None]+sky
hdr=lin(img)*light
EXPOSURE=2.06  # chosen so sunlit ground keeps its GBA colours (Gamma camera: 0.36)
# bloom: UE BloomSetup (global shader): amount = saturate((Rec709 lum * exposure - 0.4) * 0.5)
lum=hdr@np.array([0.212639,0.715169,0.072192])
bright=hdr*np.clip((lum*EXPOSURE-0.4)*0.5,0,1)[...,None]
from PIL import ImageFilter
def blurstage(a,k):
    h,w,_=a.shape; sw,sh=max(1,w>>k),max(1,h>>k)
    out=np.zeros_like(a)
    small=np.stack([np.asarray(Image.fromarray(a[...,c].astype(np.float32),'F').resize((sw,sh),Image.BILINEAR)) for c in range(3)],-1)
    sig=31/3.0
    from scipy.ndimage import gaussian_filter
    bl=np.stack([gaussian_filter(small[...,c],sig,mode='nearest') for c in range(3)],-1)
    return np.stack([np.asarray(Image.fromarray(bl[...,c],'F').resize((w,h),Image.BILINEAR)) for c in range(3)],-1)
tints=[0.3465,0.138,0.1176,0.066,0.066,0.061]
bloom=sum(t*blurstage(bright,k+1) for k,t in enumerate(tints))*6.0
hdr=(hdr+bloom)*EXPOSURE
ap=hdr@sRGB_2_AP1.T; ap=np.maximum((ap@Y)[...,None]*(1-1.25)+ap*1.25,0)
out=srgb(film(ap@AP1_2_sRGB.T)@AP1_2_sRGB.T)
Image.fromarray((out*255+0.5).astype(np.uint8)).save(sys.argv[2])
