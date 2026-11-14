#!/usr/bin/env python3
"""Turn script-doc dialogue (one list item = one text box) into .string lines for map scripts.
Wraps at WIDTH chars: line 2 of a box uses \\n, further lines scroll with \\l, boxes end with \\p.
Usage (from Python): from aria_text import strings; print(strings(["box 1", "box 2"], end="$"))"""
import textwrap
WIDTH = 36  # FireRed message box, roughly; check in game

def strings(boxes, end="$"):
    out = []
    for bi, box in enumerate(boxes):
        lines = textwrap.wrap(box, WIDTH) or [""]
        for li, ln in enumerate(lines):
            last_line = li == len(lines) - 1
            if last_line:
                term = end if bi == len(boxes) - 1 else "\\p"
            else:
                term = "\\n" if li == 0 else "\\l"
            out.append('\t.string "%s%s"' % (ln.replace('"', '\\"'), term))
    return "\n".join(out)
