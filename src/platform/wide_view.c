// PC wide view (PC port plan step 2, owner OK 2026-11-11: "do whatever you think will look the best").
//
// The overworld is drawn on a bigger virtual screen (384x216 outdoors, 320x180 indoors).
// Map layers 1-3 come straight from the game's own map data: the BG ring the game keeps in RAM
// for the area it has drawn, and the map grid / connected maps / border further out. Tiles and
// colours come from the same VRAM and palette RAM as the GBA picture, so animated water, flowers,
// palette fades, day/night and season tints all match. Sprites are drawn from gSprites with full
// coordinates, so people and effects outside the GBA screen show up too. Layer 0 (text boxes,
// menus) and GBA windows stay in the middle at their normal place.
//
// Anything that is not the plain overworld (battles, menus, intro...) uses the normal 240x160
// DrawFrame picture. During battle transitions and other field scenes the GBA picture is shown in
// the middle of the wide canvas with black around it, so the zoom does not jump.
#if defined(PORTABLE) && defined(RENDERER_EASY_DRAW)
#include "global.h"
#include "field_effect.h"
#include "constants/field_effects.h"
#include <stdbool.h>
extern void *calloc(size_t count, size_t size);
extern void free(void *ptr);
#include <string.h>
extern char *getenv(const char *);
extern int printf(const char *, ...);
extern int atoi(const char *);
#include "main.h"
#include "sprite.h"
#include "fieldmap.h"
#include "overworld.h"
#include "event_object_movement.h"
#include "field_weather.h"
#include "constants/map_types.h"
#include "platform/dma.h"
#include "platform/null_guard.h"
#include "platform/wide_view.h"
#include "decompress.h"
#include "event_data.h"
#include "constants/event_objects.h"
#include "wally_journey.h"
#include "palette.h"

extern void DrawFrame(uint16_t *pixels);
extern bool32 WallyJourney_IsBattleTransitionRunning(void);
extern void WJPC_GetFieldCameraRing(u8 *xTile, u8 *yTile);

#define MAX_W WIDE_VIEW_MAX_W
#define MAX_H WIDE_VIEW_MAX_H

static bool sEnabled = true;

s16 gWjPcViewPadX, gWjPcViewPadY, gWjPcSpawnPadX, gWjPcSpawnPadY;

void WideView_SetEnabled(bool enabled)
{
    sEnabled = enabled;
}

bool WideView_IsEnabled(void)
{
    return sEnabled;
}

// ---------------------------------------------------------------------------
// NPC loading range. Object events within the normal GBA range always load; the extra ring
// for the wide view only loads while a few object-event slots stay free, so a person next to
// the player can never be crowded out by one far away.
bool32 WJPC_SpawnOk(s16 x, s16 y)
{
    s16 left = gSaveBlock1Ptr->pos.x - 2, right = gSaveBlock1Ptr->pos.x + MAP_OFFSET_W + 2;
    s16 top = gSaveBlock1Ptr->pos.y, bottom = gSaveBlock1Ptr->pos.y + MAP_OFFSET_H + 2;
    u32 i, freeSlots = 0;

    if (x >= left && x <= right && y >= top && y <= bottom)
        return TRUE;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        if (!gObjectEvents[i].active)
            freeSlots++;
    }
    return freeSlots > 4;
}

// ---------------------------------------------------------------------------
// Which kind of frame this is.

static bool IsIndoorMap(void)
{
    return gMapHeader.mapType == MAP_TYPE_INDOOR || gMapHeader.mapType == MAP_TYPE_SECRET_BASE;
}

// Size asked for by the display code (zoom / screen shape); 0 = the built-in sizes.
static int sReqW, sReqH;

void WideView_SetViewSize(int w, int h)
{
    sReqW = w;
    sReqH = h;
}

bool WideView_IsIndoorMap(void)
{
    return IsIndoorMap();
}

static void GetViewSize(int *w, int *h)
{
    if (sReqW > 0 && sReqH > 0)
    {
        *w = sReqW < DISPLAY_WIDTH ? DISPLAY_WIDTH : sReqW > MAX_W ? MAX_W : sReqW;
        *h = sReqH < DISPLAY_HEIGHT ? DISPLAY_HEIGHT : sReqH > MAX_H ? MAX_H : sReqH;
        *w &= ~1;
        *h &= ~1;
        return;
    }
    if (IsIndoorMap())
    {
        *w = 320;
        *h = 180;
    }
    else
    {
        *w = 384;
        *h = 216;
    }
}

static bool IsOverworldBgSetup(void)
{
    if ((REG_DISPCNT & 7) != 0)
        return false;
    if (gOverworldTilemapBuffer_Bg1 == NULL || gOverworldTilemapBuffer_Bg2 == NULL || gOverworldTilemapBuffer_Bg3 == NULL)
        return false;
    if (gMapHeader.mapLayout == NULL)
        return false;
    // BG1-3 as set up by the overworld: char block 0, screen blocks 29/28/30, 32x32, 4bpp.
    if ((REG_BG1CNT & 0xFF8C) != (29 << 8) || (REG_BG2CNT & 0xFF8C) != (28 << 8) || (REG_BG3CNT & 0xFF8C) != (30 << 8))
        return false;
    return true;
}

static enum WideViewMode GetMode(void)
{
    static int dbg;
    if (getenv("WJPC_WVDEBUG") && (dbg++ % 300) == 0)
        printf("WV cb2=%p ow=%p trans=%d cnt=%04x %04x %04x disp=%04x buf=%p\n", gMain.callback2, CB2_Overworld,
               WallyJourney_IsBattleTransitionRunning(), REG_BG1CNT, REG_BG2CNT, REG_BG3CNT, REG_DISPCNT, gOverworldTilemapBuffer_Bg1);
    if (!sEnabled || getenv("WJPC_NOWIDE"))
        return WIDE_VIEW_GBA;
    if (gMain.callback2 != CB2_Overworld && gMain.callback2 != CB2_OverworldBasic)
        return WIDE_VIEW_GBA;
    if (gMain.callback2 == CB2_Overworld && !WallyJourney_IsBattleTransitionRunning() && IsOverworldBgSetup())
        return WIDE_VIEW_FIELD;
    return WIDE_VIEW_CENTER;
}

static void UpdatePads(int w, int h)
{
    if (!sEnabled)
    {
        gWjPcViewPadX = gWjPcViewPadY = gWjPcSpawnPadX = gWjPcSpawnPadY = 0;
        return;
    }
    gWjPcViewPadX = (w - DISPLAY_WIDTH) / 2;
    gWjPcViewPadY = (h - DISPLAY_HEIGHT) / 2;
    // One tile more than the visible edge: NPCs load before they scroll into view, not as they do.
    gWjPcSpawnPadX = (gWjPcViewPadX + 15) / 16 + 1;
    gWjPcSpawnPadY = (gWjPcViewPadY + 15) / 16 + 1;
}

// ---------------------------------------------------------------------------
// Map tiles for BG1-3.

static s32 sPosX, sPosY;      // gSaveBlock1Ptr->pos (map grid coords of the BG ring's first metatile)
static u8 sRingX, sRingY;     // where that metatile sits in the 32x32 ring (tile units)

// Maps around the current one, found by following connections outwards (neighbours, their
// neighbours...), each with where its top-left sits in the current map's coordinates. The GBA only
// ever needs direct neighbours; a zoomed-out view sees further (from Route 107, the route north of
// Dewford is a neighbour of a neighbour and used to pop in on crossing the map edge).
#define NEAR_MAPS_MAX 24
#define NEAR_MAPS_DEPTH 4
struct NearMap
{
    const struct MapLayout *layout;
    u8 group, num, depth;
    s32 x, y;
};
static struct NearMap sNearMaps[NEAR_MAPS_MAX];
static int sNearMapCount;
static const struct MapLayout *sNearMapsFor;
static u8 sNearMapsGroup, sNearMapsNum;

