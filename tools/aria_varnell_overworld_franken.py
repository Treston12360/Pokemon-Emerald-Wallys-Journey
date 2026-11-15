# Varnell overworld, frankensprited from official sprites (Treston's asset rule: FRLG / Emerald assets):
# Steven's hair (Emerald), Scientist's face with glasses and lab coat (the sprite Varnell already uses),
# a small mustache / goatee as Blaine's is drawn; recoloured to Varnell's battle portrait.
from PIL import Image
import sys
B = '/home/claude/pokemon-emerald-wallys-journey/graphics/object_events/pics/people/'
st = Image.open(B+'steven.png'); sc = Image.open(B+'scientist.png')
sp, cp = st.load(), sc.load()
KIND = ['down','up','side','down','down','up','up','side','side','down']
# Varnell's colours (from the portrait), as overworld-style shades
HAIR = {0xB:(160,156,190), 0xC:(106,96,128), 0xD:(58,46,72)}          # Steven's hair greys -> purple-grey
SKIN = {1:(255,216,184), 2:(246,186,150), 3:(214,134,112)}
OUTL = {4:(123,65,65), 0xF:(0,0,0)}
COAT = {0xE:(255,255,255), 0xB:(222,216,240), 0xC:(170,160,204), 0xD:(96,86,128)}
GLOVE= (49,49,66); GLOVE2=(24,24,32)
NECK = (49,47,62); NECK2=(28,26,36)
PANTS= (62,56,80)
SHOE = (32,30,40)
GLASS_FRAME=(57,57,74)
STACHE=(139,133,148); STACHE2=(98,92,108)
RED=(222,49,57)
out = Image.new('RGBA', (160, 32), (0,0,0,0)); op = out.load()

def split_row(f):
    if KIND[f] == 'up':
        return 22
    for y in range(10, 22):
        if any(cp[f*16+x, y] == 0xE for x in range(16)):
            return y - 1          # the brow row and below from the Scientist
    return 18

for f in range(10):
    sf = f if f < 9 else 0          # Steven has 9 frames; frame 9 (glasses pose) takes his front head
    lift = -1 if f == 9 else 0       # the Scientist's frame 9 head stands a row higher
    sr = split_row(f)
    for y in range(32):
        for x in range(16):
            if y < sr:
                yy = y - lift
                v = sp[sf*16+x, yy] if 0 <= yy < 32 else 0
                if v == 0: continue
                if v in HAIR: c = HAIR[v]
                elif v in SKIN: c = SKIN[v]
                elif v == 4: c = OUTL[4]
                elif v == 0xF: c = (0,0,0)
                else: c = HAIR[0xD]
            else:
                v = cp[f*16+x, y]
                if v == 0: continue
                body = y >= 22 or (f == 9 and y >= 21)
                if v in (5,6,7) and not body: c = HAIR[{5:0xB,6:0xC,7:0xD}[v]] if v != 7 else GLASS_FRAME
                elif v in (1,2,3) and not body and f == 9 and x <= 4: c = GLOVE if v != 3 else GLOVE2  # hand at his glasses
                elif v in (1,2,3) and not body: c = SKIN[v]
                elif v in (1,2,3) and body: c = GLOVE if v != 3 else GLOVE2
                elif v == 4 and not body and f == 9 and x <= 4: c = (0,0,0)   # the gloved hand's outline: black (clothing)
                elif v == 4 and not body: c = OUTL[4]
                elif v == 4: c = SHOE
                elif v in (6,7) and body: c = NECK if v == 6 else NECK2
                elif v == 5 and body: c = NECK
                elif v in COAT:
                    c = COAT[v]
                    if body and v == 0xD and y >= 26: c = PANTS
                elif v == 0xF: c = (0,0,0)
                else: c = (255,0,255)
            op[f*16+x, y] = c + (255,)
    # Steven's row just above the split may leave hair sides with no outline into the Scientist's face: fine
    if KIND[f] == 'down':
        # mustache + goatee as Blaine's: on the Scientist's mouth rows (the '2's under the glasses)
        gy = split_row(f) + 1                       # the glasses row
        y = gy + 2                                  # the mouth row
        for x in (7, 8):
            if op[f*16+x, y][:3] in (SKIN[1], SKIN[2], SKIN[3]):
                op[f*16+x, y] = STACHE + (255,)
        for x in (6, 9):                            # its tips, a shade darker, on the skin either side
            if op[f*16+x, y][:3] in (SKIN[2], SKIN[3]):
                op[f*16+x, y] = STACHE2 + (255,)
        # the pin, his left chest
        for y in range(23, 27):
            if op[f*16+10, y][:3] == COAT[0xE]:
                op[f*16+10, y] = RED + (255,); break
out.save(sys.argv[1])
