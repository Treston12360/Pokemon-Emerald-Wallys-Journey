# Wally's Journey POL-5 - WORK IN PROGRESS, NOT in pc_data/voxel (see docs/POLISH_TASKS.md POL-5).
# SPECS entry tried 2026-11-16 (session 014PwRDj):
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
#         "ground": [0x1D1, 0x091],
#         "ground_px": True,
#         "parts": lambda: [PlanPrism("door", [(12, 28), (36, 28), (36, 32), (12, 32)], 0, 29, (12, 14, 32))],
#     },
