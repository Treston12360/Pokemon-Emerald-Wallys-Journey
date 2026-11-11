#!/usr/bin/env python3
"""llvm-mingw build: GNU objcopy (elf32-i386 -> pe-i386) copies ELF relocation numbers into the
COFF object unchanged, so R_386_32 (1) shows up as COFF DIR16 and R_386_16 (20) as COFF REL32.
Fix them: R_386_32 -> IMAGE_REL_I386_DIR32. The only R_386_16 uses are 16-bit constants that
point at absolute symbols (script special numbers, battle message ids), so those are filled in
here from the Linux objects and the relocation is turned off.
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
for i in range(nsec):
    s = 20 + optsize + i * 40
    name = d[s:s + 8].rstrip(b'\0').decode()
    if name.startswith('/'):   # long section name lives in the string table
        n = strtab + int(name[1:])
        name = d[n:d.index(b'\0', n)].decode()
    rawptr, relptr, _, nrel = struct.unpack_from('<IIIH', d, s + 20)
    for r in range(nrel):
        o = relptr + r * 10
        va = struct.unpack_from('<I', d, o)[0]
        kind, sym = rels.get(name, {}).get(va, (None, None))
        if kind == 'R_386_32':
            struct.pack_into('<H', d, o + 8, 6)
        elif kind == 'R_386_16':
            p = rawptr + va
            struct.pack_into('<H', d, p, (struct.unpack_from('<H', d, p)[0] + absolute(sym)) & 0xFFFF)
            struct.pack_into('<H', d, o + 8, 0)
        else:
            sys.exit(f'{coff}: relocation at {name}+{va:#x} not found in {elf}')
open(coff, 'wb').write(d)
