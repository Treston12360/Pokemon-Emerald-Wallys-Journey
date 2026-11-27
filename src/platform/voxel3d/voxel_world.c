/*
 * Logical map view for the voxel overworld.
 *
 * Ported from the MIT-licensed src/platform/voxel/voxel_world.c of
 * pokeemerald-multiplatform (see NOTICE.md). The classification rules, the
 * instance layout and the "current map reads gBackupMapLayout, connections
 * read the static layout" split are that file's; what is new here is that
 * every payload goes through the asset layer instead of being dereferenced
 * directly, because the linker only kept stubs where the graphics used to be.
 */

#include <math.h>
#include "port_log.h"
void DecompressDataWithHeaderWram(const unsigned int *src, void *dest);
unsigned int GetDecompressedDataSize(const unsigned int *ptr);
#include "global.h"
#include "main.h"
#include "palette.h"
#include "wally_journey.h"
#include "overworld.h"
#include "event_data.h"
#include "time_events.h"
#include "constants/layouts.h"
#include "fieldmap.h"
void DecompressDataWithHeaderWram(const unsigned int *src, void *dest);
unsigned int GetDecompressedDataSize(const unsigned int *ptr);
#include "metatile_behavior.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_camera.h"
#include "constants/map_types.h"
#include "constants/maps.h"
#include "constants/metatile_behaviors.h"
#include "port_platform.h"

#include "voxel_world.h"
#include "voxel_regions.h"
#include "voxel_tree.h"
#include "voxel_atlas.h"
#include "voxel_relief.h"
#ifdef PLATFORM_3DS
#include "3ds_video.h"
#include "voxel_building.h"
#include "field_weather.h"
#include "palette.h"
#include "constants/field_weather.h"
#include "constants/weather.h"
#endif
#ifdef PORTABLE // POL-23: the game's weather for the 3D weather
#include "field_weather.h"
#include "constants/field_weather.h"
#include "constants/weather.h"
#endif

extern const struct Tileset gTileset_General;
extern const struct Tileset gTileset_Fortree;
extern const struct Tileset gTileset_GenericBuilding;

#include <string.h>
#include "voxel_env.h" // cached test switches (last include)

#include "voxel_thread.h"

/*
 * The world as placed (the maps, the walk's places, the belt's tile sets, Kanto's tile sets) - one struct, reached
 * through a per-thread pointer: the main world everywhere but on the map-building worker, which builds from a
 * snapshot of it (VoxelWorld_SnapshotTake / VoxelWorld_UseSnapshot) so the main thread can place maps meanwhile.
 * Brennan 2026-11-21 02:12, audit step 4.
 */
#define VOXEL_PLACED_MAX 32
#define BELT_PAIRS 16  /* POL-46: was 4 (BELT_LAYERS) */
struct VoxelWorldState
{
    VoxelMapInstance instances[MAX_VOXEL_MAP_INSTANCES];
    unsigned instanceCount;
    VoxelMapInstance placed[VOXEL_PLACED_MAX];
    unsigned placedCount;
    unsigned cellGen;
    const void *frlgTilesets[192];
    unsigned frlgTilesetCount;
    const void *beltPairs[BELT_PAIRS][2];
    unsigned beltPairCount;
    /* a snapshot's copy of the live map grid (gBackupMapLayout) for instance 0; NULL: the live one */
    uint16_t *liveMap;
    int liveW, liveH;
    unsigned liveCap;
};
static struct VoxelWorldState sMainWorld = { .cellGen = 1 };
static VOXEL_TLS struct VoxelWorldState *sWorldP = &sMainWorld;
#define sInstances (sWorldP->instances)
#define sInstanceCount (sWorldP->instanceCount)
#define sPlaced (sWorldP->placed)
#define sPlacedCount (sWorldP->placedCount)
#define sCellGen (sWorldP->cellGen)
#define sFrlgTilesets (sWorldP->frlgTilesets)
#define sFrlgTilesetCount (sWorldP->frlgTilesetCount)
#define sBeltPairs (sWorldP->beltPairs)
#define sBeltPairCount (sWorldP->beltPairCount)

VoxelWorldState *VoxelWorld_SnapshotNew(void)
{
    return calloc(1, sizeof(VoxelWorldState));
}

void VoxelWorld_SnapshotTake(VoxelWorldState *snap)
{
    uint16_t *keep = snap->liveMap;
    unsigned cap = snap->liveCap, n = (unsigned)(gBackupMapLayout.width * gBackupMapLayout.height);

    *snap = sMainWorld;
    snap->liveMap = keep;
    snap->liveCap = cap;
    if (gBackupMapLayout.map == NULL || n == 0)
    {
        snap->liveW = snap->liveH = 0;
        return;
    }
    if (snap->liveCap < n)
    {
        uint16_t *grown = realloc(snap->liveMap, n * sizeof(uint16_t));
        if (grown == NULL)
        {
            snap->liveW = snap->liveH = 0;
            return;
        }
        snap->liveMap = grown;
        snap->liveCap = n;
    }
    memcpy(snap->liveMap, gBackupMapLayout.map, n * sizeof(uint16_t));
    snap->liveW = gBackupMapLayout.width;
    snap->liveH = gBackupMapLayout.height;
}

void VoxelWorld_UseSnapshot(VoxelWorldState *snap)
{
    sWorldP = snap != NULL ? snap : &sMainWorld;
}

/*
 * Memo for the current batch. Small and linear on purpose: there are at most a
 * handful of distinct tilesets and layouts on screen, so a scan beats anything
 * cleverer, and every entry is dropped at VoxelWorld_BeginBatch.
 */
/* At least one per instance: past a full memo every lookup on the remaining
 * instances falls back to the asset binary search, per tile. */
#define VOXEL_MEMO_SLOTS (MAX_VOXEL_MAP_INSTANCES + 4)

struct VoxelMemo
{
    struct { const void *key, *value; } entry[VOXEL_MEMO_SLOTS];
    unsigned count;
};

static VOXEL_TLS struct VoxelMemo sAttributeMemo, sMapMemo, sBorderMemo; /* (per thread, audit step 4) */

void VoxelWorld_BeginBatch(void)
{
    sAttributeMemo.count = 0;
    sMapMemo.count = 0;
    sBorderMemo.count = 0;
}

/* Returns true and fills *out when the key is already known. */
static bool MemoGet(const struct VoxelMemo *memo, const void *key, const void **out)
{
    for (unsigned i = 0; i < memo->count; ++i)
        if (memo->entry[i].key == key)
        {
            *out = memo->entry[i].value;
            return true;
        }
    return false;
}

/* A full memo simply stops caching; correctness never depends on it. */
static void MemoPut(struct VoxelMemo *memo, const void *key, const void *value)
{
    if (memo->count >= VOXEL_MEMO_SLOTS)
        return;
    memo->entry[memo->count].key = key;
    memo->entry[memo->count].value = value;
    ++memo->count;
}

/* ── Payload resolvers ──────────────────────────────────────────────────── */

const uint16_t *Voxel_ResolveMap(const void *layoutPtr)
{
    const struct MapLayout *layout = layoutPtr;
    const void *resolved;

    if (layout == NULL || layout->map == NULL)
        return NULL;
    if (MemoGet(&sMapMemo, layout->map, &resolved))
        return resolved;
    resolved = Port_ResolveAssetPointer(layout->map);
    MemoPut(&sMapMemo, layout->map, resolved);
    return resolved;
}

const uint16_t *Voxel_ResolveMetatiles(const void *tilesetPtr)
{
    const struct Tileset *tileset = tilesetPtr;

    if (tileset == NULL || tileset->metatiles == NULL)
        return NULL;
    return Port_ResolveAssetPointer(tileset->metatiles);
}

/*
 * Memoised: classification asks for this once per tile, and resolving it is a
 * binary search over every asset in the build plus a cache lookup. Without the
 * memo a mesh rebuild spends more time finding the attribute table than
 * reading it.
 */
const uint16_t *Voxel_ResolveAttributes(const void *tilesetPtr)
{
    const struct Tileset *tileset = tilesetPtr;
    const void *resolved;

    if (tileset == NULL || tileset->metatileAttributes == NULL)
        return NULL;
    if (MemoGet(&sAttributeMemo, tileset->metatileAttributes, &resolved))
        return resolved;
    resolved = Port_ResolveAssetPointer(tileset->metatileAttributes);
    MemoPut(&sAttributeMemo, tileset->metatileAttributes, resolved);
    return resolved;
}

/* Wally's Journey (DLX-17): the tilesets' colours as the game shows them this season (autumn leaves,
 * winter, spring blossom), not as stored; kept for the last few tilesets. */
const uint16_t *Voxel_ResolvePalettes(const void *tilesetPtr)
{
    static VOXEL_TLS struct { const void *tileset; u32 key; u16 colors[16 * 16]; } sSeasonal[12];
    static VOXEL_TLS unsigned sNext;
    const struct Tileset *tileset = tilesetPtr;
    const u16 *raw;
    u32 key;

    if (tileset == NULL || tileset->palettes == NULL)
        return NULL;
    raw = (const u16 *)Port_ResolveAssetPointer(tileset->palettes);
    key = WallyJourney_FoliageKey();
    for (unsigned i = 0; i < ARRAY_COUNT(sSeasonal); i++)
        if (sSeasonal[i].tileset == tilesetPtr && sSeasonal[i].key == key)
            return sSeasonal[i].colors;
    {
        unsigned i = sNext++ % ARRAY_COUNT(sSeasonal);
        WallyJourney_SeasonalTilesetPalettes(raw, sSeasonal[i].colors, !tileset->isSecondary);
        sSeasonal[i].tileset = tilesetPtr;
        sSeasonal[i].key = key;
        return sSeasonal[i].colors;
    }
}

uint32_t VoxelWorld_SeasonKey(void)
{
    return WallyJourney_FoliageKey();
}

/* DLX-17: recolour baked 3D texels (trees, building models; RGBA5551) for the season. A lookup over all
 * 32768 colours: the General tileset's own colours become its seasonal ones exactly (the hand-made
 * palettes), anything else goes through the season formula. Rebuilt when the season key changes. */
void VoxelWorld_SeasonTexels(uint16_t *texels, size_t count)
{
    static VOXEL_TLS u16 *sLut;
    static VOXEL_TLS u32 sLutKey;
    u32 key = WallyJourney_FoliageKey();

    if ((key & 0xF) == 1 && !(key & 0x100)) /* FOLIAGE_NONE and no hand-made palettes: nothing to do */
        return;
    if (sLut == NULL)
        sLut = malloc(32768 * sizeof(u16));
    if (sLut == NULL)
        return;
    if (sLutKey != key)
    {
        const u16 *raw = (const u16 *)Port_ResolveAssetPointer(gTileset_General.palettes);
        const u16 *seasonal = Voxel_ResolvePalettes(&gTileset_General);
        for (u32 c = 0; c < 32768; c++)
            sLut[c] = WallyJourney_SeasonalColorNow(c);
        if (raw != NULL && seasonal != NULL)
            for (u32 i = 0; i < 16 * NUM_PALS_TOTAL; i++)
                if (i % 16 != 0)
                    sLut[raw[i] & 0x7FFF] = seasonal[i] & 0x7FFF;
        sLutKey = key;
    }
    for (size_t i = 0; i < count; i++)
    {
        u16 t = texels[i];
        u16 c = ((t >> 11) & 31) | (((t >> 6) & 31) << 5) | (((t >> 1) & 31) << 10);
        u16 n = sLut[c];
        texels[i] = (u16)(((n & 31) << 11) | (((n >> 5) & 31) << 6) | (((n >> 10) & 31) << 1) | (t & 1));
    }
}

/* Brennan 2026-11-20 20:15 (stall log: "belt" 150-1680 ms a frame biking and crossing maps): the belt's builder asks
 * the same questions of a cell (which map is nearest, whose border, which fill) dozens of times while building it -
 * NearestBeltMap alone was 39% of the belt's time (a sampling profile, WJPC_PROFILE). The answers are kept per cell
 * (CellMemo) until the maps placed or the belt's tile sets change (sCellGen). WJPC_NO_CELL_MEMO=1: asked every time. */
typedef struct { int x, y; unsigned gen; int value; const void *ptr; } CellMemoEntry;
#define CELL_MEMO_SIZE 32768u
static bool CellMemoOff(void)
{
    static int sOff = -1;
    if (sOff < 0)
        sOff = getenv("WJPC_NO_CELL_MEMO") != NULL;
    return sOff;
}
unsigned VoxelWorld_CellGen(void)
{
    return CellMemoOff() ? 0 : sCellGen;
}
static inline CellMemoEntry *CellMemoSlot(CellMemoEntry *table, int x, int y)
{
    unsigned h = ((unsigned)x * 73856093u) ^ ((unsigned)y * 19349663u);
    return &table[(h ^ (h >> 15)) & (CELL_MEMO_SIZE - 1)];
}
static inline bool CellMemoHit(const CellMemoEntry *e, int x, int y)
{
    return e->gen == sCellGen && e->x == x && e->y == y;
}
static void CellMemoPut(CellMemoEntry *e, int x, int y, int value, const void *ptr)
{
    e->x = x;
    e->y = y;
    e->gen = CellMemoOff() ? 0 : sCellGen; /* (never kept: never found) */
    e->value = value;
    e->ptr = ptr;
}

/* Project ARIA (K2): the tile sets of Kanto's maps, noted as maps are placed (see voxel_world.h). */

bool Voxel_IsFrlg(const void *tileset)
{
    /* (the last answer kept: asked for the same tile set over and over while the belt is built) */
    static VOXEL_TLS const void *sLast;
    static VOXEL_TLS unsigned sLastCount = ~0u;
    static VOXEL_TLS bool sLastAnswer;

    if (tileset == sLast && sLastCount == sFrlgTilesetCount && tileset != NULL)
        return sLastAnswer;
    sLast = tileset;
    sLastCount = sFrlgTilesetCount;
    sLastAnswer = false;
    for (unsigned i = 0; i < sFrlgTilesetCount; ++i)
        if (sFrlgTilesets[i] == tileset)
            return sLastAnswer = true;
    return false;
}

static void NoteFrlg(const void *tileset)
{
    if (tileset != NULL && !Voxel_IsFrlg(tileset) && sFrlgTilesetCount < ARRAY_COUNT(sFrlgTilesets))
        sFrlgTilesets[sFrlgTilesetCount++] = tileset;
}

