# gapscan.py OUT_CNT DIR  : magenta (F7 gap test) count + box per shot at full size; keeps a 960-wide jpg, deletes the pngs
import sys,glob,os
from PIL import Image
out=open(sys.argv[1],'a')
for p in sorted(glob.glob(sys.argv[2]+'/*.p.png')):
    im=Image.open(p).convert('RGB'); w,h=im.size
    r,g,b=im.split()
    m=Image.eval(r,lambda v:255 if v>200 else 0)
    m=Image.composite(m,Image.new('L',im.size,0),Image.eval(g,lambda v:255 if v<60 else 0))
    m=Image.composite(m,Image.new('L',im.size,0),Image.eval(b,lambda v:255 if v>200 else 0))
    n=m.histogram()[255]
    out.write('%s %d %s\n'%(os.path.basename(p)[:-6],n,m.getbbox()))
    os.makedirs(sys.argv[2]+"_jpg",exist_ok=True); im.resize((960,int(960*h/w))).save(sys.argv[2]+"_jpg/"+os.path.basename(p)[:-6]+".jpg",quality=85)
for p in glob.glob(sys.argv[2]+'/*.png'): os.remove(p)