static void BuildNearMaps(void)
{
    const struct MapLayout *cur = gMapHeader.mapLayout;
    int head = 0;

    if (cur == sNearMapsFor && sNearMapsGroup == gSaveBlock1Ptr->location.mapGroup && sNearMapsNum == gSaveBlock1Ptr->location.mapNum)
        return;
    sNearMapsFor = cur;
    sNearMapsGroup = gSaveBlock1Ptr->location.mapGroup;
    sNearMapsNum = gSaveBlock1Ptr->location.mapNum;
    sNearMapCount = 0;
    if (cur == NULL)
        return;
    sNearMaps[0] = (struct NearMap){cur, sNearMapsGroup, sNearMapsNum, 0, 0, 0};
    sNearMapCount = 1;
    while (head < sNearMapCount)
    {
        struct NearMap m = sNearMaps[head++];
        const struct MapHeader *hdr = head == 1 ? &gMapHeader : Overworld_GetMapHeaderByGroupAndId(m.group, m.num);
        const struct MapConnections *conns = hdr != NULL ? hdr->connections : NULL;

        if (m.depth >= NEAR_MAPS_DEPTH || conns == NULL || conns->connections == NULL)
            continue;
        for (s32 i = 0; i < conns->count && sNearMapCount < NEAR_MAPS_MAX; i++)
        {
            const struct MapConnection *c = &conns->connections[i];
            const struct MapHeader *h2 = Overworld_GetMapHeaderByGroupAndId(c->mapGroup, c->mapNum);
            const struct MapLayout *l;
            s32 x, y;
            bool seen = false;

            if (h2 == NULL || (l = h2->mapLayout) == NULL || l->map == NULL)
                continue;
            switch (c->direction)
            {
            case CONNECTION_NORTH: x = m.x + c->offset; y = m.y - l->height; break;
            case CONNECTION_SOUTH: x = m.x + c->offset; y = m.y + m.layout->height; break;
            case CONNECTION_WEST:  x = m.x - l->width; y = m.y + c->offset; break;
            case CONNECTION_EAST:  x = m.x + m.layout->width; y = m.y + c->offset; break;
            default: continue;
            }
            for (int k = 0; k < sNearMapCount; k++)
                if (sNearMaps[k].group == c->mapGroup && sNearMaps[k].num == c->mapNum)
                    seen = true;
            if (seen)
                continue;
            sNearMaps[sNearMapCount++] = (struct NearMap){l, c->mapGroup, c->mapNum, m.depth + 1, x, y};
        }
    }
}

static bool GetConnectedMetatile(s32 gx, s32 gy, u16 *outId, const struct MapLayout **outLayout)
{
    s32 x = gx - MAP_OFFSET, y = gy - MAP_OFFSET;

    BuildNearMaps();
    // Nearest maps first (the list is in order of distance), the current map itself excluded.
    for (int i = 1; i < sNearMapCount; i++)
    {
        const struct NearMap *m = &sNearMaps[i];
        s32 lx = x - m->x, ly = y - m->y;

        if (lx < 0 || ly < 0 || lx >= m->layout->width || ly >= m->layout->height)
            continue;
        *outId = m->layout->map[lx + ly * m->layout->width] & MAPGRID_METATILE_ID_MASK;
        *outLayout = m->layout;
        return true;
    }
    return false;
}

// Filler ("border") tile where there is no map at all. The GBA uses the current map's filler, so
// zoomed out it flipped (e.g. water <-> trees) on crossing a map edge. Here each spot takes the
// filler of the map nearest to it, whichever map the player is on.
static u16 GetNearestBorderMetatileFrom(s32 gx, s32 gy, const struct MapLayout **outLayout);
static u16 GetNearestBorderMetatile(s32 gx, s32 gy)
{
    const struct MapLayout *unused;
    return GetNearestBorderMetatileFrom(gx, gy, &unused);
}

static u16 GetNearestBorderMetatileFrom(s32 gx, s32 gy, const struct MapLayout **outLayout)
{
    s32 x = gx - MAP_OFFSET, y = gy - MAP_OFFSET;
    const struct NearMap *best = NULL;
    s32 bestDist = 0x7FFFFFFF;
    const struct MapLayout *l;
    s32 lx, ly;

    BuildNearMaps();
    for (int i = 0; i < sNearMapCount; i++)
    {
        const struct NearMap *m = &sNearMaps[i];
        s32 dx = x < m->x ? m->x - x : x >= m->x + m->layout->width ? x - (m->x + m->layout->width - 1) : 0;
        s32 dy = y < m->y ? m->y - y : y >= m->y + m->layout->height ? y - (m->y + m->layout->height - 1) : 0;
        // A map straight above/below this spot wins over one to the side, and that over a diagonal
        // one, so filler areas meet in straight lines instead of staircases.
        s32 d = (dx == 0 ? dy : dy == 0 ? 0x10000 + dx : 0x20000 + (dx > dy ? dx : dy));

        // Ties go to the lower map id, so the answer doesn't depend on the map the player is on.
        if (d < bestDist || (d == bestDist && (m->group << 8 | m->num) < (best->group << 8 | best->num)))
        {
            bestDist = d;
            best = m;
        }
    }
    *outLayout = gMapHeader.mapLayout;
    if (best == NULL || best->layout->border == NULL)
        return MapGridGetMetatileIdAt(gx, gy);
    l = best->layout;
    *outLayout = l;
    lx = x - best->x;
    ly = y - best->y;
    if (l->isFrlg && l->borderWidth != 0 && l->borderHeight != 0)
    {
        s32 xp = (lx + 8 * l->borderWidth) % l->borderWidth, yp = (ly + 8 * l->borderHeight) % l->borderHeight;
        if (xp < 0) xp += l->borderWidth;
        if (yp < 0) yp += l->borderHeight;
        return l->border[xp + yp * l->borderWidth] & MAPGRID_METATILE_ID_MASK;
    }
    // Same pattern the game gives that map when the player is on it (GetBorderBlockAt).
    return l->border[((lx + MAP_OFFSET + 1) & 1) + ((ly + MAP_OFFSET + 1) & 1) * 2] & MAPGRID_METATILE_ID_MASK;
}

// For the 3D view (voxel3d_pc.c): the filler metatile at map grid (gx, gy), same rule as above.
u16 WideView_NearestBorderMetatile(s32 gx, s32 gy)
{
    return GetNearestBorderMetatile(gx, gy);
}

// DLX-12: a neighbouring map with another secondary tile set (about 70 map links) is drawn with its own
// tiles and colours, not the current map's (which made its own metatiles come out as the wrong pictures).
// Its graphics are kept here; its colours are the tile set's own, season-adjusted like the game does, then
// moved the way the current map's colours are moved this frame (time of day, weather, fades): a per-channel
// line fitted from the current map's stored colours to the ones on screen.
#define FOREIGN_MAX 6
struct ForeignTiles
{
    const struct Tileset *tileset;
    u32 firstTile, firstPal;   // tile numbers / palette slots from here on are this tile set's
    u8 *tiles;
    u16 raw[16 * 16];
    u16 shown[16 * 16];
    u32 seasonKey, shownFrame, lastUse;
};
static struct ForeignTiles sForeign[FOREIGN_MAX];
static const struct ForeignTiles *sEntryForeign; // set by GetMapTileEntry for the entry it returns
static u32 sWideFrameNo, sFitFrame;
static s32 sFitA[3], sFitB[3]; // shown = (a * stored) / 256 + b, per channel

static void FitPaletteShift(void)
{
    const u16 *stored = gPlttBufferUnfaded, *shown = (const u16 *)PLTT;
    s64 n = 0, sx[3] = {0}, sy[3] = {0}, sxx[3] = {0}, sxy[3] = {0};

    if (sFitFrame == sWideFrameNo)
        return;
    sFitFrame = sWideFrameNo;
    for (u32 i = 0; i < NUM_PALS_TOTAL * 16; i++)
    {
        if (i % 16 == 0)
            continue;
        for (int c = 0; c < 3; c++)
        {
            s32 x = (stored[i] >> (5 * c)) & 31, y = (shown[i] >> (5 * c)) & 31;
            sx[c] += x; sy[c] += y; sxx[c] += x * x; sxy[c] += x * y;
        }
        n++;
    }
    for (int c = 0; c < 3; c++)
    {
        s64 var = n * sxx[c] - sx[c] * sx[c];
        if (n == 0 || var <= 0)
        {
            sFitA[c] = 256;
            sFitB[c] = 0;
            continue;
        }
        sFitA[c] = (s32)((n * sxy[c] - sx[c] * sy[c]) * 256 / var);
        sFitB[c] = (s32)((sy[c] * 256 - (s64)sFitA[c] * sx[c]) / (256 * n));
    }
}