unsigned Voxel_PrimaryMetatiles(const void *tileset)
{
    return Voxel_IsFrlg(tileset) ? NUM_METATILES_IN_PRIMARY_FRLG : NUM_METATILES_IN_PRIMARY;
}

unsigned Voxel_PrimaryTiles(const void *tileset)
{
    return Voxel_IsFrlg(tileset) ? NUM_TILES_IN_PRIMARY_FRLG : NUM_TILES_IN_PRIMARY;
}

unsigned Voxel_PrimaryPals(const void *tileset)
{
    return Voxel_IsFrlg(tileset) ? NUM_PALS_IN_PRIMARY_FRLG : NUM_PALS_IN_PRIMARY;
}

unsigned Voxel_MetatileCount(const void *tilesetPtr, unsigned limit)
{
    const struct Tileset *tileset = tilesetPtr;
    u32 bytes;

    if (tileset == NULL || tileset->metatiles == NULL)
        return 0;
    /* A linked (non-stub) payload reports no size; trust the caller's limit. */
    bytes = Port_GetAssetSizeExact(tileset->metatiles);
    if (bytes == 0)
        return limit;
    bytes /= NUM_TILES_PER_METATILE * sizeof(u16);
    return bytes < limit ? (unsigned)bytes : limit;
}

/* ── Availability ───────────────────────────────────────────────────────── */

/* Return completion separately from success, allowing bounded decoding. */
bool Voxel_LoadTilesStep(const void *tilesetPtr, uint8_t *dest, uint32_t destSize,
                         VoxelTileLoad *load, unsigned bytes)
{
    const struct Tileset *tileset = tilesetPtr;
    const uint8_t *src;
    if (load->done) return true;
    if (!tileset || !tileset->tiles || !dest) goto failed;
    src = Port_ResolveAssetPointer(tileset->tiles);
    if (!src) goto failed;
    if (!load->initialized)
    {
        if (tileset->isCompressed)
        {
            /* Wally's Journey PC: tiles use the game's own compression formats; unpack in one go. */
            unsigned int full = GetDecompressedDataSize((const unsigned int *)tileset->tiles);
            if (!full || full > destSize) goto failed;
            DecompressDataWithHeaderWram((const unsigned int *)tileset->tiles, dest);
            load->size = load->written = full;
            load->initialized = load->done = load->ok = true;
            return true;
        }
        load->packedSize = Port_GetAssetSizeExact(tileset->tiles);
        if (!load->packedSize) goto failed;
        load->size = load->packedSize;
        if (tileset->isCompressed)
        {
            if (load->packedSize < 4 || src[0] != 0x10) goto failed;
            load->size = (uint32_t)src[1] | ((uint32_t)src[2] << 8) | ((uint32_t)src[3] << 16);
            load->source = 4;
        }
        /* Wally's Journey PC: no exact asset sizes; an uncompressed tileset fills the tile area. */
        if (!tileset->isCompressed && load->size > destSize) load->size = destSize;
        if (!load->size || load->size > destSize) goto failed;
        load->initialized = true;
    }
    if (load->size > destSize) goto failed;
    if (!tileset->isCompressed)
    {
        unsigned count = load->size - load->written;
        if (count > bytes) count = bytes;
        memcpy(dest + load->written, src + load->written, count);
        load->written += count;
    }
    else while (bytes && load->written < load->size)
    {
        if (!load->remaining)
        {
            if (!load->bits)
            {
                if (load->source >= load->packedSize) goto failed;
                load->flags = src[load->source++];
                load->bits = 8;
            }
            bool match = (load->flags & 0x80) != 0;
            load->flags <<= 1;
            --load->bits;
            if (match)
            {
                if (load->packedSize - load->source < 2) goto failed;
                unsigned a = src[load->source++], b = src[load->source++];
                load->remaining = (a >> 4) + 3;
                load->distance = ((a & 15) << 8) + b + 1;
                if (load->distance > load->written) goto failed;
            }
            else
            {
                if (load->source >= load->packedSize) goto failed;
                dest[load->written++] = src[load->source++];
                --bytes;
                continue;
            }
        }
        dest[load->written] = dest[load->written - load->distance];
        ++load->written;
        --load->remaining;
        --bytes;
    }
    if (load->written == load->size) load->done = load->ok = true;
    return load->done;
failed:
    PORT_LOG("VOXEL tiles fail: comp %d src %p b0 %02x size %u\n", tileset ? tileset->isCompressed : -1, tileset ? (void*)tileset->tiles : NULL, tileset && tileset->tiles ? ((const uint8_t*)tileset->tiles)[0] : 0, load->size);
    load->done = true;
    load->ok = false;
    return true;
}

bool VoxelWorld_IsMapAvailable(void)
{
    if (gMapHeader.mapLayout == NULL)
        return false;
    if (gSaveBlock1Ptr == NULL)
        return false;
    if (gBackupMapLayout.map == NULL)
        return false;
    return gMain.callback2 == CB2_Overworld || gMain.callback2 == CB2_OverworldBasic;
}

bool VoxelBattle_GameInBattle(void)
{
    return gMain.inBattle;
}

// The 3D ground's height at a map cell (MAP_OFFSET coordinates).
static float StageLift(int x, int y)
{
    float ox = sInstanceCount > 0 ? (float)sInstances[0].originX : 0.0f;
    float oz = sInstanceCount > 0 ? (float)sInstances[0].originY : 0.0f;
    return VoxelRelief_LiftAt(x - MAP_OFFSET + ox + 0.5f, y - MAP_OFFSET + oz + 0.5f);
}

void VoxelBattle_PickStage(float *dx, float *dz, float *ground)
{
    const struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    int px = player->currentCoords.x, py = player->currentCoords.y;
    int bestX = 0, bestY = 0;
    int best = 0x7FFFFFFF;
    bool surfing = TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING | PLAYER_AVATAR_FLAG_UNDERWATER) != 0;

    for (int oy = -8; oy <= 6; oy++)
        for (int ox = -8; ox <= 8; ox++)
        {
            int tx = px + ox, ty = py + oy;
            int score = 2 * (ox < 0 ? -ox : ox) + 2 * (oy < 0 ? -oy : oy);
            int stageElev = MapGridGetElevationAt(tx, ty);
            float stageLift;
            // The stage itself: open ground (or water, for a battle started on it).
            if (MapGridGetCollisionAt(tx, ty) != 0
             || MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(tx, ty)) != surfing)
                continue;
            stageLift = StageLift(tx, ty);
            // The view: the ground ahead of the stage (north, up to 7 tiles) and back to the camera (5).
            // Blocked tiles, water (on land) and a change of height count; near ones most.
            for (int y = ty - 7; y <= ty + 5 && score < best; y++)
                for (int x = tx - 5; x <= tx + 5; x++)
                {
                    int w = (y >= ty - 2) ? 6 : (x >= tx - 3 && x <= tx + 3) ? 2 : 1;
                    u8 elev = MapGridGetElevationAt(x, y);
                    if (MapGridGetCollisionAt(x, y) != 0)
                        score += w;
                    else if (!surfing && MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x, y)))
                        score += w;
                    else if (elev != stageElev && elev != 0 && elev != 15 && stageElev != 0 && stageElev != 15)
                        score += w;
                    else if (y >= ty - 2 && fabsf(StageLift(x, y) - stageLift) > 0.4f)
                        score += w; // a drop or rise in the 3D ground (cliff edges, terraces)
                }
            if (score < best)
            {
                best = score;
                bestX = ox;
                bestY = oy;
            }
        }
    *dx = (float)bestX;
    *dz = (float)bestY;
    *ground = StageLift(px + bestX, py + bestY);
}

bool VoxelWorld_IsBattleMapAvailable(void)
{
    /* A battle leaves the map it was started from loaded, live grid and all:
     * the field picks up where it stopped when the battle is over. */
    return gMain.inBattle && gMapHeader.mapLayout != NULL && gSaveBlock1Ptr != NULL
        && gBackupMapLayout.map != NULL;
}

/* ── Instances ──────────────────────────────────────────────────────────── */

/*
 * Maps whose layout a script swaps on entering them (setmaplayoutindex in
 * their ON_TRANSITION), as the swap stands while the player is elsewhere:
 * a neighbour is drawn whole, and its header's layout showed what the game
 * hides - the Mirage Tower over the desert from Mauville, every day, years
 * after it fell - or hid what it shows: Mirage Island, out in the sea of
 * Route 130 the days it is there, rose out of it only once the player
 * crossed into the route. `swapped` says, as the script does, whether the
 * map is entered with `layoutId`; NULL: always.
 */
static bool MirageTowerHidden(void)
{
    return !FlagGet(FLAG_MIRAGE_TOWER_VISIBLE);
}

static bool MirageIslandShown(void)
{
    return IsMirageIslandPresent();
}

static const struct
{
    u8 mapGroup, mapNum;
    bool (*swapped)(void);
    u16 layoutId;
} sLayoutSwaps[] = {
    { MAP_GROUP(MAP_ROUTE111), MAP_NUM(MAP_ROUTE111), MirageTowerHidden, LAYOUT_ROUTE111_NO_MIRAGE_TOWER },
    { MAP_GROUP(MAP_ROUTE130), MAP_NUM(MAP_ROUTE130), MirageIslandShown, LAYOUT_ROUTE130_MIRAGE_ISLAND },
    { MAP_GROUP(MAP_ROUTE131), MAP_NUM(MAP_ROUTE131), NULL, LAYOUT_ROUTE131_SKY_PILLAR },
};

static u16 NeighbourLayoutId(const struct MapHeader *header, int mapGroup, int mapNum)
{
    for (unsigned i = 0; i < ARRAY_COUNT(sLayoutSwaps); ++i)
        if (sLayoutSwaps[i].mapGroup == mapGroup && sLayoutSwaps[i].mapNum == mapNum
         && (sLayoutSwaps[i].swapped == NULL || sLayoutSwaps[i].swapped())
         && Port_GetMapLayoutById(sLayoutSwaps[i].layoutId) != NULL)
            return sLayoutSwaps[i].layoutId;
    return header->mapLayoutId;
}

/* `layoutId`: the layout the map is drawn with - the current map's is the
 * save block's (a script may have swapped it: the header keeps the map's
 * first id), a neighbour's NeighbourLayoutId. The current map's layout is
 * the one the game loaded, gMapHeader's pointer (SetCurrentMapLayout moves
 * it with the save block's id), never looked up again by id. */
static void FillInstance(VoxelMapInstance *inst, const struct MapHeader *header, u16 layoutId,
                         int mapGroup, int mapNum, int originX, int originY)
{
    const struct MapLayout *layout = header == &gMapHeader || layoutId == header->mapLayoutId
                                   ? header->mapLayout : Port_GetMapLayoutById(layoutId);

    if (layout == NULL)
    {
        layout = header->mapLayout;
        layoutId = header->mapLayoutId;
    }
    inst->header = header;
    inst->layout = layout;
    inst->primaryTileset = layout->primaryTileset;
    if (layout->isFrlg)
    {
        NoteFrlg(layout->primaryTileset);
        NoteFrlg(layout->secondaryTileset);
    }
    inst->secondaryTileset = layout->secondaryTileset;
    inst->layoutId = layoutId;
    inst->headerLayoutId = header->mapLayoutId;
    inst->live = false;
    inst->mapGroup = mapGroup;
    inst->mapNum = mapNum;
    inst->originX = originX;
    inst->originY = originY;
    inst->width = layout->width;
    inst->height = layout->height;
    inst->indoor = header->mapType == MAP_TYPE_INDOOR
                || header->mapType == MAP_TYPE_SECRET_BASE;
    inst->underground = header->mapType == MAP_TYPE_UNDERGROUND;
    inst->sky = header->mapType == MAP_TYPE_TOWN || header->mapType == MAP_TYPE_CITY
             || header->mapType == MAP_TYPE_ROUTE || header->mapType == MAP_TYPE_OCEAN_ROUTE;
    /* Brennan 2026-11-24 14:28 ("Fawaway island all looks 2D", "birth island still looks 2D", "Naval rock exterior
     * looks 2D"): the islands the event tickets reach are outside, under the sky, but the game types them
     * MAP_TYPE_INDOOR - so they were laid as a room: no 3D trees, no sky, no sun. Here they are outside.
     * WJPC_ISLANDS_INDOOR=1: as rooms, as before. */
    {
        static int sOld = -1;
        if (sOld < 0)
            sOld = getenv("WJPC_ISLANDS_INDOOR") != NULL;
        if (!sOld && (mapGroup << 8 | mapNum) != 0
         && ((mapGroup << 8 | mapNum) == MAP_FARAWAY_ISLAND_ENTRANCE || (mapGroup << 8 | mapNum) == MAP_FARAWAY_ISLAND_INTERIOR
          || (mapGroup << 8 | mapNum) == MAP_BIRTH_ISLAND_EXTERIOR || (mapGroup << 8 | mapNum) == MAP_NAVEL_ROCK_EXTERIOR))
        {
            inst->indoor = false;
            inst->sky = true;
        }
    }
    /* Brennan 2026-11-24 14:28 / 14:55 ("Sootopolis gym has dark shadows walls" - circled: the ice walls' inner
     * sides): a room's models are lit by the sun's facing, so the sides of the gym's walls turned from it came out a
     * dark grey-blue, as a cave's rocks did (#29). Its walls are lit as its floor, each side keeping the shade it was
     * drawn with, as a cave's are. WJPC_SOOT_GYM_SUNLIT=1: as before. */
    {
        static int sOld = -1;
        if (sOld < 0)
            sOld = getenv("WJPC_SOOT_GYM_SUNLIT") != NULL;
        inst->flatModels = inst->underground
                        || (!sOld && ((mapGroup << 8 | mapNum) == MAP_SOOTOPOLIS_CITY_GYM_1F
                                   || (mapGroup << 8 | mapNum) == MAP_SOOTOPOLIS_CITY_GYM_B1F));
    }
    inst->lift = 0;
}

/*
 * Where the map across `conn` sits, from the map it leaves (at `from`'s
 * origin, of `from`'s size). False for a connection that is not a side.
 */
