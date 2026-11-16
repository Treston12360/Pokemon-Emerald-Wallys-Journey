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
#     plants = [piece("gacha", [(80, 16, 144, 48)], 14, leave=fl + GAME_CORNER_WALL, against=32,
#                     side=(82, 17, 94, 20))]
#
# side=(82, 17, 94, 20): the orange machine's plain top band (the interior lint refuses a side tile that
# carries the front's lines). Preview: --only game_corner_room --preview DIR -> "game_corner_room_gacha
# 214 tris exact: wrong=0 missing=0 extra=0", interior lint 0 failures.
