#ifndef GUARD_GEN5_PREVIEW_H
#define GUARD_GEN5_PREVIEW_H

// Experimental native PC screenshot prototype. Do not merge into kanto-port yet.
#include <SDL2/SDL.h>
#include "gba/gba.h"
bool G5Preview_MaskOam(const struct OamData *oam, int x, int y, int w, int h);
void G5Preview_Draw(SDL_Renderer *renderer, SDL_Rect gameViewport);
#endif
