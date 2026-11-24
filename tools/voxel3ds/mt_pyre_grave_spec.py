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

# 3) 2026-11-24 (Brennan 23:04: Mt. Pyre "still looks to be 2D" - "Back wall painted"; the 2F test: "Yes, do all
#    floors"): the floors' walls as relief - pillars and door frames at the face, panels 4 px back, doorways recessed
#    12 px, stairways 16 px. In voxel_building_specs.py, before brendan_1f():
#  def mt_pyre_wall(tag, y0, x0, x1, doors=(), stairs=(), pillars=(), solid=()):
#      """Wally's Journey (#29, Brennan 2026-11-23 23:04: Mt. Pyre "still looks to be 2D" - "Back wall painted"; his
#      pick of the 2F test: "Yes, do all floors"): one of a floor's walls (rows y0..y0+31, standing on row y0+32,
#      from x0 to x1) stood as relief, not one painted panel. The pillars (8 wide) and the doorways' frames (47 wide:
#      posts 7, opening 32 - x+7..x+39 - under a lintel 12 rows deep) stand at the wall's face; the panels between them
#      are set 4 px back (their lattice windows in a recess); a doorway's opening is a recess 12 px deep, a stairway's
#      16 px with the steps drawn on its back. `solid`: (xa, xb) stretches kept at the face (a wall's end, a flight of
#      steps drawn on it)."""
#      foot = y0 + 32
#      frames = [(x, "door") for x in doors] + [(x, "stairs") for x in stairs]
#      stand = sorted([(x, x + 8) for x in pillars] + [(x, x + 46) for x, _ in frames] + list(solid))
#      pieces = []
#      for x, kind in frames:
#          deep = 12 if kind == "door" else 16
#          pieces.append(stairwell("stairwell_%s_%s_%d" % (tag, kind, x), x + 7, x + 39, foot, foot - deep, 21,
#                                  (x + 7, y0 + 12, x + 39, foot), y0))
#      edge = x0
#      for a, b in stand + [(x1, x1)]:
#          if a > edge:
#              pieces.append(stairwell("stairwell_%s_panel_%d" % (tag, edge), edge, a, foot, foot - 4, 32,
#                                      (edge, y0, a, foot), y0))
#          edge = max(edge, b)
#      shape = [(x, y0, x + 8, foot) for x in pillars] + [(a, y0, b, foot) for a, b in solid]
#      for x, _ in frames:
#          shape += [(x, y0, x + 46, y0 + 12), (x, y0 + 12, x + 7, foot), (x + 39, y0 + 12, x + 46, foot)]
#      side = shape[0][:2] + (shape[0][0] + 8, foot) if shape else (x0, y0, x0 + 8, foot)
#      # (filled: the pillars' light faces are drawn in the floor's own colours, read as floor they were holes)
#      pieces.append(piece("wall_%s" % tag, shape, 32, fill=16, foot=foot, claim=shape, side=side))
#      return pieces
#  
#  
#  MT_PYRE_ROOMS = {
#      "1F": mt_pyre_wall("back", 0, 144, 352, stairs=[161], pillars=[240, 288, 336])
#            + mt_pyre_wall("hall", 48, 48, 144, pillars=[64, 112], solid=[(128, 144)]),
#      "2F": mt_pyre_wall("back", 0, 0, 208, doors=[17], stairs=[145], pillars=[0, 80, 112, 192]),
#      "3F": mt_pyre_wall("back", 0, 0, 208, doors=[145], stairs=[17], pillars=[0, 80, 112, 192]),
#      "4F": mt_pyre_wall("back", 0, 112, 208, stairs=[145], pillars=[112, 200])
#            + mt_pyre_wall("upper", 32, 0, 112, solid=[(96, 112)])
#            + mt_pyre_wall("front", 64, 0, 208, doors=[17], pillars=[136, 192], solid=[(144, 192)]),
#      "5F": mt_pyre_wall("back", 0, 0, 96, stairs=[17], pillars=[0, 88])
#            + mt_pyre_wall("upper", 32, 96, 208, solid=[(96, 112)])
#            + mt_pyre_wall("front", 64, 0, 208, doors=[145], pillars=[24, 48], solid=[(32, 48)]),
#      "6F": mt_pyre_wall("back", 0, 0, 208, doors=[17], pillars=[8, 64])
#            + mt_pyre_wall("front", 96, 0, 208, pillars=[152, 176], solid=[(160, 176)]),
#  }
#
#    and in SPECS, before the "mt_pyre_grave" entry:
#    ] + [
#        {
#            "name": "mt_pyre_%s_room" % floor.lower(),
#            "interior": {"layout": "LAYOUT_MT_PYRE_%s" % floor, "ground": [0x2E6, 0x2E7, 0x2E8, 0x2E9],
#                         "pieces": own_shell(pieces)},
#        }
#        for floor, pieces in MT_PYRE_ROOMS.items()
#    ] + [
#
#    Made: gen_voxel_buildings.py --only mt_pyre --output DONOR (5 min: graves and rooms), splice --layout 137-142
#    into the buildings.bin from before the graves (so each floor is the donor's whole).
