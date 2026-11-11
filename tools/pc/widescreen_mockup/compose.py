import json
from PIL import Image
import os; S=os.environ.get('WS_DIR','build/ws_mockup')
meta=json.load(open(S+'/meta.json'))
def compose(scene, W, H):
    shots=[]
    for tag,(dx,dy) in meta[str(scene)].items():
        im=Image.open('%s/shots/%d/%s/f.ppm'%(S,scene,tag)).convert('RGB')
        shots.append((abs(dx)+abs(dy),dx*16,dy*16,im.load(),tag=='+0_+0'))
    shots.sort(key=lambda s:s[0])
    out=Image.new('RGB',(W,H),(255,0,255)); o=out.load(); miss=0
    for oy in range(H):
        for ox in range(W):
            rx=ox-W//2; ry=oy-H//2
            for _,px,py,p,main in shots:
                sx=rx-px+120; sy=ry-py+80
                if 0<=sx<240 and 0<=sy<160 and (main or not (100<=sx<140 and 48<=sy<104)):
                    o[ox,oy]=p[sx,sy]; break
            else: miss+=1
    return out,miss
if __name__=='__main__':
    for sc in range(4):
        for W,H in [(240,160),(288,160),(320,180),(384,216),(480,270)]:
            im,m=compose(sc,W,H); im.save('%s/c_%d_%dx%d.png'%(S,sc,W,H)); print(sc,W,H,'missing',m)
