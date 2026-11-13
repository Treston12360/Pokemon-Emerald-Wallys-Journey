#ifndef GUARD_PLATFORM_GAMMA2D_H
#define GUARD_PLATFORM_GAMMA2D_H

#include <stdint.h>
#include <stdbool.h>

// DLX-9: Gamma Emerald's daylight on the 2D overworld picture (see gamma2d.c).
// src: w x h BGR555 pixels; argb: the output; (worldX0, worldY0): the map pixel at src's top-left;
// frame: frames since start (clouds drift); dataDir: the folder with gamma_clouds.bin.
void Gamma2D_Apply(uint32_t *argb, const uint16_t *src, int w, int h, int worldX0, int worldY0,
                   unsigned frame, const char *dataDir);

#endif // GUARD_PLATFORM_GAMMA2D_H