static const struct ForeignTiles *GetForeignTiles(const struct MapLayout *l)
{
    const struct MapLayout *cur = gMapHeader.mapLayout;
    struct ForeignTiles *f = NULL;
    u32 key;

    static int sOff = -1;
    if (sOff < 0)
        sOff = getenv("WJPC_TEST_NO_FOREIGN") != NULL; // test aid: draw neighbours as before, to compare
    if (sOff || l == NULL || cur == NULL || l->secondaryTileset == cur->secondaryTileset || l->secondaryTileset == NULL
     || l->primaryTileset != cur->primaryTileset)
        return NULL;
    for (int i = 0; i < FOREIGN_MAX; i++)
        if (sForeign[i].tileset == l->secondaryTileset)
            f = &sForeign[i];
    if (f == NULL)
    {
        const struct Tileset *ts = l->secondaryTileset;
        u32 firstTile = GetNumTilesInPrimary(l);
        u32 bytes = (NUM_TILES_TOTAL - firstTile) * 32;

        f = &sForeign[0];
        for (int i = 1; i < FOREIGN_MAX; i++)
            if (sForeign[i].lastUse < f->lastUse)
                f = &sForeign[i];
        if (ts->isCompressed)
        {
            u32 size = GetDecompressedDataSize(ts->tiles);
            if (size > bytes)
                bytes = size;
        }
        free(f->tiles);
        f->tiles = calloc(1, bytes);
        if (f->tiles == NULL)
        {
            f->tileset = NULL;
            return NULL;
        }
        if (ts->isCompressed)
            DecompressDataWithHeaderWram(ts->tiles, f->tiles);
        else
            memcpy(f->tiles, ts->tiles, (NUM_TILES_TOTAL - firstTile) * 32);
        f->tileset = ts;
        f->firstTile = firstTile;
        f->firstPal = GetNumPalsInPrimary(l);
        f->seasonKey = 0xFFFFFFFF;
        f->shownFrame = 0xFFFFFFFF;
    }
    f->lastUse = sWideFrameNo;
    key = WallyJourney_FoliageKey();
    if (f->seasonKey != key)
    {
        WallyJourney_SeasonalTilesetPalettes((const u16 *)f->tileset->palettes, f->raw, FALSE);
        f->seasonKey = key;
        f->shownFrame = 0xFFFFFFFF;
    }
    if (f->shownFrame != sWideFrameNo)
    {
        FitPaletteShift();
        for (u32 i = 0; i < 16 * 16; i++)
        {
            u16 col = 0;
            for (int c = 0; c < 3; c++)
            {
                s32 v = (((f->raw[i] >> (5 * c)) & 31) * sFitA[c]) / 256 + sFitB[c];
                col |= (u16)((v < 0 ? 0 : v > 31 ? 31 : v) << (5 * c));
            }
            f->shown[i] = col;
        }
        f->shownFrame = sWideFrameNo;
    }
    return f;
}

// Tile entry of BG `bg` (1-3) at grid tile (tx, ty) (8x8 units, map grid coords * 2).
// sEntryForeign: the neighbour's own tile set the entry belongs to, or NULL (the current map's).
static u16 GetMapTileEntry(int bg, s32 tx, s32 ty)
{
    s32 mx = tx >> 1, my = ty >> 1;          // arithmetic shift: floor for negatives
    s32 rx = mx - sPosX, ry = my - sPosY;
    u32 q = (tx & 1) + 2 * (ty & 1);
    const struct MapLayout *layout = gMapHeader.mapLayout;
    const struct MapLayout *from = layout; // the map the metatile belongs to
    const u16 *tiles;
    u16 metatileId;
    u32 layerType;

    sEntryForeign = NULL;

    bool inCurrentMap = mx >= MAP_OFFSET && my >= MAP_OFFSET && mx < MAP_OFFSET + layout->width && my < MAP_OFFSET + layout->height;

    // The part the game has drawn itself: use its ring (includes door animations). Only inside the
    // current map: past its edge the game only knows direct neighbours and its own filler, which
    // would leave a patch around the player that disagrees with the wider view.
    if (inCurrentMap && rx >= 0 && rx < 15 && ry >= 0 && ry < 15)
    {
        u32 x = (sRingX + 2 * rx + (tx & 1)) & 31;
        u32 y = (sRingY + 2 * ry + (ty & 1)) & 31;
        u16 *buf = bg == 1 ? gOverworldTilemapBuffer_Bg1 : bg == 2 ? gOverworldTilemapBuffer_Bg2 : gOverworldTilemapBuffer_Bg3;
        return buf[y * 32 + x];
    }

    if (!inCurrentMap)
    {
        // Outside the current map: the maps around it, else filler.
        if (!GetConnectedMetatile(mx, my, &metatileId, &from))
            metatileId = GetNearestBorderMetatileFrom(mx, my, &from);
        sEntryForeign = GetForeignTiles(from);
        if (sEntryForeign == NULL)
            from = layout; // same tile sets (or none to draw with): as before
    }
    else
    {
        metatileId = MapGridGetMetatileIdAt(mx, my);
    }
    if (metatileId > NUM_METATILES_TOTAL)
        metatileId = 0;
    if (from == layout)
        layerType = GetAttributeByMetatileIdAndMapLayout(metatileId, METATILE_ATTRIBUTE_LAYER_TYPE, layout->isFrlg);
    bool inPrimary = metatileId < GetNumMetatilesInPrimary(from);
    if (inPrimary)
    {
        tiles = from->primaryTileset->metatiles;
    }
    else
    {
        tiles = from->secondaryTileset->metatiles;
        metatileId -= GetNumMetatilesInPrimary(from);
    }
    if (from != layout)
    {
        // The neighbour's own attributes, same as the game would read there.
        const struct Tileset *ts = inPrimary ? from->primaryTileset : from->secondaryTileset;
        u32 attr = from->isFrlg ? ((const u32 *)ts->metatileAttributes)[metatileId]
                                : ((const u16 *)ts->metatileAttributes)[metatileId];
        layerType = ExtractMetatileAttribute(attr, METATILE_ATTRIBUTE_LAYER_TYPE, from->isFrlg);
    }
    tiles += metatileId * NUM_TILES_PER_METATILE;

    // Same as DrawMetatile() in field_camera.c.
    switch (layerType)
    {
    case METATILE_LAYER_TYPE_SPLIT:
        return bg == 3 ? tiles[q] : bg == 2 ? 0 : tiles[4 + q];
    case METATILE_LAYER_TYPE_COVERED:
        return bg == 3 ? tiles[q] : bg == 2 ? tiles[4 + q] : 0;
    default:
        return bg == 3 ? 0x3014 : bg == 2 ? tiles[q] : tiles[4 + q];
    }
}

// ---------------------------------------------------------------------------
// Sprites with full coordinates.

struct WideSprite
{
    struct OamData oam;
    s32 x, y;           // top-left in GBA screen coordinates (can be far outside 0..239)
    s16 matrix[4];
    bool marginOnly;    // a repeated weather sprite: only drawn outside the GBA screen
    bool withCamera;    // smooth motion: stays put on screen while the camera moves (player, follower)
    bool ui;            // screen-space sprite (start menu, HM picture...): drawn on the UI layer
};

static struct WideSprite sWideSprites[1024];
static int sWideSpriteCount;

static bool sAddingWithCamera; // set while BuildWideSprites adds a sprite that moves with the camera
static bool sAddingUi;         // set while BuildWideSprites adds a screen-space (UI) sprite

static void AddWideSprite(const struct OamData *oam, s32 x, s32 y, bool marginOnly)
{
    struct WideSprite *ws;
    if (sWideSpriteCount >= (int)ARRAY_COUNT(sWideSprites))
        return;
    ws = &sWideSprites[sWideSpriteCount++];
    ws->oam = *oam;
    ws->x = x;
    ws->y = y;
    ws->marginOnly = marginOnly;
    ws->withCamera = sAddingWithCamera;
    ws->ui = sAddingUi;
    if (oam->affineMode & ST_OAM_AFFINE_ON_MASK)
    {
        ws->matrix[0] = gOamMatrices[oam->matrixNum].a;
        ws->matrix[1] = gOamMatrices[oam->matrixNum].b;
        ws->matrix[2] = gOamMatrices[oam->matrixNum].c;
        ws->matrix[3] = gOamMatrices[oam->matrixNum].d;
    }
}

static void SortKeys(u64 *keys, int n)
{
    for (int i = 1; i < n; i++)
    {
        u64 k = keys[i];
        int j = i - 1;
        while (j >= 0 && keys[j] > k)
        {
            keys[j + 1] = keys[j];
            j--;
        }
        keys[j + 1] = k;
    }
}

