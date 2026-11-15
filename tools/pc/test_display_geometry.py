#!/usr/bin/env python3
"""Unit-level geometry regression checks for the native PC display.

Run from a repository checkout: python3 tools/pc/test_display_geometry.py
These checks intentionally do NOT claim to boot/test WallysJourney.exe. They
verify math invariants and guard against undoing the fractional zoom fix.
"""
from __future__ import annotations

import math
import re
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SOURCE = (ROOT / "src/platform/sdl2.c").read_text(encoding="utf-8")
ZOOM_DECL = re.search(r"static const u16 sZoomHeights\[\] = \{([^}]+)\}", SOURCE)
if not ZOOM_DECL:
    raise RuntimeError("Cannot find native display zoom targets in sdl2.c")
ZOOM_HEIGHTS = tuple(int(x.strip()) for x in ZOOM_DECL.group(1).split(","))

WIDE_MAX_W = 640
WIDE_MAX_H = 360
GBA_W = 240
GBA_H = 160


def zoom_view(out_w: int, out_h: int, level: int, indoor: bool = False):
    """Model ComputeFieldView in src/platform/sdl2.c."""
    if indoor and level > 0:
        level -= 1
    z = max(out_h / ZOOM_HEIGHTS[level],
            out_w / WIDE_MAX_W,
            out_h / WIDE_MAX_H)
    view_w = min(WIDE_MAX_W, math.ceil(out_w / z))
    view_h = min(WIDE_MAX_H, math.ceil(out_h / z))
    return z, view_w, view_h


def ui_placement(out_w: int, out_h: int, menu_open: bool):
    """Model the whole-pixel UI scale, downscaling only for a tiny viewport."""
    limit = min(out_w / GBA_W, out_h / GBA_H)
    scale = math.floor(limit)
    if scale < 1:
        scale = limit
    if scale <= 0:
        scale = 1
    ux = int(out_w - GBA_W * scale if menu_open else (out_w - GBA_W * scale) / 2)
    uy = int(out_h - GBA_H * scale)
    return ux, uy, scale


class DisplayGeometry(unittest.TestCase):
    def test_source_has_fractional_zoom(self):
        self.assertIn("static float sFieldZ", SOURCE)
        self.assertIn("sFieldZ = z", SOURCE)
        self.assertIn("sFieldViewW = (int)ceilf", SOURCE)
        self.assertIn("float scaleLimit", SOURCE)

    def test_each_zoom_is_distinct_at_common_resolutions(self):
        for w, h in ((1280, 720), (1920, 1080), (2560, 1440), (3840, 2160)):
            with self.subTest(resolution=f"{w}x{h}"):
                z = [zoom_view(w, h, i)[0] for i in range(len(ZOOM_HEIGHTS))]
                self.assertEqual(len(set(z)), len(ZOOM_HEIGHTS),
                                 "A numbered zoom setting has no visual scaling effect")
                self.assertEqual(z, sorted(z, reverse=True))

    def test_requested_view_fits_wide_buffer(self):
        sizes = ((200, 140), (320, 180), (640, 360), (960, 640),
                 (1280, 720), (1920, 1080), (2560, 1440), (3840, 2160))
        for w, h in sizes:
            for indoor in (False, True):
                for level in range(len(ZOOM_HEIGHTS)):
                    with self.subTest(w=w, h=h, indoor=indoor, level=level):
                        z, vw, vh = zoom_view(w, h, level, indoor)
                        self.assertGreater(z, 0)
                        self.assertGreater(vw, 0)
                        self.assertGreater(vh, 0)
                        self.assertLessEqual(vw, WIDE_MAX_W)
                        self.assertLessEqual(vh, WIDE_MAX_H)

    def test_ui_never_exceeds_render_target(self):
        for w, h in ((160, 120), (200, 140), (240, 160), (320, 180),
                     (640, 360), (960, 640), (1280, 720), (1920, 1080),
                     (2560, 1440), (3840, 2160)):
            for menu_open in (False, True):
                with self.subTest(w=w, h=h, menu_open=menu_open):
                    x, y, scale = ui_placement(w, h, menu_open)
                    self.assertGreaterEqual(x, 0)
                    self.assertGreaterEqual(y, 0)
                    self.assertLessEqual(x + int(GBA_W * scale), w)
                    self.assertLessEqual(y + int(GBA_H * scale), h)


if __name__ == "__main__":
    unittest.main()
