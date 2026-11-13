#ifndef GUARD_DMA_H
#define GUARD_DMA_H

#include "global.h"

#define DMA_COUNT 4

enum {
    DMA_NOW,
    DMA_VBLANK,
    DMA_HBLANK,
    DMA_SPECIAL
};

void RunDMAs(u32 type);
// DLX-3: the transfers' progress, kept and put back round an extra drawing of the frame (two slots).
void Dma_SaveState(u32 slot);
void Dma_RestoreState(u32 slot);
#endif