// Weather particles (rain, snow, ash, fog, sandstorm) are made for the 240x160 screen. They are
// laid out in a repeating pattern, so the wide view repeats them into the margins. Clouds and
// bubbles follow the map and are left alone.
static bool GetWeatherRepeat(const struct Sprite *sprite, s32 *periodX, s32 *periodY)
{
    const struct SpriteTemplate *t = sprite->template;
    if (t == NULL || sprite->coordOffsetEnabled)
        return false;
    if (t->paletteTag != PALTAG_WEATHER && t->paletteTag != PALTAG_WEATHER_2)
        return false;
    switch (t->tileTag)
    {
    case GFXTAG_FOG_H:
    case GFXTAG_ASH:
    case GFXTAG_FOG_D:
    case GFXTAG_SANDSTORM:
        // 5 columns x 4 rows of 64x64 sprites that wrap every 320 pixels.
        *periodX = 320;
        *periodY = 256;
        return true;
    case GFXTAG_RAIN:
    case TAG_NONE:          // snowflakes
        *periodX = DISPLAY_WIDTH;
        *periodY = DISPLAY_HEIGHT;
        return true;
    }
    return false;
}

static void AddSpriteOam(struct Sprite *sprite, const struct OamData *oam, s32 x, s32 y, bool marginOnly)
{
    const struct SubspriteTable *table;
    if (!sprite->subspriteTables || sprite->subspriteMode == SUBSPRITES_OFF
     || !(table = &sprite->subspriteTables[sprite->subspriteTableNum]) || !table->subsprites)
    {
        AddWideSprite(oam, x, y, marginOnly);
    }
    else
    {
        bool hFlip = (oam->matrixNum >> 3) & 1, vFlip = (oam->matrixNum >> 4) & 1;
        s32 baseX = x - sprite->centerToCornerVecX, baseY = y - sprite->centerToCornerVecY;
        u32 j;
        for (j = 0; j < table->subspriteCount; j++)
        {
            const struct Subsprite *sub = &table->subsprites[j];
            s32 sx = (s16)sub->x, sy = (s16)sub->y;
            struct OamData subOam = *oam;
            if (hFlip)
                sx = -(sx + gOamDimensions[sub->shape][sub->size].width);
            if (vFlip)
                sy = -(sy + gOamDimensions[sub->shape][sub->size].height);
            subOam.shape = sub->shape;
            subOam.size = sub->size;
            subOam.tileNum = oam->tileNum + sub->tileOffset;
            if (sprite->subspriteMode < SUBSPRITES_IGNORE_PRIORITY)
                subOam.priority = sub->priority;
            AddWideSprite(&subOam, baseX + sx, baseY + sy, marginOnly);
        }
    }
}

// Smooth motion: where each sprite was on screen last frame and whether the camera moved,
// to find the sprites that move with the camera (they must not slide with the map).
static s32 sLastSpriteX[MAX_SPRITES], sLastSpriteY[MAX_SPRITES];
static const struct SpriteTemplate *sLastSpriteTemplate[MAX_SPRITES];
static bool sCameraMoved;

// Same order and subsprite expansion as BuildOamBuffer(), without wrapping coordinates.
// DLX-12: people on neighbouring maps. The game only creates the current map's people, so zoomed out the
// next map's stood empty until the player crossed into it. They are drawn here as they start (their spot,
// their first facing), unless a flag hides them, with their pictures and colours put in the room sprite.h
// keeps for this (so they layer, fade and tint like everyone else). The real ones replace them on crossing.
extern const u16 *WJPC_ObjectEventPaletteData(u16 tag);
extern const u8 gInitialMovementTypeFacingDirections[];
static int sW, sH, sOX, sOY; // (defined below)
struct GhostPal { u16 tag; u32 lastUse; };
static struct GhostPal sGhostPals[WJPC_GHOST_PALETTES];

static int GhostPaletteSlot(u16 tag)
{
    u32 first = OBJ_PALETTE_COUNT - WJPC_GHOST_PALETTES;
    int pick = -1;

    for (int i = 0; i < WJPC_GHOST_PALETTES; i++)
        if (sGhostPals[i].tag == tag && GetSpritePaletteTagByPaletteNum(first + i) == (tag ^ 0x4000))
        {
            sGhostPals[i].lastUse = sWideFrameNo;
            return first + i;
        }
    for (int i = 0; i < WJPC_GHOST_PALETTES; i++)
        if (pick < 0 || sGhostPals[i].lastUse < sGhostPals[pick].lastUse
         || GetSpritePaletteTagByPaletteNum(first + i) != (sGhostPals[i].tag ^ 0x4000))
        {
            pick = i;
            if (GetSpritePaletteTagByPaletteNum(first + i) != (sGhostPals[i].tag ^ 0x4000))
                break;
        }
    if (sGhostPals[pick].lastUse == sWideFrameNo && GetSpritePaletteTagByPaletteNum(first + pick) == (sGhostPals[pick].tag ^ 0x4000))
        return -1; // all eight in use this frame
    {
        const u16 *data = WJPC_ObjectEventPaletteData(tag);
        u32 slot = first + pick;
        if (data == NULL)
            return -1;
        CpuCopy16(data, &gPlttBufferUnfaded[OBJ_PLTT_ID(slot)], PLTT_SIZE_4BPP);
        CpuCopy16(data, &gPlttBufferFaded[OBJ_PLTT_ID(slot)], PLTT_SIZE_4BPP);
        WJPC_SetSpritePaletteTag(slot, tag ^ 0x4000); // its own tag: real sprites never share this slot
        UpdateSpritePaletteWithTime(slot);
        sGhostPals[pick].tag = tag;
        sGhostPals[pick].lastUse = sWideFrameNo;
        return slot;
    }
}

static void AddNeighbourPeople(void)
{
    u32 tileNext = TOTAL_OBJ_TILE_COUNT - WJPC_GHOST_TILES;

    static int sOff = -1;
    if (sOff < 0)
        sOff = getenv("WJPC_TEST_NO_GHOSTS") != NULL; // test aid: no people on neighbouring maps, as before
    if (sOff || GetMode() != WIDE_VIEW_FIELD)
        return;
    BuildNearMaps();
    for (int m = 1; m < sNearMapCount; m++)
    {
        const struct NearMap *nm = &sNearMaps[m];
        const struct MapHeader *hdr = Overworld_GetMapHeaderByGroupAndId(nm->group, nm->num);
        const struct MapEvents *ev = hdr != NULL ? hdr->events : NULL;

        if (ev == NULL || ev->objectEvents == NULL)
            continue;
        for (u32 i = 0; i < ev->objectEventCount; i++)
        {
            const struct ObjectEventTemplate *t = &ev->objectEvents[i];
            const struct ObjectEventGraphicsInfo *info;
            struct OamData oam;
            u32 frame, tiles;
            s16 sx, sy;
            s32 x, y;
            int pal;
            u8 dir;

            if (t->kind == OBJ_KIND_CLONE || (t->flagId != 0 && FlagGet(t->flagId)))
                continue;
            if (t->graphicsId == OBJ_EVENT_GFX_BERRY_TREE || t->graphicsId == OBJ_EVENT_GFX_LIGHT_SPRITE
             || (t->graphicsId & OBJ_EVENT_MON)) // a berry tree's picture is its own; a light is not a person
                continue;
            SetSpritePosToMapCoords(nm->x + t->x + MAP_OFFSET, nm->y + t->y + MAP_OFFSET, &sx, &sy);
            info = GetObjectEventGraphicsInfo(t->graphicsId);
            if (info == NULL || info->images == NULL || info->oam == NULL || info->compressed)
                continue;
            x = sx + 8 - info->width / 2 + gSpriteCoordOffsetX;
            y = sy + 16 - info->height + gSpriteCoordOffsetY;
            if (x < -sOX - 64 || x > sW - sOX + 64 || y < -sOY - 64 || y > sH - sOY + 64)
                continue; // off the view
            tiles = info->size / TILE_SIZE_4BPP;
            if (tiles == 0 || tileNext + tiles > TOTAL_OBJ_TILE_COUNT)
                continue;
            pal = GhostPaletteSlot(info->paletteTag);
            if (pal < 0)
                continue;
            // Standing frame for the way it first faces: south 0, north 1, west 2, east 2 flipped.
            dir = t->movementType < NUM_MOVEMENT_TYPES ? gInitialMovementTypeFacingDirections[t->movementType] : DIR_SOUTH;
            frame = info->inanimate ? 0 : dir == DIR_NORTH ? 1 : (dir == DIR_WEST || dir == DIR_EAST) ? 2 : 0;
            memcpy((u8 *)OBJ_VRAM0 + tileNext * TILE_SIZE_4BPP, info->images[frame].data, info->size);
            oam = *info->oam;
            oam.tileNum = tileNext;
            oam.paletteNum = pal;
            oam.priority = ElevationToPriority(t->elevation);
            oam.matrixNum = (dir == DIR_EAST && !info->inanimate) ? 8 : 0; // bit 3: horizontal flip
            oam.affineMode = ST_OAM_AFFINE_OFF;
            tileNext += tiles;
            AddWideSprite(&oam, x, y, false);
        }
    }
}

