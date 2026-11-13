from compose import Map
B1='AquaHideout_B1F_Layout';B2='AquaHideout_B2F_Layout';RM='AquaHideout_UnusedRubyMap2_Layout'
m=Map([B1,B2,RM],40,27,0x228)
F=0x228
def room(x,y,w,h):
    # top wall (3 rows), side walls, bottom edge
    m.fill(x,y,w,1,0x2e0);m.fill(x,y+1,w,1,0x221);m.fill(x,y+2,w,1,0x229)
    m.fill(x+1,y+3,w-2,h-4,F)
    for j in range(h): m.put(x,y+j,0x20a);m.put(x+w-1,y+j,0x20a)
    m.put(x,y,0x211);m.put(x+w-1,y,0x211)
    m.fill(x+1,y+h-1,w-2,1,0x259);m.put(x,y+h-1,0x209);m.put(x+1,y+h-1,0x20b);m.put(x+w-1,y+h-1,0x209)
def door_h(x,y):  # opening in a bottom edge + next room's top wall below it
    for i in range(2): m.put(x+i,y,F);m.put(x+i,y+1,F);m.put(x+i,y+2,F);m.put(x+i,y+3,F)
def door_v(x,y):  # opening in a shared vertical wall
    m.put(x,y,F);m.put(x,y+1,F)
# outer void
m.fill(0,0,40,27,0x201)
# --- rooms
room(0,0,14,13)    # west: specimen hall
room(13,0,14,13)   # centre: REQUIEM core
room(26,0,14,13)   # east: reactor room
room(0,12,22,15)   # south-west: research floor
room(21,12,19,15)  # south-east: archive / quarters
# ---- REQUIEM core: central containment machine, monitored from two sides
m.fill(14,3,12,9,0x202)
m.stamp(B1,29,6,4,6,18,0)
m.stamp(B1,8,1,2,2,14,1);m.stamp(B1,8,1,2,2,16,1);m.stamp(B1,8,1,2,2,22,1);m.stamp(B1,8,1,2,2,24,1)
m.stamp(B1,24,18,2,2,15,6);m.stamp(B1,24,18,2,2,23,6)
m.stamp(B1,24,18,2,2,15,9);m.stamp(B1,24,18,2,2,23,9)
m.stamp(B1,5,6,2,2,19,8);m.stamp(B1,5,6,2,2,19,10) if False else None
m.stamp(B1,17,15,2,2,17,7);m.stamp(B1,22,15,2,2,21,7);m.stamp(B1,16,18,2,3,19,8)
# ---- reactor room: twin reactors + capsule chargers
m.stamp(B2,20,0,4,7,27,0);m.stamp(B2,26,0,4,7,32,0)
m.stamp(B2,17,6,2,4,37,3)
m.stamp(B2,21,7,2,4,28,7);m.stamp(B2,25,7,2,4,31,7);m.stamp(B2,28,7,2,4,34,7)
m.stamp(B2,20,8,1,2,30,7);m.stamp(B2,27,8,1,2,33,7)
# ---- specimen hall: three tanks, analysers
m.stamp(B1,41,6,9,3,2,3);m.stamp(B1,41,6,9,3,2,6);m.stamp(B1,41,6,9,3,2,9) if False else None
m.stamp(B1,41,6,9,3,2,3);m.stamp(B1,41,6,9,3,2,7)
m.stamp(B1,24,18,2,2,11,4);m.stamp(B1,24,18,2,2,11,8)
m.stamp(B1,5,2,2,4,11,0);m.stamp(B1,11,0,3,3,1,0) if False else None
m.stamp(B1,8,1,2,2,1,1);m.stamp(B1,8,1,2,2,3,1)
# ---- research floor (south-west): desk rows, sample cases, the big tank
m.stamp(B1,1,16,15,4,3,21)
m.stamp(RM,29,16,13,5,4,15)
m.stamp(B2,7,11,3,2,2,13);m.stamp(B2,7,11,3,2,15,13)
m.stamp(RM,28,0,3,3,9,12)
# ---- archive / quarters (south-east): crates, lounge
m.stamp(RM,38,1,4,3,23,15,skip=(0x22a,0x228));m.stamp(RM,38,1,4,3,27,16,skip=(0x22a,0x228));m.stamp(RM,52,1,2,3,35,17,skip=(0x228,))
m.stamp(B2,2,6,3,3,23,21);m.stamp(B2,2,6,3,3,26,21)
m.stamp(RM,26,7,9,5,30,13)
m.stamp(B2,33,0,1,1,0,0) if False else None

m.stamp(B2,25,17,8,4,31,20)
# ---- doorways
for (x,y) in [(13,8),(26,8)]: m.put(x,y,F);m.put(x,y+1,F);m.put(x+1 if x==26 else x,y,F)
for x in (6,7,30,31): 
    for y in range(12,16): m.put(x,y,F)
for y in (22,23): m.put(21,y,F)
# entrance lift
m.stamp(B1,28,0,3,3,10,27) if False else None
m.fill(9,26,4,1,0x228)
m.save('requiem')
