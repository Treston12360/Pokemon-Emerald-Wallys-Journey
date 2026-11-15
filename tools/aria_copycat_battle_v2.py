# Copycat battle picture v2: FRLG Lass + straight-across bangs (owner: "bangs straight across, long on the sides"),
# cast shadow under the bangs, pink hair (FRLG Copycat colour), hue-shifted ramp. Overworld unchanged from v1.
from PIL import Image
R='/home/claude/pokemon-emerald-wallys-journey/graphics/'
src=Image.open(R+'trainers/front_pics/lass_frlg.png'); pal=src.getpalette()
im=src.copy()
def row(y,x0,s):
    for i,ch in enumerate(s):
        if ch!='.': im.putpixel((x0+i,y),int(ch,16))
row(14,30,'bbbbabbbbba')      # top of bangs, light from top-left
row(15,31,'bbbbbabbab9')
row(16,30,'abbaabaaa9a')
row(17,30,'9aaaaaaaaa9')      # the straight cut: mid tone, dark only at the ends
for x in (33,36,37,38):       # cast shadow on the forehead line under the bangs
    if im.getpixel((x,18))==1: im.putpixel((x,18),2)
PINK={11:(247,148,178),10:(214,90,140),9:(140,52,98)}
LIGHT=(255,206,220); DARK=(140,52,98)
hair_extra={}
for y in range(8,17):            # official sprite uses skin colours as hair shine/outline - recolour them inside the hair
    for x in range(24,46):
        i=im.getpixel((x,y))
        nb=[im.getpixel((x+dx,y+dy)) for dx,dy in ((1,0),(-1,0),(0,1),(0,-1)) if 0<=x+dx<64 and 0<=y+dy<64]
        if i in (1,2,3) and sum(n in (9,10,11) for n in nb)>=3: hair_extra[(x,y)]=LIGHT
        if i==4 and sum(n in (9,10,11) for n in nb)>=2: hair_extra[(x,y)]=DARK
out=Image.new('RGBA',im.size,(0,0,0,0))
for y in range(64):
    for x in range(64):
        i=im.getpixel((x,y))
        if i==0: continue
        c=tuple(pal[i*3:i*3+3])
        if i in PINK and not (i==9 and y>=56): c=PINK[i]
        if (x,y) in hair_extra: c=hair_extra[(x,y)]
        out.putpixel((x,y),c+(255,))
out.save('ccC_battle.png')
G=(115,197,164,255)
b=Image.new('RGBA',(64*6,64*6),G); b.alpha_composite(out.resize((384,384),Image.NEAREST))
o=Image.open('ccB_battle.png'); a=Image.new('RGBA',(384,384),G); a.alpha_composite(o.resize((384,384),Image.NEAREST))
v=Image.new('RGBA',(800,384),(24,24,30,255)); v.paste(a,(0,0)); v.paste(b,(410,0)); v.save('ccC_view.png')