static void BuildWideSprites(void)
{
    u64 keys[MAX_SPRITES];
    int n = 0, i;

    sWideSpriteCount = 0;
    for (i = 0; i < MAX_SPRITES; i++)
    {
        struct Sprite *sprite = &gSprites[i];
        s32 y;
        if (!sprite->inUse || sprite->invisible)
            continue;
        y = sprite->y + sprite->y2 + sprite->centerToCornerVecY + (sprite->coordOffsetEnabled ? gSpriteCoordOffsetY : 0);
        // Same order as BuildOamBuffer: priority, subpriority, then lower on screen first.
        keys[n++] = ((u64)sprite->oam.priority << 40) | ((u64)sprite->subpriority << 32)
                  | ((u64)((0x7FFFFF - (y + 0x8000)) & 0xFFFFFF) << 8) | (u64)i;
    }
    SortKeys(keys, n);

    for (i = 0; i < n; i++)
    {
        struct Sprite *sprite = &gSprites[keys[i] & 0xFF];
        struct OamData oam = sprite->oam;
        s32 x = sprite->x + sprite->x2 + sprite->centerToCornerVecX;
        s32 y = sprite->y + sprite->y2 + sprite->centerToCornerVecY;
        s32 px, py;

        if (sprite->coordOffsetEnabled)
        {
            x += gSpriteCoordOffsetX;
            y += gSpriteCoordOffsetY;
        }
        // gSpriteCoordOffset is s16 but the camera totals wrap as u16: keep the wrap the game sees.
        x = (s16)x;
        y = (s16)y;

        // With the camera: a map sprite that kept its screen place while the camera moved.
        // Screen-space sprites that aren't weather (rare in the field) count too.
        {
            // Compare positions without x2/y2: the surf blob and the player bob up and down
            // while they follow the camera, and must not slide with the map.
            int idx = keys[i] & 0xFF;
            bool sameSprite = sLastSpriteTemplate[idx] == sprite->template;
            s32 bx = (s16)(x - sprite->x2), by = (s16)(y - sprite->y2);
            sAddingUi = false;
            if (sprite->coordOffsetEnabled)
                sAddingWithCamera = sCameraMoved && sameSprite && sLastSpriteX[idx] == bx && sLastSpriteY[idx] == by;
            else
            {
                sAddingWithCamera = !GetWeatherRepeat(sprite, &px, &py);
                sAddingUi = sAddingWithCamera;
            }
            sLastSpriteX[idx] = bx;
            sLastSpriteY[idx] = by;
            sLastSpriteTemplate[idx] = sprite->template;
        }

        if (GetWeatherRepeat(sprite, &px, &py))
        {
            // The game places these on the 9-bit OAM x / 8-bit OAM y and relies on the wrap.
            // Undo the wrap the way the GBA would (y >= 160 is above the screen); x is kept in
            // [-64, 448) so a column just right of the screen stays on the right.
            x &= 0x1FF;
            if (x >= 448)
                x -= 512;
            y &= 0xFF;
            if (y >= DISPLAY_HEIGHT)
                y -= 256;
            AddSpriteOam(sprite, &oam, x, y, false);
            for (int ry = -1; ry <= 1; ry++)
                for (int rx = -1; rx <= 1; rx++)
                    if (rx || ry)
                        AddSpriteOam(sprite, &oam, x + rx * px, y + ry * py, true);
        }
        else
        {
            AddSpriteOam(sprite, &oam, x, y, false);
        }
    }
    sAddingWithCamera = false;
    sAddingUi = false;
    AddNeighbourPeople();
}

// ---------------------------------------------------------------------------
// The compositor: gba_easy_draw.c's scanline renderer, widened.

#define getAlphaBit(x) ((x >> 15) & 1)
#define getRedChannel(x) ((x >>  0) & 0x1F)
#define getGreenChannel(x) ((x >>  5) & 0x1F)
#define getBlueChannel(x) ((x >>  10) & 0x1F)
#define isbgEnabled(x) ((REG_DISPCNT >> 8) & 0xF) & (1 << x)

#define WINMASK_OBJ    (1 << 4)
#define WINMASK_CLR    (1 << 5)
#define WINMASK_WINOUT (1 << 6)

#define LINE_W (MAX_W + 2 * WIDE_VIEW_SMOOTH_MARGIN) // widest canvas (smooth motion adds a margin)

struct WideLine
{
    uint16_t layers[4][LINE_W];
    uint16_t spriteLayers[4][LINE_W];
    uint16_t mapSpriteLayers[4][LINE_W]; // smooth motion: sprites that slide with the map only
    uint16_t worldSpriteLayers[4][LINE_W]; // UI layer on: every sprite except the UI ones
    uint16_t winMask[LINE_W];
    char prioritySortedBgs[4][4];
    char prioritySortedBgsCount[4];
};

static int sW, sH, sOX, sOY;   // canvas size and where the GBA screen sits in it
static uint16_t *sSmoothOverlay; // smooth motion: where BG0 goes (NULL = normal drawing)
static int sSmoothMargin;
static uint16_t *sOverlayLine;   // this scanline's row of sSmoothOverlay
static uint16_t *sUiOverlay;     // UI layer: BG0 (text boxes, menus) + screen-space sprites (NULL = off)
static uint16_t *sUiLine;        // this scanline's row of sUiOverlay

void WideView_SetUiOverlay(uint16_t *overlay)
{
    sUiOverlay = overlay;
}

void WideView_SetSmooth(uint16_t *overlay, int margin)
{
    sSmoothOverlay = overlay;
    sSmoothMargin = overlay ? margin : 0;
}

static const u8 sSpriteSizes[][2] = {{8, 16}, {8, 32}, {16, 32}, {32, 64}};

static uint16_t AlphaBlendColor(uint16_t a, uint16_t b)
{
    unsigned int eva = REG_BLDALPHA & 0x1F, evb = (REG_BLDALPHA >> 8) & 0x1F;
    unsigned int r = ((getRedChannel(a) * eva) + (getRedChannel(b) * evb)) >> 4;
    unsigned int g = ((getGreenChannel(a) * eva) + (getGreenChannel(b) * evb)) >> 4;
    unsigned int bl = ((getBlueChannel(a) * eva) + (getBlueChannel(b) * evb)) >> 4;
    if (r > 31) r = 31;
    if (g > 31) g = 31;
    if (bl > 31) bl = 31;
    return r | (g << 5) | (bl << 10) | (1 << 15);
}

static uint16_t BrightnessUp(uint16_t a)
{
    unsigned int evy = REG_BLDY & 0x1F;
    unsigned int r = getRedChannel(a) + (31 - getRedChannel(a)) * evy / 16;
    unsigned int g = getGreenChannel(a) + (31 - getGreenChannel(a)) * evy / 16;
    unsigned int b = getBlueChannel(a) + (31 - getBlueChannel(a)) * evy / 16;
    if (r > 31) r = 31;
    if (g > 31) g = 31;
    if (b > 31) b = 31;
    return r | (g << 5) | (b << 10) | (1 << 15);
}

static uint16_t BrightnessDown(uint16_t a)
{
    unsigned int evy = REG_BLDY & 0x1F;
    unsigned int r = getRedChannel(a) - getRedChannel(a) * evy / 16;
    unsigned int g = getGreenChannel(a) - getGreenChannel(a) * evy / 16;
    unsigned int b = getBlueChannel(a) - getBlueChannel(a) * evy / 16;
    return r | (g << 5) | (b << 10) | (1 << 15);
}

static bool SelectTargetB(struct WideLine *line, uint16_t *out, int prnum, int prsub, int x, bool spriteBlend)
{
    for (int p = prnum; p <= 3; p++)
    {
        if (spriteBlend && getAlphaBit(line->spriteLayers[p][x]))
        {
            *out = line->spriteLayers[p][x];
            return true;
        }
        for (int s = prsub; s < line->prioritySortedBgsCount[p]; s++)
        {
            int layer = line->prioritySortedBgs[p][s];
            if (getAlphaBit(line->layers[layer][x]) && (REG_BLDCNT & (1 << (8 + layer))) && isbgEnabled(layer))
            {
                *out = line->layers[layer][x];
                return true;
            }
            if (getAlphaBit(line->layers[layer][x]) && isbgEnabled(layer) && prnum != p)
                return false;
        }
        prsub = 0;
    }
    if (REG_BLDCNT & BLDCNT_TGT2_BD)
    {
        *out = *(uint16_t *)PLTT;
        return true;
    }
    return false;
}