static bool ConnectionOrigin(const struct MapConnection *conn, const struct MapHeader *neighbour,
                             const VoxelMapInstance *from, int *originX, int *originY)
{
    switch (conn->direction)
    {
    case CONNECTION_NORTH:
        *originX = from->originX + conn->offset;
        *originY = from->originY - neighbour->mapLayout->height;
        return true;
    case CONNECTION_SOUTH:
        *originX = from->originX + conn->offset;
        *originY = from->originY + from->height;
        return true;
    case CONNECTION_WEST:
        *originX = from->originX - neighbour->mapLayout->width;
        *originY = from->originY + conn->offset;
        return true;
    case CONNECTION_EAST:
        *originX = from->originX + from->width;
        *originY = from->originY + conn->offset;
        return true;
    default:
        return false;
    }
}

/*
 * Every map the walk out along the connections has placed, the instances
 * among them: each is a place the walk goes on from, so that a map within
 * reach is found even where the way to it leads through one that is not.
 */
#define VOXEL_WALK_REACH (2 * VOXEL_WORLD_REACH)


/* Tiles between two rectangles, the larger of the two axes; 0 touching. */
static int RectGap(const VoxelMapInstance *a, int x, int y, int width, int height)
{
    int dx = x >= a->originX + a->width ? x - (a->originX + a->width)
           : a->originX >= x + width ? a->originX - (x + width) : 0;
    int dy = y >= a->originY + a->height ? y - (a->originY + a->height)
           : a->originY >= y + height ? a->originY - (y + height) : 0;

    return dx > dy ? dx : dy;
}

/*
 * The map across each side connection of `from`. The current map's own are
 * taken as they are; one further away only where it is not a map already
 * placed and covers none of their ground - the connections of Hoenn's maps
 * do not always agree with one another's. `keep`: an instance however far it
 * lies (a connection of the current map's own neighbours); otherwise only
 * within VOXEL_WORLD_REACH of the current map, and not placed at all past
 * VOXEL_WALK_REACH.
 */
static void AddConnections(const VoxelMapInstance *from, bool near, bool keep)
{
    const struct MapHeader *header = from->header;
    const struct MapConnection *conn;

    if (header == NULL || header->connections == NULL)
        return;
    conn = header->connections->connections;
    for (s32 i = 0; i < header->connections->count; ++i, ++conn)
    {
        const struct MapHeader *neighbour;
        int originX, originY, width, height, gap;
        bool clash = false;
        VoxelMapInstance *placed;

        if (sPlacedCount >= VOXEL_PLACED_MAX)
            return;
        if (conn->direction == CONNECTION_DIVE || conn->direction == CONNECTION_EMERGE)
            continue;
        neighbour = GetMapHeaderFromConnection(conn);
        if (neighbour == NULL || neighbour->mapLayout == NULL
         || !ConnectionOrigin(conn, neighbour, from, &originX, &originY))
            continue;
        width = neighbour->mapLayout->width;
        height = neighbour->mapLayout->height;
        gap = RectGap(&sPlaced[0], originX, originY, width, height);
        if (!near && !keep && gap > VOXEL_WALK_REACH)
            continue;
        for (unsigned k = 0; !near && k < sPlacedCount && !clash; ++k)
        {
            const VoxelMapInstance *other = &sPlaced[k];

            clash = (other->mapGroup == conn->mapGroup && other->mapNum == conn->mapNum)
                 || (originX < other->originX + other->width
                     && originX + width > other->originX
                     && originY < other->originY + other->height
                     && originY + height > other->originY);
        }
        if (clash)
            continue;
        placed = &sPlaced[sPlacedCount++];
        FillInstance(placed, neighbour, NeighbourLayoutId(neighbour, conn->mapGroup, conn->mapNum),
                     conn->mapGroup, conn->mapNum, originX, originY);
        /* as high as the seam with the map it is reached from asks */
        placed->lift = from->lift + VoxelRelief_SeamStep(from, placed);
        if ((near || keep || gap <= VOXEL_WORLD_REACH) && sInstanceCount < MAX_VOXEL_MAP_INSTANCES)
            sInstances[sInstanceCount++] = *placed;
    }
}

#ifdef PLATFORM_3DS
/* Ground variant (layout, metatile) is drawn from `inst`'s own tileset of
 * that metatile, the one the atlas composes it from (voxel_atlas.c
 * AtlasSourceOf). */
static bool VariantFits(const VoxelMapInstance *inst, unsigned layoutId, unsigned metatile)
{
    const struct MapLayout *layout = Port_GetMapLayoutById((u16)layoutId);

    if (layout == NULL)
        return false;
    return metatile < NUM_METATILES_IN_PRIMARY
         ? layout->primaryTileset == inst->primaryTileset
         : layout->secondaryTileset == inst->secondaryTileset;
}
#endif

/*
 * The current map, its connections, theirs, and any map further along that
 * lies within VOXEL_WORLD_REACH of it. The game loads only the first two,
 * which is all its 15 x 10 tiles of screen can reach; the 3D view sees
 * several times as far, and a map one crossing further away was missing from
 * it - the belt of border trees stood in its place, and on the crossing that
 * made it a neighbour it had to be built from nothing while the view showed
 * black where it lies. Placed from the start, it is built ahead of time like
 * any map in the ring, and the crossing only moves the coordinates.
 *
 * Two crossings were not enough either. The view looks back the way the
 * player came - walking south out of Route 101 into Littleroot it faces
 * north - and the map two crossings behind, still on screen, became three
 * crossings away on the step that crossed: Route 102 went out of the world
 * and the belt stood where it had been, black until built. Counted by
 * distance instead of by crossings, the set of one map holds whatever the
 * view from the map beside it showed across their seam, so a crossing drops
 * nothing in sight and builds nothing that was already there.
 *
 * The set depends on the current map alone, never on the view: a change to
 * it is a new epoch, and every chunk in view verified again.
 */
/*
 * The lift each map last stood at (voxel_relief.c VoxelRelief_SeamStep): the
 * map the player comes to keeps it, so the world is not moved under them -
 * across a seam it is the lift the map already had on screen, through a
 * warp the one it had when last seen, and the chunks built for it stand.
 * Going round one of Hoenn's loops adds what the loop does not close by; a
 * lift past LIFT_FORGET is let go at the next warp, the map back at its base.
 */
#define LIFT_MEMORY 32
#define LIFT_FORGET (8 * 16)

static struct
{
    bool used;
    s16 mapGroup, mapNum;
    s16 lift;
} sLiftMemory[LIFT_MEMORY];
static unsigned sLiftMemoryNext;

static int RememberedLift(int mapGroup, int mapNum, bool *found)
{
    for (unsigned i = 0; i < LIFT_MEMORY; ++i)
        if (sLiftMemory[i].used && sLiftMemory[i].mapGroup == mapGroup
         && sLiftMemory[i].mapNum == mapNum)
        {
            *found = true;
            return sLiftMemory[i].lift;
        }
    *found = false;
    return 0;
}

static void RememberLift(const VoxelMapInstance *inst)
{
    unsigned slot = sLiftMemoryNext;

    for (unsigned i = 0; i < LIFT_MEMORY; ++i)
        if (sLiftMemory[i].used && sLiftMemory[i].mapGroup == inst->mapGroup
         && sLiftMemory[i].mapNum == inst->mapNum)
        {
            sLiftMemory[i].lift = (s16)inst->lift;
            return;
        }
    if (inst->lift == 0)
        return;     /* forgotten is 0 */
    sLiftMemory[slot].used = true;
    sLiftMemory[slot].mapGroup = (s16)inst->mapGroup;
    sLiftMemory[slot].mapNum = (s16)inst->mapNum;
    sLiftMemory[slot].lift = (s16)inst->lift;
    sLiftMemoryNext = (slot + 1u) % LIFT_MEMORY;
}

/* The lift the current map keeps: the one it had among the maps on screen
 * (a crossing), else the one remembered (a warp). */
static int CurrentLift(int mapGroup, int mapNum)
{
    bool found;
    int lift;

    for (unsigned i = 0; i < sInstanceCount; ++i)
        if (sInstances[i].mapGroup == mapGroup && sInstances[i].mapNum == mapNum)
            return sInstances[i].lift;
    lift = RememberedLift(mapGroup, mapNum, &found);
    return lift > LIFT_FORGET || lift < -LIFT_FORGET ? 0 : lift;
}

/*
 * Where the world's origin (the current map's top-left) lies in one steady frame that does not jump when a
 * connection is crossed: the new map's origin in the old layout is added. Things fixed to the ground across
 * maps (Gamma's cloud shadows) use it. A warp starts nothing new (it is a different place anyway).
 */
static int sSteadyX, sSteadyY;

void VoxelWorld_SteadyOrigin(int *x, int *y)
{
    *x = sSteadyX;
    *y = sSteadyY;
}

void VoxelWorld_BuildInstances(void)
{
    unsigned near;
    for (unsigned i = 1; i < sPlacedCount; ++i)
        if (sPlaced[i].mapGroup == gSaveBlock1Ptr->location.mapGroup && sPlaced[i].mapNum == gSaveBlock1Ptr->location.mapNum)
        {
            sSteadyX += sPlaced[i].originX;
            sSteadyY += sPlaced[i].originY;
            break;
        }
    int lift = CurrentLift(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);

#ifdef PLATFORM_3DS
    VoxelBuildings_SetVariantFits(VariantFits);
    VoxelBuildings_SetDrawings(VoxelWorld_LayoutDrawing);
#endif
    sInstanceCount = 0;
    sPlacedCount = 0;
    sCellGen++;
    if (gMapHeader.mapLayout == NULL)
        return;

    /* Instance 0 is always the current map, at the origin. */
    FillInstance(&sPlaced[0], &gMapHeader, gSaveBlock1Ptr->mapLayoutId,
                 gSaveBlock1Ptr->location.mapGroup,
                 gSaveBlock1Ptr->location.mapNum, 0, 0);
    /* Its layout is the save block's: a script that swaps it in
     * (setmaplayoutindex - Route 130's Mirage Island, Route 131's Sky Pillar,
     * the Sky Pillar's clean floors, Route 111 without its tower)
     * sets that and gMapHeader.mapLayout, never gMapHeader.mapLayoutId
     * (overworld.c, SetCurrentMapLayout). Keyed by the header's, the
     * relief, regions and buildings were the default layout's. */
    sPlaced[0].layoutId = gSaveBlock1Ptr->mapLayoutId;
    sPlaced[0].lift = sPlaced[0].indoor ? 0 : lift;
    sInstances[0] = sPlaced[0];
    sInstances[0].live = true;
    sPlacedCount = sInstanceCount = 1;
    AddConnections(&sPlaced[0], true, true);
    near = sPlacedCount;
    /* Nearest first: the walk goes outwards a crossing at a time. */
    for (unsigned i = 1; i < sPlacedCount; ++i)
        AddConnections(&sPlaced[i], false, i < near);
    for (unsigned i = 0; i < sInstanceCount; ++i)
        RememberLift(&sInstances[i]);
    sCellGen++;
}

unsigned VoxelWorld_InstanceCount(void)
{
    return sInstanceCount;
}

const VoxelMapInstance *VoxelWorld_Instance(unsigned index)
{
    return index < sInstanceCount ? &sInstances[index] : NULL;
}

const VoxelMapInstance *VoxelWorld_GetInstanceAt(int worldX, int worldY)
{
    /* The current map covers most lookups, so it is tested on its own. */
    if (sInstanceCount > 0
     && worldX >= 0 && worldX < sInstances[0].width
     && worldY >= 0 && worldY < sInstances[0].height)
        return &sInstances[0];

    for (unsigned i = 1; i < sInstanceCount; ++i)
    {
        const VoxelMapInstance *inst = &sInstances[i];
        int localX = worldX - inst->originX;
        int localY = worldY - inst->originY;

        if (localX >= 0 && localX < inst->width && localY >= 0 && localY < inst->height)
            return inst;
    }
    return NULL;
}

/* ── Block lookups ──────────────────────────────────────────────────────── */

static u16 GetRawBlock(int worldX, int worldY, const VoxelMapInstance **outInst)
{
    const VoxelMapInstance *inst = VoxelWorld_GetInstanceAt(worldX, worldY);
    int localX, localY;

    if (outInst != NULL)
        *outInst = inst;
    if (inst == NULL)
        return 0;

    localX = worldX - inst->originX;
    localY = worldY - inst->originY;

    if (inst == &sInstances[0])
    {
        /* The live layout, so scripted metatile changes are visible. */
        int bx = localX + MAP_OFFSET;
        int by = localY + MAP_OFFSET;

        if (sWorldP->liveMap != NULL)
        {
            /* (a snapshot: its own copy) */
            if (bx >= 0 && bx < sWorldP->liveW && by >= 0 && by < sWorldP->liveH)
                return sWorldP->liveMap[bx + sWorldP->liveW * by];
            return 0;
        }
        if (bx >= 0 && bx < gBackupMapLayout.width
         && by >= 0 && by < gBackupMapLayout.height)
            return gBackupMapLayout.map[bx + gBackupMapLayout.width * by];
        return 0;
    }
    else
    {
        const uint16_t *map = Voxel_ResolveMap(inst->layout);

        if (map == NULL)
            return 0;
        return map[localY * inst->width + localX];
    }
}

int VoxelWorld_GetMetatileId(int worldX, int worldY)
{
    return GetRawBlock(worldX, worldY, NULL) & MAPGRID_METATILE_ID_MASK;
}

unsigned VoxelWorld_GetCollision(int worldX, int worldY)
{
    return (GetRawBlock(worldX, worldY, NULL) & MAPGRID_COLLISION_MASK) >> MAPGRID_COLLISION_SHIFT;
}

static u16 GetMetatileAttribute(const VoxelMapInstance *inst, int metatileId)
{
    const uint16_t *attributes;
    unsigned index, split;

    if (inst == NULL)
        return 0;
    split = Voxel_PrimaryMetatiles(inst->primaryTileset);
    if (metatileId < (int)split)
    {
        attributes = Voxel_ResolveAttributes(inst->primaryTileset);
        index = (unsigned)metatileId;
    }
    else
    {
        attributes = Voxel_ResolveAttributes(inst->secondaryTileset);
        index = (unsigned)metatileId - split;
    }
    if (attributes == NULL)
        return 0;
    if (Voxel_IsFrlg(inst->primaryTileset))
    {
        /* Kanto's are 32-bit with the fields elsewhere; handed on in Emerald's 16-bit form (behaviour in
         * the low byte, layer type in bits 12-15), which is what the callers unpack. */
        u32 attr = ((const u32 *)attributes)[index];

        return (u16)(ExtractMetatileAttribute(attr, METATILE_ATTRIBUTE_BEHAVIOR, TRUE) & 0xFF)
             | (u16)(ExtractMetatileAttribute(attr, METATILE_ATTRIBUTE_LAYER_TYPE, TRUE) << 12);
    }
    return attributes[index];
}

