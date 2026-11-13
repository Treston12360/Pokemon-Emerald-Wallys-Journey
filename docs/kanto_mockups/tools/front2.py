from compose import Map
E_='CinnabarIsland_PokemonLab_Entrance_Layout';L='CinnabarIsland_PokemonLab_Lounge_Layout';R_='CinnabarIsland_PokemonLab_ResearchRoom_Layout';X='CinnabarIsland_PokemonLab_ExperimentRoom_Layout'
W,H=34,20
m=Map([X,E_,L,R_],W,H,0x81)
def wall(x,y,w):
    m.fill(x,y,w,1,0x69);m.fill(x,y+1,w,1,0x71);m.fill(x,y+2,w,1,0x79)
wall(0,0,W);m.put(0,0,0x68);m.put(0,1,0x70);m.put(0,2,0x78);m.put(W-1,0,0x6a);m.put(W-1,1,0x72);m.put(W-1,2,0x7a)
for y in range(3,H-1): m.put(0,y,0x82)
m.fill(0,H-1,W,1,0x8)
# ===== north: the public face. Lobby west, lab centre, Varnell's office east
# lobby wall: windows, portrait, display cases
m.stamp(L,1,0,4,3,1,0)          # display case + shelf
m.stamp(L,5,0,5,3,5,0)          # window / portrait / window
# lab wall: the machine bank + the big analyser
m.stamp(X,3,0,9,3,11,0);m.stamp(X,12,0,2,3,20,0)
# office: terminal, desk PC, award plaque
m.stamp(R_,1,0,1,3,23,0);m.stamp(R_,3,0,2,3,24,0);m.stamp(R_,6,0,1,3,26,0)
m.stamp(R_,11,0,4,3,28,0)       # two display cases
# lobby: carpet meeting table + sofas
m.stamp(L,4,3,7,5,2,4);m.stamp(L,0,4,1,3,1,5)
m.stamp(L,13,8,2,2,1,10) if False else None
# lab floor: twin desk stations + notes table
m.stamp(X,2,3,4,6,11,3);m.stamp(X,5,4,5,4,15,4);m.stamp(X,11,4,4,5,18,4)
m.stamp(X,13,3,2,1,20,3) if False else None
# divider between lab and office: crates
for y in (3,4): m.put(22,y,0x99)
# office floor: notes table and sofa
m.stamp(R_,2,4,8,4,24,4);m.stamp(L,14,4,1,3,32,4)
# ===== partition wall with three doorways
wall(1,9,W-2)
m.stamp(L,10,0,5,3,6,9);m.stamp(L,0,0,4,3,23,9)
for x0 in (4,16,29):
    for x in (x0,x0+1):
        for y in (9,10,11): m.put(x,y,0x81)
# ===== south: specimen gallery + archive
for x in (2,6,10): m.stamp(R_,11,6,2,3,x,13)     # boxed specimen cabinets
m.stamp(R_,11,6,4,3,14,13)
for x in (8,9,10): m.stamp(R_,12,3,1,2,x,17) if False else None
m.stamp(L,0,8,2,2,1,16);m.stamp(L,13,8,2,2,31,16)  # plants
m.stamp(R_,12,3,3,2,20,12)                          # potted palms
m.stamp(X,2,3,4,6,24,12)                            # workstation pair
m.stamp(X,11,4,4,5,29,12) if False else None
m.stamp(R_,1,4,4,4,28,13)
# boxes in the corners
for (x,y) in [(1,12),(1,13),(13,12)]: m.put(x,y,0x99)
# entrance mat
m.stamp(X,6,9,3,2,15,H-2)
m.save('front2')
