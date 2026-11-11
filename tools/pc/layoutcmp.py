#!/usr/bin/env python3
"""Compare struct layouts between the GBA build and the PC build.

usage: layoutcmp.py GBA.elf PC_OBJ_OR_ELF [...] [--only NAME,NAME]

Prints every named struct/union whose size or member offsets differ.
Both inputs need full debug info (-g).
"""
import sys
from elftools.elf.elffile import ELFFile


def die_name(die):
    a = die.attributes.get('DW_AT_name')
    return a.value.decode() if a else None


def resolve(die):
    while die is not None and die.tag in ('DW_TAG_typedef', 'DW_TAG_const_type', 'DW_TAG_volatile_type'):
        if 'DW_AT_type' not in die.attributes:
            return None
        die = die.get_DIE_from_attribute('DW_AT_type')
    return die


def type_desc(die):
    die = resolve(die)
    if die is None:
        return 'void'
    if die.tag == 'DW_TAG_pointer_type':
        return 'ptr'
    if die.tag == 'DW_TAG_array_type':
        return 'array'
    return die_name(die) or die.tag


def collect(paths):
    out = {}
    for p in paths:
        with open(p, 'rb') as f:
            elf = ELFFile(f)
            if not elf.has_dwarf_info():
                continue
            dw = elf.get_dwarf_info()
            for cu in dw.iter_CUs():
                for die in cu.iter_DIEs():
                    if die.tag not in ('DW_TAG_structure_type', 'DW_TAG_union_type'):
                        continue
                    name = die_name(die)
                    if not name or 'DW_AT_declaration' in die.attributes or name in out:
                        continue
                    size = die.attributes.get('DW_AT_byte_size')
                    if size is None:
                        continue
                    members = []
                    for ch in die.iter_children():
                        if ch.tag != 'DW_TAG_member':
                            continue
                        off = ch.attributes.get('DW_AT_data_member_location')
                        bit = ch.attributes.get('DW_AT_data_bit_offset')
                        members.append((die_name(ch), off.value if off else None,
                                        bit.value if bit else None))
                    out[name] = (size.value, members)
    return out


def main():
    args = sys.argv[1:]
    only = None
    if '--only' in args:
        i = args.index('--only')
        only = set(args[i + 1].split(','))
        del args[i:i + 2]
    gba = collect([args[0]])
    pc = collect(args[1:])
    bad = 0
    for name in sorted(gba):
        if only and name not in only:
            continue
        if name not in pc:
            continue
        gs, gm = gba[name]
        ps, pm = pc[name]
        if gs == ps and gm == pm:
            continue
        bad += 1
        print(f'{name}: GBA size {gs}, PC size {ps}')
        pmd = {m[0]: m for m in pm}
        for m in gm:
            q = pmd.get(m[0])
            if q != m:
                print(f'    {m[0]}: GBA {m[1:]}  PC {q[1:] if q else None}')
    print(f'{bad} differing types (of {len(set(gba) & set(pc))} shared)')


if __name__ == '__main__':
    main()
