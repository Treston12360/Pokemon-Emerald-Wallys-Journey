# Varnell's Celadon office (ch. 5) — interior mockup v2, FRLG Celadon Condominiums tiles (Game Freak's own office),
# stamped from the real maps so edges/shadows stay official. 13x11 so it isn't far bigger than the 4-wide house
# outside (mapping research: keep interiors in proportion), with a clear 2-3 tile walkway from the door.
import compose as C, sys
SRC=['CeladonCity_Condominiums_1F_Layout','CeladonCity_Condominiums_3F_Layout','CeladonCity_Condominiums_2F_Layout']
F1,F3,F2=SRC
W,H=13,11
m=C.Map(SRC,W,H,0x281)
m.fill(0,0,W,1,0x289); m.fill(0,1,W,1,0x291); m.fill(0,2,W,1,0x282); m.fill(0,3,1,H-4,0x282)
# top wall, left to right: two bookshelves, window, staff-only doorway (locked), window, the painting (= the
# child's drawing signed "aria", read as a sign), phone
m.stamp(F3,0,5,2,3,0,0); m.stamp(F3,0,5,2,3,2,0)
m.stamp(F3,2,0,1,2,4,0)
m.stamp(F1,1,0,3,2,5,0)
m.stamp(F3,10,0,1,2,8,0)
m.stamp(F3,4,5,2,2,9,0)
m.stamp(F3,6,5,1,2,11,0)
m.put(12,0,0x289); m.put(12,1,0x291)   # plain wall (boxes sit below)
# Varnell's desk under the drawing; boxes of papers beside it
m.stamp(F3,0,8,3,3,9,2); m.stamp(F3,3,12,1,2,12,2)
# staff desk (left), reception desk by the door (left)
m.stamp(F3,0,8,3,3,0,3)
m.stamp(F3,0,11,3,3,1,6)
# meeting table for the report-back with Blue (right), stools below
m.stamp(F1,1,9,4,4,8,5); m.stamp(F1,1,13,4,1,8,9)
# plant in the front-right corner
m.stamp(F1,14,8,1,2,12,8)
# exit row + mat (door in the middle)
m.fill(0,10,W,1,0x008); m.put(5,10,0x01a); m.put(6,10,0x01b); m.put(7,10,0x01c)
m.save(sys.argv[1] if len(sys.argv)>1 else '/tmp/claude-0/-home-claude/2bb72568-d7ee-56f4-8a6d-fd6081a29497/scratchpad/sim/celadon_office')
