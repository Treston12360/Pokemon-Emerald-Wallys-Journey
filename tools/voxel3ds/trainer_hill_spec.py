# Wally's Journey POL-5 (2026-11-16, session 014PwRDj): Trainer Hill, in 3ds_port/scripts/voxel_building_specs.py.
# Not run on its own: put these into the specs as described, then regenerate and splice (README.md here).
#
# 1) SPECS entry (before "dewford_gym_exterior"): the door on Route 111 (also placed on Route111_NoMirageTower):
#     {
#         # Wally's Journey (POL-5, Brennan 2026-11-14: "Trainer hill entrance is
#         # missing the top of the door"): the doorway cut into the cliff under
#         # Route 111's Trainer Hill (cells 30..32 x 112..113): its green cap
#         # (rows 3-9) is drawn on the cliff's face cells and was lost to the
#         # relief. A frame 29 px tall standing on the row the door's cells end,
#         # 4 deep; above its foot row the cliff stays the relief's.
#         "name": "trainer_hill_door",
#         "layout": "LAYOUT_ROUTE111",
#         "relief_bare": True,
#         "rect": (30, 112, 3, 2),
#         "ground": [0x1D1],
#         "ground_px": True,
#         # the cliff face beside the door's foot (cells 30 and 32 of row 113:
#         # its rock and the posts' feet) stands as the relief stood it, a
#         # pixel behind the frame: its cells are the model's, not the relief's
#         # (both from 1 px up: a face reaching the ground read the row under the drawing - a light line
#         # along the foot)
#         "parts": lambda: [PlanPrism("face", [(0, 31.5), (48, 31.5), (48, 32), (0, 32)], 1, 16, (0, 8, 32)),
#                           PlanPrism("door", [(12, 28), (36, 28), (36, 32), (12, 32)], 1, 29, (12, 14, 32))],
#     },
#
# 2) trainer_hill_entrance(): the lobby wall - replace 'lobby = (48, 112, 64, 128)' with:
#     # Wally's Journey (POL-5, Brennan: "the back wall looks crunched and cut off like the back wall is
#     # down too far"): the lobby's back wall is drawn rows 112-143 (its green panels down to the floor
#     # behind the counters: its panels end on row 131, the counters' tops start on 133), not 112-127 -
#     # it stands 20 tall on row 132 (was 16 on row 128: the panels' last rows lay on the floor)
#     lobby = (48, 112, 64, 132)
#    and the four wall/side pieces with:
#         piece("wall_w", [(0, 112, 112, 132)], 20, fill=8, foot=132, side=lobby),
#         piece("wall_e", [(192, 112, 304, 132)], 20, fill=8, foot=132, side=lobby),
#         piece("side_w", [], 20, side=lobby, walls=[((0, 272), (0, 132))]),
#         piece("side_e", [], 20, side=lobby, walls=[((304, 132), (304, 272))]),
#
# 3) trainer_hill_entrance(): the healing machine, in front of the counters - replace 'return kw_pieces(' with:
#     # Wally's Journey (POL-5, Brennan: "the top of the healing station ... looks crunched"): the
#     # healing machine sits in the west counter (x 31-63, rows 128-157), drawn as its tray seen from
#     # above; it was painted flat on the counter's top. A box 6 tall on the counter (12), its tray its
#     # top, its front the band over the counter's front edge (rows 151-157).
#     healer = [piece("healer", [(31, 128, 64, 158)], 6, base=12, foot=158, leave=("62c5b4",))]
#     return healer + kw_pieces(