static inline bool InGbaRect(int x, int gl)
{
    return gl >= 0 && gl < DISPLAY_HEIGHT && x >= sOX && x < sOX + DISPLAY_WIDTH;
}

// A text-mode BG from VRAM (the GBA way), only inside the GBA screen.
static void RenderVramBgLine(uint16_t control, uint16_t hoffs, uint16_t voffs, int gl, uint16_t *out)
{
    static const uint16_t mapSizes[][2] = {{32, 32}, {64, 32}, {32, 64}, {64, 64}};
    unsigned int charBase = (control >> 2) & 3, screenBase = (control >> 8) & 0x1F;
    unsigned int bpp = ((control >> 7) & 1) ? 8 : 4;
    unsigned int mapWpx = mapSizes[control >> 14][0] * 8, mapHpx = mapSizes[control >> 14][1] * 8;
    uint8_t *bgtiles = (uint8_t *)BG_CHAR_ADDR(charBase);
    uint16_t *pal = (uint16_t *)PLTT;

    if (gl < 0 || gl >= DISPLAY_HEIGHT)
        return;
    hoffs &= 0x1FF;
    voffs &= 0x1FF;
    // The HM "Pokémon shows itself" banner (BG0) carries on across the whole wide screen.
    int xFrom = 0, xTo = DISPLAY_WIDTH;
    if (FieldEffectActiveListContains(FLDEFF_FIELD_MOVE_SHOW_MON))
        xFrom = -sOX, xTo = sW - sOX;
    for (int x = xFrom; x < xTo; x++)
    {
        uint16_t *bgmap = (uint16_t *)BG_SCREEN_ADDR(screenBase);
        unsigned int xx = (unsigned int)(x + hoffs) & 0x1FF, yy = (gl + voffs) & 0x1FF;
        if (xx > 255 && mapWpx > 256)
            bgmap += 0x400;
        if (yy > 255 && mapHpx > 256)
            bgmap += (mapWpx > 256) ? 0x800 : 0x400;
        xx &= 0xFF;
        yy &= 0xFF;
        uint16_t entry = bgmap[(yy / 8) * 32 + xx / 8];
        unsigned int tileX = xx % 8, tileY = yy % 8;
        if (entry & (1 << 10)) tileX = 7 - tileX;
        if (entry & (1 << 11)) tileY = 7 - tileY;
        if (bpp == 4)
        {
            uint8_t pixel = bgtiles[(entry & 0x3FF) * 32 + tileY * 4 + tileX / 2];
            pixel = (tileX & 1) ? pixel >> 4 : pixel & 0xF;
            if (pixel)
                out[sOX + x] = pal[16 * ((entry >> 12) & 0xF) + pixel] | 0x8000;
        }
        else
        {
            uint8_t pixel = bgtiles[(entry & 0x3FF) * 64 + tileY * 8 + tileX];
            if (pixel)
                out[sOX + x] = pal[pixel] | 0x8000;
        }
    }
}

// BG1-3 of the overworld, from map data, across the whole canvas width.
static void RenderMapBgLine(int bg, uint16_t control, uint16_t hoffs, uint16_t voffs, int gl, uint16_t *out)
{
    uint8_t *bgtiles = (uint8_t *)BG_CHAR_ADDR((control >> 2) & 3);
    uint16_t *pal = (uint16_t *)PLTT;
    // Screen pixel (gx, gl) shows ring pixel (gx + hoffs, gl + voffs); the ring's tile
    // (sRingX, sRingY) is map grid metatile (sPosX, sPosY).
    s32 dx = (s32)((hoffs - sRingX * 8) & 0xFF);
    s32 dy = (s32)((voffs - sRingY * 8) & 0xFF);
    if (dx >= 128) dx -= 256;
    if (dy >= 128) dy -= 256;
    s32 wy = sPosY * 16 + dy + gl;
    s32 wx0 = sPosX * 16 + dx - sOX;
    s32 lastTx = 0x7FFFFFFF;
    uint16_t entry = 0;
    const struct ForeignTiles *foreign = NULL;

    for (int x = 0; x < sW; x++)
    {
        s32 wx = wx0 + x;
        s32 tx = wx >> 3, ty = wy >> 3;
        if (tx != lastTx)
        {
            entry = GetMapTileEntry(bg, tx, ty);
            foreign = sEntryForeign;
            lastTx = tx;
        }
        unsigned int tileX = wx & 7, tileY = wy & 7;
        if (entry & (1 << 10)) tileX = 7 - tileX;
        if (entry & (1 << 11)) tileY = 7 - tileY;
        u32 tileNum = entry & 0x3FF, palNum = (entry >> 12) & 0xF;
        const uint8_t *tile = foreign != NULL && tileNum >= foreign->firstTile
                            ? foreign->tiles + (tileNum - foreign->firstTile) * 32
                            : bgtiles + tileNum * 32;
        uint8_t pixel = tile[tileY * 4 + tileX / 2];
        pixel = (tileX & 1) ? pixel >> 4 : pixel & 0xF;
        if (pixel)
            out[x] = (foreign != NULL && palNum >= foreign->firstPal ? foreign->shown[16 * palNum + pixel]
                                                                     : pal[16 * palNum + pixel]) | 0x8000;
    }
}

static void DrawWideSprites(struct WideLine *line, int vy, bool windowsEnabled)
{
    uint8_t *tiledata = (uint8_t *)(VRAM_ + 0x10000);
    unsigned int blendMode = (REG_BLDCNT >> 6) & 3;

    for (int i = sWideSpriteCount - 1; i >= 0; i--)
    {
        const struct WideSprite *ws = &sWideSprites[i];
        const struct OamData *oam = &ws->oam;
        bool isAffine = oam->affineMode & 1;
        bool doubleSize = (oam->affineMode >> 1) & 1;
        int width, height;
        s16 m00 = 0x100, m01 = 0, m10 = 0, m11 = 0x100;

        if (!isAffine && doubleSize)
            continue;
        if (oam->shape == 0)
            width = height = (1 << oam->size) * 8;
        else if (oam->shape == 1)
            width = sSpriteSizes[oam->size][1], height = sSpriteSizes[oam->size][0];
        else if (oam->shape == 2)
            width = sSpriteSizes[oam->size][0], height = sSpriteSizes[oam->size][1];
        else
            continue;

        int halfW = width / 2, halfH = height / 2;
        if (isAffine)
        {
            m00 = ws->matrix[0]; m01 = ws->matrix[1]; m10 = ws->matrix[2]; m11 = ws->matrix[3];
            if (doubleSize)
            {
                halfW *= 2;
                halfH *= 2;
            }
        }
        int cx = ws->x + sOX + halfW, cy = ws->y + sOY + halfH;
        if (vy < cy - halfH || vy >= cy + halfH)
            continue;

        int localY = vy - cy;
        bool flipX = !isAffine && ((oam->matrixNum >> 3) & 1);
        bool flipY = !isAffine && ((oam->matrixNum >> 4) & 1);
        bool is8bpp = oam->bpp & 1;
        bool semiTransparent = oam->objMode == 1, objWin = oam->objMode == 2;
        uint16_t *pixels = line->spriteLayers[oam->priority];
        uint16_t *mapPixels = (sOverlayLine != NULL && !ws->withCamera && !ws->ui) ? line->mapSpriteLayers[oam->priority] : NULL;
        uint16_t *worldPixels = (sUiLine != NULL && !ws->ui) ? line->worldSpriteLayers[oam->priority] : NULL;

        for (int localX = -halfW; localX < halfW; localX++)
        {
            int gx = localX + cx;
            if (gx < 0 || gx >= sW)
                continue;
            if (ws->marginOnly && InGbaRect(gx, vy - sOY))
                continue;
            int texX = ((m00 * localX + m01 * localY) >> 8) + width / 2;
            int texY = ((m10 * localX + m11 * localY) >> 8) + height / 2;
            if (texX < 0 || texY < 0 || texX >= width || texY >= height)
                continue;
            if (flipX) texX = width - texX - 1;
            if (flipY) texY = height - texY - 1;
            int block = (texY / 8) * (width / 8) + texX / 8;
            uint16_t pixel;
            uint16_t *palette = (uint16_t *)(PLTT + 0x200);
            if (!is8bpp)
            {
                pixel = tiledata[((block + oam->tileNum) * 32 + (texY % 8) * 4 + (texX % 8) / 2) & (OBJ_VRAM0_SIZE - 1)];
                pixel = (texX & 1) ? pixel >> 4 : pixel & 0xF;
                palette += oam->paletteNum * 16;
            }
            else
            {
                pixel = tiledata[((block * 2 + oam->tileNum) * 32 + (texY % 8) * 8 + texX % 8) & (OBJ_VRAM0_SIZE - 1)];
            }
            if (!pixel)
                continue;
            uint16_t color = palette[pixel];
            if (objWin)
            {
                if (line->winMask[gx] & WINMASK_WINOUT)
                    line->winMask[gx] = (REG_WINOUT >> 8) & 0x3F;
                continue;
            }
            bool winBlend = !windowsEnabled || (line->winMask[gx] & WINMASK_CLR);
            if ((blendMode == 1 && (REG_BLDCNT & BLDCNT_TGT1_OBJ) && winBlend) || semiTransparent)
            {
                uint16_t b;
                if (SelectTargetB(line, &b, oam->priority, 0, gx, false))
                    color = AlphaBlendColor(color, b);
            }
            else if ((REG_BLDCNT & BLDCNT_TGT1_OBJ) && winBlend)
            {
                if (blendMode == 2)
                    color = BrightnessUp(color);
                else if (blendMode == 3)
                    color = BrightnessDown(color);
            }
            pixels[gx] = color | (1 << 15);
            if (mapPixels != NULL)
                mapPixels[gx] = color | (1 << 15);
            if (worldPixels != NULL)
                worldPixels[gx] = color | (1 << 15);
        }
    }
}

