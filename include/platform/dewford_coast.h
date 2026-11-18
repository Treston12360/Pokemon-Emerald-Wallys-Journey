#ifndef GUARD_DEWFORD_COAST_H
#define GUARD_DEWFORD_COAST_H

#include "constants/map_groups.h"

// Backdrop only. Coordinates are relative to the top-left of Dewford.
// Route 106 joins town at its x=60, and Route 107 starts at town x=20.
static inline int DewfordCoastMaterialCount(int map)
{
    return (map == MAP_ROUTE106 || map == MAP_ROUTE107 || map == MAP_DEWFORD_TOWN) ? 13 : 0;
}

static const unsigned short sDewfordCoastMaterials[] = {
    0x170, 0x123, 0x124, 0x125, 0x12b, 0x12c, 0x12d,
    0x11b, 0x11c, 0x11d, 0x243, 0x239, 0x23a
};

static inline int DewfordCoastTile(int map, int x, int y)
{
    switch (map)
    {
    case MAP_ROUTE106: x -= 60; y -= 20; break;
    case MAP_ROUTE107: x += 20; break;
    case MAP_DEWFORD_TOWN: break;
    default: return -1;
    }

    static const char coast[][64] = {
    "[sssssssssssssTTTTTTTTTTTTTTTTTTTussssssssssssssssssssssssssss]",
    "lsssssssssssssTTTTTTTTTTTTTTTTTuusssssssssssssssssssssssssssss]",
    ".lssssssssssssuTTTTTTTTTTTTTTTusssssssssssssssssssssssssssssss]",
    "..lssssssssssssuTTTTTTTTTTTTuussssssssssssssssssssssssssssssss]",
    "...l_sssssssssssuuuTTTTTTTTTssssssssssssssssssssssssssssssssss]",
    ".....l__sssssssssssuuTTTTTTussssssssssssssssssssssssssssssssss]",
    "........l__ssssssssssuTTTuusssssssssssssssssssssssssssssssssss]",
    "...........l___sssssssuuTsssssssssssssssssssssssssssssssssssss]",
    "...............l__ssssssutssssssssssssssssssssssssssssssssssss]",
    "..................l__ssssussssssssssssssssssssssssssssssssssss]",
    ".....................l_ssssttsssssssssssssssssssssssssssssssss]",
    ".......................l_ssuutssssssssssssssssssssssssssssssss]",
    ".........................lsssTtsssssssssssssssssssssssssssssss]",
    "..........................lssuTtssssssssssssssssssssssssssssss]",
    "...........................[ssuTtsssssssssssssssssssssssssssss]",
    "...........................lsssuusssssssssssssssssssssssssssss]",
    "............................[sssssssssssssssssssssssssssssssss]",
    "............................lsssssssssssssssssssssssssssssssss]",
    ".............................[ssssssssssssssssssssssssssssssss]",
    ".............................lssssssssssssssssssssssssssssssss]",
    "..............................lsssuTTTTTTTuuuTTTTTTTTTTTTTTTusr",
    "...............................lsssuuuTTTTsssTTTTTTTTTTTTTTusr.",
    "................................l_ssssuuuusssTTTTTTTTTTTTuusr..",
    "..................................l__ssssssssuuuuTTTTTuuussr...",
    ".....................................l___ssssssssuuuuussssr....",
    ".........................................l____ssssssssss_r.....",
    "..............................................l____sss_r.......",
    "...................................................l_r.........",
    };
    if (x < -34 || x > 28 || y < 0 || y >= 28)
        return 0x170;
    switch (coast[y][x + 34])
    {
    case 'T': return 0x243;
    case 't': return 0x239;
    case 'u': return 0x23a;
    case '[': return 0x123;
    case 's': return 0x124;
    case ']': return 0x125;
    case 'l': return 0x12b;
    case '_': return 0x12c;
    case 'r': return 0x12d;
    case 'L': return 0x11b;
    case '^': return 0x11c;
    case 'R': return 0x11d;
    default: return 0x170;
    }
}
#endif
