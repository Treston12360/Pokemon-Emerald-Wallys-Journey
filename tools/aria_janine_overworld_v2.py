# ARIA Janine overworld v2: runs v1 (tools/aria_janine_overworld.py, expected as j3.py in cwd) then adds
# ninja pants (skin below the sash -> suit) and the pink scarf tail. Writes j4.png.
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
for f in front:  # end hangs down over her left shoulder
    for (x,y,c) in [(10,25,PB),(10,26,PC),(10,27,PD)]: put(f,x,y,c)
for f in back:   # knot at the back of the neck, ends hanging
    for (x,y,c) in [(7,25,PC),(8,25,PC),(7,26,PD),(8,27,PD)]: put(f,x,y,c)
out.save('j4.png')
Z=8
bg=Image.new('RGBA',(W*Z,32*Z),(115,197,164,255))
bg.alpha_composite(out.resize((W*Z,32*Z),Image.NEAREST)); bg.save('j4_big.png')
