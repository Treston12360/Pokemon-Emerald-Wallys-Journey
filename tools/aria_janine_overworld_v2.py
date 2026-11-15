# ARIA Janine overworld v2 (APPROVED, Treston 2026-11-15 'Sounds good'). Run in a folder that also holds
# tools/aria_janine_overworld.py copied as j3.py. Adds ninja pants + pink scarf tail/bow, writes j5.png.
# The game files graphics/object_events/pics/people/janine.png + palettes/janine.pal are j5.png indexed
# (index 0 = transparent (115,197,164), then the 14 colours sorted).
exec(open('j3.py').read().replace("out.save('j3.png')","").replace("Z=8","Z=8\nimport sys; sys.exit(0) if False else None"))
PD=(132,48,84)
K=(0,0,0)
def put(f,x,y,c): out.putpixel((f*16+x,y),c+(255,))
# 1) ninja pants: skin below the sash becomes suit, brown skin outline becomes black
for f in range(W//16):
    for y in range(28,H):
        for x in range(16):
            i=idx(f*16+x,y)
            if i in (1,2,3): put(f,x,y,OUT[12])
            elif i==4: put(f,x,y,K)
# 2) scarf tail
side=[2,7,8]; front=[0,3,4,9]; back=[1,5,6]
for f in side:   # trails behind her (to the right), with outline
    for (x,y,c) in [(10,24,PB),(11,24,PB),(11,25,PB),(12,25,PC),(12,26,PC),(13,26,PD),
                    (10,23,K),(11,23,K),(12,24,K),(13,25,K),(14,26,K),(11,26,K),(12,27,K),(13,27,K)]:
        if out.getpixel((f*16+x,y))[3]==0 or c!=K: put(f,x,y,c)
for f in front:  # end hangs down over her left shoulder, 2 px wide with a shade
    for (x,y,c) in [(10,25,PB),(11,25,PC),(10,26,PC),(11,26,PD),(10,27,PD)]: put(f,x,y,c)
for f in back:   # bow at the back of the neck: two light loops, dark centre, two ends
    for (x,y,c) in [(6,25,PB),(7,25,PD),(8,25,PD),(9,25,PB),(6,26,PC),(9,26,PC),(7,26,PC),(8,26,PC),(7,27,PD),(8,27,PD)]: put(f,x,y,c)
out.save('j5.png')
Z=8
bg=Image.new('RGBA',(W*Z,32*Z),(115,197,164,255))
bg.alpha_composite(out.resize((W*Z,32*Z),Image.NEAREST)); bg.save('j5_big.png')
