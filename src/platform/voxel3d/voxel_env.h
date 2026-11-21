#ifndef GUARD_VOXEL_ENV_H
#define GUARD_VOXEL_ENV_H
/* Brennan 2026-11-20 20:15 (stall log: "belt" 150-1680 ms a frame biking and crossing maps): the scenery builders
 * ask for test switches (WJPC_...) in their per-cell code, many times a cell; on Windows each getenv() walks the
 * whole environment. Asked once per switch now: the answer is kept by the name's address (each name is a string
 * literal), the environment does not change while the game runs. Included last in the files that build scenery. */
#include <stdlib.h>
#include <stdint.h>
#include "voxel_thread.h"

static inline const char *VoxelEnv(const char *name)
{
    enum { N = 256 };
    static VOXEL_TLS const char *sName[N];  /* (per thread: audit step 4) */
    static VOXEL_TLS const char *sValue[N];
    unsigned h = (unsigned)(((uintptr_t)name >> 2) * 2654435761u) >> 24;

    for (unsigned i = 0; i < N; i++, h = (h + 1) & (N - 1))
    {
        if (sName[h] == name)
            return sValue[h];
        if (sName[h] == NULL)
        {
            sValue[h] = (getenv)(name);
            sName[h] = name;
            return sValue[h];
        }
    }
    return (getenv)(name);
}
#define getenv(name) VoxelEnv(name)

#endif