unsigned VoxelWorld_GetMetatileBehavior(int worldX, int worldY)
{
    const VoxelMapInstance *inst = NULL;
    u16 block = GetRawBlock(worldX, worldY, &inst);

    return UNPACK_BEHAVIOR(GetMetatileAttribute(inst, block & MAPGRID_METATILE_ID_MASK));
}

bool VoxelWorld_IsVisibleReflectiveSurface(int worldX, int worldY)
{
    const VoxelMapInstance *inst = NULL;
    u16 block = GetRawBlock(worldX, worldY, &inst);
    unsigned metatileId, behavior;

    if (inst == NULL)
        return false;
    metatileId = block & MAPGRID_METATILE_ID_MASK;
    behavior = UNPACK_BEHAVIOR(GetMetatileAttribute(inst, metatileId));
    if (!MetatileBehavior_IsReflective(behavior) && !MetatileBehavior_IsIce(behavior))
        return false;
    /* A bridge's deck over a pond (Route 104's pier, the only one there is):
     * the GBA shows a walker's reflection under the bridge, in the water the
     * planks hide; here the deck is no water surface, and a reflection laid
     * on it was drawn over the planks. The water beside it still reflects. */
    if (behavior == MB_REFLECTION_UNDER_BRIDGE)
        return false;

    /* Fortree's 0x288-0x29A puddle family contains grass artwork despite its
     * MB_PUDDLE behavior. The original 2D ground effect treats it as nearby
     * water; a 3D surface reflection would instead be visible on the grass.
     * Blue puddle metatiles 0x274-0x27D retain their reflections. */
    if (behavior == MB_PUDDLE && inst->secondaryTileset == &gTileset_Fortree
     && metatileId >= 0x288 && metatileId <= 0x29A)
        return false;
    return true;
}

/*
 * A sprite of priority 3 lies over BG3 alone. A metatile's lower layer is on
 * BG3 unless it is a NORMAL one (BG2, over the sprite), and its upper layer
 * on BG2 or BG1, both over it: the sprite shows where the cell is COVERED or
 * SPLIT and its upper layer draws nothing (index 0). Read from the tiles the
 * field has loaded (the current map's tilesets), a metatile at a time,
 * cached: the cards are laid in the VBlank handler.
 */
#define UNDER_GROUND_CACHE 64u

bool VoxelWorld_UnderGroundMask(int worldX, int worldY, uint16_t rows[16])
{
#ifdef PLATFORM_3DS
    static struct
    {
        const void *primary, *secondary;
        uint16_t metatileId;
        uint16_t rows[16];
    } sCache[UNDER_GROUND_CACHE];
    const VoxelMapInstance *inst = NULL;
    u16 block = GetRawBlock(worldX, worldY, &inst);
    const uint8_t *tiles = CtrVideo_GetBgVram();
    const uint16_t *entries;
    unsigned metatileId, slot;

    if (inst == NULL || tiles == NULL || sInstanceCount == 0
     || inst->primaryTileset != sInstances[0].primaryTileset
     || inst->secondaryTileset != sInstances[0].secondaryTileset)
        return false;
    metatileId = block & MAPGRID_METATILE_ID_MASK;
    slot = metatileId % UNDER_GROUND_CACHE;
    if (sCache[slot].primary == inst->primaryTileset && sCache[slot].secondary == inst->secondaryTileset
     && sCache[slot].metatileId == metatileId)
    {
        memcpy(rows, sCache[slot].rows, sizeof(sCache[slot].rows));
        return true;
    }
    entries = metatileId < (int)Voxel_PrimaryMetatiles(inst->primaryTileset)
            ? Voxel_ResolveMetatiles(inst->primaryTileset)
            : Voxel_ResolveMetatiles(inst->secondaryTileset);
    if (entries == NULL)
        return false;
    entries += (metatileId < (int)Voxel_PrimaryMetatiles(inst->primaryTileset)
                ? metatileId : metatileId - (int)Voxel_PrimaryMetatiles(inst->primaryTileset)) * NUM_TILES_PER_METATILE;
    memset(rows, 0, 16 * sizeof(rows[0]));
    if (UNPACK_LAYER_TYPE(GetMetatileAttribute(inst, metatileId)) != METATILE_LAYER_TYPE_NORMAL)
        for (unsigned q = 0; q < 4; ++q)
        {
            uint16_t entry = entries[4 + q];
            const uint8_t *tile = tiles + (entry & 0x3FF) * 32u;

            for (unsigned y = 0; y < 8; ++y)
                for (unsigned x = 0; x < 8; ++x)
                {
                    unsigned sx = (entry & 0x400) ? 7 - x : x, sy = (entry & 0x800) ? 7 - y : y;
                    uint8_t packed = tile[sy * 4 + sx / 2];

                    if (((sx & 1) ? packed >> 4 : packed & 0xF) == 0)
                        rows[(q >> 1) * 8 + y] |= (uint16_t)(1u << ((q & 1) * 8 + x));
                }
        }
    sCache[slot].primary = inst->primaryTileset;
    sCache[slot].secondary = inst->secondaryTileset;
    sCache[slot].metatileId = (uint16_t)metatileId;
    memcpy(sCache[slot].rows, rows, sizeof(sCache[slot].rows));
    return true;
#else
    (void)worldX;
    (void)worldY;
    (void)rows;
    return false;
#endif
}

bool VoxelWorld_UsesTreeSprites(const VoxelMapInstance *inst)
{
    return inst != NULL && !inst->indoor && inst->primaryTileset == &gTileset_General;
}

/* ── Classification ─────────────────────────────────────────────────────── */

/*
 * What a cell is drawn as, when no model or relief covers it.
 *
 * Nothing here raises terrain. A cell stands up only when something was built
 * for it from its own drawing - a building (voxel_building.c), a tree
 * (voxel_tree.c), a sign (voxel_sign.c) or the relief of a map read off its
 * art (voxel_relief.c) - and those passes answer for their cells before this
 * is asked. What is left is ground, whatever its collision bit says: guessing
 * walls, roofs and cliffs from it is what put blocks where nothing stands.
 *
 * The exceptions are small things that are not terrain: water, which lies
 * flush with the ground, and indoor furniture.
 */
/*
 * The few behaviours that say what a cell looks like, asked through the
 * game's own predicates: water lies flush with the ground, counters and machines
 * stand up. Everything else says nothing (VOXEL_SHAPE_COUNT).
 */
static VoxelVisualShape BehaviorShape(u8 behavior)
{
    if (MetatileBehavior_IsSurfableWaterOrUnderwater(behavior)
     || MetatileBehavior_IsPuddle(behavior)
     || MetatileBehavior_IsShallowFlowingWater(behavior)
     || behavior == MB_HOT_SPRINGS
     || behavior == MB_REFLECTION_UNDER_BRIDGE)
        return VOXEL_SHAPE_WATER;
    if (MetatileBehavior_IsCounter(behavior))
        return VOXEL_SHAPE_COUNTER;
    if (MetatileBehavior_IsPC(behavior)
     || MetatileBehavior_IsSecretBasePC(behavior)
     || MetatileBehavior_IsPlayerRoomPCOn(behavior)
     || behavior == MB_SECRET_BASE_REGISTER_PC
     || behavior == MB_TELEVISION
     || behavior == MB_CABLE_BOX_RESULTS_1)
        return VOXEL_SHAPE_FURNITURE;
    return VOXEL_SHAPE_COUNT;
}

VoxelVisualShape VoxelWorld_ClassifyTile(int worldX, int worldY)
{
    const VoxelMapInstance *inst = NULL;
    u16 block = GetRawBlock(worldX, worldY, &inst);   /* one instance search for all three reads */
    int metatileId;
    VoxelVisualShape announced;

    if (inst == NULL)
        return VOXEL_SHAPE_VOID;

    metatileId = block & MAPGRID_METATILE_ID_MASK;
    if (VoxelWorld_UsesTreeSprites(inst)
     && (VoxelTree_Part(inst->secondaryTileset, metatileId) >= 0
         || VoxelTree_GroundMetatile(inst->secondaryTileset, metatileId) != metatileId))
    {
        int ground = VoxelTree_GroundMetatile(inst->secondaryTileset, metatileId);

        /* a crown's row over water, or a trunk standing in it, is that
         * water where the ground pass lays it */
        if ((VoxelTree_Part(inst->secondaryTileset, metatileId) < 0
             || !VoxelTree_LaysGround(inst->secondaryTileset, metatileId))
         && BehaviorShape(UNPACK_BEHAVIOR(GetMetatileAttribute(inst, ground))) == VOXEL_SHAPE_WATER)
            return VOXEL_SHAPE_WATER;
        return VOXEL_SHAPE_FLAT;
    }
    if (VoxelRegions_RoleAt((unsigned)inst->layoutId,
                            worldX - inst->originX, worldY - inst->originY)
        == VOXEL_ROLE_SIGNPOST)
        return VOXEL_SHAPE_SIGN;

    /* The cells whose behaviour says what they are: water and furniture. */
    announced = BehaviorShape(UNPACK_BEHAVIOR(GetMetatileAttribute(inst, metatileId)));
    if (announced != VOXEL_SHAPE_COUNT)
        return announced;

    if (inst->indoor)
    {
        /* The black filler outside the rooms: read off the drawing, since
         * its id is the tileset's (622 is the Pokemon Center's floor). */
        if (VoxelAtlas_IsVoid(inst, metatileId))
            return VOXEL_SHAPE_VOID;

        /*
         * Beds, tables and rugs announce nothing at all: Emerald gives them no
         * behaviour of their own, and they are not guessed from their ids.
         * The reference's list for the general indoor tileset names other
         * things in it: a rug's border stood up as a row of black television
         * sets and a bed (House1), the back wall's top row as mats, and a
         * table as two of its four cells. A room's furniture stands up when
         * the room is modelled from its drawing (voxel_building.h).
         */
    }

    return VOXEL_SHAPE_FLAT;
}

/* ── Queries used by the camera and the renderer ────────────────────────── */

void VoxelWorld_GetMapDimensions(int *width, int *height)
{
    if (gMapHeader.mapLayout != NULL)
    {
        if (width != NULL) *width = gMapHeader.mapLayout->width;
        if (height != NULL) *height = gMapHeader.mapLayout->height;
    }
    else
    {
        if (width != NULL) *width = 0;
        if (height != NULL) *height = 0;
    }
}

void VoxelWorld_GetPlayerWorldCoords(float *worldX, float *worldZ)
{
    const struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    float x = (float)(player->currentCoords.x - MAP_OFFSET);
    float z = (float)(player->currentCoords.y - MAP_OFFSET);

    if (sInstanceCount > 0)
    {
        x += (float)sInstances[0].originX;
        z += (float)sInstances[0].originY;
    }
    if (worldX != NULL) *worldX = x;
    if (worldZ != NULL) *worldZ = z;
}

VoxelWeatherClass VoxelWorld_Weather(void)
{
#if defined(PLATFORM_3DS) || defined(PORTABLE) // PC too: POL-23 (3D weather)
    switch (GetCurrentWeather())
    {
    case WEATHER_SUNNY:
    case WEATHER_DROUGHT:
        return VOXEL_WEATHER_SUN;
    case WEATHER_RAIN:
    case WEATHER_RAIN_THUNDERSTORM:
    case WEATHER_DOWNPOUR:
        return VOXEL_WEATHER_RAIN;
    case WEATHER_FOG_HORIZONTAL:
    case WEATHER_FOG_DIAGONAL:
    case WEATHER_UNDERWATER:
    case WEATHER_UNDERWATER_BUBBLES:
        return VOXEL_WEATHER_FOG;
    case WEATHER_SNOW:
        return VOXEL_WEATHER_PARTICLES;
    case WEATHER_VOLCANIC_ASH:
        return VOXEL_WEATHER_ASH;
    case WEATHER_SANDSTORM:
        return VOXEL_WEATHER_SAND;
    case WEATHER_SHADE:
    case WEATHER_SUNNY_CLOUDS:
        return VOXEL_WEATHER_SHADE;
    }
#endif
    return VOXEL_WEATHER_CLEAR;
}

float VoxelWorld_WeatherAmount(void)
{
#if defined(PLATFORM_3DS) || defined(PORTABLE) // PC too: POL-23 (3D weather)
    float amount;

    switch (GetCurrentWeather())
    {
    case WEATHER_RAIN:
    case WEATHER_RAIN_THUNDERSTORM:
    case WEATHER_DOWNPOUR:
        /* The drops turned on, one by one, up to 10, 16 or 24. */
        return (float)gWeatherPtr->curRainSpriteIndex / MAX_RAIN_SPRITES;
    case WEATHER_VOLCANIC_ASH:
        if (!gWeatherPtr->ashSpritesCreated)
            return 0.0f;
        break;
    case WEATHER_SANDSTORM:
        if (!gWeatherPtr->sandstormSpritesCreated)
            return 0.0f;
        break;
    default:
        return 0.0f;
    }
    /* Both come in to EVA 16 and leave to 0. */
    amount = (float)gWeatherPtr->currBlendEVA / 16.0f;
    return amount > 1.0f ? 1.0f : amount;
#else
    return 0.0f;
#endif
}

/* The bolt's colour map is 19, the storm's own 3 (Thunderstorm_Main). */
float VoxelWorld_Lightning(void)
{
#if defined(PLATFORM_3DS) || defined(PORTABLE) // PC too: POL-23
    if (GetCurrentWeather() != WEATHER_RAIN_THUNDERSTORM || gWeatherPtr->colorMapIndex <= 3)
        return 0.0f;
    return gWeatherPtr->colorMapIndex >= 19 ? 1.0f : (float)(gWeatherPtr->colorMapIndex - 3) / 16.0f;
#else
    return 0.0f;
#endif
}

/*
 * The fog's own blend: the sprites come in at EVA 12 (4 under water) over a
 * few frames and leave the same way, so the world's fog follows them.
 */
float VoxelWorld_FogDensity(void)
{
#ifdef PLATFORM_3DS
    float density;

    if (!gWeatherPtr->fogHSpritesCreated && !gWeatherPtr->fogDSpritesCreated)
        return 0.0f;
    density = (float)gWeatherPtr->currBlendEVA / 12.0f;
    return density > 1.0f ? 1.0f : density;
#else
    return 0.0f;
#endif
}

