# Ch. 5 office v2 picture (owner fixes 2026-11-21): exterior crop + interior with people, from the REAL map data
# written by tools/aria_celadon_office_map.py. Usage (repo root):
#   python3 tools/aria_celadon_office_map.py --preview OUT/off2.png   (+ ext2_crop.png from render.py, see commit)
#   python3 docs/kanto_mockups/tools/celadon_office_v2_sheet.py OUT
import sys
from PIL import Image, ImageDraw, ImageFont
S=sys.argv[1]; P='graphics/object_events/pics/people/'
def spr(f,frame):
    src=Image.open(P+f); fr=src.convert('RGBA').crop((frame*16,0,frame*16+16,32))
    if src.mode=='P':
        px=src.crop((frame*16,0,frame*16+16,32)); m=Image.new('L',(16,32)); m.putdata([0 if v==0 else 255 for v in px.getdata()]); fr.putalpha(m)
    return fr
base=Image.open(S+'/off2.png').convert('RGBA')
D,U,L=0,1,2
for f,fr,x,y in [('worker_f.png',D,3,5),('varnell.png',D,9,3),('blue.png',L,12,6),('scientist.png',U,1,6),('scientist.png',D,6,2),('scientist.png',L,1,8),('brendan/walking.png',U,8,2)]:
    base.alpha_composite(spr(f,fr),(x*16,y*16-16))
big=base.resize((13*48,11*48),Image.NEAREST); d=ImageDraw.Draw(big)
for x,y in [(8,2),(9,2)]: d.rectangle((x*48+2,y*48+2,x*48+45,y*48+45),outline=(255,220,0),width=3)
ext=Image.open(S+'/ext2_crop.png').convert('RGBA')
W=ext.width+big.width+60; top=40+max(ext.height,big.height); H=top+170
o=Image.new('RGB',(W,H),(24,24,30)); o.paste(ext,(20,40)); o.paste(big,(ext.width+40,40)); d=ImageDraw.Draw(o)
ft=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',15)
d.text((20,12),'OUTSIDE: path opened straight from the street to the door; sign moved one tile right of it',fill='white',font=ft)
d.text((ext.width+40,12),'INSIDE (13x11): the drawing has clear floor under it (yellow boxes)',fill='white',font=ft)
for i,t in enumerate(['Fence sign: "Varnell Research Office". It was blocking the path; now it stands right of the path, nothing between street and door.',
 "Drawing moved off Varnell's desk: stand on a yellow tile, face up, press A. Optional, any time.",
 "Varnell's desk moved right, under the phone and window. Meeting table, reception desk, staff desk, boxes and plant kept.",
 'People: receptionist behind her desk, Varnell beside his desk, Blue across the table, three staff (5.3 lines).',
 'Player shown on a reading tile. When Blue leaves he walks round the top of the table and down the middle walkway.']):
    d.text((20,top+20+i*26),t,fill='white',font=ft)
o.save('docs/kanto_mockups/ch5_celadon_office_v2.png')