static bool WinVert(uint16_t reg, int gl)
{
    int bottom = reg & 0xFF, top = (reg >> 8) & 0xFF;
    if (top > bottom)
        return gl >= top || gl < bottom;
    return gl >= top && gl < bottom;
}

static bool WinHorz(uint16_t reg, int gx)
{
    int right = reg & 0xFF, left = (reg >> 8) & 0xFF;
    if (left > right)
        return gx >= left || gx < right;
    return gx >= left && gx < right;
}

static void CompositeLine(struct WideLine *linePtr, uint16_t *pixels, bool windowsEnabled, unsigned int blendMode, bool withBg0, uint16_t (*spriteSrc)[LINE_W]);

static void DrawWideScanline(uint16_t *pixels, int vy)
{
    static struct WideLine line;
    int gl = vy - sOY;
    unsigned int blendMode = (REG_BLDCNT >> 6) & 3;
    int bgnum;

    memset(line.layers, 0, sizeof(line.layers));
    memset(line.spriteLayers, 0, sizeof(line.spriteLayers));
    if (sOverlayLine != NULL)
        memset(line.mapSpriteLayers, 0, sizeof(line.mapSpriteLayers));
    if (sUiLine != NULL)
        memset(line.worldSpriteLayers, 0, sizeof(line.worldSpriteLayers));
    memset(line.winMask, 0, sizeof(line.winMask));
    memset(line.prioritySortedBgsCount, 0, sizeof(line.prioritySortedBgsCount));

    for (bgnum = 0; bgnum < 4; bgnum++)
    {
        uint16_t cnt = *(uint16_t *)(REG_ADDR_BG0CNT + bgnum * 2);
        int prio = cnt & 3;
        line.prioritySortedBgs[prio][line.prioritySortedBgsCount[prio]++] = bgnum;
    }
    for (bgnum = 3; bgnum >= 0; bgnum--)
    {
        if (!isbgEnabled(bgnum))
            continue;
        uint16_t cnt = *(uint16_t *)(REG_ADDR_BG0CNT + bgnum * 2);
        uint16_t hofs = *(uint16_t *)(REG_ADDR_BG0HOFS + bgnum * 4);
        uint16_t vofs = *(uint16_t *)(REG_ADDR_BG0VOFS + bgnum * 4);
        if (bgnum >= 1)
            RenderMapBgLine(bgnum, cnt, hofs, vofs, gl, line.layers[bgnum]);
        else
            RenderVramBgLine(cnt, hofs, vofs, gl, line.layers[bgnum]);
    }

    bool windowsEnabled = (REG_DISPCNT & (DISPCNT_WIN0_ON | DISPCNT_WIN1_ON))
                       || ((REG_DISPCNT & DISPCNT_OBJWIN_ON) && (REG_DISPCNT & DISPCNT_OBJ_ON));
    if (windowsEnabled)
    {
        // Outside the GBA screen, use the window setting of the nearest screen edge pixel,
        // so a window that covers the whole screen (or a dark cave's darkness) carries on.
        int cgl = gl < 0 ? 0 : gl >= DISPLAY_HEIGHT ? DISPLAY_HEIGHT - 1 : gl;
        bool w0 = (REG_DISPCNT & DISPCNT_WIN0_ON) && WinVert(REG_WIN0V, cgl);
        bool w1 = (REG_DISPCNT & DISPCNT_WIN1_ON) && WinVert(REG_WIN1V, cgl);
        for (int x = 0; x < sW; x++)
        {
            int gx = x - sOX;
            if (gx < 0)
                gx = 0;
            else if (gx >= DISPLAY_WIDTH)
                gx = DISPLAY_WIDTH - 1;
            if (w0 && WinHorz(REG_WIN0H, gx))
                line.winMask[x] = REG_WININ & 0x3F;
            else if (w1 && WinHorz(REG_WIN1H, gx))
                line.winMask[x] = (REG_WININ >> 8) & 0x3F;
            else
                line.winMask[x] = (REG_WINOUT & 0x3F) | WINMASK_WINOUT;
        }
    }

    if (REG_DISPCNT & DISPCNT_OBJ_ON)
        DrawWideSprites(&line, vy, windowsEnabled);

    if (sUiLine != NULL)
    {
        // UI layer: the world (map + every non-UI sprite, no BG0) goes to `pixels` (or is split
        // further for smooth motion below); wherever BG0 or UI sprites change the result, the
        // full picture goes to the UI layer, which the display code scales on its own.
        static uint16_t full[LINE_W], world[LINE_W];
        memcpy(full, pixels, sW * sizeof(uint16_t));
        memcpy(world, pixels, sW * sizeof(uint16_t));
        CompositeLine(&line, full, windowsEnabled, blendMode, true, line.spriteLayers);
        CompositeLine(&line, world, windowsEnabled, blendMode, false, line.worldSpriteLayers);
        // A pixel belongs to the UI layer where BG0 or a UI sprite draws on it, even when it comes
        // out the same colour as the map under it (a white text box over the white floor dots of a
        // Pokémon Center): comparing colours alone left holes there, and through them showed
        // whatever lies behind the enlarged UI layer on screen (black outside the room).
        bool bg0On = isbgEnabled(0);
        for (int x = 0; x < sW; x++)
        {
            bool ui = full[x] != world[x];
            if (!ui && bg0On && getAlphaBit(line.layers[0][x]) && (!windowsEnabled || (line.winMask[x] & 1)))
                ui = true;
            for (int p = 0; p < 4 && !ui; p++)
                if (getAlphaBit(line.spriteLayers[p][x]) && !getAlphaBit(line.worldSpriteLayers[p][x])
                    && (!windowsEnabled || (line.winMask[x] & WINMASK_OBJ)))
                    ui = true;
            if (ui)
                sUiLine[x] = full[x] | 0x8000;
        }
        if (sOverlayLine != NULL)
        {
            CompositeLine(&line, pixels, windowsEnabled, blendMode, false, line.mapSpriteLayers);
            for (int x = 0; x < sW; x++)
                if (world[x] != pixels[x])
                    sOverlayLine[x] = world[x] | 0x8000;
        }
        else
        {
            memcpy(pixels, world, sW * sizeof(uint16_t));
        }
        return;
    }
    if (sOverlayLine != NULL)
    {
        // Smooth motion: the map picture (no BG0, no sprites that move with the camera) goes to
        // `pixels`; wherever those change the result, the full picture goes to the overlay.
        static uint16_t full[LINE_W];
        memcpy(full, pixels, sW * sizeof(uint16_t));
        CompositeLine(&line, full, windowsEnabled, blendMode, true, line.spriteLayers);
        CompositeLine(&line, pixels, windowsEnabled, blendMode, false, line.mapSpriteLayers);
        for (int x = 0; x < sW; x++)
            if (full[x] != pixels[x])
                sOverlayLine[x] = full[x] | 0x8000;
        return;
    }
    CompositeLine(&line, pixels, windowsEnabled, blendMode, true, line.spriteLayers);
}