bool VoxelWorld_Underground(void)
{
    return gMapHeader.mapType == MAP_TYPE_UNDERGROUND;
}

bool VoxelWorld_MapIsConnected(void)
{
    return gMapHeader.connections != NULL && gMapHeader.connections->count > 0;
}

unsigned VoxelWorld_ClosedEdges(void)
{
    return GetMapClosedEdges();
}

/*
 * The fade the game has put on the background palettes, as one blend towards
 * a colour. The world's textures are baked from the tilesets' own palettes,
 * so a fade - which rewrites the shown palette every frame - never reaches
 * them; the renderer applies this instead. Sprites need nothing: they are
 * decoded from the shown palette, fade and all.
 *
 * The shown tileset colours are fitted against the unfaded ones as
 * shown = (1 - amount) * unfaded + amount * colour, least squares over the
 * three channels: exact for a fade towards black, white or the grey of the
 * battle intro, and close for anything else a script does. Only while a fade
 * is under way or has left the screen faded: at rest, a weather colour map
 * (rain darkening the palette) stays the lighting grade's job, as before.
 */
static float FadeChannel(unsigned color, unsigned shift)
{
    return (float)((color >> shift) & 31);
}

static void FadeFit(const u16 *shown, const u16 *unfaded, unsigned count,
                    float *amount, float rgb[3])
{
    float n = (float)count, cov = 0.0f, var = 0.0f;
    float su[3] = {0}, sf[3] = {0}, suu[3] = {0}, suf[3] = {0};

    for (unsigned i = 0; i < count; ++i)
        for (unsigned c = 0; c < 3; ++c)
        {
            float u = FadeChannel(unfaded[i], c * 5), f = FadeChannel(shown[i], c * 5);

            su[c] += u;
            sf[c] += f;
            suu[c] += u * u;
            suf[c] += u * f;
        }
    for (unsigned c = 0; c < 3; ++c)
    {
        cov += n * suf[c] - su[c] * sf[c];
        var += n * suu[c] - su[c] * su[c];
    }
    /* Colours all alike tell nothing of the slope: take them as the target. */
    float keep = var > 1.0f ? cov / var : 0.0f;

    if (keep < 0.0f) keep = 0.0f;
    if (keep > 1.0f) keep = 1.0f;
    *amount = 1.0f - keep;
    for (unsigned c = 0; c < 3; ++c)
    {
        float offset = (sf[c] - keep * su[c]) / n;
        float value = *amount > 0.01f ? offset / *amount : 0.0f;

        if (value < 0.0f) value = 0.0f;
        if (value > 31.0f) value = 31.0f;
        rgb[c] = value / 31.0f;
    }
}

bool VoxelWorld_ScreenFade(float *amount, float rgb[3])
{
    *amount = 0.0f;
    rgb[0] = rgb[1] = rgb[2] = 0.0f;
#if !defined(PLATFORM_3DS)
    /* PC (DLX-5): in a battle the world stands in for the scenery and fades as its palettes do
     * (BG 2-4, LoadBattleTerrainGfx). POL-16: the field's fades (doors, warps) are the 3DS's, below. */
    if (gMain.inBattle)
    {
        const u16 *shown = (const u16 *)PLTT + BG_PLTT_ID(2), *unfaded = gPlttBufferUnfaded + BG_PLTT_ID(2);
        if (memcmp(shown, unfaded, 3 * 16 * sizeof(u16)) == 0)
            return false;
        FadeFit(shown, unfaded, 3 * 16, amount, rgb);
        if (*amount < 1.0f / 32.0f)
            *amount = 0.0f;
        return *amount > 0.0f;
    }
#endif
#if defined(PLATFORM_3DS) || defined(PORTABLE)
    {
        bool fading = gPaletteFade.active || gPaletteFade.y != 0
                   || (gWeatherPtr != NULL && gWeatherPtr->palProcessingState == WEATHER_PAL_STATE_SCREEN_FADING_IN)
                   || (gWeatherPtr != NULL && gWeatherPtr->palProcessingState == WEATHER_PAL_STATE_SCREEN_FADING_OUT);
        const u16 *shown = (const u16 *)PLTT, *unfaded = gPlttBufferUnfaded;
        unsigned count = NUM_PALS_TOTAL * 16;

        /*
         * In a battle the world stands in for the scenery, and fades as its
         * three palettes do (BG 2-4, LoadBattleTerrainGfx), whatever does it:
         * a palette fade, or a move blending the background towards a colour
         * without one. The battlers' palettes fade on their own.
         */
        if (gMain.inBattle)
        {
            shown += BG_PLTT_ID(2);
            unfaded += BG_PLTT_ID(2);
            count = 3 * 16;
            fading = true;
        }
        if (!fading || memcmp(shown, unfaded, count * sizeof(u16)) == 0)
            return false;
        FadeFit(shown, unfaded, count, amount, rgb);
        /* Below one step of the game's own 16 it is rounding, not a fade. */
        if (*amount < 1.0f / 32.0f)
            *amount = 0.0f;
    }
#endif
    return *amount > 0.0f;
}

#ifdef PORTABLE
/*
 * POL-26b (Treston, 2026-11-14: 3D gets night; "Match our 2D"): the time-of-day colour the 2D game puts on the
 * map's palettes, as a per-channel line shown = mul * stored + add (0..1 colours), so the 3D world can take the
 * same colour. Worked out from the blend itself, exactly as palette.c TimeMixPalettes_Internal does it per
 * colour: a tint multiplies (tint / 256, clamped at full), a blend moves towards its colour by coeff * 2 / 32;
 * the start blend has weight / 256, the end blend the rest. Colours flagged for the alternate/default light
 * colour (bit 15: lamps) and the lit-window palettes (overworld.c WallyJourney_ApplyNightLights) are not in this
 * line - the 3D view's own lamps and windows do those (later step). False: no tint (indoors, DNS off, day).
 */
static void BlendLine(const struct BlendSettings *blend, float mul[3], float add[3])
{
    u32 color = blend->blendColor;
    for (int c = 0; c < 3; c++)
    {
        if (blend->isTint)
        {
            mul[c] = (float)((color >> (8 * c)) & 0xFF) / 256.0f;
            add[c] = 0.0f;
        }
        else
        {
            float k = (float)(blend->coeff * 2) / 32.0f;
            mul[c] = 1.0f - k;
            add[c] = k * (float)((color >> (5 * c)) & 31) / 31.0f;
        }
    }
}

/* POL-26b: windows lit at night, at the times the 2D game lights its own (overworld.c WJ_WindowsLit). */
bool VoxelWorld_WindowsLit(void)
{
    return WallyJourney_WindowsLitNow() && getenv("WJPC_3D_NO_WINDOWS") == NULL;
}

bool VoxelWorld_TimeTint(float mul[3], float add[3])
{
    float m0[3], a0[3], m1[3], a1[3], w = gTimeBlend.weight / 256.0f;
    bool any = false;

    for (int c = 0; c < 3; c++)
    {
        mul[c] = 1.0f;
        add[c] = 0.0f;
    }
    if (gMapHeader.mapLayout == NULL || !MapHasNaturalLight(gMapHeader.mapType) || getenv("WJPC_3D_NO_TIME_TINT") != NULL)
        return false;
    BlendLine(&gTimeBlend.startBlend, m0, a0);
    BlendLine(&gTimeBlend.endBlend, m1, a1);
    for (int c = 0; c < 3; c++)
    {
        mul[c] = w * m0[c] + (1.0f - w) * m1[c];
        add[c] = w * a0[c] + (1.0f - w) * a1[c];
        any |= mul[c] < 0.999f || mul[c] > 1.001f || add[c] > 0.001f;
    }
    return any;
}
#endif

void VoxelWorld_GetLocation(int *mapGroup, int *mapNum)
{
    if (mapGroup != NULL) *mapGroup = gSaveBlock1Ptr != NULL ? gSaveBlock1Ptr->location.mapGroup : -1;
    if (mapNum != NULL) *mapNum = gSaveBlock1Ptr != NULL ? gSaveBlock1Ptr->location.mapNum : -1;
}

static int sMaterialView[4], sMaterialMargin;
static bool sMaterialViewSet;

void VoxelWorld_SetMaterialView(const int rect[4], int margin)
{
    sMaterialViewSet = rect != NULL;
    if (rect != NULL) memcpy(sMaterialView, rect, sizeof(sMaterialView));
    sMaterialMargin = margin;
}

/*
 * What lies past a map's edge, in voxel.
 *
 * The voxel camera sees past a map's edges, further than the 2D field does,
 * and what it sees there is the map's border block tiled outwards: a wood on
 * Rustboro's west, beyond its sea. Map by map, an edge (or a stretch of one)
 * can be filled with another metatile instead, one that goes on from what the
 * map shows there: the sea going on as sea. Voxel only; the 2D field keeps
 * the game's border. The stretches are the map's fixes (relief_fixes.json
 * "edges", set in the editor's mountain page), read with the relief
 * (VoxelRelief_EdgeFill): the metatile, and the height its top stands at if
 * given - else water is laid level with the map's own sea there
 * (BorderWaterLevel, voxel_mesh_builder.c), anything else as the border is.
 */

/* The metatile an edge fill puts at (worldX, worldY), or -1. */
const VoxelMapInstance *VoxelWorld_BeltRing(int worldX, int worldY);

static bool BeltPairDrawn(const VoxelMapInstance *m);

static int EdgeFillAtUncached(int worldX, int worldY, const VoxelMapInstance **art);
static int EdgeFillAt(int worldX, int worldY, const VoxelMapInstance **art)
{
    VOXEL_TLS_TABLE(CellMemoEntry, sMemo, CELL_MEMO_SIZE);
    if (sMemo == NULL)
        return EdgeFillAtUncached(worldX, worldY, art);
    CellMemoEntry *e = CellMemoSlot(sMemo, worldX, worldY);

    if (!CellMemoHit(e, worldX, worldY))
    {
        const VoxelMapInstance *a = NULL;
        int v = EdgeFillAtUncached(worldX, worldY, &a);
        CellMemoPut(e, worldX, worldY, v, a);
    }
    if (art != NULL && e->value >= 0)
        *art = e->ptr;
    return e->value;
}
static int EdgeFillAtUncached(int worldX, int worldY, const VoxelMapInstance **art)
{
    VoxelEdgeFill fill;

    const VoxelMapInstance *ring;

    if (sInstanceCount == 0)
        return -1;
    /* POL-30: the fill of the map the cell stands with (VoxelWorld_BeltRing), not only the current map's - a
     * map's filled edge (Fallarbor's ash, Rustboro's sea) came and went at the crossing. Only a fill the current
     * map's atlas can draw: the same tile sets, or the shared primary set's metatile. */
    ring = VoxelWorld_BeltRing(worldX, worldY);
    if (ring == NULL || !VoxelRelief_EdgeFillAt(ring, worldX, worldY, &fill))
        return -1;
    if (art != NULL)
        *art = &sInstances[0];
    if (ring != &sInstances[0]
     && (ring->primaryTileset != sInstances[0].primaryTileset
      || (ring->secondaryTileset != sInstances[0].secondaryTileset
       && fill.metatile >= (int)Voxel_PrimaryMetatiles(ring->primaryTileset))))
    {
        /* (or, with the renderer's belt layers, one drawn with that map's own atlas) */
        if (VoxelWorld_BeltOld() || !BeltPairDrawn(ring))
            return -1;
        if (art != NULL)
            *art = ring;
    }
    return fill.metatile;
}

/* The edge fills' metatiles for this map, kept in its atlas. */
static void MarkEdgeFills(const VoxelMapInstance *inst, uint8_t *used)
{
    int metatile;

    for (unsigned i = 0; VoxelRelief_EdgeFills((unsigned)inst->layoutId, i, &metatile); ++i)
        if (metatile >= 0 && metatile < NUM_METATILES_TOTAL)
            used[metatile] = 2;
    /* (Brennan 2026-11-27 04:40: the pond on the mountain west of Lavaridge - the General's water, which no map on
     * Lavaridge's tile sets uses; voxel_mesh_builder.c LavaridgePondWater) */
    if (inst->layoutId == LAYOUT_LAVARIDGE_TOWN && VoxelWorld_N13Idea())
        used[0x170] = 2;
}

/* The cut records' grounds and walls, in the window MarkUsedMetatiles reads
 * the cells in, as important as the cells there. */
typedef struct
{
    const VoxelMapInstance *inst;
    int x0, y0, x1, y1;
    uint8_t *used;
} CutMark;

static void MarkCut(int x, int y, int ground, int wall, void *ctx)
{
    const CutMark *mark = ctx;
    const VoxelMapInstance *inst = mark->inst;
    unsigned priority;

    if (x < mark->x0 || y < mark->y0 || x >= mark->x1 || y >= mark->y1)
        return;
    priority = !sMaterialViewSet ||
        (x + inst->originX >= sMaterialView[0] && x + inst->originX < sMaterialView[2]
      && y + inst->originY >= sMaterialView[1] && y + inst->originY < sMaterialView[3]) ? 2u : 1u;
    if (ground >= 0 && ground < (int)VOXEL_METATILE_IDS && mark->used[ground] < priority)
        mark->used[ground] = (uint8_t)priority;
    if (wall >= 0 && wall < (int)VOXEL_METATILE_IDS && mark->used[wall] < priority)
        mark->used[wall] = (uint8_t)priority;
}

