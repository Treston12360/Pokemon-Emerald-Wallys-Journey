# Part colour ramps, light -> dark (taken from the battle portrait's colours)
MATS = {
 'hair':  [(178,180,194),(132,134,150),(98,98,116),(70,70,86),(48,48,60)],
 'skin':  [(250,222,198),(234,192,162),(200,150,120),(150,102,84)],
 'lens':  [(54,64,82),(30,34,46),(150,190,220),(176,206,232)],
 'glove': [(104,106,122),(58,58,70),(30,30,38)],
 'coat':  [(252,251,255),(228,226,242),(194,190,218),(152,148,182),(110,106,142)],
 'shirt': [(62,62,76),(34,34,42),(20,20,26)],
 'pants': [(76,78,94),(50,52,64),(34,34,44)],
 'shoe':  [(96,94,108),(36,34,42),(18,18,22)],
 'red':   [(226,52,58),(255,140,120)],
 'beard': [(86,80,88),(118,110,116)],
}
RAMP = {
 'hair':  [(214,216,228),(176,178,194),(140,142,160),(108,108,128),(80,80,98),(56,56,72),(40,40,52)],
 'skin':  [(255,232,212),(246,210,182),(228,182,150),(204,152,122),(172,120,96),(130,88,74)],
 'lens':  [(70,84,108),(48,56,74),(32,36,50),(20,22,32)],
 'glove': [(122,124,142),(84,84,100),(56,56,68),(36,36,46),(22,22,30)],
 'coat':  [(255,255,255),(240,238,250),(222,218,238),(200,194,226),(174,166,206),(146,138,182),(116,108,154)],
 'shirt': [(74,72,90),(52,50,64),(36,34,46),(24,22,30)],
 'pants': [(88,90,108),(66,68,84),(48,50,64),(32,32,44)],
 'shoe':  [(120,118,134),(62,60,72),(32,30,38),(16,16,20)],
 'red':   [(255,120,110),(232,56,62),(170,30,40)],
 'beard': [(128,118,122),(100,92,98),(76,70,78)],
}
BIAS = {'hair': 0.30, 'coat': 0.02, 'skin': 0.05, 'lens': 0.45, 'red': 0.1, 'glove': 0.2, 'shirt': 0.2}
RIM = {'hair': (150,170,200), 'coat': (196,206,236)}
OUTLINE = {'hair': (38,36,52), 'skin': (88,50,44), 'coat': (64,58,92), 'lens': (16,16,22), 'shirt': (14,14,18),
           'glove': (14,14,18), 'pants': (20,20,28), 'shoe': (12,12,16), 'beard': (40,34,40), 'red': (90,20,24),
           'def': (26,24,34)}

def ramp_at(mt, t, rim):
    r = RAMP[mt]
    t = max(0.0, min(0.999, t))
    i = t * (len(r) - 1); a = int(i); b = min(a + 1, len(r) - 1); w = i - a
    c = tuple(int(r[a][j] * (1 - w) + r[b][j] * w + 0.5) for j in range(3))
    if rim and mt in RIM:
        c = tuple(int(c[j] * 0.6 + RIM[mt][j] * 0.4) for j in range(3))
    return c

def details(op, f, k, M):
    def m(x, y): return M[y][x] if 0 <= x < 16 and 0 <= y < len(M) else None
    put = lambda x, y, c: op.__setitem__((f*16+x, y), c + (255,))
    H = len(M)
    # hair strands: swept back - every other pixel on a diagonal a touch lighter
    for y in range(H):
        for x in range(16):
            if M[y][x] == 'hair' and (x + 2*y) % 7 == 0 and m(x-1, y) == 'hair':
                c = op[f*16+x, y][:3]; put(x, y, tuple(min(255, v + 34) for v in c))
    # lenses: top row lighter streak, glint
    lens = [(x, y) for y in range(H) for x in range(16) if M[y][x] == 'lens']
    if lens:
        for x, y in lens: put(x, y, (24, 26, 38))
        top = min(y for x, y in lens)
        lens.sort(key=lambda p: (p[1], p[0])); x, y = lens[0]; put(x, y, (120, 158, 196))

    # gloves and shoes: a shine
    for part, col in (('glove', (150, 152, 172)), ('shoe', (140, 138, 156))):
        pts = [(x, y) for y in range(H) for x in range(16) if M[y][x] == part]
        seen = set()
        for x, y in sorted(pts, key=lambda p: (p[1], p[0])):
            key = (x // 6)
            if key in seen: continue
            seen.add(key); put(x, y, col)
    if k == 'down':
        # belt buckle: the pants' top row, middle
        rows = sorted(set(y for y in range(H) for x in range(16) if M[y][x] == 'pants'))
        if rows:
            y = rows[0]
            for x in (7, 8):
                if M[y][x] in ('pants', 'shirt'): put(x, y, (200, 204, 216) if x == 7 else (150, 152, 168))
        # lapels: the coat pixel next to the black front, both sides, lighter
        for y in range(H):
            for x in range(1, 15):
                if M[y][x] == 'coat' and (m(x+1, y) == 'shirt' or m(x-1, y) == 'shirt'):
                    put(x, y, (246, 244, 255) if x < 8 else (214, 208, 234))
