# Copycat draft B: FRLG Lass (official) recoloured: pink hair (her FRLG colour), blue/white outfit like her FRLG dress.
from PIL import Image
R='/home/claude/pokemon-emerald-wallys-journey/graphics/'
# hue-shifted pink ramp: highlights warmer, shadows cooler/purpler
PINK=[(255,204,214),(247,148,178),(214,90,140),(140,52,98)]
def recolor(path, mapping, keep=lambda x,y,i:False):
    im=Image.open(path); pal=im.getpalette()
    out=Image.new('RGBA',im.size,(0,0,0,0))
    for y in range(im.height):
        for x in range(im.width):
            i=im.getpixel((x,y))
            if i==0: continue
            c=tuple(pal[i*3:i*3+3])
            if i in mapping and not keep(x,y,i): c=mapping[i]
            out.putpixel((x,y),c+(255,))
    return out
# battle picture: hair 11 light,10 mid,9 dark (9 is also the shoes -> keep below y=56)
bp=recolor(R+'trainers/front_pics/lass_frlg.png',{11:PINK[1],10:PINK[2],9:PINK[3]},keep=lambda x,y,i:i==9 and y>=56)
bp.save('ccB_battle.png')
# overworld: hair 6,7 + its outline 13; dress 8,9,10 -> blue
# v3: white top + red skirt to match the v3 battle picture (Emerald Lass outfit)
# ribbon (11,12) made blue to match the battle picture's blue hair tie
ow=recolor(R+'object_events/pics/people/lass_frlg.png',{6:PINK[1],7:PINK[2],13:PINK[3],8:(222,230,238),9:(255,98,90),10:(197,65,65),11:(65,172,213),12:(65,106,164)})
ow.save('ccB_ow.png')