void VoxelWorld_MarkUsedMetatiles(const void *primaryTileset, const void *secondaryTileset,
                                  uint8_t *used)
{
    for (unsigned i = 0; i < sInstanceCount; ++i)
    {
        const VoxelMapInstance *inst = &sInstances[i];
        const struct MapLayout *layout = inst->layout;

        if (inst->primaryTileset != primaryTileset
         || inst->secondaryTileset != secondaryTileset)
            continue;

        int x0 = 0, y0 = 0, x1 = inst->width, y1 = inst->height;
        if (sMaterialViewSet)
        {
            if (x0 < sMaterialView[0] - sMaterialMargin - inst->originX)
                x0 = sMaterialView[0] - sMaterialMargin - inst->originX;
            if (y0 < sMaterialView[1] - sMaterialMargin - inst->originY)
                y0 = sMaterialView[1] - sMaterialMargin - inst->originY;
            if (x1 > sMaterialView[2] + sMaterialMargin - inst->originX)
                x1 = sMaterialView[2] + sMaterialMargin - inst->originX;
            if (y1 > sMaterialView[3] + sMaterialMargin - inst->originY)
                y1 = sMaterialView[3] + sMaterialMargin - inst->originY;
        }
        MarkEdgeFills(inst, used); /* POL-30: every map's on these tile sets (was the current map's only) */
        const uint16_t *map = i == 0 ? gBackupMapLayout.map : Voxel_ResolveMap(layout);
        if (map != NULL)
            for (int y = y0; y < y1; ++y)
                for (int x = x0; x < x1; ++x)
                {
                    int bx = i == 0 ? x + MAP_OFFSET : x;
                    int by = i == 0 ? y + MAP_OFFSET : y;
                    int stride = i == 0 ? gBackupMapLayout.width : inst->width;
                    if (i == 0 && (bx < 0 || by < 0 || bx >= stride || by >= gBackupMapLayout.height))
                        continue;
                    unsigned m = map[by * stride + bx] & MAPGRID_METATILE_ID_MASK;
                    unsigned priority = !sMaterialViewSet ||
                        (x + inst->originX >= sMaterialView[0] && x + inst->originX < sMaterialView[2]
                      && y + inst->originY >= sMaterialView[1] && y + inst->originY < sMaterialView[3]) ? 2u : 1u;
                    if (used[m] < priority) used[m] = priority;
                }
        /* What the mesh draws from the relief's records besides the cells'
         * own metatiles: the ground a cut tile runs on, its cliff's face.
         * Read off the layout, they are not always in the live grid - a
         * script's setmetatile (Route 120's bridge once its Kecleon is
         * gone) took the only cell drawing that ground, and the bridge's
         * water under its deck was a hole showing the black behind. */
        {
            CutMark mark = {inst, x0, y0, x1, y1, used};

            VoxelRelief_ForEachCut(inst, MarkCut, &mark);
        }
    }
    /* The belt is drawn with the current map's atlas, and from a neighbour's
     * border as well as its own (VoxelWorld_BorderMetatile): any border whose
     * metatiles these tilesets hold. */
    for (unsigned i = 0; i < sInstanceCount; ++i)
    {
        const VoxelMapInstance *inst = &sInstances[i];
        const struct MapLayout *layout = inst->layout;
        const uint16_t *border;
        bool secondary = false;

        if (inst->primaryTileset != primaryTileset || layout == NULL || layout->border == NULL)
            continue;
        border = Port_ResolveAssetPointer(layout->border);
        if (border == NULL)
            continue;
        /* POL-30: and the edge fills of a map on another secondary set that are the primary set's */
        if (inst->secondaryTileset != secondaryTileset)
        {
            int metatile;

            for (unsigned k = 0; VoxelRelief_EdgeFills((unsigned)inst->layoutId, k, &metatile); ++k)
                if (metatile >= 0 && metatile < (int)Voxel_PrimaryMetatiles(primaryTileset))
                    used[metatile] = 2;
        }
        for (unsigned t = 0; t < 4; ++t)
            secondary |= (border[t] & MAPGRID_METATILE_ID_MASK) >= Voxel_PrimaryMetatiles(primaryTileset);
        if (secondary && inst->secondaryTileset != secondaryTileset)
            continue;
        for (unsigned t = 0; t < 4; ++t)
            used[border[t] & MAPGRID_METATILE_ID_MASK] = 2;
    }
    /* The replacement removes canopy fringes even on maps that never used
     * their bare ground tile. Keep that material available in the atlas. */
    if (primaryTileset == &gTileset_General)
        for (int m = 0; m < NUM_METATILES_TOTAL; ++m)
        {
            int ground = VoxelTree_GroundMetatile(secondaryTileset, m);
            if (used[m] > used[ground])
                used[ground] = used[m];
        }
}

/* A layout's own drawing, not the live grid (voxel_world.h). */
const uint16_t *VoxelWorld_LayoutDrawing(int layoutId, int *width, int *height)
{
    const struct MapLayout *layout;

    if (layoutId <= 0 || layoutId > 0xFFFF)
        return NULL;
    layout = Port_GetMapLayoutById((u16)layoutId);
    if (layout == NULL || layout->width <= 0 || layout->height <= 0)
        return NULL;
    *width = layout->width;
    *height = layout->height;
    return Voxel_ResolveMap(layout);
}

/*
 * A digest of the whole live grid, which is the only map data that changes
 * while it is on screen: connections are read from their static layouts. When
 * it is unchanged no chunk can have gone stale, so the per-chunk signatures
 * below need computing only on the frames where it moves - instead of every
 * visible chunk, every frame.
 *
 * Two words at a time and a multiply per word: a few thousand cycles for the
 * largest layout, far less than one chunk's signature.
 */
uint32_t VoxelWorld_LiveDigest(void)
{
    const u16 *map = gBackupMapLayout.map;
    uint32_t hash = 2166136261u;
    unsigned area, i;

    if (map == NULL || gBackupMapLayout.width <= 0 || gBackupMapLayout.height <= 0)
        return 0;
    area = (unsigned)gBackupMapLayout.width * (unsigned)gBackupMapLayout.height;
    hash = (hash ^ area) * 16777619u;
    hash = (hash ^ (uint32_t)(uintptr_t)map) * 16777619u;
    for (i = 0; i + 1 < area; i += 2)
        hash = (hash ^ ((uint32_t)map[i] | ((uint32_t)map[i + 1] << 16))) * 16777619u;
    if (i < area)
        hash = (hash ^ map[i]) * 16777619u;
    return hash;
}

uint32_t VoxelWorld_BlockHash(int x0, int y0, int x1, int y1)
{
    uint32_t hash = 2166136261u;
    uint16_t row[80];

    /* Resolve/copy a span per map and row instead of finding an instance and
     * resolving its payload for every tile of every overlapping light halo.
     * Reverse instance order preserves GetInstanceAt's first-match precedence. */
    if (x1 > x0 && x1 - x0 <= (int)(sizeof(row) / sizeof(row[0])))
    {
        for (int y = y0; y < y1; ++y)
        {
            memset(row, 0, (size_t)(x1 - x0) * sizeof(row[0]));
            for (unsigned i = sInstanceCount; i-- > 0;)
            {
                const VoxelMapInstance *inst = &sInstances[i];
                const uint16_t *map;
                int left = x0 > inst->originX ? x0 : inst->originX;
                int right = x1 < inst->originX + inst->width ? x1 : inst->originX + inst->width;
                if (left >= right || y < inst->originY || y >= inst->originY + inst->height)
                    continue;
                memset(row + left - x0, 0, (size_t)(right - left) * sizeof(row[0]));
                if (i == 0)
                {
                    int by = y - inst->originY + MAP_OFFSET;
                    if (by < 0 || by >= gBackupMapLayout.height)
                        continue;
                    if (left < inst->originX - MAP_OFFSET) left = inst->originX - MAP_OFFSET;
                    if (right > inst->originX + gBackupMapLayout.width - MAP_OFFSET)
                        right = inst->originX + gBackupMapLayout.width - MAP_OFFSET;
                    if (left >= right) continue;
                    map = gBackupMapLayout.map + by * gBackupMapLayout.width
                        + left - inst->originX + MAP_OFFSET;
                }
                else
                {
                    map = Voxel_ResolveMap(inst->layout);
                    if (map == NULL) continue;
                    map += (y - inst->originY) * inst->width + left - inst->originX;
                }
                memcpy(row + left - x0, map, (size_t)(right - left) * sizeof(row[0]));
            }
            for (int x = 0; x < x1 - x0; ++x)
                hash = (hash ^ row[x]) * 16777619u;
        }
        return hash;
    }

    /* FNV-1a over the raw blocks, not the metatile ids: collision and
     * elevation are part of what the classifier reads, so a change to them is
     * a change to the geometry even when the drawing is the same. */
    for (int y = y0; y < y1; ++y)
        for (int x = x0; x < x1; ++x)
        {
            hash ^= GetRawBlock(x, y, NULL);
            hash *= 16777619u;
        }
    return hash;
}

/* A map's 2x2 border block made of the primary tile set's metatiles only. */
static bool BorderIsPrimaryOnlyUncached(const VoxelMapInstance *m);
static bool BorderIsPrimaryOnly(const VoxelMapInstance *m)
{
    static VOXEL_TLS unsigned sGen[MAX_VOXEL_MAP_INSTANCES];
    static VOXEL_TLS bool sAnswer[MAX_VOXEL_MAP_INSTANCES];

    if (m >= sInstances && m < sInstances + MAX_VOXEL_MAP_INSTANCES && !CellMemoOff())
    {
        unsigned i = (unsigned)(m - sInstances);
        if (sGen[i] != sCellGen)
        {
            sAnswer[i] = BorderIsPrimaryOnlyUncached(m);
            sGen[i] = sCellGen;
        }
        return sAnswer[i];
    }
    return BorderIsPrimaryOnlyUncached(m);
}
static bool BorderIsPrimaryOnlyUncached(const VoxelMapInstance *m)
{
    const void *source = ((const struct MapLayout *)m->layout)->border, *resolved;
    const uint16_t *border;

    if (!MemoGet(&sBorderMemo, source, &resolved))
    {
        resolved = Port_ResolveAssetPointer(source);
        MemoPut(&sBorderMemo, source, resolved);
    }
    border = resolved;
    if (border == NULL)
        return false;
    for (unsigned t = 0; t < 4; ++t)
        if ((border[t] & MAPGRID_METATILE_ID_MASK) >= Voxel_PrimaryMetatiles(m->primaryTileset))
            return false;
    return true;
}

/*
 * The loaded outdoor map nearest a cell of the belt; a tie goes to the lower map number, not to the current
 * map, so the answer is the same from both sides of a crossing. `art`: only maps whose border the current
 * map's atlas can draw -
 * DLX-25: maps on the current map's tile sets;
 * POL-30 (Brennan 2026-11-14: the background changes from one to the other between routes, e.g. Route 110's sea
 * turning into Route 103's trees): and maps on another secondary tile set whose border is made of the shared
 * primary set's metatiles only (the same art here) - the atlas already holds those (MarkUsedMetatiles' "any
 * border whose metatiles these tilesets hold").
 */
/*
 * POL-30 (Brennan 2026-11-15: "fix route 113" - its ash trees are on Fallarbor's tile set, which Route 111's and
 * Route 112's atlas cannot draw): the renderer can draw belt cells with other loaded maps' atlases as well
 * (voxel_gl.c, belt layers) and says here which tile set pairs it has ready. A map on one of them gives the
 * belt its border like any other; VoxelWorld_BorderArtMap says whose tile sets a cell's metatile is of.
 */

void VoxelWorld_SetBeltPairs(const void *const (*pairs)[2], unsigned count)
{
    sBeltPairCount = count > BELT_PAIRS ? BELT_PAIRS : count;
    sCellGen++;
    for (unsigned i = 0; i < sBeltPairCount; ++i)
    {
        sBeltPairs[i][0] = pairs[i][0];
        sBeltPairs[i][1] = pairs[i][1];
    }
}

static bool BeltPairDrawn(const VoxelMapInstance *m)
{
    for (unsigned i = 0; i < sBeltPairCount; ++i)
        if (sBeltPairs[i][0] == m->primaryTileset && sBeltPairs[i][1] == m->secondaryTileset)
            return true;
    return false;
}

/* WJPC_BELT_OLD=1: the belt as before POL-30 (the current map decides), to compare. */
bool VoxelWorld_BeltOld(void)
{
    static int sOld = -1;

    if (sOld < 0)
        sOld = getenv("WJPC_BELT_OLD") != NULL;
    return sOld;
}

/*
 * Brennan 2026-11-26 22:09 (Route 118, "route 118 fix what is in red boxes" - the mountain across the river:
 * "straight, uniform, bare cliff tiers stacked like rigid stairs"; where mountain, river and the upper-right forest
 * meet "a sharp, unnatural triangle wedge with straight, rigid edges"; "Curve and soften the shoreline and forest
 * edge so the water, mountain base, and tree line flow naturally into one another"; "Let trees wrap organically
 * around the base of the mountain and along curved riverbanks"): the belt north of Route 118, between Route 111's
 * east edge and Route 119's west edge, was each map's border by the nearest map - Route 111's rock, Route 118's
 * sea, Route 119's woods - cut along straight 45-degree lines: the sea a triangle pointing north between the rock
 * and the woods. Now the water is a lake where the triangle was (along the map's edge as before: behind the rock
 * mound and the map's trees), its shores curved and uneven and its top rounded, narrowing into a river that winds away
 * north and ends R118N_RIVER_END rows out; a band of trees (1-3 cells, Route 119's woods) along its west shore at the
 * mountain's foot, and a row or two north of the map's own trees; the rock (Route 111's) west of that, the woods
 * east of the water. Past the river's end the cut between rock and woods drifts back to the straight line it was by
 * R118N_ROWS rows out. Each part is that map's border, as before (its level, its art); the ledges and trees on the
 * rock: voxel_mesh_builder.c (R118FieldJitter, R118FieldRate, VoxelMesh_LedgeTree, VoxelMesh_TreeOverDrop). Only
 * where all three maps are placed as Hoenn places them. WJPC_R118_NORTH_OLD=1: as before.
 */
#define R118N_WIDTH 40          /* Route 111's east edge to Route 119's west edge */
#define R118N_ROWS 64
#define R118N_RIVER_END 48.0f
/* Brennan 2026-11-27 03:57 (#13, "show me pictures of your idea for those places and i will say yes or no"): with
 * WJPC_N13_IDEA=1 the area runs on up the whole strip between Route 111 and Route 119 (R118N_STRIP_ROWS, Route 111's
 * east side - his #13 "long straight ridges"): past the river the line between the rock and the woods keeps wandering,
 * the trees along it, the rock's ledges uneven with trees on them, drifting back to the straight line only over the
 * strip's last R118N_STRIP_FADE rows. Off (the default until he says yes): as N82. */
#define R118N_STRIP_ROWS 140
#define R118N_STRIP_FADE 16.0f

bool VoxelWorld_N13Idea(void)
{
    return VoxelWorld_N13Variant() != 0;
}

/* which of the ideas (test aid while they are drawn for him): WJPC_N13_IDEA=1, 2, 3 ... (anything else set: 1) */
int VoxelWorld_N13Variant(void)
{
    static int sVar = -1;

    if (sVar < 0)
    {
        const char *e = getenv("WJPC_N13_IDEA");
        sVar = e == NULL ? 0 : atoi(e) > 0 ? atoi(e) : 1;
    }
    return sVar;
}

