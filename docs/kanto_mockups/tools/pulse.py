import math, recolor as C
from PIL import Image
frames=[]
seq=[('void',0.80+0.22*math.sin(i/8*2*math.pi)) for i in range(8)]*2+[('red',1.05),('red',0.85),('red',1.05),('red',0.85),('red',1.05),('red',0.85)]
cache={}
for name,p in seq:
    k=(name,round(p,3))
    if k not in cache:
        C.PULSE=p; C.run(name)
        im=Image.open(f'bay2_{name}_mons.png').convert('RGB')
        cache[k]=im.resize((im.width*3//4,im.height*3//4),Image.NEAREST)
    frames.append(cache[k])
frames[0].save('bay_pulse.gif',save_all=True,append_images=frames[1:],duration=[110]*16+[160]*6,loop=0)
