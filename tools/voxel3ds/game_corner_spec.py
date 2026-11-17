# Wally's Journey POL-9 (2026-11-16, session 014PwRDj): Mauville Game Corner's prize machines in 3D,
# in 3ds_port/scripts/voxel_building_specs.py, mauville_game_corner(). Not run on its own: make the change,
# regenerate buildings.bin (gen_voxel_buildings.py --output DONOR), then splice the room (README.md here):
#   splice_buildings.py --layout pc_data/voxel/buildings.bin DONOR OUT 91
#
# Replace
#     plants = [piece("plant_%d" % k, [(x, 20, x + 16, 47)], 27, card=True,
#                     leave=fl + GAME_CORNER_WALL) for k, x in enumerate((80, 96, 112))]
# with
#     # Wally's Journey (POL-9, Brennan 2026-11-14: "the gatcha machines need to be fixed as they are in parts or
#     # not fully 3D"): the four prize machines against the back wall (pixels 80-143, rows 16-47) were three
#     # cards ("plant_0..2") and the fourth (purple) left painted on the wall. One box, as the arcade cabinets
#     # beside them are.
#     plants = [piece("gacha", [(80, 16, 144, 48)], 24, leave=fl + GAME_CORNER_WALL, against=32,
#                     side=(82, 17, 94, 20))]
#
# side=(82, 17, 94, 20): the orange machine's plain top band (the interior lint refuses a side tile that
# carries the front's lines). Preview: --only game_corner_room --preview DIR -> "game_corner_room_gacha
# 214 tris exact: wrong=0 missing=0 extra=0", interior lint 0 failures.

# POL-9 follow-up (Brennan 2026-11-16 19:12: "the prize machines look scrunched down they need to be raised at the top
# to make it look full, the vending machines also do this same thing. imagine them as tall rectangles instead of
# squares"): the same drawing on taller, shallower boxes - gacha 14 -> 24 px, and the two machines at the back left
#     piece("arcade", [(0, 18, 32, 48)], 14, ...)  ->  piece("arcade", [(0, 18, 32, 48)], 22, ...)
# Preview: gacha 396 tris exact wrong=0 missing=0 extra=0; arcade 138 tris exact. Respliced layout 91.

# POL-9 front columns (Brennan 2026-11-16: "you can still walk through the columns at the front of the game corner";
# asked, he chose "Pull the porch back"), in game_corner(art) of voxel_building_specs.py: the "portal" HeightSolid
# (8 px forward of the facade, posts on Mauville row 14 - cells the cartridge lets you walk on) is left out, and the
# Mauville "game_corner" entry's keep/exact drop (40, 64, 48, 72), (64, 64, 72, 72) - the posts' feet stay painted on
# the ground. Preview: game_corner 272 tris exact. Spliced: splice_buildings.py OURS DONOR OUT 3 5 10 --replace.
