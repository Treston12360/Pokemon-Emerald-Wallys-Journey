# Wally's Journey #29 (Brennan 2026-11-23 21:03: "make all detail inside caves 3D"; 23:04: "all caves should be 3D
# models"): the specs added to 3ds_port/scripts/voxel_building_specs.py (that folder is not in git) for the Cave
# tileset's floor detail. Put them back in that list (before the mt_pyre_grave entry) to make the models again.
#
#   {"name": "cave_rock",
#    "components": {"secondary": "gTileset_Cave", "tiles": {0x202, 0x203, 0x250, 0x262, 0x266, 0x330, 0x2D5, 0x35A},
#                   "height": 6, "block": 1, "upper": True, "single": True},
#    "ground": [0x211, 0x201]},
#   {"name": "cave_stalagmite", "components": {"secondary": "gTileset_Cave", "tiles": {0x21F, 0x227}, "height": 10,
#    "upper": True}, "ground": [0x211, 0x201]},
#   {"name": "cave_ladder", "components": {"secondary": "gTileset_Cave",
#    "tiles": {0x20F, 0x217, 0x216, 0x341, 0x377, 0x37F}, "height": 3, "upper": True}, "ground": [0x211, 0x201]},
#   {"name": "cave_crystal",
#    "components": {"secondary": "gTileset_Cave", "tiles": {0x31F, 0x326, 0x348, 0x34D, 0x34E},
#                   "height": 8, "block": 1, "single": True},
#    "ground": [0x322, 0x320, 0x333, 0x31B],
#    "ground_rgb": [(90, 65, 57), (115, 90, 82), (156, 123, 123), (197, 164, 156), (230, 205, 205)]},
#
# cave_rock: the piles of stones (0x202, 0x203) and the boulders (0x250, 0x262, 0x266, 0x330) drawn on the upper
# layer over the floor; one model a cell ("single", mt_pyre_grave_spec.py), its top and a front 6 px tall.
# cave_crystal: Cave of Origin's red crystals, 8 px tall; ground_rgb: the floor's own colours, cleared from round
# them (without it their sides were dark teal and a ghost of the floor stood with them).
#
# Made: gen_voxel_buildings.py --only cave_ --output DONOR (35 models, 214 placements), then spliced
# (splice_buildings.py --layout OURS DONOR OUT LID) into the layouts that had no placements before:
#   132 133 134 Granite Cave 1F/B1F/B2F, 149 Seafloor Cavern Room 2, 162 Cave of Origin B1F, 163 Victory Road 1F,
#   166 Shoal Cave Low Tide Stairs Room, 285 Victory Road B1F, 288 Granite Cave Steven's Room, 312 Cave of Origin
#   Unused B4F, 389 Desert Underpass, 400 Artisan Cave B1F, 409 Terra Cave End, 420 Altering Cave.
# (The donor's pages for layouts 8 and 357, Sootopolis, come from other specs whose names start "cave_": not
# spliced, those already have their own.)
#
# Renderer (voxel_building.c UnlitCave): a model in a cave is not lit by the sun's facing - its faces turned from
# the sun had shade 0 and came out black. WJPC_CAVE_MODELS_SUNLIT=1: as before.
#
# Batch 2 (2026-11-24): + 0x2D5 (Ancient Tomb / Desert Ruins / Island Cave boulder) and 0x35A (Shoal Cave's mossy
# rock) in cave_rock; cave_stalagmite (Artisan Cave's two-cell stalagmites); cave_ladder (ladders leaning on
# ledges, Shoal Cave's icy one too). Not 0x35B: Shoal Cave's tide pool (water in a ring of stones) - stood up as a
# rock it lost its water. Spliced (--layout) into 131 132 134 162 163 165 166 167 280 281 285 312 328 389 400. NOT
# into 286 (Victory Road B2F): it keeps two placements of another model (the rocks in its water) that a whole-layout
# splice drops - its 2 new ladders wait for a donor that has both.
#
# Renderer, with it: a cave's model pages are graded twice like the cave floor (voxel_gl.c sPageCave: graded once
# they were paler and greyer than the same rock painted beside them); a cave's models cast no sun shadow
# (voxel_lighting.c: a dark smear beside them); and they are lit as the floor, not by their facing (shade + 4 marks
# them, voxel_building.c CaveFlatShade -> the shader's vFlat: their sides came out grey-green). Old behaviour:
# WJPC_CAVE_PAGES_ONE_GRADE, WJPC_CAVE_MODELS_CAST, WJPC_CAVE_MODELS_NORMAL_LIT.
#
# Victory Road B2F (2026-11-24, Brennan 20:40 "move onto the next"): its ladders, with the rocks in its water in the
# same donor - gen_voxel_buildings.py --only sea_rock,cave_ (a comma list of name prefixes: the script's
# build_models(tuple(args.only.split(","))) in 3ds_port) - then splice_buildings.py --layout OURS DONOR OUT 286. The
# donor's layout 286: the two sea rocks (22,12), (23,14) and four ladders, one page.
