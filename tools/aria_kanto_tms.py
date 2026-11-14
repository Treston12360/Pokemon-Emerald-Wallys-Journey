#!/usr/bin/env python3
"""S-ITEMS (Project ARIA, owner approved 2026-11-14): swap every FireRed TM in Kanto for the new TM51-TM100.

All 50 Gen 3 TMs are already found in Hoenn, so Kanto gives only the new ones (docs/KANTO_TM_LIST.md).
Per file: ITEM_TMxx -> new item, "TMxx" in that file's text -> new number. Text that names the old MOVE is
rewritten by hand afterwards (see the commit). Flag names (FLAG_GOT_TM39_FROM_BROCK...) are kept on purpose:
they are save data, so old saves still remember what was already picked up.
Run once; it is not idempotent on purpose (it refuses if the file has no old TM left).
"""
import re, sys
MAP = {
 'PewterCity_Gym_Frlg': {39: 51}, 'CeruleanCity_Gym_Frlg': {3: 52}, 'VermilionCity_Gym_Frlg': {34: 53},
 'CeladonCity_Gym_Frlg': {19: 54}, 'FuchsiaCity_Gym_Frlg': {6: 55}, 'SaffronCity_Gym_Frlg': {4: 56},
 'CinnabarIsland_Gym_Frlg': {38: 57}, 'ViridianCity_Gym_Frlg': {26: 58},
 'CeladonCity_DepartmentStore_2F_Frlg': {5: 59, 15: 60, 28: 61, 31: 62, 43: 63, 45: 64},
 'CeladonCity_DepartmentStore_Roof_Frlg': {16: 65, 20: 66, 33: 67},
 'CeladonCity_GameCorner_PrizeRoom_Frlg': {13: 68, 23: 69, 24: 70, 30: 71, 35: 72},
 'SaffronCity_MrPsychicsHouse_Frlg': {29: 73}, 'Route12_NorthEntrance_2F_Frlg': {27: 74},
 'CeruleanCity_Frlg': {28: 75}, 'FiveIsland_MemorialPillar_Frlg': {42: 76},
}
def swap(path, m):
    s = open(path, encoding='utf-8').read(); o = s
    for old, new in m.items():
        s = re.sub(r'\bITEM_TM%02d\b' % old, 'ITEM_TM%d' % new, s)
        # text only: inside .string lines, not labels/flags
        s = re.sub(r'(\.string "[^"\n]*?)\bTM%02d\b' % old, lambda mm: mm.group(1) + 'TM%d' % new, s)
        s = re.sub(r'(\.string "[^"\n]*?)\bTM%02d\b' % old, lambda mm: mm.group(1) + 'TM%d' % new, s)
    assert s != o, path
    open(path, 'w', encoding='utf-8').write(s)
for mp, m in MAP.items():
    swap(f'data/maps/{mp}/scripts.inc', m)
# ground balls, in file order: the first 24 get TM77-TM100, the last 4 good repeatable items (owner rule)
p = 'data/scripts/item_ball_scripts_frlg.inc'
s = open(p, encoding='utf-8').read()
olds = re.findall(r'\bITEM_TM(\d\d)\b', s)
assert len(olds) == 28, len(olds)
extra = ['ITEM_RARE_CANDY', 'ITEM_BOTTLE_CAP', 'ITEM_RARE_CANDY', 'ITEM_BOTTLE_CAP']
it = iter(['ITEM_TM%d' % n for n in range(77, 101)] + extra)
s = re.sub(r'\bITEM_TM\d\d\b', lambda mm: next(it), s)
open(p, 'w', encoding='utf-8').write(s)
print('done')
