#!/usr/bin/env python3
"""llvm-mingw build: repair a COFF object made by GNU objcopy (elf32-i386 -> pe-i386) from the Linux
ELF data objects. objcopy gets three things wrong, so every relocation is rebuilt from the ELF one:
- it copies the ELF relocation number as-is: R_386_32 (1) becomes COFF DIR16 and R_386_16 (20) REL32.
  R_386_32 -> IMAGE_REL_I386_DIR32. R_386_16 only points at absolute symbols (script special numbers,
  battle message ids): the value is filled in here from the Linux objects and the relocation turned off.
- it drops section symbols, so relocations against a section (e.g. map group tables pointing into
  .rodata) point at a random symbol. Section symbols are added back and those relocations retargeted.
- every other relocation is checked to point at the same symbol name as in the ELF object.
usage: coff_fixrel.py elf_object coff_object"""
import glob, os, struct, subprocess, sys

elf, coff = sys.argv[1], sys.argv[2]
rels = {}   # section -> {offset: (type, symbol)}
sec = None
for line in subprocess.run(['readelf', '-rW', elf], capture_output=True, text=True).stdout.splitlines():
    if line.startswith("Relocation section '.rel"):
        sec = line.split("'")[1][4:]
        rels[sec] = {}
        continue
    parts = line.split()
    if sec and len(parts) >= 3 and parts[2].startswith('R_386_'):
        if parts[2] not in ('R_386_32', 'R_386_16'):
            sys.exit(f'{elf}: unexpected relocation type {parts[2]}')
        rels[sec][int(parts[0], 16)] = (parts[2], parts[4] if len(parts) > 4 else None)
elf_section_syms = set()
for line in subprocess.run(['readelf', '-sW', elf], capture_output=True, text=True).stdout.splitlines():
    p = line.split()
    if len(p) >= 8 and p[3] == 'SECTION':
        elf_section_syms.add(p[7] if len(p) > 7 else None)

absyms = None
def absolute(name):
    global absyms
    if absyms is None:
        absyms = {}
        objs = glob.glob(os.path.join(os.path.dirname(elf), '*.o'))
        for line in subprocess.run(['nm'] + objs, capture_output=True, text=True).stdout.splitlines():
            p = line.split()
            if len(p) == 3 and p[1] == 'A':
                absyms[p[2]] = int(p[0], 16)
    if name not in absyms:
        sys.exit(f'{elf}: no absolute value for {name}')
    return absyms[name]

d = bytearray(open(coff, 'rb').read())
_, nsec, _, symptr, nsyms, optsize, _ = struct.unpack_from('<HHIIIHH', d, 0)
strtab = symptr + nsyms * 18
assert strtab + struct.unpack_from('<I', d, strtab)[0] == len(d), 'string table must end the file'

def symname(i):
    e = d[symptr + i * 18:symptr + i * 18 + 18]
    if e[:4] == b'\0\0\0\0':
        n = strtab + struct.unpack_from('<I', e, 4)[0]
        return d[n:d.index(b'\0', n)].decode()
    return e[:8].rstrip(b'\0').decode()

byname = {}
i = 0
while i < nsyms:
    byname.setdefault(symname(i), i)
    i += 1 + d[symptr + i * 18 + 17]

sections = []
for i in range(nsec):
    s = 20 + optsize + i * 40
    name = d[s:s + 8].rstrip(b'\0').decode()
    if name.startswith('/'):   # long section name lives in the string table
        n = strtab + int(name[1:])
        name = d[n:d.index(b'\0', n)].decode()
    sections.append((s, name))

new_syms = []   # section symbols to append: (section name, section number)
secsym = {}
def section_symbol(name):
    if name not in secsym:
        num = [n for _, n in sections].index(name) + 1
        secsym[name] = nsyms + len(new_syms)
        new_syms.append((name, num))
    return secsym[name]

for s, name in sections:
    rawptr, relptr, _, nrel = struct.unpack_from('<IIIH', d, s + 20)
    for r in range(nrel):
        o = relptr + r * 10
        va, si = struct.unpack_from('<II', d, o)
        kind, sym = rels.get(name, {}).get(va, (None, None))
        if kind is None:
            sys.exit(f'{coff}: relocation at {name}+{va:#x} not found in {elf}')
        if kind == 'R_386_16':
            p = rawptr + va
            struct.pack_into('<H', d, p, (struct.unpack_from('<H', d, p)[0] + absolute(sym)) & 0xFFFF)
            struct.pack_into('<H', d, o + 8, 0)
            continue
        struct.pack_into('<H', d, o + 8, 6)
        if sym in elf_section_syms and sym in [n for _, n in sections]:
            struct.pack_into('<I', d, o + 4, section_symbol(sym))
        elif symname(si) != '_' + sym:
            if '_' + sym not in byname:
                sys.exit(f'{coff}: {name}+{va:#x} needs symbol {sym}, not in the COFF object')
            struct.pack_into('<I', d, o + 4, byname['_' + sym])

if new_syms:
    tail = bytes(d[strtab:])
    del d[strtab:]
    for name, num in new_syms:
        nb = name.encode()
        if len(nb) <= 8:
            ent = nb.ljust(8, b'\0')
        else:   # put the long name at the end of the string table
            off = len(tail)
            tail = tail + nb + b'\0'
            ent = b'\0\0\0\0' + struct.pack('<I', off)
        d += ent + struct.pack('<IhHBB', 0, num, 0, 3, 0)   # value 0, section, type 0, STATIC, no aux
    tail = struct.pack('<I', len(tail)) + tail[4:]
    d += tail
    struct.pack_into('<I', d, 12, nsyms + len(new_syms))
open(coff, 'wb').write(d)
