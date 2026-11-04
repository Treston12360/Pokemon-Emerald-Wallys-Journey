import sys, importlib.util, numpy as np
from PIL import Image
spec=importlib.util.spec_from_file_location('bd','/home/claude/pokemon-emerald-wallys-journey/tools/wj3d_backdrop.py'); bd=importlib.util.module_from_spec(spec); spec.loader.exec_module(bd)
a=bd.load_capture(sys.argv[1]).astype(np.uint8)
Image.fromarray(a).save(sys.argv[2])
im=Image.fromarray(a).resize((720,480),Image.NEAREST)
from PIL import ImageDraw
d=ImageDraw.Draw(im)
for x in range(0,240,20): d.line([(x*3,0),(x*3,8)],fill=(255,0,0)); d.text((x*3+1,9),str(x),fill=(255,0,0))
for y in range(0,160,20): d.line([(0,y*3),(8,y*3)],fill=(255,0,0)); d.text((10,y*3+1),str(y),fill=(255,0,0))
im.save(sys.argv[2].replace('.png','_x3.png'))