/* how many rows north of Route 118's edge the area reaches */
static int R118NRows(void)
{
    return VoxelWorld_N13Idea() ? R118N_STRIP_ROWS : R118N_ROWS;
}

float VoxelWorld_R118NorthRateFade(int localY)
{
    float v = (float)-localY;

    if (VoxelWorld_N13Idea())
        return v <= (float)R118N_STRIP_ROWS - R118N_STRIP_FADE ? 1.0f
             : v >= (float)R118N_STRIP_ROWS ? 0.0f : ((float)R118N_STRIP_ROWS - v) / R118N_STRIP_FADE;
    v = 1.0f - (v - R118N_RIVER_END) / 16.0f;
    return v > 1.0f ? 1.0f : v < 0.0f ? 0.0f : v;
}

static float R118NHash(int i, uint32_t seed)
{
    uint32_t h = (uint32_t)i * 374761393u + seed * 668265263u;

    h = (h ^ (h >> 13)) * 1274126177u;
    return (float)((h ^ (h >> 16)) & 0xFFFFu) / 65535.0f * 2.0f - 1.0f;
}

/* smooth value noise along one axis, -1..1, a new value every unit */
static float R118NNoise(float t, uint32_t seed)
{
    float fl = floorf(t), f = t - fl, a = R118NHash((int)fl, seed), b = R118NHash((int)fl + 1, seed);

    f = f * f * (3.0f - 2.0f * f);
    return a + (b - a) * f;
}

static float R118NSmooth(float a, float b, float t)
{
    t = (t - a) / (b - a);
    t = t < 0.0f ? 0.0f : t > 1.0f ? 1.0f : t;
    return t * t * (3.0f - 2.0f * t);
}

/* the three maps, when placed round the belt as Hoenn places them (kept per thread until the maps change) */
static bool R118NorthMaps(const VoxelMapInstance **m111, const VoxelMapInstance **m118, const VoxelMapInstance **m119)
{
    static VOXEL_TLS unsigned sGen = 0;
    static VOXEL_TLS bool sOk;
    static VOXEL_TLS const VoxelMapInstance *s111, *s118, *s119;

    if (sGen == 0 || sGen != sCellGen)
    {
        s111 = s118 = s119 = NULL;
        for (unsigned i = 0; i < sInstanceCount; ++i)
        {
            const VoxelMapInstance *m = &sInstances[i];

            if (m->layout == NULL || m->indoor)
                continue;
            /* (Route 111 is laid out again once Mirage Tower has fallen: the same land round it) */
            if (m->layoutId == LAYOUT_ROUTE111 || m->layoutId == LAYOUT_ROUTE111_NO_MIRAGE_TOWER)
                s111 = m;
            else if (m->layoutId == LAYOUT_ROUTE118)
                s118 = m;
            else if (m->layoutId == LAYOUT_ROUTE119)
                s119 = m;
        }
        sOk = s111 != NULL && s118 != NULL && s119 != NULL
           && s111->originX + s111->width == s118->originX && s111->originY + s111->height == s118->originY
           && s119->originX == s118->originX + R118N_WIDTH && s119->originY + s119->height == s118->originY
           && s111->height >= R118NRows() && s119->height >= R118NRows();
        sGen = CellMemoOff() ? 0 : sCellGen;
    }
    *m111 = s111;
    *m118 = s118;
    *m119 = s119;
    return sOk;
}

/* the river's banks v rows north of Route 118's edge (cells from Route 118's west edge), FALSE past its end: a lake
 * where the triangle was (behind the rock mound and the map's trees as before, x 6-36 along the map's edge), its
 * top rounded, curving in to the river; the river winding north, narrowing to its end */
static bool R118NBanks(float v, float *left, float *right)
{
    static const float kKeys[][3] = {
        {  0.0f,  5.5f, 36.5f }, {  3.0f,  5.7f, 36.3f }, {  6.0f,  6.6f, 35.4f }, {  9.0f,  8.3f, 33.7f },
        { 11.0f, 10.1f, 31.9f }, { 13.0f, 12.9f, 29.1f }, { 14.5f, 16.0f, 26.0f }, { 16.5f, 19.0f, 25.0f },
        { 22.0f, 19.5f, 25.0f }, { 27.0f, 21.0f, 25.5f }, { 33.0f, 18.5f, 23.0f }, { 39.0f, 17.0f, 21.5f },
        { 44.0f, 18.5f, 22.0f }, { R118N_RIVER_END, 20.0f, 20.5f },
    };
    const unsigned n = sizeof(kKeys) / sizeof(kKeys[0]);
    float fade;

    if (v < 0.0f || v >= R118N_RIVER_END)
        return false;
    for (unsigned k = 0; k + 1 < n; ++k)
        if (v <= kKeys[k + 1][0])
        {
            float t = (v - kKeys[k][0]) / (kKeys[k + 1][0] - kKeys[k][0]);

            *left = kKeys[k][1] + (kKeys[k + 1][1] - kKeys[k][1]) * t;
            *right = kKeys[k][2] + (kKeys[k + 1][2] - kKeys[k][2]) * t;
            break;
        }
    /* (uneven: each bank wanders a cell or so - not along the map's edge, where the lake meets the map's own
     * shore as it did) */
    fade = R118NSmooth(2.0f, 6.0f, v);
    *left -= fade * 1.6f * R118NNoise(v / 3.5f, 11u);
    *right += fade * 1.6f * R118NNoise(v / 3.5f, 23u);
    return true;
}

/* the trees along the west bank, cells wide (1-3) */
static float R118NBand(float v)
{
    return 1.6f + 1.1f * R118NNoise(v / 2.8f, 37u);
}

int VoxelWorld_R118North(int worldX, int worldY, int *localX, int *localY)
{
    static int sOld = -1;
    const VoxelMapInstance *m111, *m118, *m119;
    float u, v, l, r;
    int lx, ly;

    if (sOld < 0)
        sOld = getenv("WJPC_R118_NORTH_OLD") != NULL;
    if (sOld || !R118NorthMaps(&m111, &m118, &m119))
        return VOXEL_R118N_NONE;
    lx = worldX - m118->originX;
    ly = worldY - m118->originY;
    if (localX != NULL)
        *localX = lx;
    if (localY != NULL)
        *localY = ly;
    if (lx < 0 || lx >= R118N_WIDTH || ly >= 0 || ly < -R118NRows())
        return VOXEL_R118N_NONE;
    u = (float)lx + 0.5f;
    v = (float)-ly - 0.5f;
    if (R118NBanks(v, &l, &r))
    {
        if (u > l && u < r)
            return VOXEL_R118N_WATER;
        if (u >= r)
            return VOXEL_R118N_WOODS;
        if (u > l - R118NBand(v))
            return VOXEL_R118N_BANK;
        /* (a row or two of trees north of the map's own wood along its edge, before the rock) */
        if (v < 1.5f + 1.2f * (R118NNoise((float)lx / 2.5f, 53u) + 1.0f))
        {
            int mt = VoxelWorld_GetMetatileId(m118->originX + lx, m118->originY);

            if (mt >= 0 && VoxelTree_Part(m118->secondaryTileset, mt) >= 0)
                return VOXEL_R118N_BANK;
        }
        return VOXEL_R118N_ROCK;
    }
    {
        /* (WJPC_N13_IDEA: on up the strip, back to the straight line only over its last rows) */
        float t = VoxelWorld_N13Idea() ? R118NSmooth((float)R118N_STRIP_ROWS - R118N_STRIP_FADE, (float)R118N_STRIP_ROWS, v)
                                       : R118NSmooth(R118N_RIVER_END, (float)R118N_ROWS, v);
        float cut = 20.25f - 0.25f * t + (1.0f - t) * 1.5f * R118NNoise(v / 4.0f, 59u);

        if (u >= cut)
            return VOXEL_R118N_WOODS;
        if (u >= cut - R118NBand(v) * (1.0f - t))
            return VOXEL_R118N_BANK;
        return VOXEL_R118N_ROCK;
    }
}

/* whether a map's border may be drawn from here (NearestBeltMapUncached's rule for `art`) */
static bool BeltArtOk(const VoxelMapInstance *m)
{
    if (((const struct MapLayout *)m->layout)->border == NULL)
        return false;
    if (m->primaryTileset != sInstances[0].primaryTileset || m->secondaryTileset != sInstances[0].secondaryTileset)
    {
        if (VoxelWorld_BeltOld())
            return false;
        if (!(m->primaryTileset == sInstances[0].primaryTileset && BorderIsPrimaryOnly(m)) && !BeltPairDrawn(m))
            return false;
    }
    return true;
}

static const VoxelMapInstance *NearestBeltMapUncached(int worldX, int worldY, bool art);
static const VoxelMapInstance *NearestBeltMap(int worldX, int worldY, bool art)
{
    VOXEL_TLS_TABLE(CellMemoEntry, sMemo, 2 * CELL_MEMO_SIZE);
    if (sMemo == NULL)
        return NearestBeltMapUncached(worldX, worldY, art);
    CellMemoEntry *e = CellMemoSlot(sMemo + (art ? CELL_MEMO_SIZE : 0), worldX, worldY);

    if (!CellMemoHit(e, worldX, worldY))
        CellMemoPut(e, worldX, worldY, 0, NearestBeltMapUncached(worldX, worldY, art));
    return e->ptr;
}
static const VoxelMapInstance *NearestBeltMapUncached(int worldX, int worldY, bool art)
{
    const VoxelMapInstance *best = NULL;
    int bestD = 0x7FFFFFFF;

    /* (Brennan 2026-11-26 22:09: north of Route 118 the river's parts, VoxelWorld_R118North) */
    if (!VoxelWorld_BeltOld())
    {
        int part = VoxelWorld_R118North(worldX, worldY, NULL, NULL);

        if (part != VOXEL_R118N_NONE)
        {
            const VoxelMapInstance *m111, *m118, *m119, *own;

            R118NorthMaps(&m111, &m118, &m119);
            own = part == VOXEL_R118N_ROCK ? m111 : part == VOXEL_R118N_WATER ? m118 : m119;
            if (!art || BeltArtOk(own))
                return own;
        }
    }
    for (unsigned i = 0; i < sInstanceCount; ++i)
    {
        const VoxelMapInstance *m = &sInstances[i];
        int dx, dy, d;

        if (m->layout == NULL || m->indoor)
            continue;
        if (art && !BeltArtOk(m))
            continue;
        dx = worldX < m->originX ? m->originX - worldX : worldX >= m->originX + m->width ? worldX - (m->originX + m->width - 1) : 0;
        dy = worldY < m->originY ? m->originY - worldY : worldY >= m->originY + m->height ? worldY - (m->originY + m->height - 1) : 0;
        /* POL-30 (Brennan 2026-11-15: "trees and the mountain meshing together" north-west of Route 111): by
         * the straight-line distance, not the larger of the two - with that every cell diagonally off a map's
         * corner was as near as the map straight beside it, the tie cut the belt along a diagonal, and one
         * map's trees stood in the steps of the other's rock. Now the cut runs straight out from where two
         * maps meet. */
        d = VoxelWorld_BeltOld() ? (dx > dy ? dx : dy) : dx * dx + dy * dy;
        if (d < bestD || (d == bestD && (VoxelWorld_BeltOld() ? i == 0 : best != NULL
                          && (m->mapGroup << 8 | m->mapNum) < (best->mapGroup << 8 | best->mapNum))))
        {
            bestD = d;
            best = m;
        }
    }
    return best;
}

/* POL-30: the map a cell of the belt stands with - its level and base are that map's nearest edge's, whichever
 * map the player is on (it was always the current map's: the belt beside the next map rose or sank at the
 * crossing). The current map when there is no other. */
const VoxelMapInstance *VoxelWorld_BeltRing(int worldX, int worldY)
{
    const VoxelMapInstance *best;

    if (sInstanceCount == 0)
        return NULL;
    if (sInstances[0].indoor || sInstances[0].layout == NULL || VoxelWorld_BeltOld())
        return &sInstances[0];
    best = NearestBeltMap(worldX, worldY, false);
    return best != NULL ? best : &sInstances[0];
}

/* POL-30: the loaded map whose tile sets (and atlas) a belt cell's metatile (VoxelWorld_BorderMetatile) is of:
 * the current map, unless the cell is another tile set pair's map's border and not the shared primary set's. */
static const VoxelMapInstance *BorderArtMapUncached(int worldX, int worldY);
const VoxelMapInstance *VoxelWorld_BorderArtMap(int worldX, int worldY)
{
    VOXEL_TLS_TABLE(CellMemoEntry, sMemo, CELL_MEMO_SIZE);
    if (sMemo == NULL)
        return BorderArtMapUncached(worldX, worldY);
    CellMemoEntry *e = CellMemoSlot(sMemo, worldX, worldY);

    if (!CellMemoHit(e, worldX, worldY))
        CellMemoPut(e, worldX, worldY, 0, BorderArtMapUncached(worldX, worldY));
    return e->ptr;
}
static const VoxelMapInstance *BorderArtMapUncached(int worldX, int worldY)
{
    const VoxelMapInstance *best;

    if (sInstanceCount == 0)
        return NULL;
    if (sInstances[0].indoor || sInstances[0].layout == NULL)
        return &sInstances[0];
    if (EdgeFillAt(worldX, worldY, &best) >= 0)
        return best;
    best = NearestBeltMap(worldX, worldY, true);
    if (best == NULL || best == &sInstances[0]
     || (best->primaryTileset == sInstances[0].primaryTileset
      && (best->secondaryTileset == sInstances[0].secondaryTileset || BorderIsPrimaryOnly(best))))
        return &sInstances[0];
    return best;
}

/* POL-30: edge fill k of loaded map i that the belt may draw (EdgeFillAt's rule), for the same check. */
bool VoxelWorld_BeltFillOf(unsigned i, unsigned k, int *metatile)
{
    const VoxelMapInstance *m;

    if (i >= sInstanceCount || sInstances[0].layout == NULL)
        return false;
    m = &sInstances[i];
    if (m->layout == NULL || m->indoor || m->primaryTileset != sInstances[0].primaryTileset
     || !VoxelRelief_EdgeFills((unsigned)m->layoutId, k, metatile))
        return false;
    if (m->secondaryTileset != sInstances[0].secondaryTileset && *metatile >= (int)Voxel_PrimaryMetatiles(m->primaryTileset))
        *metatile = -1; /* not drawn from here */
    return true;
}

