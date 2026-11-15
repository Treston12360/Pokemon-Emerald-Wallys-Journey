# ARIA v5 (Treston 2026-11-15: "Can you go even crazier on it?"): re-lights the v4 draft per pixel.
# Usage (from tools/): python3 aria_varnell_overworld_relight.py ../graphics/hicolor/varnell_overworld_v4_draft.png OUT.png
# Needs ramps.py beside it. DRAFT for approval, not in the game.
# Varnell overworld v5: v4's parts, re-lit per pixel (volume, rim light, contact shadow) + hand details.
import sys, math
from PIL import Image
sys.path.insert(0, sys.argv[0].rsplit('/',1)[0] if '/' in sys.argv[0] else '.')
src = Image.open(sys.argv[1]).convert('RGBA'); W, H = src.size; sp = src.load()
exec(open((sys.argv[0].rsplit('/',1)[0] + '/' if '/' in sys.argv[0] else '') + 'aria_varnell_ramps.py').read())
KIND = {0:'down',1:'up',2:'side',3:'down',4:'down',5:'up',6:'up',7:'side',8:'side',9:'down'}

def mat_of(c):
    for name, ramp in MATS.items():
        if c in ramp: return name
    return None

out = Image.new('RGBA', (W, H), (0,0,0,0)); op = out.load()
for f in range(W // 16):
    k = KIND[f]
    M = [[None]*16 for _ in range(H)]
    for y in range(H):
        for x in range(16):
            c = sp[f*16+x, y]
            if c[3] == 0: continue
            M[y][x] = mat_of(c[:3]) or 'out'
    def m(x, y): return M[y][x] if 0 <= x < 16 and 0 <= y < H else None
    for y in range(H):
        for x in range(16):
            mt = M[y][x]
            if mt is None: continue
            if mt == 'out':
                continue
            # normal from the part's own mask: towards where the part ends
            nx = (1 if m(x+1,y) != mt else 0) - (1 if m(x-1,y) != mt else 0)
            ny = (1 if m(x,y+1) != mt else 0) - (1 if m(x,y-1) != mt else 0)
            # distance to the part's edge (up to 3)
            d = 0
            for r in range(1, 4):
                if all(m(x+dx, y+dy) == mt for dx in range(-r, r+1) for dy in range(-r, r+1) if abs(dx)+abs(dy) == r):
                    d = r
                else: break
            lit = (-nx * 0.7 - ny * 0.9) * 0.5 + 0.5          # light from up-left: edges facing it brighter
            vol = 0.15 * min(d, 2)                              # rounder inside
            t = 0.55 * (1 - lit) + 0.25 * (y - 11) / 20 + (0.0 if mt in ('lens',) else -vol)
            t += BIAS.get(mt, 0.0)
            # contact shadows: coat just under the head / chin, skin under the hair
            if mt in ('coat', 'shirt') and any(m(x, y-j) in ('skin', 'hair', 'beard') for j in (1, 2)): t += 0.22
            if mt == 'skin' and m(x, y-1) == 'hair': t += 0.25
            # rim light: the shadow side's edge picks up a cool light
            rim = mt in ('hair', 'coat') and (m(x+1, y) in (None, 'out')) and k != 'up'
            op[f*16+x, y] = ramp_at(mt, t, rim) + (255,)
    # outlines: dark, tinted by the part inside, a little lighter where lit (up-left)
    for y in range(H):
        for x in range(16):
            if M[y][x] != 'out': continue
            inside = [m(x+dx, y+dy) for dx, dy in ((1,0),(-1,0),(0,1),(0,-1)) if m(x+dx, y+dy) not in (None, 'out')]
            base = OUTLINE.get(inside[0] if inside else 'def', OUTLINE['def'])
            litside = m(x+1, y) not in (None, 'out') or m(x, y+1) not in (None, 'out')
            op[f*16+x, y] = tuple(min(255, v + (14 if litside else 0)) for v in base) + (255,)
    details(op, f, k, M)
out.save(sys.argv[2])
