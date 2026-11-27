#!/usr/bin/env python3
"""voxel/trees.rgba5551 is 256x256 since 2026-11-26 (N77, voxel_tree.h VOXEL_TREE_TEXTURE_DIM); Emerald 3DS's tree
generator writes it 128x128. This grows a 128x128 one to 256x256: every old pixel keeps its place (its x, y), the
new three quarters are left clear (alpha 0), the PICA 8x8 Morton tiling redone for the new width.
Then the Polish cards go back in their scripts (see README.md, "trees.rgba5551").

usage: grow_trees_texture.py IN_128.rgba5551 OUT_256.rgba5551
"""
import struct
import sys


def tex(x, y, w):
    m = (x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3)
    return ((y // 8) * (w // 8) + x // 8) * 64 + m


def main():
    src = open(sys.argv[1], 'rb').read()
    assert len(src) == 128 * 128 * 2, 'the input must be 128x128 (it is %d bytes)' % len(src)
    out = bytearray(256 * 256 * 2)
    for y in range(128):
        for x in range(128):
            v = struct.unpack_from('<H', src, 2 * tex(x, y, 128))[0]
            struct.pack_into('<H', out, 2 * tex(x, y, 256), v)
    open(sys.argv[2], 'wb').write(out)


if __name__ == '__main__':
    main()
