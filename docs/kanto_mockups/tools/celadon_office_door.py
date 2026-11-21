# Celadon exterior mockup: give the doorless building at (44..47, 17..21) a door, the FRLG way
# (same metatiles as Celadon's other doored houses: 314 315 316 / 2e5 03d 2e7 / 30e 16e 30f).
import render as R,struct,sys
l=R.layout('CeladonCity_Layout'); ts=R.TS(l['primary_tileset'],l['secondary_tileset'])
b=open(R.ROOT+'/'+l['blockdata_filepath'],'rb').read(); W,H=l['width'],l['height']
ids=[struct.unpack('<H',b[i*2:i*2+2])[0]&0x3ff for i in range(W*H)]
def put(x,y,v): ids[y*W+x]=v
for x,v in zip(range(44,47),(0x314,0x315,0x316)): put(x,20,v)
for x,v in zip(range(44,47),(0x2e5,0x03d,0x2e7)): put(x,21,v)
for x,v in zip(range(44,47),(0x30e,0x16e,0x30f)): put(x,22,v)
out=sys.argv[1] if len(sys.argv)>1 else '/tmp/claude-0/-home-claude/2bb72568-d7ee-56f4-8a6d-fd6081a29497/scratchpad/sim/celadon_door.png'
im=R.render_ids(ts,ids,W,H,out)
im.crop((34*16,12*16,56*16,27*16)).resize((22*48,15*48),R.Image.NEAREST).save(out.replace('.png','_crop.png'))
