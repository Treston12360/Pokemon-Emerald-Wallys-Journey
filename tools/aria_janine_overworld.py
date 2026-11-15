# ARIA: Janine overworld draft builder. Recolours the official FRLG Cooltrainer F (purple ponytail)
# to Janine's battle-art colours (indigo hair + ninja suit, pink hair tie / scarf / sash).
# Writes j3.png + j3_big.png in the cwd. Picture: docs/kanto_mockups/janine_overworld_v1.png.
# Not in game until Treston approves.
from PIL import Image
R='/home/claude/pokemon-emerald-wallys-journey/graphics/object_events/pics/people/'
src=Image.open(R+'cooltrainer_f.png')
pal=src.getpalette()
HAIR={8:(140,120,184),9:(96,76,140),10:(52,42,78)}
OUT={11:(104,84,140),12:(64,50,86),13:(36,29,47)}
PB,PC=(240,120,160),(196,76,120)
W,H=src.size
out=Image.new('RGBA',(W,H),(0,0,0,0))
idx=lambda x,y: src.getpixel((x,y))
scarf=set()
for f in range(W//16):
    for y in (23,24,25):
        x=0
        while x<16:
            if idx(f*16+x,y)==15:
                s=x
                while x<16 and idx(f*16+x,y)==15: x+=1
                if x-s>=3:
                    for xx in range(s,x): scarf.add((f*16+xx,y))
                    for xx in (s-1,x):
                        if 0<=xx<16 and idx(f*16+xx,y) in (12,13): scarf.add((f*16+xx,y))
            else: x+=1
for y in range(H):
    for x in range(W):
        i=idx(x,y)
        if i==0: continue
        col=tuple(pal[i*3:i*3+3])
        if i in HAIR: col=HAIR[i]
        if i in OUT: col=OUT[i]
        if i==12: col = PC if y<29 else OUT[13]
        if (x,y) in scarf: col=PB
        out.putpixel((x,y),col+(255,))
out.save('j3.png')
Z=8
bg=Image.new('RGBA',(W*Z,32*Z),(115,197,164,255))
bg.alpha_composite(out.resize((W*Z,32*Z),Image.NEAREST)); bg.save('j3_big.png')
