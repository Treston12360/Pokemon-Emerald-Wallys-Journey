# Varnell Research Office (Celadon), ch. 5 — mockup from the FRLG Celadon Condominiums tileset (Game Freak's office).
# Every piece is stamped from the real maps so edges/shadows stay official. Rendered by render.py (same as the game).
import compose as C
SRC=['CeladonCity_Condominiums_1F_Layout','CeladonCity_Condominiums_3F_Layout','CeladonCity_Condominiums_2F_Layout']
W,H=15,13
m=C.Map(SRC,W,H,0x281)
F1,F3,F2=SRC
# walls: top wall (2 rows) + floor shadow row under it, shadow column on the left
m.fill(0,0,W,1,0x289); m.fill(0,1,W,1,0x291); m.fill(0,2,W,1,0x282); m.fill(0,3,1,H-4,0x282)
# top wall fixtures, left to right
m.stamp(F3,0,5,2,3,0,0)        # bookshelf x0-1 (3 rows)
m.stamp(F3,0,5,2,3,2,0)        # bookshelf x2-3
m.stamp(F3,2,0,1,2,4,0)        # window x4
m.stamp(F1,1,0,3,2,6,0)        # dark doorway x6-8  -> "staff only" (locked)
m.stamp(F3,10,0,1,2,9,0)       # window x9
m.stamp(F3,4,5,2,2,10,0)       # painting x10-11  -> the child's drawing signed "aria" (sign event)
m.stamp(F2,6,5,1,2,12,0) if False else m.stamp(F3,6,5,1,2,12,0)   # phone x12
m.stamp(F1,0,6,1,2,13,1)       # plant x13 (on the wall face row + floor)
m.put(13,0,0x289)
# staff desks (PC + papers), left side
m.stamp(F3,0,8,3,3,0,4); m.stamp(F3,4,8,3,3,4,4)
# Varnell's desk under the drawing, boxes beside it
m.stamp(F3,0,8,3,3,10,3); m.stamp(F3,3,12,1,2,13,3)
# meeting table (the report-back with Blue), stools below
m.stamp(F1,1,9,4,4,5,7); m.stamp(F1,1,13,4,1,5,11)
# reception: a desk by the entrance, plants in the front corners
m.stamp(F3,0,11,3,3,1,9)
m.stamp(F1,14,8,1,2,14,9); m.stamp(F1,9,8,1,2,11,9) if False else None
# exit row with the entrance mat (bottom centre)
m.fill(0,12,W,1,0x008); m.put(6,12,0x01a); m.put(7,12,0x01b); m.put(8,12,0x01c)
m.save('/tmp/claude-0/-home-claude/2bb72568-d7ee-56f4-8a6d-fd6081a29497/scratchpad/sim/celadon_office')
