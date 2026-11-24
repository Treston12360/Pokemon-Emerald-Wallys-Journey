# Wally's Journey #29 (Brennan 2026-11-23 21:03: "make all detail inside caves 3D"; 23:04: "all caves should be 3D
# models"): the specs added to 3ds_port/scripts/voxel_building_specs.py (that folder is not in git) for the Cave
# tileset's floor detail. Put them back in that list (before the mt_pyre_grave entry) to make the models again.
#
#   {"name": "cave_rock",
#    "components": {"secondary": "gTileset_Cave", "tiles": {0x202, 0x203, 0x250, 0x262, 0x266, 0x330},
#                   "height": 6, "block": 1, "upper": True, "single": True},
#    "ground": [0x211, 0x201]},
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
