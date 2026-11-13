import colorsys, importlib, sys
from PIL import Image
import compose, render as R
def q(c): return tuple(min(248,max(0,int(round(v/8))*8)) for v in c)   # GBA 15-bit colour
PULSE=1.0
def scheme(name):
    def f(rgb):
        r,g,b=[v/255 for v in rgb];h,s,v=colorsys.rgb_to_hsv(r,g,b)
        blue = 0.55<h<0.72 and s>0.35          # fluid / water
        cyan = 0.42<h<=0.55 and s>0.25         # screens, lights
        green= 0.15<h<0.42 and s>0.12           # floors
        plat = 0.38<h<0.62 and s<=0.35 and not (blue or cyan and s>0.25)
        warm = (h<0.12 or h>0.9) and s>0.3      # stools, warning parts
        hue,acc = (0.76,0.50) if name=='void' else (0.995,0.50)
        if blue:  h,s,v = hue, min(1,s*1.2), (0.35+0.65*v)*PULSE   # glowing fluid
        elif cyan: h,s,v = acc, 0.55, min(1,0.45+0.75*v)          # screens stay lit
        elif plat and not cyan: h,s,v = (0.76 if name=='void' else 0.0), 0.30, 0.16+0.40*v**1.5   # pod platforms, faint glow tint
        elif green: h,s,v = hue-0.04 if name=='void' else 0.02, 0.22, 0.10+0.48*v**1.4   # dark floor, kept contrast
        elif warm: h,s,v = (0.92 if name=='void' else 0.08), s, v  # accents
        else:
            # metal: dark body, bright edges stay bright (rails, highlights)
            s = (0.10+0.20*s) if name=='void' else (0.04+0.06*s)
            v = 0.10+0.95*v**1.6
            h = 0.70 if name=='void' else 0.0
        return q(tuple(int(x*255) for x in colorsys.hsv_to_rgb(h,s,min(1,v))))
    return f
def run(name):
    importlib.reload(R);importlib.reload(compose)
    orig=R.TS.__init__
    def init(self,*a):
        orig(self,*a);fx=scheme(name)
        self.pal=[[c if i==0 else fx(c) for i,c in enumerate(p)] for p in self.pal]
    R.TS.__init__=init
    src=open('bay2.py').read().replace("'bay2'","'bay2_"+name+"'").replace("'bay2.png'","'bay2_"+name+".png'").replace("bay2_mons","bay2_"+name+"_mons")
    tint={'void':(150,60,210),'red':(220,40,40)}[name]
    src=src.replace("(int(r*.72+40*.28),int(g*.72+90*.28),int(b*.72+200*.28),255)",f"(int(r*.7+{tint[0]}*.3),int(g*.7+{tint[1]}*.3),int(b*.7+{tint[2]}*.3),255)")
    exec(compile(src,'bay2','exec'),{'__name__':'x'})
for n in ('void','red'): run(n)
a=Image.open('bay2_void_mons.png');b=Image.open('bay2_red_mons.png')
c=Image.new('RGB',(a.width*2+16,a.height),(10,10,12));c.paste(a,(0,0));c.paste(b,(a.width+16,0))
c.resize((c.width*3//2,c.height*3//2),Image.NEAREST).save('bay_schemes.png')
