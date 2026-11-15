# ARIA (Treston 2026-11-15: "Go even crazier on varnells sprite ... super hq his Overworld sprite"): repaints
# graphics/object_events/pics/people/varnell.png frame by frame (same size, frames, animation) in full colour after
# his battle portrait (graphics/hicolor/trainers/varnell.png). Usage: python3 tools/aria_varnell_overworld_paint.py OUT.png
# DRAFT for approval: not in the game yet (graphics/hicolor/list.txt does not list it).
from PIL import Image, ImageDraw
import sys
SRC='/home/claude/pokemon-emerald-wallys-journey/graphics/object_events/pics/people/varnell.png'
im=Image.open(SRC); px=im.load(); W,H=im.size
NF=W//16
HAIR=[(178,180,194),(132,134,150),(98,98,116),(70,70,86),(48,48,60)]
SKIN=[(250,222,198),(234,192,162),(200,150,120),(150,102,84)]
GLOVE=[(104,106,122),(58,58,70),(30,30,38)]
COAT=[(252,251,255),(228,226,242),(194,190,218),(152,148,182),(110,106,142)]
SHIRT=[(62,62,76),(34,34,42),(20,20,26)]
PANTS=[(76,78,94),(50,52,64),(34,34,44)]
SHOE=[(96,94,108),(36,34,42),(18,18,22)]
LENS=[(54,64,82),(30,34,46),(150,190,220)]
RED=[(226,52,58),(255,140,120)]
OUT={'hair':(44,42,58),'skin':(92,52,44),'coat':(60,64,86),'dark':(16,16,22),'def':(26,26,34)}

def cls(f,x,y):
    v=px[f*16+x,y]
    return v
def frame_kind(f):
    return {0:'down',1:'up',2:'side',3:'down',4:'down',5:'up',6:'up',7:'side',8:'side',9:'down'}[f]

def body_top(f):
    # first row of the body (coat): where index B/C/D/E first appears below the head
    for y in range(20,32):
        row=[px[f*16+x,y] for x in range(16)]
        if sum(1 for v in row if v in (0xB,0xC,0xD))>=2: return y
    return 23

out=Image.new('RGBA',(W,H),(0,0,0,0)); op=out.load()
for f in range(NF):
    kind=frame_kind(f); bt=body_top(f)
    # face rows: rows with skin above body
    for y in range(H):
        for x in range(16):
            v=px[f*16+x,y]
            if v==0: continue
            c=None
            light = (15-x)/15*0.5 + (31-y)/31*0.5   # lit from top-left
            if v in (5,6,7) and y<bt:
                # hair / brows / glasses frame
                base={5:1,6:2,7:3}[v]
                if light>0.62 and base>0: base-=1
                if y<=12 and v==5: base=0
                c=HAIR[base]
            elif v in (1,2,3) and y<bt and f==9 and x<=4 and y>=17:
                c=GLOVE[{1:0,2:1,3:2}[v]]   # the hand at his glasses: gloved
            elif v in (1,2,3) and y<bt:
                c=SKIN[{1:0,2:1,3:2}[v]]
            elif v==0xE and y<bt:
                # old white lenses -> dark tinted glasses
                c=LENS[1]
            elif v==4 and y<bt:
                c=SHIRT[1] if y==bt-1 else HAIR[4]
            elif v in (1,2,3) and y==bt and 4<=x<=11:
                c=SHIRT[1] if v!=1 else SHIRT[0]   # the neck: black turtleneck
            elif v in (1,2,3) and y>=bt:
                c=GLOVE[{1:0,2:1,3:1}[v]]
            elif v==4:
                c=SHOE[1]
            elif v==6 and y>=bt:
                c=SHIRT[1]
            elif v==7 and y>=bt:
                c=SHIRT[2]
            elif v==0xE:
                c=COAT[0] if light>0.45 else COAT[1]
            elif v==0xB:
                c=COAT[2]
            elif v==0xC:
                c=COAT[3]
            elif v==0xD:
                c=COAT[4] if y<bt+3 else PANTS[1]
            elif v==0xF:
                c=None  # outline later
            elif v in (8,9,0xA):
                c=RED[0]
            if c: op[f*16+x,y]=c+(255,)
    # outlines tinted by neighbour
    for y in range(H):
        for x in range(16):
            if px[f*16+x,y]!=0xF: continue
            nb=[]
            for dx,dy in ((1,0),(-1,0),(0,1),(0,-1)):
                xx,yy=x+dx,y+dy
                if 0<=xx<16 and 0<=yy<H and op[f*16+xx,yy][3]: nb.append(op[f*16+xx,yy][:3])
            k='def'
            if any(c in HAIR for c in nb): k='hair'
            elif any(c in SKIN for c in nb): k='skin'
            elif any(c in COAT for c in nb): k='coat'
            if y<bt and any(c==LENS[1] for c in nb): k='dark'
            op[f*16+x,y]=OUT[k]+(255,)
# hand details
BEARD=(86,80,88); BEARD2=(118,110,116); GLINT=(176,206,232)
for f in range(NF):
    kind=frame_kind(f); bt=body_top(f)
    # glasses glint: the top-left lens pixel
    lens=[(x,y) for y in range(H) for x in range(16) if op[f*16+x,y][:3]==LENS[1]]
    if lens:
        lens.sort(key=lambda p:(p[1],p[0])); x,y=lens[0]; op[f*16+x,y]=GLINT+(255,)
        if kind!='side':
            right=[p for p in lens if p[1]==y and p[0]>=8]
            if right: op[f*16+right[0][0],right[0][1]]=GLINT+(255,)
    if kind in ('down','side'):
        # goatee: the skin row just above the neck - its middle pixels
        rows=[y for y in range(bt) if any(op[f*16+x,y][:3] in SKIN for x in range(16))]
        if rows:
            y=rows[-1]
            xs=[x for x in range(16) if op[f*16+x,y][:3] in SKIN]
            if kind=='down':
                mid=[x for x in xs if 6<=x<=9]
            else:
                mid=xs[:3]
            for x in mid: op[f*16+x,y]=BEARD+(255,)
            # stubble on the row above at the chin's sides
            if kind=='down' and y-1>=0:
                for x in (6,9):
                    if op[f*16+x,y-1][:3] in SKIN: op[f*16+x,y-1]=BEARD2+(255,)
    if kind=='down':
        # red pin on his left chest (screen right)
        for y in range(bt+1,bt+4):
            if op[f*16+10,y][:3] in COAT[:3]:
                op[f*16+10,y]=RED[0]+(255,); break
out.save(sys.argv[1] if len(sys.argv)>1 else 'v1.png')
