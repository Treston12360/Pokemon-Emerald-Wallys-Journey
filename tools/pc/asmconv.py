#!/usr/bin/env python3
"""Convert GBA (ARM gas) data assembly into x86 gas syntax for the PC build.

Reads stdin, writes stdout. Only data directives are expected; any real ARM
instruction left in the input will fail to assemble, which is what we want.
"""
import os
import re
import sys

DROP = re.compile(r'^\s*\.(thumb|arm|code|thumb_func|syntax|cpu|fpu|eabi_attribute|force_thumb|ltorg|pool|arch)\b')
ALIGN = re.compile(r'^(\s*)\.align\b\s*([^,\n]+)(,.*)?$')
TYPE = re.compile(r'(\.type\s+[^,]+,\s*)%(\w+)')


def strip_comment(line):
    out = []
    in_str = False
    i = 0
    while i < len(line):
        c = line[i]
        if in_str:
            out.append(c)
            if c == '\\' and i + 1 < len(line):
                out.append(line[i + 1])
                i += 2
                continue
            if c == '"':
                in_str = False
        else:
            if c == '"':
                in_str = True
                out.append(c)
            elif c == '@':
                if i > 0 and line[i - 1] == '\\':
                    out.append(c)  # \@ macro counter
                else:
                    break
            else:
                out.append(c)
        i += 1
    return ''.join(out)


def convert(line):
    line = strip_comment(line.rstrip('\n'))
    if DROP.match(line):
        return ''
    m = ALIGN.match(line)
    if m:
        return f'{m.group(1)}.p2align {m.group(2).strip()}{m.group(3) or ""}'
    line = TYPE.sub(r'\1@\2', line)
    line = re.sub(r'(^|\s)\.4byte\b', r'\1.int', line)
    line = re.sub(r'(^|\s)\.word\b', r'\1.int', line)
    line = re.sub(r'(^|\s)\.2byte\b', r'\1.short', line)
    line = re.sub(r'(^|\s)\.hword\b', r'\1.short', line)
    return line


INCLUDE = re.compile(r'^\s*\.include\s+"([^"]+)"')
SEARCH = ['.', 'include', 'sound']


def emit(lines, w, depth=0):
    for line in lines:
        m = INCLUDE.match(line)
        if m and depth < 16:
            for d in SEARCH:
                path = os.path.join(d, m.group(1))
                if os.path.exists(path):
                    with open(path, encoding='latin-1') as f:
                        emit(f, w, depth + 1)
                    break
            else:
                w(line.rstrip('\n') + '\n')
            continue
        w(convert(line) + '\n')


def main():
    sys.stdin.reconfigure(encoding='latin-1')
    sys.stdout.reconfigure(encoding='latin-1')
    emit(sys.stdin, sys.stdout.write)


if __name__ == '__main__':
    main()