/* POL-30: the border metatiles the belt may draw from loaded map i (VoxelWorld_BorderMetatile's rule), so the
 * renderer can see its atlas holds them. FALSE: that map's border is not used. */
bool VoxelWorld_BeltBorderOf(unsigned i, int ids[4])
{
    const VoxelMapInstance *m;
    const void *resolved;

    if (i >= sInstanceCount || sInstances[0].layout == NULL)
        return false;
    m = &sInstances[i];
    if (m->layout == NULL || m->indoor || m->primaryTileset != sInstances[0].primaryTileset
     || ((const struct MapLayout *)m->layout)->border == NULL)
        return false;
    if (m->secondaryTileset != sInstances[0].secondaryTileset && !BorderIsPrimaryOnly(m))
        return false;
    if (!MemoGet(&sBorderMemo, ((const struct MapLayout *)m->layout)->border, &resolved) || resolved == NULL)
    {
        resolved = Port_ResolveAssetPointer(((const struct MapLayout *)m->layout)->border);
        if (resolved == NULL)
            return false;
    }
    for (unsigned t = 0; t < 4; ++t)
        ids[t] = ((const uint16_t *)resolved)[t] & MAPGRID_METATILE_ID_MASK;
    return true;
}

/*
 * The metatile Emerald tiles outside the map.
 *
 * A window reaches past the edge of the world on purpose - that is where the
 * connected maps are - but where no map reaches either, the renderer drew
 * nothing and the view ended in the clear colour. The 2D game does not: it
 * tiles the layout's 2x2 border block outwards for ever, which is why a town
 * looks like it sits in a wood rather than on a plate over a black sky.
 *
 * Whose border, and in which phase, is fieldmap.c's to say: the nearest map's
 * whose border the current tilesets draw, the same answer the 2D camera
 * gives, and the same from either side of a crossing. Past an edge filled
 * for the current map (VoxelRelief_EdgeFill), its fill.
 */
static int BorderMetatileRaw(int worldX, int worldY)
{
    const uint16_t *border;
    const void *source, *resolved;
    u8 index;
    int fill;

    if (sInstanceCount == 0 || sInstances[0].layout == NULL)
        return -1;
    fill = EdgeFillAt(worldX, worldY, NULL);
    if (fill >= 0)
        return fill;
    source = MapGridGetCameraBorderAt(worldX - sInstances[0].originX,
                                      worldY - sInstances[0].originY, &index);
    /*
     * DLX-25 (Wally's Journey PC, owner's choice 2026-11-13): the nearest map's border, not the current map's,
     * so the belt stays the same across a crossing (Dewford's trees used to turn into Route 107's sea when you
     * crossed). Only maps on the current map's tile sets: another pair's metatile ids mean other art here.
     * Phase from that map's own origin, as the 2D game tiles it.
     */
    {
        const VoxelMapInstance *best = NearestBeltMap(worldX, worldY, true);

        if (best != NULL && best != &sInstances[0])
        {
            source = ((const struct MapLayout *)best->layout)->border;
            index = (u8)((((worldY - best->originY) & 1) << 1) | ((worldX - best->originX) & 1));
        }
    }
    if (source == NULL)
        return -1;
    if (!MemoGet(&sBorderMemo, source, &resolved))
    {
        resolved = Port_ResolveAssetPointer(source);
        MemoPut(&sBorderMemo, source, resolved);
    }
    border = resolved;
    if (border == NULL)
        return -1;
    return (int)(border[index] & MAPGRID_METATILE_ID_MASK);
}

static bool RawIsWater(int x, int y, int *mt)
{
    *mt = BorderMetatileRaw(x, y);
    return *mt >= 0 && VoxelWorld_GetInstanceAt(x, y) == NULL
        && BehaviorShape(UNPACK_BEHAVIOR(GetMetatileAttribute(VoxelWorld_BorderArtMap(x, y), *mt))) == VOXEL_SHAPE_WATER;
}

/*
 * Brennan 2026-11-20 19:48 (Route 121's woods over the sea: "a jagged diagonal staircase of flat green grass blocks
 * sticking out behind the back row of 3D trees ... only the blue sea water renders behind the back row"): where the
 * belt's wood (a map's border of large trees) meets another map's sea along a diagonal, the trees cut by it are not
 * drawn (voxel_tree.c, whole trees only) and their cells stood as ground. Such a cell - a General large tree's part
 * whose tree is not whole, beside the sea - is the sea. WJPC_BELT_HALF_TREE_GROUND=1: as before.
 */
static int BorderMetatileUncached(int worldX, int worldY);
int VoxelWorld_BorderMetatile(int worldX, int worldY)
{
    VOXEL_TLS_TABLE(CellMemoEntry, sMemo, CELL_MEMO_SIZE);
    if (sMemo == NULL)
        return BorderMetatileUncached(worldX, worldY);
    CellMemoEntry *e = CellMemoSlot(sMemo, worldX, worldY);

    if (!CellMemoHit(e, worldX, worldY))
        CellMemoPut(e, worldX, worldY, BorderMetatileUncached(worldX, worldY), NULL);
    return e->value;
}
/* Brennan 2026-11-26 22:09 (Mt. Chimney: "scatter some trees in the south section of the mountain that flows with the
 * rest of the trees"): the land south of Mt. Chimney (its border, the pink rock 271) carries its small ash-covered
 * trees (Lavaridge 2CD, voxel_tree.c TRUNK_ASH): as thick as the map's own wood along its edge, thinning out over the
 * next dozen rows to a few scattered trees, and fading out past its sides. A hash of the cell decides (the same trees
 * every time). WJPC_CHIMNEY_SOUTH_BARE=1: none, as before. */
#define MT_CHIMNEY_LAYOUT 136
static bool ChimneySouthCell(int worldX, int worldY, int *lx, int *ly)
{
    static int sBare = -1;
    const VoxelMapInstance *m = NULL;

    if (sBare < 0)
        sBare = getenv("WJPC_CHIMNEY_SOUTH_BARE") != NULL;
    if (sBare)
        return false;
    for (unsigned i = 0; i < sInstanceCount && m == NULL; ++i)
        if (sInstances[i].layoutId == MT_CHIMNEY_LAYOUT && !sInstances[i].indoor && sInstances[i].layout != NULL)
            m = &sInstances[i];
    if (m == NULL)
        return false;
    *lx = worldX - m->originX;
    *ly = worldY - (m->originY + m->height);
    return *ly >= 0 && *ly <= 28 && *lx >= -8 && *lx < m->width + 8;
}

/* TRUE in the land south of Mt. Chimney that carries its scattered trees (no other trees placed there: the ledge
 * trees of the General's, voxel_mesh_builder.c VoxelMesh_LedgeTree) */
bool VoxelWorld_ChimneySouth(int worldX, int worldY)
{
    int lx, ly;
    return ChimneySouthCell(worldX, worldY, &lx, &ly);
}

static int ChimneySouthTree(int worldX, int worldY);
/* the ash tree there (Lavaridge 2CD) or -1: a tree standing on the land as it is (VoxelMesh_LedgeTree places it on the
 * field's shelf, voxel_tree.c draws it), the land itself unchanged - laid as a wood of the belt, its floor and its
 * walls opened slivers of sky down the slopes south of the map */
int VoxelWorld_ChimneySouthTree(int worldX, int worldY)
{
    if (VoxelWorld_GetInstanceAt(worldX, worldY) != NULL)
        return -1;
    return ChimneySouthTree(worldX, worldY);
}

static int ChimneySouthTree(int worldX, int worldY)
{
    int lx, ly, side, w = 40;
    unsigned h;
    float p;

    if (!ChimneySouthCell(worldX, worldY, &lx, &ly))
        return -1;
    for (unsigned i = 0; i < sInstanceCount; ++i)
        if (sInstances[i].layoutId == MT_CHIMNEY_LAYOUT)
            w = sInstances[i].width;
    side = lx < 0 ? -lx : lx >= w ? lx - w + 1 : 0;
    /* (thick along the map's own wood, then scattered: "scatter some trees") */
    p = ly == 0 ? 0.7f : ly == 1 ? 0.55f : ly < 8 ? 0.55f - (float)(ly - 1) * 0.065f : 0.08f;
    p *= 1.0f - (float)side / 9.0f;
    h = (unsigned)worldX * 73856093u ^ (unsigned)worldY * 19349663u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return (float)(h & 0xFFFF) / 65536.0f < p ? 0x2CD : -1;
}

static int BorderMetatileUncached(int worldX, int worldY)
{
    static const int kDir[4][2] = { {0, -1}, {1, 0}, {-1, 0}, {0, 1} };
    static int sOld = -1;
    int mt = BorderMetatileRaw(worldX, worldY), part, ox, oy, w;

    if (sOld < 0)
        sOld = getenv("WJPC_BELT_HALF_TREE_GROUND") != NULL;
    if (sOld || mt < 0x1D4 || mt > 0x1FD)
        return mt;
    part = VoxelTree_Part(NULL, mt);
    if (part < 0 || part > 3)
        return mt;
    ox = worldX - (part & 1);
    oy = worldY - (part >> 1);
    for (int k = 0; k < 4; ++k)
    {
        int m = BorderMetatileRaw(ox + (k & 1), oy + (k >> 1));

        if (VoxelWorld_GetInstanceAt(ox + (k & 1), oy + (k >> 1)) != NULL || m < 0 || VoxelTree_Part(NULL, m) != k)
            goto half;
    }
    return mt;
half:
    for (int d = 0; d < 4; ++d)
        if (RawIsWater(worldX + kDir[d][0], worldY + kDir[d][1], &w))
            return w;
    return mt;
}

bool VoxelWorld_BorderIsWater(int worldX, int worldY)
{
    int metatileId = VoxelWorld_BorderMetatile(worldX, worldY);

    return metatileId >= 0 && !sInstances[0].indoor
        && BehaviorShape(UNPACK_BEHAVIOR(GetMetatileAttribute(VoxelWorld_BorderArtMap(worldX, worldY), metatileId)))
           == VOXEL_SHAPE_WATER;
}

static unsigned AddPayload(const void **payloads, unsigned count, unsigned max, const void *ptr)
{
    if (ptr == NULL || count >= max)
        return count;
    for (unsigned i = 0; i < count; ++i)
        if (payloads[i] == ptr)
            return count;
    payloads[count] = ptr;
    return count + 1;
}

static unsigned AddMapPayloads(const void **payloads, unsigned count, unsigned max,
                               const struct MapLayout *layout)
{
    const struct Tileset *tilesets[2];

    if (layout == NULL)
        return count;
    tilesets[0] = layout->primaryTileset;
    tilesets[1] = layout->secondaryTileset;
    count = AddPayload(payloads, count, max, layout->border);
    for (unsigned t = 0; t < 2; ++t)
    {
        if (tilesets[t] == NULL)
            continue;
        count = AddPayload(payloads, count, max, tilesets[t]->metatileAttributes);
        count = AddPayload(payloads, count, max, tilesets[t]->metatiles);
        count = AddPayload(payloads, count, max, tilesets[t]->palettes);
        count = AddPayload(payloads, count, max, tilesets[t]->tiles);
    }
    return count;
}

/*
 * The pictures of a map's people. The game spawns an object event as the
 * camera comes within reach of it and reads its picture then: in a town full
 * of them, walking was one read off the card after another, each a frame the
 * CPU came back to too late. The first frame's picture holds them all.
 */
static unsigned AddObjectPayloads(const void **payloads, unsigned count, unsigned max,
                                  const struct MapHeader *header)
{
    const struct MapEvents *events = header != NULL ? header->events : NULL;

    if (events == NULL || events->objectEvents == NULL)
        return count;
    for (unsigned i = 0; i < events->objectEventCount && count < max; ++i)
    {
        const struct ObjectEventGraphicsInfo *info =
            GetObjectEventGraphicsInfo(events->objectEvents[i].graphicsId);

        if (info != NULL && info->images != NULL)
            count = AddPayload(payloads, count, max, info->images[0].data);
    }
    return count;
}

unsigned VoxelWorld_NearbyPayloads(const void **payloads, unsigned max)
{
    unsigned count = 0;

    /* The current map's, then its people, the maps around it, and theirs. */
    if (sInstanceCount > 0)
    {
        count = AddMapPayloads(payloads, count, max, sInstances[0].layout);
        count = AddObjectPayloads(payloads, count, max, sInstances[0].header);
    }
    for (unsigned i = 1; i < sInstanceCount; ++i)
        count = AddMapPayloads(payloads, count, max, sInstances[i].layout);
    for (unsigned i = 1; i < sInstanceCount; ++i)
        count = AddObjectPayloads(payloads, count, max, sInstances[i].header);
    for (unsigned i = 1; i < sInstanceCount; ++i)
    {
        const struct MapHeader *header = sInstances[i].header;
        const struct MapConnection *conn;

        if (header == NULL || header->connections == NULL)
            continue;
        conn = header->connections->connections;
        for (s32 c = 0; c < header->connections->count; ++c, ++conn)
        {
            const struct MapHeader *next;

            if (conn->direction == CONNECTION_DIVE || conn->direction == CONNECTION_EMERGE)
                continue;
            next = GetMapHeaderFromConnection(conn);
            if (next != NULL)
                count = AddMapPayloads(payloads, count, max, next->mapLayout);
        }
    }
    return count;
}

uint32_t VoxelWorld_PayloadSignature(void)
{
    uint32_t hash = 2166136261u;
    for (unsigned i = 0; i < sInstanceCount; ++i)
    {
        const struct MapHeader *header = sInstances[i].header;
        const struct MapEvents *events = header != NULL ? header->events : NULL;
        hash = (hash ^ (uint32_t)(uintptr_t)sInstances[i].layout) * 16777619u;
        hash = (hash ^ (uint32_t)(uintptr_t)header) * 16777619u;
        if (events == NULL || events->objectEvents == NULL)
            continue;
        for (unsigned j = 0; j < events->objectEventCount; ++j)
        {
            const struct ObjectEventGraphicsInfo *info =
                GetObjectEventGraphicsInfo(events->objectEvents[j].graphicsId);
            const void *payload = info != NULL && info->images != NULL ? info->images[0].data : NULL;
            hash = (hash ^ (uint32_t)(uintptr_t)payload) * 16777619u;
        }
    }
    return hash;
}