static void CompositeLine(struct WideLine *linePtr, uint16_t *pixels, bool windowsEnabled, unsigned int blendMode, bool withBg0, uint16_t (*spriteSrc)[LINE_W])
{
#define line (*linePtr)
    for (int prnum = 3; prnum >= 0; prnum--)
    {
        for (int prsub = line.prioritySortedBgsCount[prnum] - 1; prsub >= 0; prsub--)
        {
            int bg = line.prioritySortedBgs[prnum][prsub];
            if (!isbgEnabled(bg) || (bg == 0 && !withBg0))
                continue;
            uint16_t *src = line.layers[bg];
            for (int x = 0; x < sW; x++)
            {
                uint16_t color = src[x];
                bool winEffect = true;
                if (!getAlphaBit(color))
                    continue;
                if (windowsEnabled)
                {
                    winEffect = (line.winMask[x] & WINMASK_CLR) != 0;
                    if (!(line.winMask[x] & (1 << bg)))
                        continue;
                }
                if (blendMode != 0 && (REG_BLDCNT & (1 << bg)) && winEffect)
                {
                    uint16_t b;
                    if (blendMode == 1)
                    {
                        if (SelectTargetB(&line, &b, prnum, prsub + 1, x, (REG_BLDCNT & BLDCNT_TGT2_OBJ) != 0))
                            color = AlphaBlendColor(color, b);
                    }
                    else if (blendMode == 2)
                        color = BrightnessUp(color);
                    else
                        color = BrightnessDown(color);
                }
                pixels[x] = color;
            }
        }
        uint16_t *src = spriteSrc[prnum];
        for (int x = 0; x < sW; x++)
        {
            if (getAlphaBit(src[x]))
            {
                if (windowsEnabled && !(line.winMask[x] & WINMASK_OBJ))
                    continue;
                pixels[x] = src[x];
            }
        }
    }
#undef line
}

static uint16_t BackdropColor(void)
{
    unsigned int blendMode = (REG_BLDCNT >> 6) & 3;
    uint16_t c = *(uint16_t *)PLTT;
    if (REG_BLDCNT & BLDCNT_TGT1_BD)
    {
        if (blendMode == 2)
            c = BrightnessUp(c);
        else if (blendMode == 3)
            c = BrightnessDown(c);
    }
    return c;
}

static void DrawFieldFrame(uint16_t *out)
{
    int vy;
    for (vy = 0; vy < sH; vy++)
    {
        int gl = vy - sOY;
        bool gbaLine = gl >= 0 && gl < DISPLAY_HEIGHT;
        uint16_t *pixels = &out[vy * sW];

        if (gbaLine)
        {
            REG_VCOUNT = gl;
            if (((REG_DISPSTAT >> 8) & 0xFF) == REG_VCOUNT)
            {
                REG_DISPSTAT |= INTR_FLAG_VCOUNT;
                if (REG_DISPSTAT & DISPSTAT_VCOUNT_INTR)
                    PcGuard_Call0(gIntrTable[0]);
            }
        }

        uint16_t backdrop = BackdropColor();
        for (int x = 0; x < sW; x++)
            pixels[x] = backdrop;
        if (sSmoothOverlay != NULL)
        {
            sOverlayLine = &sSmoothOverlay[vy * sW];
            memset(sOverlayLine, 0, sW * sizeof(uint16_t));
        }
        if (sUiOverlay != NULL)
        {
            sUiLine = &sUiOverlay[vy * sW];
            memset(sUiLine, 0, sW * sizeof(uint16_t));
        }
        DrawWideScanline(pixels, vy);
        sOverlayLine = NULL;
        sUiLine = NULL;

        if (gbaLine)
        {
            REG_DISPSTAT |= INTR_FLAG_HBLANK;
            RunDMAs(DMA_HBLANK);
            if (REG_DISPSTAT & DISPSTAT_HBLANK_INTR)
                PcGuard_Call0(gIntrTable[3]);
            REG_DISPSTAT &= ~INTR_FLAG_HBLANK;
            REG_DISPSTAT &= ~INTR_FLAG_VCOUNT;
        }
    }
}

// A game frame that is not shown (fast forward): no drawing, but the per-line work the game
// hangs on a frame still runs (line-count interrupt = sound, HBlank copies and HBlank interrupt).
void WideView_SkipFrame(void)
{
    for (int gl = 0; gl < DISPLAY_HEIGHT; gl++)
    {
        REG_VCOUNT = gl;
        if (((REG_DISPSTAT >> 8) & 0xFF) == REG_VCOUNT)
        {
            REG_DISPSTAT |= INTR_FLAG_VCOUNT;
            if (REG_DISPSTAT & DISPSTAT_VCOUNT_INTR)
                PcGuard_Call0(gIntrTable[0]);
        }
        REG_DISPSTAT |= INTR_FLAG_HBLANK;
        RunDMAs(DMA_HBLANK);
        if (REG_DISPSTAT & DISPSTAT_HBLANK_INTR)
            PcGuard_Call0(gIntrTable[3]);
        REG_DISPSTAT &= ~INTR_FLAG_HBLANK;
        REG_DISPSTAT &= ~INTR_FLAG_VCOUNT;
    }
}

// A battle transition running over the field (the 3D view draws the field under it).
bool32 WideView_BattleTransitionOnField(void)
{
    // BattleTransition_StartOnField runs the field on CB2_OverworldBasic while it plays.
    return (gMain.callback2 == CB2_Overworld || gMain.callback2 == CB2_OverworldBasic)
        && gMapHeader.mapLayout != NULL && IsOverworldBgSetup();
}

void WideView_DrawFrame(uint16_t *out, struct WideViewFrame *frame)
{
    static uint16_t gba[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    enum WideViewMode mode = GetMode();
    int w, h;

    GetViewSize(&w, &h);
    UpdatePads(w, h);
    sWideFrameNo++;
    frame->mode = mode;
    frame->margin = 0;
    frame->camX = frame->camY = 0;
    frame->gbaX = frame->gbaY = 0;

    if (mode == WIDE_VIEW_GBA)
    {
        frame->width = DISPLAY_WIDTH;
        frame->height = DISPLAY_HEIGHT;
        memset(out, 0, DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t));
        DrawFrame(out);
        return;
    }

    sW = w;
    sH = h;
    sOX = (w - DISPLAY_WIDTH) / 2;
    sOY = (h - DISPLAY_HEIGHT) / 2;
    frame->width = w;
    frame->height = h;

    if (mode == WIDE_VIEW_CENTER)
    {
        memset(gba, 0, sizeof(gba));
        DrawFrame(gba);
        memset(out, 0, w * h * sizeof(uint16_t));
        for (int y = 0; y < DISPLAY_HEIGHT; y++)
            memcpy(&out[(y + sOY) * w + sOX], &gba[y * DISPLAY_WIDTH], DISPLAY_WIDTH * sizeof(uint16_t));
        return;
    }

    {
        // Test only: WJPC_FORCE_WEATHER=<id> switches the weather once the field is up.
        static int forced;
        const char *fw = getenv("WJPC_FORCE_WEATHER");
        if (fw && !forced++)
            SetCurrentAndNextWeatherNoDelay((u8)atoi(fw));
    }
    sPosX = gSaveBlock1Ptr->pos.x;
    sPosY = gSaveBlock1Ptr->pos.y;
    WJPC_GetFieldCameraRing(&sRingX, &sRingY);
    {
        // World pixel at the top-left of the GBA screen (same maths as RenderMapBgLine, BG1).
        s32 dx = (s32)((REG_BG1HOFS - sRingX * 8) & 0xFF), dy = (s32)((REG_BG1VOFS - sRingY * 8) & 0xFF);
        if (dx >= 128) dx -= 256;
        if (dy >= 128) dy -= 256;
        frame->camX = sPosX * 16 + dx;
        frame->camY = sPosY * 16 + dy;
    }
    {
        static int lastCamX, lastCamY;
        sCameraMoved = frame->camX != lastCamX || frame->camY != lastCamY;
        lastCamX = frame->camX;
        lastCamY = frame->camY;
    }
    if (sSmoothOverlay != NULL)
    {
        // Draw a bigger canvas; the visible picture is the middle w x h.
        frame->margin = sSmoothMargin;
        sW = w + 2 * sSmoothMargin;
        sH = h + 2 * sSmoothMargin;
        sOX += sSmoothMargin;
        sOY += sSmoothMargin;
    }
    frame->gbaX = sOX;
    frame->gbaY = sOY;
    BuildWideSprites();
    DrawFieldFrame(out);
}

#endif // PORTABLE && RENDERER_EASY_DRAW
