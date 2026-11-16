// Experimental Gen 5 animated GIF overlay: isolated ARIA visual test branch.
// Run with WJPC_GEN5_PREVIEW=1 and preview/beldum.gif +
// preview/sceptile-back.gif. Not a production battle sprite architecture.
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "main.h"
#include "battle.h"
#include "sprite.h"
#include "constants/species.h"
#include "gen5_preview.h"

#if defined(NATIVE_LINUX)
#include <SDL2/SDL_image.h>
extern void BattleMainCB2(void);

struct PreviewGif
{
    IMG_Animation *animation;
    SDL_Texture *frames[256];
    int totalDelay;
    bool attempted;
};
static struct PreviewGif sGif[2];
static const char *const sPaths[2] = {"preview/sceptile-back.gif", "preview/beldum.gif"};
static const int sSpecies[2] = {SPECIES_SCEPTILE, SPECIES_BELDUM};
static bool sPreviewEnabled;

static bool PreviewBattleActive(void)
{
    if (!sPreviewEnabled)
        sPreviewEnabled = getenv("WJPC_GEN5_PREVIEW") != NULL;
    return sPreviewEnabled && gMain.callback2 == BattleMainCB2;
}

static void LoadAnimation(SDL_Renderer *renderer, int which)
{
    struct PreviewGif *g = &sGif[which];
    if (g->attempted)
        return;
    g->attempted = true;
    g->animation = IMG_LoadAnimation(sPaths[which]);
    if (!g->animation)
    {
        SDL_Log("gen5-preview missing %s: %s", sPaths[which], IMG_GetError());
        return;
    }
    for (int i = 0; i < g->animation->count && i < 256; i++)
    {
        g->frames[i] = SDL_CreateTextureFromSurface(renderer, g->animation->frames[i]);
        if (g->frames[i])
        {
            SDL_SetTextureBlendMode(g->frames[i], SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(g->frames[i], SDL_ScaleModeNearest);
        }
        g->totalDelay += g->animation->delays[i] > 0 ? g->animation->delays[i] : 100;
    }
    SDL_Log("gen5-preview %s loaded %d frames, %dx%d, %dms",
            sPaths[which], g->animation->count,
            g->animation->w, g->animation->h, g->totalDelay);
}

bool G5Preview_MaskOam(const struct OamData *oam, int x, int y, int w, int h)
{
    (void)oam;
    if (!PreviewBattleActive() || w < 16 || h < 16)
        return false;
    for (int b = 0; b < 2; b++)
    {
        if (gBattleMons[b].species != sSpecies[b])
            continue;
        u8 id = gBattlerSpriteIds[b];
        if (id >= MAX_SPRITES || !gSprites[id].inUse)
            continue;
        const struct Sprite *sp = &gSprites[id];
        int cx = sp->x + sp->x2, cy = sp->y + sp->y2;
        if (abs((x + w / 2) - cx) <= 36 && abs((y + h / 2) - cy) <= 36)
            return true; // raster-time only; original battle logic still runs
    }
    return false;
}

void G5Preview_Draw(SDL_Renderer *renderer, SDL_Rect view)
{
    if (!PreviewBattleActive())
        return;
    for (int b = 0; b < 2; b++)
    {
        if (gBattleMons[b].species != sSpecies[b])
            continue;
        u8 id = gBattlerSpriteIds[b];
        if (id >= MAX_SPRITES || !gSprites[id].inUse)
            continue;
        struct Sprite *sp = &gSprites[id];
        if (sp->invisible)
            continue;
        LoadAnimation(renderer,b);
        struct PreviewGif *a = &sGif[b];
        if (!a->animation || a->totalDelay <= 0)
            continue;
        int cycle = (int)(SDL_GetTicks64() % a->totalDelay), idx = 0;
        for (int n=0; n<a->animation->count && n<256; n++)
        {
            int delay=a->animation->delays[n] > 0 ? a->animation->delays[n] : 100;
            if (cycle < delay) {idx=n;break;}
            cycle-=delay;
        }
        if (!a->frames[idx])
            continue;
        // A DS-size image is drawn 80/96 native pixels tall for a 240x160 GBA
        // canvas, preserving the original pixel art with nearest-neighbour sampling.
        int h = b == 0 ? 90 : 76;
        int w = a->animation->w * h / a->animation->h;
        int centerX = sp->x + sp->x2;
        int centerY = sp->y + sp->y2;
        SDL_Rect dst = {
            view.x + (centerX - w/2) * view.w / 240,
            view.y + (centerY - h/2) * view.h / 160,
            w * view.w / 240,
            h * view.h / 160
        };
        SDL_RenderCopy(renderer, a->frames[idx], NULL, &dst);
    }
}
#else
bool G5Preview_MaskOam(const struct OamData *oam, int x, int y, int w, int h)
{
    (void)oam; (void)x; (void)y; (void)w; (void)h;
    return false;
}
void G5Preview_Draw(SDL_Renderer *renderer, SDL_Rect viewport)
{
    (void)renderer; (void)viewport;
}
#endif
