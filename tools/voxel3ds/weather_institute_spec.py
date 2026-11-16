# Wally's Journey (POL-36, 2026-11-15, session 014PwRDj): the Weather Institute's exterior model.
# Not run on its own: paste the two functions into 3ds_port/scripts/voxel_building_specs.py before
# `SPECS = [`, and the spec entry into SPECS before "dewford_gym_exterior" (see README.md here).


def wj_rounded_plan(x0, x1, z0, z1, rx, rz, seg=6, rz_back=None):
    """Wally's Journey: a rectangle (x0..x1, z0..z1) with elliptical corners
    (rx across, rz deep), as a plan for PlanPrism, in the order the space
    block's plan runs: the back west to east, then the front east to west."""
    pts = []
    rb = rz if rz_back is None else rz_back
    corners = [(x1 - rx, z0 + rb, -90.0, 0.0, rb), (x1 - rx, z1 - rz, 0.0, 90.0, rz),
               (x0 + rx, z1 - rz, 90.0, 180.0, rz), (x0 + rx, z0 + rb, 180.0, 270.0, rb)]
    for cx, cz, a0, a1, r in corners:
        for k in range(seg + 1):
            a = math.radians(a0 + (a1 - a0) * k / seg)
            pts.append((cx + rx * math.cos(a), cz + r * math.sin(a)))
    return pts


def weather_institute():
    """Wally's Journey (POL-36, Brennan 2026-11-15): Route 119's Weather
    Institute (rect cells 1..11 x 26..33, 176 x 128 px). Emerald 3DS has no
    model for it and drew it flat on the ground. Read off the drawing:

        a rounded body x 8-168, its glass ends standing on row 98, its wall
        51 px tall (the glass from row 47), the roof back to row 5;
        a portico x 48-128 standing out to row 119, 62 px tall (its pillar
        tops at row 57); two domes on the roof, centred x 32 and 144.

    Every face reads the drawing projected (PlanPrism, ProjDome): the
    model only has to stand where the drawing shows it."""
    body_h, porch_h = 51, 62
    body = PlanPrism("body", wj_rounded_plan(8, 168, 56, 112, 40, 14, rz_back=10), 0, body_h,
                     (10, 30, 98))
    porch = PlanPrism("porch", [(48, 100), (128, 100), (128, 119), (48, 119)], 0, porch_h,
                      (50, 58, 119))
    domes = [ProjDome("dome_w", 32, 79, 13, 7, 20, y0=body_h),
             ProjDome("dome_e", 144, 79, 13, 7, 20, y0=body_h)]
    return [body, porch] + domes


# --- the SPECS entry ---
#     {
#         # Wally's Journey (POL-36): the Weather Institute on Route 119, drawn
#         # flat in Emerald 3DS. Its rectangle's corner cells that are rock,
#         # trees, grass or the sign are not the model's.
#         "name": "weather_institute_exterior",
#         "layout": "LAYOUT_ROUTE119",
#         "rect": (1, 26, 11, 8),
#         "ground": [GRASS],
#         "owned": {(i, j) for i in range(11) for j in range(8)}
#                  - {(0, 0), (10, 0), (0, 6), (0, 7), (1, 7), (2, 7), (8, 7), (9, 7), (10, 7)},
#         "parts": weather_institute,
#     },
