# Wally's Journey #29 (Brennan 2026-11-23 21:03: "make all detail inside caves 3D, hand tune all of them"), session
# 014PwRDj: Mt. Pyre's gravestones. 3ds_port/ is not in git (local exclude), so the two changes are here to paste.
# Not run on its own.
#
# 1) 3ds_port/scripts/voxel_building_specs.py, a SPECS entry (before "crate"):
#     {
#         "name": "mt_pyre_grave",
#         "components": {
#             "secondary": "gTileset_Facility",
#             "tiles": {0x2ED, 0x2EE, 0x326, 0x3D7},
#             "height": 9, "block": 1, "upper": True, "single": True,
#         },
#         "ground": [0x2E6],
#     },
#    The stone is drawn on the upper layer over the floor: its rounded top rows 0-6, its engraved front rows 7-15
#    (a stone 9 px tall). 0x2ED / 0x3D7 are the same stone over the other floor.
#
# 2) 3ds_port/scripts/gen_voxel_buildings.py, component_specs(), after 'seen.add((x, y))' of a new run:
#                 while todo and comp.get("single"):
#                     cells.append(todo.pop())
#    (`single`: each cell its own object, never a run - the stones stand in columns one behind another; as one
#    run the seams between them had no front.)
#
# Made: gen_voxel_buildings.py --only mt_pyre_grave --output DONOR (4 min), then for Mt. Pyre 1F-6F (buildings
# layout ids 137-142): splice_buildings.py --layout OURS DONOR OUT <id>. 264 placements, 15 models. Those rooms had
# no models before. Whoever regenerates buildings.bin keeps the spec.
