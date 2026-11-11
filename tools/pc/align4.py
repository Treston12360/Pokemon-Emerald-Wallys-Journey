#!/usr/bin/env python3
"""Make preprocessed C lay out structs like the GBA compiler does.

The GBA build uses -mabi=apcs-gnu, where every struct and union is at least
4-byte aligned and its size is rounded up to a multiple of 4. x86 does not do
that, so save data, map data written in assembly and anything else shared as
raw bytes would end up at different offsets. This adds
__attribute__((aligned(4))) after the closing brace of every struct/union
definition that is not packed.

usage: align4.py < in.i > out.i
"""
import re
import sys

ATTR = '__attribute__((aligned(4)))'
IDENT = re.compile(r'[A-Za-z_]\w*')


def skip_ws(s, i):
    n = len(s)
    while i < n and s[i] in ' \t\r\n':
        i += 1
    return i


def skip_parens(s, i):
    # s[i] == '('
    depth = 0
    n = len(s)
    while i < n:
        c = s[i]
        if c == '"' or c == "'":
            i = skip_string(s, i)
            continue
        if c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    return i


def skip_string(s, i):
    q = s[i]
    i += 1
    n = len(s)
    while i < n:
        if s[i] == '\\':
            i += 2
            continue
        if s[i] == q:
            return i + 1
        i += 1
    return i


def struct_head(s, i):
    """At s[i:] just after 'struct'/'union'. Return (brace_pos, packed) or None."""
    packed = False
    n = len(s)
    while True:
        i = skip_ws(s, i)
        if i >= n:
            return None
        if s.startswith('__attribute__', i):
            j = skip_ws(s, i + len('__attribute__'))
            if j < n and s[j] == '(':
                k = skip_parens(s, j)
                if 'packed' in s[j:k]:
                    packed = True
                i = k
                continue
            return None
        m = IDENT.match(s, i)
        if m:
            i = m.end()
            continue
        if s[i] == '{':
            return i, packed
        return None


def main():
    s = sys.stdin.read()
    n = len(s)
    out_inserts = []
    stack = []  # entries: None for ordinary brace, or packed flag for struct brace
    i = 0
    struct_braces = {}
    system = False
    while i < n:
        c = s[i]
        if c == '"' or c == "'":
            i = skip_string(s, i)
            continue
        if c == '#' and (i == 0 or s[i - 1] == '\n'):
            j = s.find('\n', i)
            line = s[i:j if j >= 0 else n]
            mm = re.match(r'#\s*\d+\s+"([^"]*)"', line)
            if mm:
                system = mm.group(1).startswith(('/', '<'))
            i = n if j < 0 else j + 1
            continue
        if c.isalpha() or c == '_':
            m = IDENT.match(s, i)
            word = m.group(0)
            if word in ('struct', 'union'):
                h = struct_head(s, m.end())
                if h and not system:
                    struct_braces[h[0]] = h[1]
            i = m.end()
            continue
        if c == '{':
            stack.append(struct_braces.pop(i, None))
        elif c == '}':
            kind = stack.pop() if stack else None
            if kind is not None:
                packed = kind
                j = skip_ws(s, i + 1)
                if s.startswith('__attribute__', j):
                    k = skip_ws(s, j + len('__attribute__'))
                    if k < n and s[k] == '(' and 'packed' in s[k:skip_parens(s, k)]:
                        packed = True
                if not packed:
                    out_inserts.append(i + 1)
        i += 1
    parts = []
    last = 0
    for pos in out_inserts:
        parts.append(s[last:pos])
        parts.append(' ' + ATTR)
        last = pos
    parts.append(s[last:])
    sys.stdout.write(''.join(parts))


if __name__ == '__main__':
    main()
