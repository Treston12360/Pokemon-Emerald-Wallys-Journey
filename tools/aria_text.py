#!/usr/bin/env python3
"""Turn script-doc dialogue (one list item = one text box) into .string lines for map scripts.
Wraps at WIDTH chars: line 2 of a box uses \\n, further lines scroll with \\l, boxes end with \\p.
Usage (from Python): from aria_text import strings; print(strings(["box 1", "box 2"], end="$"))"""
import textwrap
WIDTH = 36  # FireRed message box, roughly; check in game

def strings(boxes, end="$", width=None):
    W = width or WIDTH
    out = []
    for bi, box in enumerate(boxes):
        lines = textwrap.wrap(box, W) or [""]
        for li, ln in enumerate(lines):
            last_line = li == len(lines) - 1
            if last_line:
                term = end if bi == len(boxes) - 1 else "\\p"
            else:
                term = "\\n" if li == 0 else "\\l"
            ln = ln.replace('\u2014', '-')  # em dash: not in the charmap
            q = iter(['\u201c', '\u201d'] * 50)
            ln = ''.join(next(q) if ch == '"' else ch for ch in ln)  # straight quotes -> curly (charmap)
            out.append('\t.string "%s%s"' % (ln, term))
    return "\n".join(out)


def call_strings(speaker, boxes):
    """A PokéNav call (pokenavcall): rings, the message, hang-up click — Emerald's own format.
    Pass speaker=None and put `setspeaker SP_NAME_X` before pokenavcall: the call window shows the name box
    (owner 2026-11-14). A speaker string is only added as a 'Name: ' prefix when there's no name box."""
    out = ['\t.string "… … … … … …\\n"', '\t.string "… … … … … Beep!\\p"']
    boxes = list(boxes)
    if speaker:
        boxes[0] = speaker + ": " + boxes[0]
    body = strings(boxes, end="\\p", width=32)
    return "\n".join(out) + "\n" + body + '\n\t.string "… … … … … …\\n"\n\t.string "… … … … … Click!$"'
