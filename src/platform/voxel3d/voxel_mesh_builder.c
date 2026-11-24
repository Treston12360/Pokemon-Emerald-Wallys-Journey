#include <stdio.h>
#include <stdlib.h>
/*
 * CPU geometry for the voxel overworld. See voxel_mesh_builder.h.
 *
 * The shapes, heights, face culling rules and the camera-side cutaway are
 * ported from the MIT-licensed src/platform/voxel/voxel_mesh.c of
 * pokeemerald-multiplatform (see NOTICE.md). The immediate-mode emission is
 * replaced by the builder, and the reference's 2048x2048 "wall consumed" array
 * is not ported at all: nothing here remembers what it has already emitted, so
 * a cell can be built without building its neighbours first.
 */

#include "port_log.h"
#include <math.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "voxel_mesh_builder.h"
#include "voxel_atlas.h"
#include "voxel_tree.h"
#include "voxel_lighting.h"
#include "voxel_sign.h"
#include "voxel_building.h"
#include "voxel_relief.h"
#include "voxel_env.h" // cached test switches (last include)

/*
 * Face shades, from the greys the reference gives its untextured side faces.
 * Here every face is textured and the shade multiplies the sample, so the
 * ordering is what matters: a lit top, a bright south face towards the camera,
 * a dark north face away from it.
 */
#define SHADE_TOP VOXEL_FACE_SHADE(1.00f)
#define SHADE_SOUTH VOXEL_FACE_SHADE(0.60f)
#define SHADE_NORTH VOXEL_FACE_SHADE(0.50f)
#define SHADE_WEST VOXEL_FACE_SHADE(0.65f)
#define SHADE_EAST VOXEL_FACE_SHADE(0.55f)
bool VoxelMesh_RailWaterArt(const VoxelMapInstance *inst, int x, int y);
int VoxelMesh_RailFloorArt(const VoxelMapInstance *inst);

/*
 * Solid furniture parts barely vary with facing. The shades above are meant
 * for metatile art, which is bright; a flat colour that is already dark goes
 * to black when multiplied by 0.55, which is how the first build turned every
 * television into a hole in the floor. Enough variation to read as a box, no
 * more.
 */
#define SOLID_LIT VOXEL_FACE_SHADE(1.00f)
#define SOLID_FRONT VOXEL_FACE_SHADE(0.94f)
#define SOLID_BACK VOXEL_FACE_SHADE(0.88f)
#define SOLID_SIDE VOXEL_FACE_SHADE(0.91f)

#include "voxel_thread.h"
/* (the window, its shapes and the builder's caches are per thread: maps are built on a worker too, audit step 4) */
static VOXEL_TLS int sWindowX0, sWindowY0, sWindowW, sWindowH;

/*
 * Classification of each tile in the window, computed at most once per
 * rebuild. 0 means "not yet", otherwise shape + 1.
 *
 * VoxelWorld_ClassifyTile is not cheap - it reads the block, the collision of
 * up to five neighbours and the metatile behaviour, which reaches into the
 * asset layer - and the rebuild asks for the same tile about five times: once
 * for itself, and once as the neighbour of each of its four neighbours when
 * they decide which faces to cull.
 */
static VOXEL_TLS uint8_t sShapes[VOXEL_WINDOW_MAX * VOXEL_WINDOW_MAX];

VoxelVisualShape VoxelMesh_Classify(int x, int y)
{
    int cx = x - sWindowX0, cy = y - sWindowY0;
    unsigned slot;
    VoxelVisualShape shape;

    if (cx < 0 || cx >= sWindowW || cy < 0 || cy >= sWindowH)
        return VoxelWorld_ClassifyTile(x, y);
    slot = (unsigned)(cy * VOXEL_WINDOW_MAX + cx);
    if (sShapes[slot] != 0)
        return (VoxelVisualShape)(sShapes[slot] - 1);
    shape = VoxelWorld_ClassifyTile(x, y);
    sShapes[slot] = (uint8_t)(shape + 1);
    return shape;
}

void VoxelBuilder_SetStretch(VoxelBuilder *builder, const VoxelMapInstance *inst)
{
    if (!VoxelRelief_Stretch(inst, &builder->stretchFrom, &builder->stretchTo, &builder->stretchAdd))
        builder->stretchAdd = 0.0f;
}

float VoxelBuilder_StretchZ(const VoxelBuilder *builder, float z)
{
    if (builder->stretchAdd <= 0.0f || z <= builder->stretchFrom)
        return z + builder->base;
    if (z >= builder->stretchTo)
        return z + builder->stretchAdd + builder->base;
    return builder->stretchFrom + (z - builder->stretchFrom)
         * (builder->stretchTo - builder->stretchFrom + builder->stretchAdd)
         / (builder->stretchTo - builder->stretchFrom) + builder->base;
}

void VoxelBuilder_Init(VoxelBuilder *builder, VoxelVertex *storage, unsigned capacity)
{
    builder->vertices = storage;
    builder->capacity = capacity;
    builder->count = 0;
    builder->dropped = 0;
    builder->atlas = NULL;
    builder->uncovered = 0;
    builder->originX = 0.0f;
    builder->originZ = 0.0f;
    builder->lift = 0.0f;
    builder->shift = 0.0f;
    builder->base = 0.0f;
    builder->stretchFrom = builder->stretchTo = builder->stretchAdd = 0.0f;
    builder->artShaded = false;
    builder->lighting = false;
    builder->lightingRefine = true;
    builder->lightingConstant = -1.0f;
    builder->rounded = false;
    builder->vertexFace = false;
    builder->statics = NULL;
    builder->staticCapacity = 0;
    builder->staticCount = 0;
}

void VoxelBuilder_SetOrigin(VoxelBuilder *builder, int originX, int originZ)
{
    builder->originX = (float)originX;
    builder->originZ = (float)originZ;
}

/* Does not clear `uncovered`: a rebuild walks several instances and several
 * atlases, and the caller wants the total. VoxelBuilder_Init clears it. */
void VoxelBuilder_SetAtlas(VoxelBuilder *builder, const struct VoxelAtlasMap *atlas)
{
    builder->atlas = atlas;
}

void VoxelBuilder_Tri(VoxelBuilder *builder, const VoxelVertex *a,
                      const VoxelVertex *b, const VoxelVertex *c)
{
    if (builder->count + 3 > builder->capacity)
    {
        if (builder->dropped == 0)
            fprintf(stderr, "[3D] builder full: capacity %u vertices, triangles dropped\n", builder->capacity);
        ++builder->dropped;
        return;
    }
    const VoxelVertex *v[3] = { a, b, c };

    for (int i = 0; i < 3; ++i)
    {
        VoxelVertex *out = &builder->vertices[builder->count++];

        *out = *v[i];
        out->x -= builder->originX;
        out->y += builder->lift + builder->base;
        out->z = VoxelBuilder_StretchZ(builder, out->z + builder->shift) - builder->originZ;
    }
}

void VoxelBuilder_Quad(VoxelBuilder *builder, const VoxelVertex *a, const VoxelVertex *b,
                       const VoxelVertex *c, const VoxelVertex *d)
{
    {
        /* test aid: WJPC_QUAD_TRACE="x z y": who lays a quad over world point (x, z) at height y */
        static int sOn = -1;
        static float tx, tz, ty;
        if (sOn < 0)
            sOn = getenv("WJPC_QUAD_TRACE") != NULL && sscanf(getenv("WJPC_QUAD_TRACE"), "%f %f %f", &tx, &tz, &ty) == 3;
        if (sOn)
        {
            float x0 = fminf(fminf(a->x, b->x), fminf(c->x, d->x)), x1 = fmaxf(fmaxf(a->x, b->x), fmaxf(c->x, d->x));
            float z0 = fminf(fminf(a->z, b->z), fminf(c->z, d->z)) + builder->shift;
            float z1 = fmaxf(fmaxf(a->z, b->z), fmaxf(c->z, d->z)) + builder->shift;
            float y = a->y + builder->lift + builder->base;
            float zz = VoxelBuilder_StretchZ(builder, z0) - builder->originZ, zz1 = VoxelBuilder_StretchZ(builder, z1) - builder->originZ;
            if (tx > x0 - builder->originX && tx < x1 - builder->originX && tz > zz && tz < zz1 && fabsf(y - ty) < 0.004f
             && a->y == b->y && b->y == c->y)
                fprintf(stderr, "QUADTRACE %p y %.4f z %.3f-%.3f x %.3f-%.3f\n", __builtin_return_address(0), y, zz, zz1, x0, x1);
        }
    }
#if CTR_VOXEL_LIGHTING
    if (builder->lighting)
    {
        VoxelLighting_Quad(builder, a, b, c, d);
        return;
    }
#endif
    /* Whole or not at all: half a quad is a sliver along its diagonal. */
    if (builder->count + 6 > builder->capacity)
    {
        if (builder->dropped == 0)
            fprintf(stderr, "[3D] builder full: capacity %u vertices, quads dropped\n", builder->capacity);
        builder->dropped += 2;
        return;
    }
    VoxelBuilder_Tri(builder, a, b, c);
    VoxelBuilder_Tri(builder, a, c, d);
}

/* ── Window bookkeeping ─────────────────────────────────────────────────── */

static void WindowReset(int x0, int y0, int x1, int y1)
{
    sWindowX0 = x0;
    sWindowY0 = y0;
    sWindowW = x1 - x0;
    sWindowH = y1 - y0;
    if (sWindowW > VOXEL_WINDOW_MAX) sWindowW = VOXEL_WINDOW_MAX;
    if (sWindowH > VOXEL_WINDOW_MAX) sWindowH = VOXEL_WINDOW_MAX;
    /* Row by row rather than the whole 6.4 KiB: a chunk build opens a
     * rectangle of about 10x18, and clearing the other 6 KiB of a cache it
     * will never touch costs more than the classifications it saves. */
    for (int row = 0; row < sWindowH; ++row)
        memset(&sShapes[row * VOXEL_WINDOW_MAX], 0, (size_t)sWindowW);
}

/* ── UV lookup ──────────────────────────────────────────────────────────── */

/* Returns false when the atlas has no pixels for this tile's metatile; the
 * caller emits nothing, and `uncovered` has already asked for a rebuild. */
/* UVs of an explicit metatile id in the bound atlas. */
static bool MetatileUV(VoxelBuilder *builder, int metatileId,
                       float *u0, float *v0, float *u1, float *v1)
{
    unsigned slot;

    if (metatileId < 0 || metatileId >= (int)VOXEL_METATILE_IDS)
        return false;
    slot = builder->atlas->slotOf[metatileId];
    if (slot == 0 || slot == VOXEL_SLOT_PENDING)
    {
        ++builder->uncovered;
        return false;
    }
    if (slot == VOXEL_SLOT_ABSENT)
        return false;
    VoxelAtlas_SlotUV(slot - 1, u0, v0, u1, v1);
    return true;
}

bool VoxelMesh_MetatileUV(VoxelBuilder *builder, int metatileId,
                          float *u0, float *v0, float *u1, float *v1)
{
    return MetatileUV(builder, metatileId, u0, v0, u1, v1);
}

int VoxelMesh_CableArtSwap(const VoxelMapInstance *inst, int metatile);
bool VoxelMesh_TileUV(VoxelBuilder *builder, int x, int y,
                   float *u0, float *v0, float *u1, float *v1)
{
    unsigned slot;
    int metatile;

    /*
     * Nothing stands on a tile that belongs to no map on screen, and indoors
     * VOID is the black out-of-bounds filler. Asking either for its metatile
     * gives an id the atlas was never built with, which used to be counted as
     * a gap - so a wall whose top reached past the building had the atlas
     * rebuilt, every frame, for geometry that must not be drawn at all.
     */
    if (VoxelMesh_Classify(x, y) == VOXEL_SHAPE_VOID)
        return false;
    metatile = VoxelWorld_GetMetatileId(x, y);
    {
        const VoxelMapInstance *inst = VoxelWorld_GetInstanceAt(x, y);
        if (VoxelWorld_UsesTreeSprites(inst))
            metatile = VoxelTree_GroundMetatile(inst->secondaryTileset, metatile);
        /* Mt Pyre outside (Brennan 2026-11-19 22:30, the grass square on the rebuilt east corner: "change it to
         * rock"): drawn in 3D as the rock round it (General 0x071); the map itself (2D, walking) is unchanged.
         * WJPC_NO_ART_SWAP=1: as drawn. */
        /* Mt Pyre summit (Brennan 2026-11-20 12:30, the green square on the ridge end east of the upper stairs:
         * "make it rock"): the ridge's west-end cells (27,15) 068 and (27,16) 078 drawn as the ridge's own rock. */
        static const struct { uint16_t layout; uint8_t x, y; uint16_t as; } kArtSwap[] = {
            { 302, 31, 23, 0x071 },
            { 303, 27, 15, 0x069 },
            { 303, 27, 16, 0x07C },
            /* Meteor Falls 1F (Brennan 2026-11-23 06:28, the dark patches by the bridge's east end: "Try redrawing
             * them to plain lighter rock"): the dark diagonal rock (0x215, 0x228, 0x218) drawn as its plain rock 0x23C */
            { 125, 23, 13, 0x23C }, { 125, 22, 14, 0x23C }, { 125, 23, 14, 0x23C }, { 125, 22, 15, 0x23C },
            { 125, 23, 15, 0x23C }, { 125, 22, 16, 0x23C }, { 125, 23, 16, 0x23C }, { 125, 22, 17, 0x23C },
            /* Route 133 (Brennan 2026-11-23 17:17, the left rock's north edge under the round boulder: blue slivers
             * "still persist"): the rock's north-edge art (0x157) carries a strip of sea along its top, seen as a blue
             * line where the boulder stands behind instead of the sea - drawn as the plain rock top (0x0DF) there */
            { 49, 15, 8, 0x0DF }, { 49, 16, 8, 0x0DF }, { 49, 17, 8, 0x0DF }, { 49, 18, 8, 0x0DF }, { 49, 19, 8, 0x0DF },
            { 49, 20, 8, 0x0DF },
            { 49, 19, 7, 0x15E }, { 49, 20, 7, 0x157 },   /* (the rock's north-east corner one cell west: relief.bin) */
        };
        static int sNoSwap = -1;
        if (sNoSwap < 0)
            sNoSwap = getenv("WJPC_NO_ART_SWAP") != NULL;
        if (!sNoSwap && VoxelMesh_RailWaterArt(inst, x, y))
            metatile = VoxelMesh_RailFloorArt(inst);   /* the river / grass under the 3D rails (EmitRailCell) */
        /* the plain rock under the cable car's drawn line and posts (the 3D ones, EmitCableCell) - if the atlas has it */
        {
            int plain = VoxelMesh_CableArtSwap(inst, metatile);
            if (plain != metatile && builder->atlas->slotOf[plain] != 0 && builder->atlas->slotOf[plain] != VOXEL_SLOT_PENDING
             && builder->atlas->slotOf[plain] != VOXEL_SLOT_ABSENT)
                metatile = plain;
        }
        for (unsigned i = 0; inst != NULL && !sNoSwap && i < sizeof(kArtSwap) / sizeof(kArtSwap[0]); ++i)
            if (inst->layoutId == kArtSwap[i].layout && x - inst->originX == kArtSwap[i].x
             && y - inst->originY == kArtSwap[i].y)
                metatile = kArtSwap[i].as;
    }
    slot = builder->atlas->slotOf[metatile];

    if (slot == 0 || slot == VOXEL_SLOT_PENDING)
    {
        ++builder->uncovered;
        return false;
    }
    if (slot == VOXEL_SLOT_ABSENT)
        return false;
    VoxelAtlas_SlotUV(slot - 1, u0, v0, u1, v1);
    return true;
}

/* ── Face emission ──────────────────────────────────────────────────────── */

/* Horizontal face at height `h`, spanning the tile inset from its edges. */
void VoxelMesh_Top(VoxelBuilder *b, float wx, float wz, float h, float inset,
                    float u0, float v0, float u1, float v1, float shade)
{
    float x0 = wx + inset, x1 = wx + 1.0f - inset;
    float z0 = wz + inset, z1 = wz + 1.0f - inset;

    VoxelBuilder_Quad(b,
        &(VoxelVertex){x0, h, z0, u0, v0, shade},
        &(VoxelVertex){x1, h, z0, u1, v0, shade},
        &(VoxelVertex){x1, h, z1, u1, v1, shade},
        &(VoxelVertex){x0, h, z1, u0, v1, shade});
}

void VoxelMesh_South(VoxelBuilder *b, float wx, float wz, float yBottom, float yTop,
                      float inset, float u0, float v0, float u1, float v1)
{
    float x0 = wx + inset, x1 = wx + 1.0f - inset, z = wz + 1.0f - inset;

    VoxelBuilder_Quad(b,
        &(VoxelVertex){x0, yBottom, z, u0, v1, SHADE_SOUTH},
        &(VoxelVertex){x1, yBottom, z, u1, v1, SHADE_SOUTH},
        &(VoxelVertex){x1, yTop,    z, u1, v0, SHADE_SOUTH},
        &(VoxelVertex){x0, yTop,    z, u0, v0, SHADE_SOUTH});
}

void VoxelMesh_North(VoxelBuilder *b, float wx, float wz, float yBottom, float yTop,
                      float inset, float u0, float v0, float u1, float v1)
{
    float x0 = wx + inset, x1 = wx + 1.0f - inset, z = wz + inset;

    VoxelBuilder_Quad(b,
        &(VoxelVertex){x1, yBottom, z, u0, v1, SHADE_NORTH},
        &(VoxelVertex){x0, yBottom, z, u1, v1, SHADE_NORTH},
        &(VoxelVertex){x0, yTop,    z, u1, v0, SHADE_NORTH},
        &(VoxelVertex){x1, yTop,    z, u0, v0, SHADE_NORTH});
}

void VoxelMesh_West(VoxelBuilder *b, float wx, float wz, float yBottom, float yTop,
                     float inset, float u0, float v0, float u1, float v1)
{
    float x = wx + inset, z0 = wz + inset, z1 = wz + 1.0f - inset;

    VoxelBuilder_Quad(b,
        &(VoxelVertex){x, yBottom, z0, u0, v1, SHADE_WEST},
        &(VoxelVertex){x, yBottom, z1, u1, v1, SHADE_WEST},
        &(VoxelVertex){x, yTop,    z1, u1, v0, SHADE_WEST},
        &(VoxelVertex){x, yTop,    z0, u0, v0, SHADE_WEST});
}

void VoxelMesh_East(VoxelBuilder *b, float wx, float wz, float yBottom, float yTop,
                     float inset, float u0, float v0, float u1, float v1)
{
    float x = wx + 1.0f - inset, z0 = wz + inset, z1 = wz + 1.0f - inset;

    VoxelBuilder_Quad(b,
        &(VoxelVertex){x, yBottom, z1, u0, v1, SHADE_EAST},
        &(VoxelVertex){x, yBottom, z0, u1, v1, SHADE_EAST},
        &(VoxelVertex){x, yTop,    z0, u1, v0, SHADE_EAST},
        &(VoxelVertex){x, yTop,    z1, u0, v0, SHADE_EAST});
}

/* ── Single tiles ───────────────────────────────────────────────────────── */

/* Height of the block a shape is drawn as, or 0 for a flat tile.
 * The values are the reference's. Terrain has none: what stands
 * up outdoors is modelled or read off its drawing, never raised from here.
 * Water lies flush with the ground: recessed, its rim drew a line along every
 * shore that the art, which already draws its own bank, does not have. */
static float ShapeHeight(VoxelVisualShape shape)
{
    switch (shape)
    {
    case VOXEL_SHAPE_DECAL:   return 0.02f; /* rugs and mats, just off the floor */
    case VOXEL_SHAPE_COUNTER: return 0.70f;
    default:                  return 0.0f;
    }
}

/* ── Furniture ──────────────────────────────────────────────────────────── */

/* A box of one flat colour, inset from the tile edges. Used for the parts the
 * reference draws untextured: table sides and legs, television bodies. */
static void EmitSolidBox(VoxelBuilder *b, float wx, float wz, VoxelSolidColor color,
                         float x0, float z0, float x1, float z1, float yBottom, float yTop)
{
    float u0, v0, u1, v1;

    /* Written out rather than routed through the face helpers, which span a
     * whole tile; a leg is a tenth of one. */
    VoxelAtlas_SolidUV(color, &u0, &v0, &u1, &v1);
    VoxelBuilder_Quad(b,
        &(VoxelVertex){wx + x0, yTop, wz + z0, u0, v0, SOLID_LIT},
        &(VoxelVertex){wx + x1, yTop, wz + z0, u1, v0, SOLID_LIT},
        &(VoxelVertex){wx + x1, yTop, wz + z1, u1, v1, SOLID_LIT},
        &(VoxelVertex){wx + x0, yTop, wz + z1, u0, v1, SOLID_LIT});
    /* South */
    VoxelBuilder_Quad(b,
        &(VoxelVertex){wx + x0, yBottom, wz + z1, u0, v1, SOLID_FRONT},
        &(VoxelVertex){wx + x1, yBottom, wz + z1, u1, v1, SOLID_FRONT},
        &(VoxelVertex){wx + x1, yTop,    wz + z1, u1, v0, SOLID_FRONT},
        &(VoxelVertex){wx + x0, yTop,    wz + z1, u0, v0, SOLID_FRONT});
    /* North */
    VoxelBuilder_Quad(b,
        &(VoxelVertex){wx + x1, yBottom, wz + z0, u0, v1, SOLID_BACK},
        &(VoxelVertex){wx + x0, yBottom, wz + z0, u1, v1, SOLID_BACK},
        &(VoxelVertex){wx + x0, yTop,    wz + z0, u1, v0, SOLID_BACK},
        &(VoxelVertex){wx + x1, yTop,    wz + z0, u0, v0, SOLID_BACK});
    /* West */
    VoxelBuilder_Quad(b,
        &(VoxelVertex){wx + x0, yBottom, wz + z0, u0, v1, SOLID_SIDE},
        &(VoxelVertex){wx + x0, yBottom, wz + z1, u1, v1, SOLID_SIDE},
        &(VoxelVertex){wx + x0, yTop,    wz + z1, u1, v0, SOLID_SIDE},
        &(VoxelVertex){wx + x0, yTop,    wz + z0, u0, v0, SOLID_SIDE});
    /* East */
    VoxelBuilder_Quad(b,
        &(VoxelVertex){wx + x1, yBottom, wz + z1, u0, v1, SOLID_SIDE},
        &(VoxelVertex){wx + x1, yBottom, wz + z0, u1, v1, SOLID_SIDE},
        &(VoxelVertex){wx + x1, yTop,    wz + z0, u1, v0, SOLID_SIDE},
        &(VoxelVertex){wx + x1, yTop,    wz + z1, u0, v0, SOLID_SIDE});
}

/*
 * Furniture: VoxelWorld_ClassifyTile announces televisions only (TABLE, BED
 * and DECAL are kept in the enum for the scene dumps but never produced).
 */
static void EmitFurniture(VoxelBuilder *builder, int x, int y,
                          float u0, float v0, float u1, float v1)
{
    float wx = (float)x, wz = (float)y;
    /* A television: a stand, a screen facing south, and a dark body. */
    const float stand = 0.3f, top = 1.0f;

    EmitSolidBox(builder, wx, wz, VOXEL_SOLID_GREY,
                 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, stand);
    EmitSolidBox(builder, wx, wz, VOXEL_SOLID_BLACK,
                 0.0f, 0.3f, 1.0f, 0.8f, stand, top);
    /* The metatile's own art is the screen. */
    VoxelBuilder_Quad(builder,
        &(VoxelVertex){wx,        stand, wz + 0.79f, u0, v1, SHADE_SOUTH},
        &(VoxelVertex){wx + 1.0f, stand, wz + 0.79f, u1, v1, SHADE_SOUTH},
        &(VoxelVertex){wx + 1.0f, top,   wz + 0.79f, u1, v0, SHADE_SOUTH},
        &(VoxelVertex){wx,        top,   wz + 0.79f, u0, v0, SHADE_SOUTH});
}

/* A sign or a potted plant: the metatile stood upright across the tile. */
static void EmitUpright(VoxelBuilder *builder, int x, int y,
                        float u0, float v0, float u1, float v1)
{
    float wx = (float)x, wz = (float)y + 0.5f;
    const float height = 1.0f;

    VoxelBuilder_Quad(builder,
        &(VoxelVertex){wx,        0.0f,   wz, u0, v1, SHADE_TOP},
        &(VoxelVertex){wx + 1.0f, 0.0f,   wz, u1, v1, SHADE_TOP},
        &(VoxelVertex){wx + 1.0f, height, wz, u1, v0, SHADE_TOP},
        &(VoxelVertex){wx,        height, wz, u0, v0, SHADE_TOP});
}

static void EmitTile(VoxelBuilder *builder, int x, int y, VoxelVisualShape shape)
{
    float wx = (float)x, wz = (float)y;
    float u0, v0, u1, v1;
    float h = ShapeHeight(shape);

    if (!VoxelMesh_TileUV(builder, x, y, &u0, &v0, &u1, &v1))
        return;

    switch (shape)
    {
    case VOXEL_SHAPE_FURNITURE:
        EmitFurniture(builder, x, y, u0, v0, u1, v1);
        return;
    case VOXEL_SHAPE_SIGN:
        EmitUpright(builder, x, y, u0, v0, u1, v1);
        return;
    default:
        break;
    }

    VoxelMesh_Top(builder, wx, wz, h, 0.0f, u0, v0, u1, v1, SHADE_TOP);
    if (h == 0.0f)
        return;

    {
        VoxelVisualShape south = VoxelMesh_Classify(x, y + 1);
        VoxelVisualShape north = VoxelMesh_Classify(x, y - 1);
        VoxelVisualShape west = VoxelMesh_Classify(x - 1, y);
        VoxelVisualShape east = VoxelMesh_Classify(x + 1, y);

        /* A face is hidden by the void and by a neighbour of the same height. */
        if (south != VOXEL_SHAPE_VOID && south != shape)
            VoxelMesh_South(builder, wx, wz, 0.0f, h, 0.0f, u0, v0, u1, v1);
        if (north != VOXEL_SHAPE_VOID && north != shape)
            VoxelMesh_North(builder, wx, wz, 0.0f, h, 0.0f, u0, v0, u1, v1);
        if (west != VOXEL_SHAPE_VOID && west != shape)
            VoxelMesh_West(builder, wx, wz, 0.0f, h, 0.0f, u0, v0, u1, v1);
        if (east != VOXEL_SHAPE_VOID && east != shape)
            VoxelMesh_East(builder, wx, wz, 0.0f, h, 0.0f, u0, v0, u1, v1);
    }
}

/*
 * A lifted cell: its own metatile laid on its relief lattice, every point of
 * the drawing at (u, h, v + h). Level cells need one quad; slopes take the
 * 4-pixel lattice. Shade 1: the drawing's own light is already in it.
 */
#if CTR_VOXEL_LIGHTING
/*
 * The sun's face term at lattice point (a, c) of cell (x, y), from the slope
 * a tile across: two points either side, reaching into the neighbouring
 * cells. The lattice follows the drawing pixel by pixel - a ledge's berm
 * rises 3 px on one row and 2 on the next - and a quad's own normal turned
 * that into stripes a quarter of a tile apart wherever a slope faces away
 * from the sun. The shape of a mountain or a berm is a tile or more across;
 * over an even span the alternation cancels out and the shape stays.
 *
 * Points (u, h, v + h): the surface's tangents are (1, hu, hu) and
 * (0, hv, 1 + hv), so its upward normal is (-hu, 1 + hv, -hv).
 */
static float ReliefHeight(const int16_t *const cells[3][3], int a, int c)
{
    const int n = VOXEL_RELIEF_SIDE - 1;
    int cx = a < 0 ? 0 : a > n ? 2 : 1, cz = c < 0 ? 0 : c > n ? 2 : 1;
    const int16_t *g = cells[cz][cx];

    if (g == NULL)
        return 0.0f;
    a -= (cx - 1) * n;
    c -= (cz - 1) * n;
    return (float)g[c * VOXEL_RELIEF_SIDE + a];
}

static float ReliefFace(const int16_t *const cells[3][3], int a, int c)
{
    /* pixels over four lattice steps (a tile): tiles per tile */
    float hu = (ReliefHeight(cells, a + 2, c) - ReliefHeight(cells, a - 2, c)) / 16.0f;
    float hv = (ReliefHeight(cells, a, c + 2) - ReliefHeight(cells, a, c - 2)) / 16.0f;

    return VoxelLighting_Face(-hu, 1.0f + hv, -hv);
}
#endif

/* A cut tile's flat ground, on the relief's own lattice: every edge it
 * shares with a relief cell beside it has that cell's points on it, so no
 * T-junction opens a crack between them. */
static bool OnFloor(const int16_t *g, int i, int j, int floor)
{
    return g[j * VOXEL_RELIEF_SIDE + i] <= floor && g[j * VOXEL_RELIEF_SIDE + i + 1] <= floor
        && g[(j + 1) * VOXEL_RELIEF_SIDE + i] <= floor && g[(j + 1) * VOXEL_RELIEF_SIDE + i + 1] <= floor;
}

/* Where a cut tile's ground runs on under the rock beside it. */
static const int8_t kCutFill[][2] = { { 0, 1 }, { -1, 1 }, { 1, 1 }, { -1, 0 }, { 1, 0 } };

/* A cut tile's drawing, flat at its foot (the builder's lift), in the
 * relief's own grid: its quads meet the relief's at every lattice point and
 * leave no crack between them. */
/* Does the 4x4 pixel block (i, j) of a cut tile have rock in it? `rows` are
 * the tile's background bits (VoxelRelief_CutVariant): a clear bit is rock. */
static int CutRockPixels(const uint8_t *rows, int i, int j)
{
    int rock = 0;

    for (int py = 4 * j; py < 4 * j + 4; ++py)
    {
        unsigned bits = (unsigned)rows[2 * py] | ((unsigned)rows[2 * py + 1] << 8);

        for (int px = 4 * i; px < 4 * i + 4; ++px)
            if (!((bits >> px) & 1u))
                ++rock;
    }
    return rock;
}

/*
 * A cut tile's flat layer, at its foot. Where a block of the drawing has rock
 * in it the layer is `gu0..` - the plain ground behind it - and where it is all
 * background, the tile's own drawing (a shore's foam, a coast's edge): the
 * rock the drawing carries, flat at the foot, was a second copy of the lifted
 * rock, which the camera's perspective moves off it - a wedge stuck to the
 * ground beside the mountain. With no rows or no ground, the whole tile.
 */
static void EmitCutGround(VoxelBuilder *b, int x, int y, float u0, float v0, float u1, float v1,
                          const uint8_t *rows, const float *gnd, const int16_t *g, int floor)
{
    const int n = VOXEL_RELIEF_SIDE - 1;
    int lo = floor, hi = INT16_MIN;
    /* a mountain's rock, a level or more of it over the foot; a ledge's lip
     * keeps its whole drawing flat under it, as it always had. Over the
     * foot, not over the cell's own lowest point: a rim or a band's corner
     * lifted whole (its lattice flat a level up) left its rock drawn again
     * flat at the foot, seen past the rim as the mountain painted on the
     * ground behind it */
    bool mountain;

    for (int k = 0; g != NULL && k < VOXEL_RELIEF_SIDE * VOXEL_RELIEF_SIDE; ++k)
    {
        if (g[k] < lo) lo = g[k];
        if (g[k] > hi) hi = g[k];
    }
    mountain = g != NULL && hi - lo >= 16;
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < n; ++i)
        {
            float xa = x + i / (float)n, xb = x + (i + 1) / (float)n;
            float za = y + j / (float)n, zb = y + (j + 1) / (float)n;
            float ua, ub, va, vb;
            int rock = rows != NULL ? CutRockPixels(rows, i, j) : 0;

            /* a block that is mostly rock lying on the foot stays its own
             * drawing: it is the flank itself there (EmitRelief leaves it to
             * this layer). Anywhere the rock is lifted, its copy flat at the
             * foot is a second outline beside the lifted one - a row of grey
             * teeth along a flank's foot -, and the ground is drawn */
            if (rows != NULL && gnd != NULL && rock > 0
             && (rock < 12 || (mountain && !OnFloor(g, i, j, floor))))
            {
                ua = gnd[0] + (gnd[2] - gnd[0]) * i / n; ub = gnd[0] + (gnd[2] - gnd[0]) * (i + 1) / n;
                va = gnd[1] + (gnd[3] - gnd[1]) * j / n; vb = gnd[1] + (gnd[3] - gnd[1]) * (j + 1) / n;
            }
            else
            {
                ua = u0 + (u1 - u0) * i / n; ub = u0 + (u1 - u0) * (i + 1) / n;
                va = v0 + (v1 - v0) * j / n; vb = v0 + (v1 - v0) * (j + 1) / n;
            }

            VoxelBuilder_Quad(b, &(VoxelVertex){xa, 0.0f, za, ua, va, SHADE_TOP},
                              &(VoxelVertex){xb, 0.0f, za, ub, va, SHADE_TOP},
                              &(VoxelVertex){xb, 0.0f, zb, ub, vb, SHADE_TOP},
                              &(VoxelVertex){xa, 0.0f, zb, ua, vb, SHADE_TOP});
        }
}

/* A cut tile cut along its diagonals (VoxelRelief_CutDiagonals). */
typedef VoxelCutDiagonals CutDiagonals;

/* A triangle of the cell, its vertices at pixels (X, Y) of the tile, cut by
 * the diagonals: what is left drawn as quads (a fan of the convex piece),
 * an odd one left over as a triangle. */
static void EmitCutTri(VoxelBuilder *b, const VoxelVertex *v0, const VoxelVertex *v1,
                       const VoxelVertex *v2, const float px[3][2], const CutDiagonals *cut)
{
    VoxelVertex poly[2][3 + 4];
    float pp[2][3 + 4][2];
    int n = 3, cur = 0;

    poly[0][0] = *v0; poly[0][1] = *v1; poly[0][2] = *v2;
    for (int i = 0; i < 3; ++i)
    {
        pp[0][i][0] = px[i][0];
        pp[0][i][1] = px[i][1];
    }
    for (int k = 0; k < cut->count && n > 0; ++k)
    {
        int m = 0, nxt = cur ^ 1;

        for (int i = 0; i < n; ++i)
        {
            int j = (i + 1) % n;
            float di = cut->a[k] * pp[cur][i][0] + cut->b[k] * pp[cur][i][1] - cut->c[k];
            float dj = cut->a[k] * pp[cur][j][0] + cut->b[k] * pp[cur][j][1] - cut->c[k];

            if (di <= 0.0f)
            {
                poly[nxt][m] = poly[cur][i];
                pp[nxt][m][0] = pp[cur][i][0];
                pp[nxt][m][1] = pp[cur][i][1];
                ++m;
            }
            if ((di < 0.0f && dj > 0.0f) || (di > 0.0f && dj < 0.0f))
            {
                float t = di / (di - dj);
                const VoxelVertex *p = &poly[cur][i], *q = &poly[cur][j];

                poly[nxt][m] = (VoxelVertex){ p->x + (q->x - p->x) * t, p->y + (q->y - p->y) * t,
                                              p->z + (q->z - p->z) * t, p->u + (q->u - p->u) * t,
                                              p->v + (q->v - p->v) * t,
                                              p->shade + (q->shade - p->shade) * t };
                pp[nxt][m][0] = pp[cur][i][0] + (pp[cur][j][0] - pp[cur][i][0]) * t;
                pp[nxt][m][1] = pp[cur][i][1] + (pp[cur][j][1] - pp[cur][i][1]) * t;
                ++m;
            }
        }
        n = m;
        cur = nxt;
    }
    for (int i = 1; i + 1 < n; i += 2)
    {
        if (i + 2 < n)
            VoxelBuilder_Quad(b, &poly[cur][0], &poly[cur][i], &poly[cur][i + 1], &poly[cur][i + 2]);
        else
        {
#if CTR_VOXEL_LIGHTING
            if (b->lighting)
                VoxelLighting_Tri(b, &poly[cur][0], &poly[cur][i], &poly[cur][i + 1]);
            else
#endif
                VoxelBuilder_Tri(b, &poly[cur][0], &poly[cur][i], &poly[cur][i + 1]);
        }
    }
}

/* A quad of the cell (p0..p3 at pixels px), split into two triangles as
 * VoxelBuilder_Quad splits it, each cut by the diagonals; whole when none
 * of them reaches it. */
static void EmitCutQuad(VoxelBuilder *b, const VoxelVertex *p0, const VoxelVertex *p1,
                        const VoxelVertex *p2, const VoxelVertex *p3, const float px[4][2],
                        const CutDiagonals *cut)
{
    bool inside = true, outside = false;

    for (int k = 0; k < cut->count; ++k)
    {
        int in = 0;

        for (int i = 0; i < 4; ++i)
            in += cut->a[k] * px[i][0] + cut->b[k] * px[i][1] <= cut->c[k];
        inside = inside && in == 4;
        outside = outside || in == 0;
    }
    if (outside)
        return;
    if (inside)
    {
        VoxelBuilder_Quad(b, p0, p1, p2, p3);
        return;
    }
    {
        const float t0[3][2] = { { px[0][0], px[0][1] }, { px[1][0], px[1][1] }, { px[2][0], px[2][1] } };
        const float t1[3][2] = { { px[0][0], px[0][1] }, { px[2][0], px[2][1] }, { px[3][0], px[3][1] } };

        EmitCutTri(b, p0, p1, p2, t0, cut);
        EmitCutTri(b, p0, p2, p3, t1, cut);
    }
}

/*
 * Ground run on under cell (x, y) at the builder's lift: a quad two pixels
 * past the cell, so that no ray between it and the floors beside it - a
 * pixel or two higher - finds the clear colour. Past a side where the cell
 * beyond is lower than the run on within those two pixels (a band falling
 * away from it) it stops at the cell's edge: past it, it stood over the
 * band's foot as a slit of ground along the joint.
 */
static void EmitRunOn(VoxelBuilder *b, const VoxelMapInstance *inst, int x, int y,
                      float u0, float v0, float u1, float v1)
{
    static const int8_t kSide[4][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
    const int n = VOXEL_RELIEF_SIDE;
    const float m = 2.0f / 16.0f, level = b->lift * 16.0f;
    float pad[4];

    for (int k = 0; k < 4; ++k)
    {
        int nx = x + kSide[k][0], ny = y + kSide[k][1];
        const int16_t *g;
        float low = 0.0f;

        pad[k] = m;
        if (VoxelWorld_GetInstanceAt(nx, ny) != inst)
            continue;
        g = VoxelRelief_Cell(inst, nx, ny);
        if (g != NULL)
        {
            low = INT16_MAX;
            for (int t = 0; t < n; ++t)
            {
                /* the neighbour's edge facing this cell, and its points a
                 * lattice step in: two pixels in is half way */
                int edge, in;

                if (k < 2)
                {
                    int a = k == 0 ? n - 1 : 0;
                    edge = t * n + a;
                    in = t * n + (k == 0 ? a - 1 : a + 1);
                }
                else
                {
                    int c = k == 2 ? n - 1 : 0;
                    edge = c * n + t;
                    in = (k == 2 ? c - 1 : c + 1) * n + t;
                }
                if (g[edge] < low) low = g[edge];
                if ((g[edge] + g[in]) * 0.5f < low) low = (g[edge] + g[in]) * 0.5f;
            }
        }
        if (low < level - 0.01f)
            pad[k] = 0.0f;
    }
    VoxelBuilder_Quad(b, &(VoxelVertex){x - pad[0], 0.0f, y - pad[2], u0, v0, SHADE_TOP},
                      &(VoxelVertex){x + 1 + pad[1], 0.0f, y - pad[2], u1, v0, SHADE_TOP},
                      &(VoxelVertex){x + 1 + pad[1], 0.0f, y + 1 + pad[3], u1, v1, SHADE_TOP},
                      &(VoxelVertex){x - pad[0], 0.0f, y + 1 + pad[3], u0, v1, SHADE_TOP});
}

/* `metatile` >= 0 draws the cell with that atlas id (a cut tile's). With
 * `onFloor`, every quad lying on `floor` (pixels) is left out: the ground
 * drawn flat under it is that quad already, and the two would fight for
 * its depth. `rows`: the cut tile's background bits, cut along its
 * diagonals where it is a polygon (VoxelRelief_CutDiagonals). */
static void EmitReliefUV(VoxelBuilder *b, int x, int y, const int16_t *g, const int16_t *s,
                         float u0, float v0, float u1, float v1,
                         bool onFloor, int floor, const uint8_t *rows);

/*
 * Brennan 2026-11-24 13:14 (Shoal Cave: "The stairs ... don't look 3D ... Check and make sure stairs are 3D and that
 * they are accessible to go up and down them"): a cave's flight of stairs (the Cave tileset's 0x204, Shoal Cave's
 * icy 0x363) on its raised rock. Laid as its relief, every point as far south as it is high, a flight a rock's height
 * up one cell is an upright wall with the steps drawn on it. Here it is real steps: one a quarter of its rise (4 px
 * of a 16 px climb), tread and riser, from the floor at its top to the floor at its foot; where that is steeper than
 * the player's path (one cell up per cell along), its foot runs on over the floor below, half a cell. The drawing
 * is laid on it by the 45-degree screen key (z - y): its drawn steps where the real ones are. Treads lit full, risers
 * a little darker, the flight's sides darker still. WJPC_CAVE_STAIRS_FLAT=1: the relief's wall, as before.
 */
extern const struct Tileset gTileset_Cave;

static bool CaveStairCell(int x, int y, const VoxelMapInstance **out)
{
    static int sOff = -1;
    const VoxelMapInstance *inst;
    int m;

    if (sOff < 0)
        sOff = getenv("WJPC_CAVE_STAIRS_FLAT") != NULL;
    if (sOff || (inst = VoxelWorld_GetInstanceAt(x, y)) == NULL || !inst->underground
     || inst->secondaryTileset != (const void *)&gTileset_Cave)
        return false;
    m = VoxelWorld_GetMetatileId(x, y);
    *out = inst;
    return m == 0x204 || m == 0x363;
}

static void StairVertex(VoxelVertex *v, float x, float h, float z, float u, float q0, float q1,
                        float v0, float v1, float shade)
{
    float q = z - h, t = q1 > q0 ? (q - q0) / (q1 - q0) : 0.0f;

    t = t < 0.0f ? 0.0f : t > 1.0f ? 1.0f : t;
    *v = (VoxelVertex){ x, h, z, u, v0 + (v1 - v0) * t, shade + 4.0f };
}

static bool EmitCaveStairs(VoxelBuilder *b, const VoxelMapInstance *inst, int x, int y, const int16_t *g,
                           const int16_t *s, float u0, float v0, float u1, float v1)
{
    const int n5 = VOXEL_RELIEF_SIDE;
    float hn = g[2] / 16.0f, hs = g[(n5 - 1) * n5 + 2] / 16.0f;
    float zn = y + s[2] / 16.0f, zs = y + 1.0f + s[(n5 - 1) * n5 + 2] / 16.0f;
    float rise = hn - hs, run, q0, q1;
    int steps;
    bool lighting = b->lighting;

    if (rise < 2.0f / 16.0f)
        return false;
    run = zs - zn;
    /* steeper than the path: on over the floor at its foot, if there is floor there */
    if (run < rise * 0.5f)
    {
        const int16_t *below = VoxelRelief_Cell(inst, x, y + 1);
        float fb = below != NULL ? below[2] / 16.0f : 0.0f;

        if (fabsf(fb - hs) < 1.0f / 16.0f)
            run = rise * 0.5f;
    }
    if (run < 1.0f / 16.0f)
        return false;
    zs = zn + run;
    steps = (int)lroundf(rise * 4.0f);
    steps = steps < 1 ? 1 : steps;
    q0 = zn - hn;
    q1 = zs - hs;
    b->lighting = false;  /* (their own shades: a cave has no sun) */
    for (int k = 0; k < steps; ++k)
    {
        float ht = hn - rise * k / steps, hb = hn - rise * (k + 1) / steps;
        float za = zn + run * k / steps, zb = zn + run * (k + 1) / steps;
        VoxelVertex p[4];

        /* tread */
        StairVertex(&p[0], x, ht, za, u0, q0, q1, v0, v1, 1.0f);
        StairVertex(&p[1], x + 1, ht, za, u1, q0, q1, v0, v1, 1.0f);
        StairVertex(&p[2], x + 1, ht, zb, u1, q0, q1, v0, v1, 1.0f);
        StairVertex(&p[3], x, ht, zb, u0, q0, q1, v0, v1, 1.0f);
        VoxelBuilder_Quad(b, &p[0], &p[1], &p[2], &p[3]);
        /* riser */
        StairVertex(&p[0], x, ht, zb, u0, q0, q1, v0, v1, 0.84f);
        StairVertex(&p[1], x + 1, ht, zb, u1, q0, q1, v0, v1, 0.84f);
        StairVertex(&p[2], x + 1, hb, zb, u1, q0, q1, v0, v1, 0.84f);
        StairVertex(&p[3], x, hb, zb, u0, q0, q1, v0, v1, 0.84f);
        VoxelBuilder_Quad(b, &p[0], &p[1], &p[2], &p[3]);
        /* the sides: the step's block down to the foot's floor, in the flight's edge columns */
        for (int e = 0; e < 2; ++e)
        {
            float sx = (float)(x + e), su = e == 0 ? u0 + (u1 - u0) / 32.0f : u1 - (u1 - u0) / 32.0f;

            StairVertex(&p[0], sx, ht, za, su, q0, q1, v0, v1, 0.68f);
            StairVertex(&p[1], sx, ht, zb, su, q0, q1, v0, v1, 0.68f);
            StairVertex(&p[2], sx, hs, zb, su, q0, q1, v0, v1, 0.68f);
            StairVertex(&p[3], sx, hs, za, su, q0, q1, v0, v1, 0.68f);
            VoxelBuilder_Quad(b, &p[0], &p[1], &p[2], &p[3]);
        }
    }
    b->lighting = lighting;
    return true;
}

static void EmitRelief(VoxelBuilder *b, int x, int y, const int16_t *g, const int16_t *s,
                       int artY, int metatile, bool onFloor, int floor, const uint8_t *rows)
{
    float u0, v0, u1, v1;
    const VoxelMapInstance *stairs = NULL;

    if (metatile >= 0 ? !MetatileUV(b, metatile, &u0, &v0, &u1, &v1)
                      : !VoxelMesh_TileUV(b, x, artY, &u0, &v0, &u1, &v1))
        return;
    if (artY == y && b->lift == 0.0f && CaveStairCell(x, y, &stairs)
     && EmitCaveStairs(b, stairs, x, y, g, s, u0, v0, u1, v1))
        return;
    EmitReliefUV(b, x, y, g, s, u0, v0, u1, v1, onFloor, floor, rows);
}

/* A tree's cell on a slope (voxel_tree.c): the trunk's ground, drawn from
 * the trees' texture at u0..v1, laid on the cell's relief. One on the rock
 * (VoxelMesh_TreeOnRock) is the ground pass's. */
void VoxelMesh_ReliefGround(VoxelBuilder *b, const VoxelMapInstance *inst, int x, int y,
                            float u0, float v0, float u1, float v1)
{
    EmitReliefUV(b, x, y, VoxelRelief_Cell(inst, x, y), VoxelRelief_Depth(inst, x, y),
                 u0, v0, u1, v1, false, 0, NULL);
}

/* Is a tree's cell drawn as the rock its cut record names (`wall` with
 * VOXEL_RELIEF_FACE_TOP, voxel_relief.h): a wood on a mountain's band, the
 * band under the trees? Its relief is then the ground pass's, in the tiles'
 * atlas: laid by the tree pass, drawn with the trees' texture, the band's
 * coordinates came out as stripes of leaves down Route 120's bank. */
bool VoxelMesh_TreeOnRock(const VoxelMapInstance *inst, int x, int y)
{
    float foot;
    int ground, wall = -1;
    unsigned sides = 0;

    return VoxelRelief_Cell(inst, x, y) != NULL
        && VoxelRelief_Cut(inst, x, y, &foot, &ground, &wall, &sides) != -1
        && (sides & VOXEL_RELIEF_FACE_TOP) && wall >= 0 && wall != VOXEL_RELIEF_NO_FACE;
}

/* Does the rectangle of lattice heights (or depths) from point (a0, c0),
 * wa steps across and wc down, lie on one plane, every point where its
 * corners put it? Exact: they are whole pixels. */
static bool SpanFlat(const int16_t *v, int a0, int c0, int wa, int wc)
{
    const int k0 = c0 * VOXEL_RELIEF_SIDE + a0;
    const int du = v[k0 + wa] - v[k0], dv = v[k0 + wc * VOXEL_RELIEF_SIDE] - v[k0];

    for (int c = 0; c <= wc; ++c)
        for (int a = 0; a <= wa; ++a)
            if (wa * wc * v[k0 + c * VOXEL_RELIEF_SIDE + a] != wa * wc * v[k0] + a * wc * du + c * wa * dv)
                return false;
    return true;
}

#if CTR_VOXEL_LIGHTING
/* Do the lattice's shades over that rectangle run as its corners' do - as
 * one quad's triangles draw them - within half a step of the eight bits a
 * colour is drawn in? The face term of a flank bends smoothly into the
 * cells round it. */
static bool SpanLinear(const float *shade, int a0, int c0, int wa, int wc)
{
    const int k0 = c0 * VOXEL_RELIEF_SIDE + a0;
    const float du = shade[k0 + wa] - shade[k0], dv = shade[k0 + wc * VOXEL_RELIEF_SIDE] - shade[k0];

    for (int c = 0; c <= wc; ++c)
        for (int a = 0; a <= wa; ++a)
            if (fabsf(shade[k0 + c * VOXEL_RELIEF_SIDE + a] - (shade[k0] + du * a / wa + dv * c / wc))
                >= 0.5f / 255.0f)
                return false;
    return true;
}
#endif

/* Is any quad of the rectangle from quad (i0, j0), wa by wc, drawn already? */
static bool SpanDrawn(unsigned drawn, int i0, int j0, int wa, int wc)
{
    const int n = VOXEL_RELIEF_SIDE - 1;

    for (int j = j0; j < j0 + wc; ++j)
        for (int i = i0; i < i0 + wa; ++i)
            if (drawn & (1u << (j * n + i)))
                return true;
    return false;
}

/* The rectangle of a relief cell's lattice from quad (i0, j0), wa quads
 * across and wc down, drawn as one quad; `face` the lattice's face terms
 * (vertexFace), or NULL. */
static void EmitReliefSpan(VoxelBuilder *b, int x, int y, const int16_t *g, const int16_t *s,
                           float u0, float v0, float u1, float v1, const float *face,
                           int i0, int j0, int wa, int wc, bool onFloor, int floor,
                           const uint8_t *rows, const CutDiagonals *cut)
{
    const int n = VOXEL_RELIEF_SIDE - 1;
    static const int di[4] = { 0, 1, 1, 0 }, dj[4] = { 0, 0, 1, 1 };
    VoxelVertex p[4];
    float raise = 0.0f, px[4][2];

    bool one = wa == 1 && wc == 1;

    if (one && onFloor && OnFloor(g, i0, j0, floor))
    {
        /* the flat layer under rock is plain ground (EmitCutGround):
         * rock on the foot is drawn, half a pixel over it */
        int rock = rows != NULL ? CutRockPixels(rows, i0, j0) : 0;

        if (rock == 0 || rock >= 12)
            return;         /* all ground, or the flat layer is the rock itself */
        raise = 0.5f / 16.0f;
    }
    for (int k = 0; k < 4; ++k)
    {
        int a = i0 + di[k] * wa, c = j0 + dj[k] * wc, at = c * VOXEL_RELIEF_SIDE + a;

        p[k] = (VoxelVertex){ x + a / (float)n, g[at] / 16.0f + raise, y + c / (float)n + s[at] / 16.0f,
                              u0 + (u1 - u0) * a / n, v0 + (v1 - v0) * c / n,
                              face != NULL ? face[at] : SHADE_TOP };
        px[k][0] = 16.0f * a / n;
        px[k][1] = 16.0f * c / n;
    }
    /* Split along the crease: a mountain's corner is two planes meeting on
     * a diagonal of the cell (gen_voxel_relief.py), and a quad on it has its
     * two corners off the crease level with each other - cut the other way
     * it would fold across the crease. (A rectangle of several is flat.) */
    if (one && g[j0 * VOXEL_RELIEF_SIDE + i0] == g[(j0 + 1) * VOXEL_RELIEF_SIDE + i0 + 1]
     && g[j0 * VOXEL_RELIEF_SIDE + i0 + 1] != g[(j0 + 1) * VOXEL_RELIEF_SIDE + i0])
    {
        const float pr[4][2] = { { px[1][0], px[1][1] }, { px[2][0], px[2][1] },
                                 { px[3][0], px[3][1] }, { px[0][0], px[0][1] } };

        EmitCutQuad(b, &p[1], &p[2], &p[3], &p[0], pr, cut);
    }
    else
        EmitCutQuad(b, &p[0], &p[1], &p[2], &p[3], px, cut);
}

static void EmitReliefUV(VoxelBuilder *b, int x, int y, const int16_t *g, const int16_t *s,
                         float u0, float v0, float u1, float v1,
                         bool onFloor, int floor, const uint8_t *rows)
{
    const int n = VOXEL_RELIEF_SIDE - 1;
    CutDiagonals cut;

    VoxelRelief_CutDiagonals(rows, &cut);
    /* The drawing's own light is in it, a face's included: shadows only. */
    b->artShaded = true;
    if (!VoxelRelief_IsSlope(g) && !VoxelRelief_IsSlope(s))
    {
        float h = g[0] / 16.0f, d = s[0] / 16.0f;
        static const float kWhole[4][2] = { { 0, 0 }, { 16, 0 }, { 16, 16 }, { 0, 16 } };

        /* Through the lit quad, as all ground is: what stands in the sun
         * (a house beside the mountain) shades it. */
        EmitCutQuad(b, &(VoxelVertex){x, h, y + d, u0, v0, SHADE_TOP},
                    &(VoxelVertex){x + 1, h, y + d, u1, v0, SHADE_TOP},
                    &(VoxelVertex){x + 1, h, y + 1 + d, u1, v1, SHADE_TOP},
                    &(VoxelVertex){x, h, y + 1 + d, u0, v1, SHADE_TOP}, kWhole, &cut);
        b->artShaded = false;
        return;
    }
#if CTR_VOXEL_LIGHTING
    const VoxelMapInstance *inst = VoxelWorld_GetInstanceAt(x, y);
    const int16_t *cells[3][3];
    float face[VOXEL_RELIEF_SIDE * VOXEL_RELIEF_SIDE];

    /* The face table is read only by a lit build: an unlit one (indoors)
     * skips its 8 neighbour lookups and 25 normals. */
    if (b->lighting)
    {
        for (int dz = -1; dz <= 1; ++dz)
            for (int dx = -1; dx <= 1; ++dx)
                cells[dz + 1][dx + 1] = dx == 0 && dz == 0 ? g
                                      : VoxelWorld_GetInstanceAt(x + dx, y + dz) == inst
                                      ? VoxelRelief_Cell(inst, x + dx, y + dz) : NULL;
        for (int c = 0; c <= n; ++c)
            for (int a = 0; a <= n; ++a)
                face[c * VOXEL_RELIEF_SIDE + a] = ReliefFace(cells, a, c);
    }
    b->vertexFace = b->lighting;
    /* its shadows read ahead, with fewer rays (VoxelLighting_Lattice) */
    bool latticeLit = b->lighting && b->lightingConstant < 0.0f && !b->rounded;
    VoxelVertex lattice[VOXEL_RELIEF_SIDE * VOXEL_RELIEF_SIDE];

    if (latticeLit)
    {
        for (int c = 0; c <= n; ++c)
            for (int a = 0; a <= n; ++a)
            {
                int k = c * VOXEL_RELIEF_SIDE + a;

                lattice[k] = (VoxelVertex){ x + a / (float)n, g[k] / 16.0f, y + c / (float)n + s[k] / 16.0f,
                                            0.0f, 0.0f, face[k] };
            }
        VoxelLighting_Lattice(b, lattice);
    }
#endif
    /*
     * A rectangle of the lattice that is one plane, uncut, and lit as its
     * corners run across it is the same picture as one quad: its corners,
     * its texture's corners. Tried largest first - the whole cell, its
     * halves, its quarters, rows and columns of them - and what is left is
     * drawn quad by quad. A ramp's or a sloped flank's cell drew all sixteen,
     * and the mountains' routes put twice a town's vertices before the GPU
     * (Route 114: 66,000, 10 ms of an Old 3DS frame, port.log 2026-10-26).
     */
    static const int8_t kSpans[][2] = { { 4, 4 }, { 4, 2 }, { 2, 4 }, { 2, 2 }, { 4, 1 }, { 1, 4 },
                                        { 2, 1 }, { 1, 2 }, { 1, 1 } };
    unsigned drawn = 0;     /* the quads drawn so far, bit j * n + i */
    const float *faces = NULL;
#if CTR_VOXEL_LIGHTING
    float shade[VOXEL_RELIEF_SIDE * VOXEL_RELIEF_SIDE];
    bool shaded = false;

    if (b->vertexFace)
        faces = face;
#endif

    for (unsigned t = 0; t < sizeof(kSpans) / sizeof(kSpans[0]); ++t)
    {
        const int wa = kSpans[t][0], wc = kSpans[t][1];
        const bool one = wa == 1 && wc == 1;

        /* (a cut tile's quads are cut one by one) */
        if (!one && cut.count != 0)
            continue;
        for (int j0 = 0; j0 < n; j0 += wc)
            for (int i0 = 0; i0 < n; i0 += wa)
            {
                bool merge = one;

                if (SpanDrawn(drawn, i0, j0, wa, wc))
                    continue;
                if (!one && SpanFlat(g, i0, j0, wa, wc) && SpanFlat(s, i0, j0, wa, wc))
                {
                    merge = true;
                    for (int j = j0; j < j0 + wc && merge; ++j)
                        for (int i = i0; i < i0 + wa && merge; ++i)
                            merge = !onFloor || !OnFloor(g, i, j, floor);
#if CTR_VOXEL_LIGHTING
                    if (merge && latticeLit)
                    {
                        if (!shaded)
                            VoxelLighting_LatticeShades(b, lattice, shade);
                        shaded = true;
                        merge = SpanLinear(shade, i0, j0, wa, wc);
                    }
#endif
                }
                if (!merge)
                    continue;
                for (int j = j0; j < j0 + wc; ++j)
                    for (int i = i0; i < i0 + wa; ++i)
                        drawn |= 1u << (j * n + i);
                EmitReliefSpan(b, x, y, g, s, u0, v0, u1, v1, faces, i0, j0, wa, wc,
                               onFloor, floor, rows, &cut);
            }
    }
    b->vertexFace = false;
    b->artShaded = false;
}

/* ── Entry point ────────────────────────────────────────────────────────── */

void VoxelMesh_BeginWindow(int x0, int y0, int x1, int y1)
{
    WindowReset(x0, y0, x1, y1);
}

/*
 * Where the world does not close up, two maps meet at different levels
 * (voxel_relief.h: a map stands at its base) and the higher one's edge is a
 * step down to the other. It is closed by a skirt of the edge cell's own
 * drawing, hanging from the edge to the ground beyond it, so the seam is a
 * low wall and never a gap onto the clear colour.
 */
static bool BorderWaterLevel(int x, int y, float *level, float *shift);
static int BeltMetatileFor(int x, int wantY, int y);
/* POL-30: the base a cell of the belt is built over - its own map's (VoxelWorld_BeltRing), not instance 0's. */
static float BeltBase(int x, int y)
{
    return VoxelRelief_Base(VoxelWorld_BeltRing(x, y));
}

/* A cell of the belt past the last map that is a tree's (VoxelTree_EmitBorder):
 * flat trunks and tilted crowns, nothing standing. */
static bool TreeBeltAt(int x, int y)
{
    const VoxelMapInstance *ring = VoxelWorld_BorderArtMap(x, y); /* POL-30: was instance 0 */

    return ring != NULL && VoxelWorld_GetInstanceAt(x, y) == NULL && VoxelWorld_UsesTreeSprites(ring)
        && VoxelTree_Part(ring->secondaryTileset, VoxelWorld_BorderMetatile(x, y)) >= 0;
}

/*
 * A cell's ground along one edge (d as EmitSeamSkirt's: north, south, west,
 * east), point by point of the lattice: heights in pixels over its base and
 * how far south each point stands, in cells from the edge's own row or
 * column. West, east and south, a cell with a cut record stands on its
 * foot: the rock over it closes those sides down to the base itself
 * (EmitCliffWalls). Everywhere else it is the edge as drawn, so a back is
 * closed right up to its edge and no further - one wall at the cell's
 * centre height stood half a level over a sloped back, a strip of the
 * tile's own drawing. `stands` (if wanted) has a bit a stretch of the edge
 * a wall may stand on: along a cut tile's back, not where its north row of
 * pixels is cleared - the wall showed through them as teeth, as on the map
 * (EmitNorthFace).
 */
static void SeamEdge(const VoxelMapInstance *inst, int x, int y, int d,
                     float h[VOXEL_RELIEF_SIDE], float z[VOXEL_RELIEF_SIDE], unsigned *stands)
{
    const int16_t *grid = VoxelRelief_Cell(inst, x, y);
    const int16_t *depth = VoxelRelief_Depth(inst, x, y);
    const unsigned last = VOXEL_RELIEF_SIDE - 1;
    float foot = 0.0f;
    int ground = -1, wall = -1;
    unsigned sides = 0;
    int cut = grid != NULL ? VoxelRelief_Cut(inst, x, y, &foot, &ground, &wall, &sides) : -1;
    bool onFoot = cut != -1 && d != 0 && wall >= 0;
    const uint8_t *rows = NULL;
    unsigned layout, metatile;

    if (stands != NULL)
    {
        *stands = ~0u;
        if (d == 0 && cut >= 0 && cut != VOXEL_RELIEF_NO_VARIANT
         && VoxelRelief_CutVariant((unsigned)cut, &layout, &metatile, &rows))
        {
            uint16_t clear[16];
            const unsigned px = 16 / (VOXEL_RELIEF_SIDE - 1);

            VoxelRelief_CutClear(rows, clear);
            for (unsigned k = 0; k + 1 < VOXEL_RELIEF_SIDE; ++k)
                if ((clear[0] >> (k * px)) & ((1u << px) - 1u))
                    *stands &= ~(1u << k);
        }
    }

    for (unsigned k = 0; k < VOXEL_RELIEF_SIDE; ++k)
    {
        unsigned i = d == 0 ? k : d == 1 ? last * VOXEL_RELIEF_SIDE + k
                   : k * VOXEL_RELIEF_SIDE + (d == 2 ? 0 : last);

        h[k] = onFoot ? foot * 16.0f : grid != NULL ? grid[i] : 0.0f;
        z[k] = (onFoot ? foot * 16.0f : depth != NULL ? depth[i] : 0.0f) / 16.0f;
    }
}

static bool BeltTopWorld(int x, int y, float *top, float *z, unsigned *drawnOut);
static bool BorderAt(int x, int y);
static bool BeltIsTree(int x, int y);
static bool FieldOn(void);

/*
 * POL-46 (Brennan 2026-11-17 21:00: "Completely eliminate the blue triangle gap at the seam"; "tile the rock texture
 * cleanly by world-space height ... instead of stretching UVs"). A map's west, east or north edge standing over the
 * belt beside it (Route 112's mountain over the belt north of Lavaridge) was open down to the belt: only an edge
 * under the belt had a wall. It stands a wall of the map's cliff face from the belt's top up to the edge, point by
 * point of the edge, a tile of the face a level (by world height, never stretched). WJPC_EDGE_OPEN=1: as before.
 */
/* Q10 (Brennan 2026-11-20 12:27, Route 121 east, Lilycove's west edge over the sea: "it shows the ground underneath
 * dirt not covered up by sea water"): a wall closed down to belt water (EdgeOverBelt) is the map's own coast cliff,
 * drawn with the map's own cliff face (Lilycove's brown rock), not the face of another map's belt terraces (a dark
 * one). WJPC_EDGE_WATER_FACE_OLD=1: as before. */
static bool WaterFaceOld(void)
{
    static int sOld = -1;
    if (sOld < 0)
        sOld = getenv("WJPC_EDGE_WATER_FACE_OLD") != NULL;
    return sOld != 0;
}

static void EdgeOverBelt(VoxelBuilder *b, const VoxelMapInstance *inst, int x, int y, int d, int nx, int ny)
{
    static int sOpen = -1;
    const int16_t *grid = VoxelRelief_Cell(inst, x, y);
    const int16_t *depth = VoxelRelief_Depth(inst, x, y);
    const unsigned last = VOXEL_RELIEF_SIDE - 1;
    float beltTop, beltZ, base = VoxelRelief_Base(inst), h[VOXEL_RELIEF_SIDE], z[VOXEL_RELIEF_SIDE], top = -1e9f;
    float u0, v0, u1, v1;
    unsigned drawn;
    int face;

    static int sSouthOld = -1;
    if (sOpen < 0)
        sOpen = getenv("WJPC_EDGE_OPEN") != NULL;
    if (sSouthOld < 0)
        sSouthOld = getenv("WJPC_EDGE_SOUTH_OLD") != NULL;
    /* Audit batch 2 (Mt. Chimney's and Route 112's south edges: little triangles of sky): the land in front of a
     * map's south edge is held under the edge where another map's cap reaches it (a level every 2 cells), and the
     * south side was the one never closed. Now it is, as the others: the edge's own cliff down to the belt's ground
     * (under the land in front, where that stands higher). WJPC_EDGE_SOUTH_OLD=1: open as before. */
    /* POL-56 (Brennan 2026-11-19 14:23, Route 120 south-west: "a light-blue skybox triangle"): the edge there slopes
     * a level down across one cell (48 -> 32 px) over a belt WOOD cell, which BeltTopWorld turns down - the triangle
     * between the sloped edge and the wood's flat ground stayed open. A wood cell south of the edge is closed the
     * same way (its ground is the belt's base). WJPC_EDGE_SKIP_WOODS=1: as before. */
    static int sSkipWoods = -1;
    if (sSkipWoods < 0)
        sSkipWoods = getenv("WJPC_EDGE_SKIP_WOODS") != NULL;
    if (sOpen || grid == NULL || (d == 1 && sSouthOld))
        return;
    /* POL-58 (Brennan 2026-11-19 14:23, Route 121 east: "Blue Skybox Leak Around Sea Rocks"): Lilycove's west edge
     * stands over the belt's sea north of Route 121 - BeltTopWorld turns water down, so the edge was open down to
     * the sea, a slanted strip of sky. An edge over belt water is closed down to the water's level.
     * WJPC_EDGE_SKIP_WATER=1: as before. */
    static int sSkipWater = -1;
    float wl = 0.0f, ws = 0.0f;
    bool overWater = false;
    if (sSkipWater < 0)
        sSkipWater = getenv("WJPC_EDGE_SKIP_WATER") != NULL;
    if (!BeltTopWorld(nx, ny, &beltTop, &beltZ, &drawn))
    {
        bool open = !BorderAt(nx, ny) || VoxelWorld_GetInstanceAt(nx, ny) != NULL;
        overWater = !open && !sSkipWater && BorderWaterLevel(nx, ny, &wl, &ws);
        if (getenv("WJPC_EDGE_DUMP") != NULL && !overWater)
            fprintf(stderr, "EDGESKIP %d %d d%d over %d,%d open %d tree %d\n", x, y, d, nx, ny, (int)open,
                    (int)BeltIsTree(nx, ny));
        if (!overWater && (sSkipWoods || d != 1 || open || !BeltIsTree(nx, ny)))
            return;
    }
    else if (getenv("WJPC_EDGE_DUMP") != NULL)
        fprintf(stderr, "EDGELAND %d %d d%d over %d,%d beltTop %.2f drawn %u\n", x, y, d, nx, ny, beltTop, drawn);
    /* down to the belt's ground: the belt cell beside the wall's foot is drawn rows away (each stands as far south
     * as it is high), so the wall goes to the ground and the belt in front hides what is under its tops */
    beltTop = BeltBase(nx, ny) - base + (overWater ? wl - 1.0f : 0.0f);
    for (unsigned k = 0; k < VOXEL_RELIEF_SIDE; ++k)
    {
        unsigned i = d == 0 ? k : d == 1 ? last * VOXEL_RELIEF_SIDE + k : k * VOXEL_RELIEF_SIDE + (d == 2 ? 0 : last);
        h[k] = grid[i] / 16.0f;
        z[k] = (float)y + (d == 0 ? 0.0f : d == 1 ? 1.0f : k / (float)last) + (depth ? depth[i] : grid[i]) / 16.0f;
        if (h[k] > top)
            top = h[k];
    }
    /* Brennan 2026-11-22 19:55 (Fiery Path: "this back wall flickers when walking"): its north row is the top of an
     * upright cliff face (two cells of face stood as far south as they are high), so this wall stood in the face's
     * own plane, from the ground up - the two fought (z-fighting) as the camera moved: stripes over the lower half.
     * A north edge's wall now stands half a pixel behind the edge (north): hidden behind the map's own face where
     * there is one, and from the south no gap opens between them (the wall runs on down behind the edge).
     * WJPC_EDGE_NORTH_IN_PLANE=1: in the plane, as before. */
    {
        static int sInPlane = -1;
        if (sInPlane < 0)
            sInPlane = getenv("WJPC_EDGE_NORTH_IN_PLANE") != NULL;
        if (d == 0 && !sInPlane)
            for (unsigned k = 0; k < VOXEL_RELIEF_SIDE; ++k)
                z[k] -= 1.0f / 32.0f;
    }
    if (getenv("WJPC_EDGE_DUMP") != NULL)
        fprintf(stderr, "EDGEBELT %d %d d%d over %d,%d water %d wl %.2f beltTop %.2f top %.2f h %.2f %.2f %.2f %.2f %.2f\n",
                x, y, d, nx, ny, (int)overWater, wl, beltTop, top, h[0], h[1], h[2], h[3], h[4]);
    if (top <= beltTop + 0.05f)
        return;
    face = VoxelRelief_FaceTile(inst);
    /* beside the drawn belt's terraces, its rock (one mountain): the drawn map's cliff face - not for a coast over
     * the sea, which is this map's own cliff (Q10 below) */
    for (unsigned i = 0; i < VoxelWorld_InstanceCount() && !(overWater && face >= 0 && !WaterFaceOld()); ++i)
    {
        const VoxelMapInstance *m = VoxelWorld_Instance(i);
        if (m != NULL && m->layout != NULL && !m->indoor && VoxelRelief_Belt(m) != 0
         && abs(nx - (m->originX + m->width / 2)) <= m->width / 2 + 24 && abs(ny - (m->originY + m->height / 2)) <= m->height / 2 + 24
         && VoxelRelief_FaceTile(m) >= 0)
        {
            face = VoxelRelief_FaceTile(m);
            break;
        }
    }
    if (overWater && getenv("WJPC_EDGE_WATER_FACE") != NULL)
        face = (int)strtol(getenv("WJPC_EDGE_WATER_FACE"), NULL, 16);   /* test aid: the face over the sea */
    /* Brennan 2026-11-22 02:04 (Route 112's south border, two red boxes: the cliff faces "textured with a bright
     * red-and-white striped rock texture that clashes sharply with the rest of the mountainside ... Replace [it] with
     * the exact earthy tan/brown rocky cliff texture shown inside the blue box" - the land in front): a map's edge
     * standing over the field's plain land drops to it in that land's own drawing (the field cell's top, as
     * EmitFieldCell lays it), not the map's or the drawn belt's cliff face - one rock for the mountainside.
     * WJPC_EDGE_FACE_OWN=1: as before. */
    {
        float wl3, ws3;
        if (!overWater && FieldOn() && BorderAt(nx, ny) && !BeltIsTree(nx, ny) && !BorderWaterLevel(nx, ny, &wl3, &ws3)
         && getenv("WJPC_EDGE_FACE_OWN") == NULL)
        {
            int land = BeltMetatileFor(nx, ny, ny);
            float lu0, lv0, lu1, lv1;
            if (land >= 0 && MetatileUV(b, land, &lu0, &lv0, &lu1, &lv1))
                face = land;
        }
    }
    if (getenv("WJPC_EDGE_DUMP") != NULL)
        fprintf(stderr, "EDGEFACE %d %d d%d face %03x water %d\n", x, y, d, face, (int)overWater);
    if (face < 0 || !MetatileUV(b, face, &u0, &v0, &u1, &v1))
    {
        if (!VoxelMesh_TileUV(b, x, y, &u0, &v0, &u1, &v1))
            return;
    }
    /* Q10 (Brennan 2026-11-20 16:22, "there is still blue sky showing" at Lilycove's north-west corner): a north
     * edge stands as far south as it is high, and the strip between the map's own edge (z = y) and its wall (z[k])
     * belongs to the map, so the belt lays no sea there - over the sea it was a slot of sky, seen past the corner.
     * The sea is laid on into it, at its own level. WJPC_EDGE_SEA_STRIP_OFF=1: as before. */
    {
        static int sStripOff = -1;
        float su0, sv0, su1, sv1;
        if (sStripOff < 0)
            sStripOff = getenv("WJPC_EDGE_SEA_STRIP_OFF") != NULL;
        if (overWater && d == 0 && !sStripOff
         && MetatileUV(b, VoxelWorld_BorderMetatile(nx, ny), &su0, &sv0, &su1, &sv1))
            for (unsigned k = 0; k < last; ++k)
            {
                float xa = (float)x + k / (float)last, xb = (float)x + (k + 1) / (float)last;
                float sa = su0 + (su1 - su0) * k / (float)last, sb = su0 + (su1 - su0) * (k + 1) / (float)last;
                float wy = beltTop + 1.0f;

                if (z[k] <= (float)y && z[k + 1] <= (float)y)
                    continue;
                VoxelBuilder_Quad(b,
                    &(VoxelVertex){xa, wy, (float)y, sa, sv0, SHADE_TOP},
                    &(VoxelVertex){xb, wy, (float)y, sb, sv0, SHADE_TOP},
                    &(VoxelVertex){xb, wy, z[k + 1], sb, sv1, SHADE_TOP},
                    &(VoxelVertex){xa, wy, z[k],     sa, sv1, SHADE_TOP});
            }
    }
    for (unsigned k = 0; k < last; ++k)
    {
        float xa = d <= 1 ? (float)x + k / (float)last : d == 2 ? (float)x : (float)x + 1.0f;
        float xb = d <= 1 ? (float)x + (k + 1) / (float)last : xa;
        float hi = h[k] > h[k + 1] ? h[k] : h[k + 1];
        float shade = d == 0 ? SHADE_NORTH : d == 1 ? SHADE_SOUTH : d == 2 ? SHADE_WEST : SHADE_EAST;
        float ua = u0 + (u1 - u0) * k / (float)last, ub = u0 + (u1 - u0) * (k + 1) / (float)last;

        /* a band a level of the face at a time, by world height */
        for (float lo = floorf(beltTop + base) - base; lo < hi; lo += 1.0f)
        {
            float b0 = lo > beltTop ? lo : beltTop, b1 = lo + 1.0f;
            float ta = b1 < h[k] ? b1 : h[k], tb = b1 < h[k + 1] ? b1 : h[k + 1];
            float fa0 = b0 - lo, fa1 = ta - lo, fb1 = tb - lo;

            if (ta <= b0 && tb <= b0)
                continue;
            if (ta < b0) ta = b0, fa1 = b0 - lo;
            if (tb < b0) tb = b0, fb1 = b0 - lo;
            VoxelBuilder_Quad(b,
                &(VoxelVertex){xa, b0, z[k],     ua, v1 - (v1 - v0) * fa0, shade},
                &(VoxelVertex){xb, b0, z[k + 1], ub, v1 - (v1 - v0) * fa0, shade},
                &(VoxelVertex){xb, tb, z[k + 1], ub, v1 - (v1 - v0) * fb1, shade},
                &(VoxelVertex){xa, ta, z[k],     ua, v1 - (v1 - v0) * fa1, shade});
        }
    }
}

static void EmitSeamSkirt(VoxelBuilder *b, const VoxelMapInstance *inst, int x, int y)
{
    static const int kDir[4][2] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
    float wx = (float)x, southGap = 0.0f;
    float u0, v0, u1, v1;
    bool uv = false;

    for (int d = 0; d < 4; ++d)
    {
        int nx = x + kDir[d][0], ny = y + kDir[d][1];
        const VoxelMapInstance *other;
        float ourH[VOXEL_RELIEF_SIDE], ourZ[VOXEL_RELIEF_SIDE];
        float theirH[VOXEL_RELIEF_SIDE], theirZ[VOXEL_RELIEF_SIDE];
        float rebase16;
        unsigned stands;
        bool any = false;

        if (nx >= inst->originX && ny >= inst->originY
         && nx < inst->originX + inst->width && ny < inst->originY + inst->height)
            continue;
        other = VoxelWorld_GetInstanceAt(nx, ny);
        if (other == NULL && !inst->indoor)
            EdgeOverBelt(b, inst, x, y, d, nx, ny);
        if (other == NULL && !inst->indoor && d != 1)
        {
            /*
             * The belt past the last map (VoxelMesh_EmitBorder) stands at the
             * base of instance 0. An edge lowered under it - Rustboro's sea,
             * Route 117's wood - left a step up to it with nothing in it, a
             * band of the clear colour along the map's edge. It is closed by
             * a wall of the edge cell's own drawing, from the edge up to the
             * belt. Not along the south edge: there the belt stands in front.
             * Its foot is the edge as the cell draws it, point by point of
             * the lattice: a rock or a slope at the edge stands each point at
             * its own height, and that far south (voxel_relief.h), and a foot
             * level at the centre's height left slivers of the clear under
             * it. Its top is where the belt stands, unmoved. Not round a
             * belt laid as the drawing (VoxelRelief_Belt): that meets every
             * edge at its height, and the map's faces close the steps. The
             * belt is drawn as far south as its base is high, as a map is
             * (VoxelBuilder_StretchZ): its top's edge is that far from ours.
             */
            const VoxelMapInstance *ring = VoxelWorld_BeltRing(nx, ny); /* POL-30: was instance 0 */
            float belt = ring != NULL ? VoxelRelief_Base(ring) - VoxelRelief_Base(inst) : 0.0f;
            const float beltZ = belt;
            const int16_t *grid = VoxelRelief_Cell(inst, x, y);
            const int16_t *depth = VoxelRelief_Depth(inst, x, y);
            const unsigned last = VOXEL_RELIEF_SIDE - 1;
            float footH[VOXEL_RELIEF_SIDE], footZ[VOXEL_RELIEF_SIDE], along[VOXEL_RELIEF_SIDE];
            float lowest, level, shift;
            bool water;

            if (ring == NULL || VoxelRelief_Belt(ring) != 0)
                continue;
            /* a belt of water lies at its own level (BorderWaterLevel), a
             * belt of land at its edge's (VoxelMesh_BorderLevel) */
            water = BorderWaterLevel(nx, ny, &level, &shift);
            if (water || VoxelMesh_BorderLevel(nx, ny, &level, &shift))
                belt += level;
            lowest = belt;
            for (unsigned k = 0; k < VOXEL_RELIEF_SIDE; ++k)
            {
                /* the north row, or the west or east column */
                unsigned i = d == 0 ? k : k * VOXEL_RELIEF_SIDE + (d == 2 ? 0 : last);
                along[k] = k / (float)last;
                footH[k] = (grid ? grid[i] : 0) / 16.0f;
                footZ[k] = (float)y + (d == 0 ? 0.0f : along[k]) + (depth ? depth[i] : 0) / 16.0f;
                if (footH[k] < lowest)
                    lowest = footH[k];
            }
            if (lowest >= belt - 0.05f)
                continue;
            /* Brennan 2026-11-21 21:31 (21p's upright rock step on Route 111's west edge by Route 117: "Remove that
             * steep upright rock slab ... continue the horizontal stepped brown cliff tiers"; and Route 117's own
             * north-east corner cell, a wall two levels up to the ring's rock): beside the belt's plain land no wall
             * is drawn - the belt's own ground closes those seams (its risers run on along the edge). Beside the SEA
             * and beside a WOOD they stay: without them a sliver opens (Ever Grande's east cliff over the sea; Route
             * 104's west edge, a level under its wood, seen from Route 105). Checked with the magenta gap test (F7):
             * every outdoor map's west and east edges (200 views) at 1280x720, the 18 views where those walls show at
             * 1920x1080 and 2560x1440, every outdoor map's north edge (100 views) at 1920x1080 and 2560x1440
             * (Brennan 21:53: test at his resolutions). WJPC_EDGE_LAND_WALLS=1: drawn as before;
             * WJPC_EDGE_SIDE_WALLS_OFF=1 (test aid): none at all. */
            {
                static int sLandWalls = -1, sNone = -1;
                if (sLandWalls < 0)
                {
                    sLandWalls = getenv("WJPC_EDGE_LAND_WALLS") != NULL;
                    sNone = getenv("WJPC_EDGE_SIDE_WALLS_OFF") != NULL;
                }
                bool plain = !water && !BeltIsTree(nx, ny);

                if (getenv("WJPC_EDGE_DUMP") != NULL)
                    fprintf(stderr, "EDGELAND %d %d d%d water %d tree %d foot %.2f..%.2f belt %.2f drawn %d\n", x, y, d,
                            (int)water, (int)BeltIsTree(nx, ny), footH[0], footH[last], belt,
                            (int)!(sNone || (plain && !sLandWalls)));
                if (sNone || (plain && !sLandWalls))
                    continue;
            }
            /* Brennan 2026-11-21 17:25 (image 7, the same slab, below): the wall was the edge cell's own drawing -
             * grass, a green wall. It is the map's cliff face (VoxelRelief_FaceTile) now, a rock cliff where the ground
             * meets higher land. WJPC_EDGE_WALL_OWN_DRAWING=1: the cell's drawing, as before. */
            if (!uv)
            {
                static int sOwn = -1;
                int face;

                if (sOwn < 0)
                    sOwn = getenv("WJPC_EDGE_WALL_OWN_DRAWING") != NULL;
                face = sOwn ? -1 : VoxelRelief_FaceTile(inst);
                /* (a map with no cliff of its own - Route 117 - meets the ring's rock: the ring's face tile, when the
                 * two share their tilesets, so the atlas has it) */
                if (face < 0 && !sOwn && ring->primaryTileset == inst->primaryTileset
                 && ring->secondaryTileset == inst->secondaryTileset)
                    face = VoxelRelief_FaceTile(ring);
                uv = face >= 0 && MetatileUV(b, face, &u0, &v0, &u1, &v1);
                if (getenv("WJPC_EDGE_DUMP") != NULL)
                    fprintf(stderr, "EDGEWALL %d %d d%d foot %.2f..%.2f footZ %.2f lowest %.2f belt %.2f beltZ %.2f level %.2f shift %.2f instFace %d ringFace %d tiles %d/%d vs %d/%d uv %d\n",
                            x, y, d, footH[0], footH[last], footZ[0], lowest, belt, beltZ, level, shift, VoxelRelief_FaceTile(inst), VoxelRelief_FaceTile(ring),
                            (int)inst->primaryTileset, (int)inst->secondaryTileset,
                            (int)ring->primaryTileset, (int)ring->secondaryTileset, (int)uv);
            }
            if (!uv && !(uv = VoxelMesh_TileUV(b, x, y, &u0, &v0, &u1, &v1)))
                return;
            /* Brennan 2026-11-21 17:25 (image 7, Route 117 by the house: "a raised/tilted green grass block sticking up
             * higher than the surrounding ground"): Route 111's west edge south of its cliffs lies two levels under
             * the ring, and is drawn two rows north (its depth). The wall's foot was there, its top at the edge's own
             * row: it leant two tiles south, over the ring's lower terraces - the green slab. The top now stands over
             * the foot, an upright wall where the edge is drawn. (The ring's own shift there would do as well, but
             * beside Route 112 it differs from the map's by two rows: a saw-tooth of leaning walls.)
             * WJPC_EDGE_WALL_TOP_OLD=1: as before. */
            static int sTopOld = -1;
            if (sTopOld < 0)
                sTopOld = getenv("WJPC_EDGE_WALL_TOP_OLD") != NULL;
#define TOPZ(i, old) (sTopOld ? (old) : footZ[i])
            for (unsigned k = 0; k < last; ++k)
            {
                float hA = belt - footH[k], hB = belt - footH[k + 1];
                float fA = hA < 1.0f ? hA : 1.0f, fB = hB < 1.0f ? hB : 1.0f;

                if (hA <= 0.05f && hB <= 0.05f)
                    continue;
                if (d == 0) /* faces south, on the map's north edge, where the belt's face ends */
                {
                    float xa = wx + along[k], xb = wx + along[k + 1];
                    float ua = u0 + (u1 - u0) * along[k], ub = u0 + (u1 - u0) * along[k + 1];

                    VoxelBuilder_Quad(b,
                        &(VoxelVertex){xa, footH[k],     footZ[k],     ua, v0 + (v1 - v0) * fA, SHADE_SOUTH},
                        &(VoxelVertex){xb, footH[k + 1], footZ[k + 1], ub, v0 + (v1 - v0) * fB, SHADE_SOUTH},
                        &(VoxelVertex){xb, belt,         TOPZ(k + 1, (float)y + beltZ), ub, v0, SHADE_SOUTH},
                        &(VoxelVertex){xa, belt,         TOPZ(k, (float)y + beltZ),     ua, v0, SHADE_SOUTH});
                }
                else
                {
                    float za = TOPZ(k, (float)y + along[k] + beltZ), zb = TOPZ(k + 1, (float)y + along[k + 1] + beltZ);
                    float va = v0 + (v1 - v0) * along[k], vb = v0 + (v1 - v0) * along[k + 1];

                    if (d == 2) /* faces east, on the west edge */
                        VoxelBuilder_Quad(b,
                            &(VoxelVertex){wx, belt,         zb,           u0,                  vb, SHADE_EAST},
                            &(VoxelVertex){wx, belt,         za,           u0,                  va, SHADE_EAST},
                            &(VoxelVertex){wx, footH[k],     footZ[k],     u0 + (u1 - u0) * fA, va, SHADE_EAST},
                            &(VoxelVertex){wx, footH[k + 1], footZ[k + 1], u0 + (u1 - u0) * fB, vb, SHADE_EAST});
                    else /* faces west, on the east edge */
                        VoxelBuilder_Quad(b,
                            &(VoxelVertex){wx + 1.0f, belt,         za,           u1,                  va, SHADE_WEST},
                            &(VoxelVertex){wx + 1.0f, belt,         zb,           u1,                  vb, SHADE_WEST},
                            &(VoxelVertex){wx + 1.0f, footH[k + 1], footZ[k + 1], u1 - (u1 - u0) * fB, vb, SHADE_WEST},
                            &(VoxelVertex){wx + 1.0f, footH[k],     footZ[k],     u1 - (u1 - u0) * fA, va, SHADE_WEST});
                }
            }
#undef TOPZ
            continue;
        }
        /* POL-56 (Brennan 2026-11-19 14:23, Route 120 south-west: "a light-blue skybox triangle is bleeding through
         * the trees/ground seam"): a grass strip of the belt there, not a tree - the same slope of the edge's drawing
         * closes any land belt under the edge, not only a wood. WJPC_SOUTH_SKIRT_TREES=1: trees only. */
        static int sTreesOnly = -1;
        if (sTreesOnly < 0)
            sTreesOnly = getenv("WJPC_SOUTH_SKIRT_TREES") != NULL;
        if (other == NULL && d == 1 && !inst->indoor
         && (TreeBeltAt(nx, ny) || (!sTreesOnly && VoxelWorld_BorderMetatile(nx, ny) >= 0 && !VoxelWorld_BorderIsWater(nx, ny))))
        {
            /*
             * South of the last map, a belt of trees: no wall stands in
             * front, its trunks lie flat at the belt's level. An edge lowered
             * under it on a drawn map ends that much further north, and the
             * slope from it up to the belt was open: through the crowns, the
             * clear colour at every tree (the Safari Zone's south-east, Route
             * 120). The edge cell's drawing goes on to the belt, point by
             * point of the lattice, as it does to a map at another level.
             */
            const VoxelMapInstance *ring = VoxelWorld_BeltRing(nx, ny); /* POL-30: was instance 0 */
            const int16_t *ours = VoxelRelief_Depth(inst, x, y);
            const int16_t *oursH = VoxelRelief_Cell(inst, x, y);
            const unsigned last = (VOXEL_RELIEF_SIDE - 1) * VOXEL_RELIEF_SIDE;
            float belt = (VoxelRelief_Base(ring) - VoxelRelief_Base(inst)) * 16.0f;
            /* (the belt as far south as its base is high, as the map is) */
            float beltZ = (float)y + 1.0f + belt / 16.0f;
            float level, shift;
            static int sSkirtZOld = -1;

            if (sSkirtZOld < 0)
                sSkirtZOld = getenv("WJPC_SOUTH_SKIRT_Z_OLD") != NULL;
            if (oursH == NULL || VoxelRelief_Belt(ring) != 0)
                continue;
            if (VoxelMesh_BorderLevel(nx, ny, &level, &shift))
            {
                belt += level * 16.0f;
                /* Brennan 2026-11-21 01:56 (image 7, Route 112: "massive stretched vertical pillars ... shooting
                 * straight up into the sky"): the belt's top is drawn as far south as its own shift says
                 * (EmitBorderNorth: y + shift), not at our edge. Where the belt stands levels over the edge (south of
                 * Route 112: 6-11), the skirt rose straight up from the edge to the belt's height - a wall of the
                 * edge cell's drawing (trees, rock) eleven tiles tall. Its top now meets the belt where the belt is
                 * drawn. WJPC_SOUTH_SKIRT_Z_OLD=1: as before. */
                if (!sSkirtZOld)
                    beltZ += shift;
            }
            /* Brennan 2026-11-21 17:25 (image 4, Route 112's south border: "stretched walls of rock and trees are
             * shooting straight up into the sky"): south of Route 112 the belt is the field's plain land (EmitFieldCell,
             * BeltField: it meets the map's edge exactly), but this skirt took its top from VoxelMesh_BorderLevel's
             * edge-fill mountain there - 3 to 11 levels over the edge - and drew the edge cell's grass up to it: long
             * green slabs five rows out. Beside the field's plain land no skirt, as beside it on the other sides
             * (21q). WJPC_EDGE_LAND_WALLS=1: as before. */
            {
                static int sLandWalls = -1;
                float wl2, ws2;
                if (sLandWalls < 0)
                    sLandWalls = getenv("WJPC_EDGE_LAND_WALLS") != NULL;
                if (!sLandWalls && FieldOn() && !BeltIsTree(nx, ny) && !BorderWaterLevel(nx, ny, &wl2, &ws2))
                    continue;
            }
            if (getenv("WJPC_EDGE_DUMP") != NULL)
                fprintf(stderr, "SOUTHSKIRT %d %d belt %.2f beltZ %.2f level %.2f shift %.2f tree %d\n", x, y, belt / 16.0f,
                        beltZ, level, shift, (int)TreeBeltAt(nx, ny));
            if (!uv && !(uv = VoxelMesh_TileUV(b, x, y, &u0, &v0, &u1, &v1)))
                return;
            for (unsigned k = 0; k + 1 < VOXEL_RELIEF_SIDE; ++k)
            {
                float hA = oursH[last + k], hB = oursH[last + k + 1];
                float zA = (float)y + 1.0f + (ours ? ours[last + k] : 0) / 16.0f;
                float zB = (float)y + 1.0f + (ours ? ours[last + k + 1] : 0) / 16.0f;
                float xa = wx + k / (float)(VOXEL_RELIEF_SIDE - 1);
                float xb = wx + (k + 1) / (float)(VOXEL_RELIEF_SIDE - 1);
                float ua = u0 + (u1 - u0) * k / (float)(VOXEL_RELIEF_SIDE - 1);
                float ub = u0 + (u1 - u0) * (k + 1) / (float)(VOXEL_RELIEF_SIDE - 1);

                if (belt - hA < 1.0f && belt - hB < 1.0f)
                    continue;
                VoxelBuilder_Quad(b,
                    &(VoxelVertex){xa, hA / 16.0f,   zA,                ua, v1, SHADE_TOP},
                    &(VoxelVertex){xb, hB / 16.0f,   zB,                ub, v1, SHADE_TOP},
                    &(VoxelVertex){xb, belt / 16.0f, beltZ,             ub, v0, SHADE_TOP},
                    &(VoxelVertex){xa, belt / 16.0f, beltZ,             ua, v0, SHADE_TOP});
            }
            continue;
        }
        if (other == NULL || other == inst || other->indoor)
            continue;
        /* both edges point by point, theirs over our base (a step where the
         * maps' bases differ, or where one edge stands over the other) */
        rebase16 = (VoxelRelief_Base(other) - VoxelRelief_Base(inst)) * 16.0f;
        SeamEdge(inst, x, y, d, ourH, ourZ, &stands);
        SeamEdge(other, nx, ny, d ^ 1, theirH, theirZ, NULL);
        for (unsigned k = 0; k < VOXEL_RELIEF_SIDE; ++k)
        {
            theirH[k] += rebase16;
            any = any || ourH[k] - theirH[k] > 0.8f;
        }
        if (getenv("WJPC_SEAM_DUMP") != NULL) // (test aid: each seam cell's two edges, point by point)
        {
            fprintf(stderr, "SEAM %d:%d %d %d d%d other %d:%d stands %x any %d lay %d/%d grid %d/%d ours", inst->mapGroup, inst->mapNum, x, y, d,
                    other->mapGroup, other->mapNum, stands, (int)any, inst->layoutId, other->layoutId,
                    VoxelRelief_Cell(inst, x, y) != NULL, VoxelRelief_Cell(other, nx, ny) != NULL);
            for (unsigned k = 0; k < VOXEL_RELIEF_SIDE; ++k)
                fprintf(stderr, " %.1f", ourH[k]);
            fprintf(stderr, " theirs");
            for (unsigned k = 0; k < VOXEL_RELIEF_SIDE; ++k)
                fprintf(stderr, " %.1f", theirH[k]);
            fprintf(stderr, "\n");
        }
        if (d == 1)
        {
            /*
             * A point stands as far south as it is high in the world
             * (voxel_relief.h), its map's base and all, so two maps meeting
             * at one level meet in the same row. Should their edges still lie
             * a row apart, the edge cell's drawing goes on south, 1:1, from
             * our edge to theirs at every point of the lattice - the water's
             * own animated tile, a band's slope as it falls -, never a gap
             * onto the clear colour.
             */
            const int16_t *ours = VoxelRelief_Depth(inst, x, y);
            const int16_t *theirs = VoxelRelief_Depth(other, nx, ny);
            const int16_t *oursH = VoxelRelief_Cell(inst, x, y);
            const int16_t *theirsH = VoxelRelief_Cell(other, nx, ny);
            /* their heights over our base, in pixels (and their depths: our
             * builder draws our base's) */
            float rebase = (VoxelRelief_Base(other) - VoxelRelief_Base(inst)) * 16.0f;
            const unsigned last = (VOXEL_RELIEF_SIDE - 1) * VOXEL_RELIEF_SIDE;
            float hA[VOXEL_RELIEF_SIDE], zA[VOXEL_RELIEF_SIDE];
            float hB[VOXEL_RELIEF_SIDE], zB[VOXEL_RELIEF_SIDE];
            float gap = 0.0f;
            bool meets = true;

            /* our south edge and their north edge, point by point of the
             * lattice: the same ground (a band's slope too) a row apart */
            for (unsigned k = 0; k < VOXEL_RELIEF_SIDE; ++k)
            {
                hA[k] = oursH ? oursH[last + k] : 0.0f;
                zA[k] = (float)y + 1.0f + (ours ? ours[last + k] : 0) / 16.0f;
                hB[k] = (theirsH ? theirsH[k] : 0.0f) + rebase;
                zB[k] = (float)y + 1.0f + ((theirs ? theirs[k] : 0) + rebase) / 16.0f;
                if (zB[k] - zA[k] > gap)
                    gap = zB[k] - zA[k];
                if (fabsf(hB[k] - hA[k]) > 1.0f)
                    meets = false;
            }
            /* (a map with an ease meets it with no gap: VoxelRelief_Stretch) */
            if (gap > 0.02f && meets && !VoxelRelief_Stretch(inst, NULL, NULL, NULL))
            {
                /* A row of the drawing a cell of the gap: a gap of more than
                 * one (ground two levels under the base: Route 111's south
                 * end over Mauville) repeats the edge row, never stretches it. */
                unsigned rows = (unsigned)ceilf(gap - 0.02f);
                float vb;

                if (!uv && !(uv = VoxelMesh_TileUV(b, x, y, &u0, &v0, &u1, &v1)))
                    return;
                vb = v0 + (v1 - v0) * (gap / (float)rows < 1.0f ? gap / (float)rows : 1.0f);
                for (unsigned s = 0; s < rows; ++s)
                {
                    float f0 = s / (float)rows, f1 = (s + 1) / (float)rows;

                    for (unsigned k = 0; k + 1 < VOXEL_RELIEF_SIDE; ++k)
                    {
                        float xa = wx + k / (float)(VOXEL_RELIEF_SIDE - 1);
                        float xb = wx + (k + 1) / (float)(VOXEL_RELIEF_SIDE - 1);
                        float ua = u0 + (u1 - u0) * k / (float)(VOXEL_RELIEF_SIDE - 1);
                        float ub = u0 + (u1 - u0) * (k + 1) / (float)(VOXEL_RELIEF_SIDE - 1);
#define SEAM_H(i, f) ((hA[i] + (hB[i] - hA[i]) * (f)) / 16.0f)
#define SEAM_Z(i, f) (zA[i] + (zB[i] - zA[i]) * (f))
                        VoxelBuilder_Quad(b,
                            &(VoxelVertex){xa, SEAM_H(k, f0),     SEAM_Z(k, f0),     ua, v0, SHADE_TOP},
                            &(VoxelVertex){xb, SEAM_H(k + 1, f0), SEAM_Z(k + 1, f0), ub, v0, SHADE_TOP},
                            &(VoxelVertex){xb, SEAM_H(k + 1, f1), SEAM_Z(k + 1, f1), ub, vb, SHADE_TOP},
                            &(VoxelVertex){xa, SEAM_H(k, f1),     SEAM_Z(k, f1),     ua, vb, SHADE_TOP});
#undef SEAM_H
#undef SEAM_Z
                    }
                }
                /* a step down is hung from where the ground now ends */
                southGap = gap;
            }
        }
        if (!any)
            continue;
        if (!uv && !(uv = VoxelMesh_TileUV(b, x, y, &u0, &v0, &u1, &v1)))
            return;
        /* a wall of the cell's own drawing from their edge up to ours, each
         * stretch of the lattice where ours stands over theirs, at our
         * edge's points (a point h up stands h rows south) */
        for (unsigned k = 0; k + 1 < VOXEL_RELIEF_SIDE; ++k)
        {
            const float last = (float)(VOXEL_RELIEF_SIDE - 1);
            float ta = ourH[k] / 16.0f, tb = ourH[k + 1] / 16.0f;
            float la = fminf(theirH[k], ourH[k]) / 16.0f, lb = fminf(theirH[k + 1], ourH[k + 1]) / 16.0f;
            float fa = fminf(1.0f, ta - la), fb = fminf(1.0f, tb - lb);
            float ka = k / last, kb = (k + 1) / last;

            if ((ta - la <= 0.05f && tb - lb <= 0.05f) || !((stands >> k) & 1u))
                continue;
            if (d == 0) /* faces north, at the north edge */
            {
                float za = (float)y + ourZ[k], zb = (float)y + ourZ[k + 1];
                float ua = u0 + (u1 - u0) * ka, ub = u0 + (u1 - u0) * kb;

                VoxelBuilder_Quad(b,
                    &(VoxelVertex){wx + kb, tb, zb, ub, v0,                 SHADE_NORTH},
                    &(VoxelVertex){wx + ka, ta, za, ua, v0,                 SHADE_NORTH},
                    &(VoxelVertex){wx + ka, la, za, ua, v0 + (v1 - v0) * fa, SHADE_NORTH},
                    &(VoxelVertex){wx + kb, lb, zb, ub, v0 + (v1 - v0) * fb, SHADE_NORTH});
            }
            else if (d == 1) /* faces south, at the south edge */
            {
                float za = (float)y + 1.0f + ourZ[k] + southGap, zb = (float)y + 1.0f + ourZ[k + 1] + southGap;
                float ua = u0 + (u1 - u0) * ka, ub = u0 + (u1 - u0) * kb;

                VoxelBuilder_Quad(b,
                    &(VoxelVertex){wx + ka, ta, za, ua, v1,                 SHADE_SOUTH},
                    &(VoxelVertex){wx + kb, tb, zb, ub, v1,                 SHADE_SOUTH},
                    &(VoxelVertex){wx + kb, lb, zb, ub, v1 - (v1 - v0) * fb, SHADE_SOUTH},
                    &(VoxelVertex){wx + ka, la, za, ua, v1 - (v1 - v0) * fa, SHADE_SOUTH});
            }
            else
            {
                float za = (float)y + ka + ourZ[k], zb = (float)y + kb + ourZ[k + 1];
                float va = v0 + (v1 - v0) * ka, vb = v0 + (v1 - v0) * kb;

                if (d == 2) /* faces west, at the west edge */
                    VoxelBuilder_Quad(b,
                        &(VoxelVertex){wx, ta, za, u0,                 va, SHADE_WEST},
                        &(VoxelVertex){wx, tb, zb, u0,                 vb, SHADE_WEST},
                        &(VoxelVertex){wx, lb, zb, u0 + (u1 - u0) * fb, vb, SHADE_WEST},
                        &(VoxelVertex){wx, la, za, u0 + (u1 - u0) * fa, va, SHADE_WEST});
                else /* faces east, at the east edge */
                    VoxelBuilder_Quad(b,
                        &(VoxelVertex){wx + 1.0f, tb, zb, u1,                 vb, SHADE_EAST},
                        &(VoxelVertex){wx + 1.0f, ta, za, u1,                 va, SHADE_EAST},
                        &(VoxelVertex){wx + 1.0f, la, za, u1 - (u1 - u0) * fa, va, SHADE_EAST},
                        &(VoxelVertex){wx + 1.0f, lb, zb, u1 - (u1 - u0) * fb, vb, SHADE_EAST});
            }
        }
        /* Audit #20 (Brennan 2026-11-23 21:03, Route 131 / Pacifidlog: "fix this issue, use the sea around it"): down a
         * west or east seam each row's wall stands where its row is drawn, as far south as it is high; where the next
         * row south is a level higher it is drawn a row further south, and the slope of ground between the two rows
         * (a cliff's face, Pacifidlog's east rocks at rows 33-34) had no wall under it: a gap onto the sky down to the
         * sea beside it. The wall goes on from this row's south end to the next row's north end, down to the other
         * map's edge (the sea's level there). WJPC_SEAM_ROW_GAP_OPEN=1: as before. */
        if ((d == 2 || d == 3) && y + 1 < inst->originY + inst->height && VoxelWorld_GetInstanceAt(nx, ny + 1) == other)
        {
            static int sOpen = -1;
            float nH[VOXEL_RELIEF_SIDE], nZ[VOXEL_RELIEF_SIDE], tH[VOXEL_RELIEF_SIDE], tZ[VOXEL_RELIEF_SIDE];
            const unsigned last = VOXEL_RELIEF_SIDE - 1;
            float zEnd, zNext;

            if (sOpen < 0)
                sOpen = getenv("WJPC_SEAM_ROW_GAP_OPEN") != NULL;
            if (sOpen)
                continue;
            SeamEdge(inst, x, y + 1, d, nH, nZ, NULL);
            SeamEdge(other, nx, ny + 1, d ^ 1, tH, tZ, NULL);
            zEnd = (float)y + 1.0f + ourZ[last];
            zNext = (float)y + 1.0f + nZ[0];
            if (zNext - zEnd > 0.02f)
            {
                float ta = ourH[last] / 16.0f, tb = nH[0] / 16.0f;
                float la = fminf(theirH[last], ourH[last]) / 16.0f, lb = fminf(tH[0] + rebase16, nH[0]) / 16.0f;
                float fa = fminf(1.0f, ta - la), fb = fminf(1.0f, tb - lb);
                float ex = d == 2 ? wx : wx + 1.0f;

                if (ta - la > 0.05f || tb - lb > 0.05f)
                {
                    if (d == 2)
                        VoxelBuilder_Quad(b,
                            &(VoxelVertex){ex, ta, zEnd,  u0,                  v1, SHADE_WEST},
                            &(VoxelVertex){ex, tb, zNext, u0,                  v1, SHADE_WEST},
                            &(VoxelVertex){ex, lb, zNext, u0 + (u1 - u0) * fb, v1, SHADE_WEST},
                            &(VoxelVertex){ex, la, zEnd,  u0 + (u1 - u0) * fa, v1, SHADE_WEST});
                    else
                        VoxelBuilder_Quad(b,
                            &(VoxelVertex){ex, tb, zNext, u1,                  v1, SHADE_EAST},
                            &(VoxelVertex){ex, ta, zEnd,  u1,                  v1, SHADE_EAST},
                            &(VoxelVertex){ex, la, zEnd,  u1 - (u1 - u0) * fa, v1, SHADE_EAST},
                            &(VoxelVertex){ex, lb, zNext, u1 - (u1 - u0) * fb, v1, SHADE_EAST});
                }
            }
        }
    }
}

/*
 * A cell's cliffs (voxel_relief.h), wherever its west, east or south edge
 * stands over its neighbour's. Down the west and east the rock's own edge
 * goes on to the ground, each pixel of the drawing's edge column drawn down
 * as far as the step falls: the camera sees such a wall edge on, and what
 * it shows is the rock's outline going down, never a sliver of anything
 * else (a cut tile's clear background stays clear down it too). Down the
 * south, the mountain's face (`wall`, or none for VOXEL_RELIEF_NO_FACE), a
 * level of it a level of the wall. Never the north edge, the back.
 */
/*
 * Whether a back stands as a wall straight up (EmitNorthFace): the cell north
 * of it in the same map, a level and the fill's two pixels under it lower
 * somewhere along the edge.
 */
#define VOXEL_NORTH_FACE_MIN 18     /* pixels: a level, and the fill's two under it */

static bool NorthFaceStands(const VoxelMapInstance *inst, int x, int y, const int16_t *g)
{
    const int n = VOXEL_RELIEF_SIDE - 1;
    const int16_t *o;

    if (VoxelWorld_GetInstanceAt(x, y - 1) != inst)
        return false;
    o = VoxelRelief_Cell(inst, x, y - 1);
    for (int k = 0; k <= n; ++k)
        if (g[k] - (o != NULL ? o[n * VOXEL_RELIEF_SIDE + k] : 0) >= VOXEL_NORTH_FACE_MIN)
            return true;
    return false;
}

/* a side's corners pushed south to the back's wall at z `wall` */
static void WallClip(float wall, float *za, float *zb, float *zc, float *zd)
{
    *za = fmaxf(*za, wall);
    *zb = fmaxf(*zb, wall);
    *zc = fmaxf(*zc, wall);
    *zd = fmaxf(*zd, wall);
}

/* a step between two stretches of a back's wall: the side of the rock at x,
 * from za to zb, from `bot` up to `top` pixels, a tile a level */
static void WallStep(VoxelBuilder *b, float x, float za, float zb, float bot, float top, bool west,
                     float u0, float v0, float u1, float v1)
{
    for (float h0 = bot; h0 < top; h0 = floorf(h0 / 16.0f + 1.0f) * 16.0f)
    {
        float h1 = fminf(top, floorf(h0 / 16.0f + 1.0f) * 16.0f), base = floorf(h0 / 16.0f) * 16.0f;
        float va = v1 + (v0 - v1) * (h0 - base) / 16.0f, vb = v1 + (v0 - v1) * (h1 - base) / 16.0f;
        float ua = u0 + (u1 - u0) * (za - floorf(za)), ub = ua + (u1 - u0) * fminf(1.0f, zb - za);

        if (west)
            VoxelBuilder_Quad(b, &(VoxelVertex){x, h0 / 16.0f, za, ua, va, SHADE_WEST},
                              &(VoxelVertex){x, h0 / 16.0f, zb, ub, va, SHADE_WEST},
                              &(VoxelVertex){x, h1 / 16.0f, zb, ub, vb, SHADE_WEST},
                              &(VoxelVertex){x, h1 / 16.0f, za, ua, vb, SHADE_WEST});
        else
            VoxelBuilder_Quad(b, &(VoxelVertex){x, h0 / 16.0f, zb, ub, va, SHADE_EAST},
                              &(VoxelVertex){x, h0 / 16.0f, za, ua, va, SHADE_EAST},
                              &(VoxelVertex){x, h1 / 16.0f, za, ua, vb, SHADE_EAST},
                              &(VoxelVertex){x, h1 / 16.0f, zb, ub, vb, SHADE_EAST});
    }
}

/* the part of a wall polygon at or under (sign 1) or at or over (sign -1)
 * `limit`, in height (or in z, `onZ`): its corners and where its edges
 * cross it */
static int ClipWall(const VoxelVertex *in, int n, float limit, float sign, bool onZ, VoxelVertex *out)
{
    int m = 0;

    for (int i = 0; i < n; ++i)
    {
        const VoxelVertex *a = &in[i], *c = &in[(i + 1) % n];
        float da = sign * ((onZ ? a->z : a->y) - limit), dc = sign * ((onZ ? c->z : c->y) - limit);

        if (da <= 0.0f)
            out[m++] = *a;
        if ((da < 0.0f && dc > 0.0f) || (da > 0.0f && dc < 0.0f))
        {
            float t = da / (da - dc);
            VoxelVertex v = *a;

            v.x += (c->x - a->x) * t;
            v.y += (c->y - a->y) * t;
            v.z += (c->z - a->z) * t;
            v.u += (c->u - a->u) * t;
            v.v += (c->v - a->v) * t;
            out[m++] = v;
        }
    }
    return m;
}

/* a fan from the first corner, two triangles a quad */
static void WallFan(VoxelBuilder *b, const VoxelVertex *p, int n)
{
    for (int i = 1; i + 1 < n; i += 2)
        VoxelBuilder_Quad(b, &p[0], &p[i], &p[i + 1], &p[i + 2 < n ? i + 2 : i + 1]);
}

/*
 * A wall standing more than a level, a tile a level: the quad `p` (its
 * corners round it, placed) cut in bands down from `top`, each band the
 * tile again, its row v0 at the band's top and v1 a level down. Stretched
 * over the whole height, a wall of a cave carved deep (Meteor Falls' sides,
 * nine levels and more) was one tile smeared to a few dark rows. A south
 * wall keeps the u of its corners; a west or east one (`across` 1 or -1)
 * takes it from where it stands in the world, a tile a cell of z from u0 to
 * u1 (-1: the other way), cut where the cells meet: on a drawn map its
 * points stand as far south as they are high, and such a wall runs over
 * several cells of z.
 */
static void WallBands(VoxelBuilder *b, const VoxelVertex p[4], float top, float v0, float v1,
                      int across, float u0, float u1)
{
    float lo = fminf(fminf(p[0].y, p[1].y), fminf(p[2].y, p[3].y));

    for (float band = top; band > lo + 1.0f / 64.0f; band -= 1.0f)
    {
        VoxelVertex under[8], piece[8];
        int n = ClipWall(p, 4, band, 1.0f, false, under);
        float zlo, zhi;

        n = ClipWall(under, n, band - 1.0f, -1.0f, false, piece);
        if (n < 3)
            continue;
        for (int i = 0; i < n; ++i)
            piece[i].v = v0 + (v1 - v0) * (band - piece[i].y);
        if (across == 0)
        {
            WallFan(b, piece, n);
            continue;
        }
        zlo = zhi = piece[0].z;
        for (int i = 1; i < n; ++i)
        {
            zlo = fminf(zlo, piece[i].z);
            zhi = fmaxf(zhi, piece[i].z);
        }
        for (float cell = floorf(zlo); cell < zhi; cell += 1.0f)
        {
            VoxelVertex south[12], tile[12];
            int m = ClipWall(piece, n, cell + 1.0f, 1.0f, true, south);

            m = ClipWall(south, m, cell, -1.0f, true, tile);
            if (m < 3)
                continue;
            for (int i = 0; i < m; ++i)
                tile[i].u = across > 0 ? u0 + (u1 - u0) * (tile[i].z - cell) : u1 - (u1 - u0) * (tile[i].z - cell);
            WallFan(b, tile, m);
        }
    }
}

static void EmitCliffWalls(VoxelBuilder *b, const VoxelMapInstance *inst, int x, int y,
                           const int16_t *g, const int16_t *s, int wall, int art, int artY,
                           unsigned sides, bool backed)
{
    static const int16_t kLevel[VOXEL_RELIEF_SIDE * VOXEL_RELIEF_SIDE];
    const int n = VOXEL_RELIEF_SIDE - 1, R = VOXEL_RELIEF_SIDE;
    float f0, g0, f1, g1;      /* the face */
    float o0, p0, o1, p1;      /* the cell's own drawing */
    bool face = wall != VOXEL_RELIEF_NO_FACE && MetatileUV(b, wall, &f0, &g0, &f1, &g1);
    bool own = art >= 0 ? MetatileUV(b, art, &o0, &p0, &o1, &p1)
                        : VoxelMesh_TileUV(b, x, artY, &o0, &p0, &o1, &p1);
    /* a hand-built block: its west and east walls are the face as well */
    bool blockWalls = face && (sides & VOXEL_RELIEF_BLOCK_WALLS) != 0;
    /* a back standing straight up (EmitNorthFace): the sides stop at it. Run
     * on at 45 degrees as the old sloped back was, their north end hung past
     * the wall as a fin over the lower ground beside it */
    const bool walled = backed && NorthFaceStands(inst, x, y, g);

    for (int d = 0; d < 3; ++d)
    {
        int nx = x + (d == 0 ? -1 : d == 1 ? 1 : 0), ny = y + (d == 2 ? 1 : 0), st;
        const int16_t *o = NULL, *os = NULL;

        if (d == 2 ? !face : !(own || blockWalls))
            continue;
        if (VoxelWorld_GetInstanceAt(nx, ny) == inst)
        {
            o = VoxelRelief_Cell(inst, nx, ny);
            os = o != NULL ? VoxelRelief_Depth(inst, nx, ny) : NULL;
        }
        if (o == NULL)
            o = os = kLevel;
        if (d < 2 && !(sides & VOXEL_RELIEF_THIN_DECK) && o != kLevel)
        {
            /* beside a bridge's thin deck: the water under it runs up to
             * this cell's side, walled down to the water (seen past the
             * deck's edge, the gap under it showed the void) */
            static VOXEL_TLS int16_t under[VOXEL_RELIEF_SIDE * VOXEL_RELIEF_SIDE];
            float nfoot;
            int ng, nw;
            unsigned ns = 0;

            if (VoxelRelief_Cut(inst, nx, ny, &nfoot, &ng, &nw, &ns) != -1 && (ns & VOXEL_RELIEF_THIN_DECK))
            {
                for (int i = 0; i < R * R; ++i)
                    under[i] = (int16_t)lroundf(nfoot * 16.0f);
                o = under;
            }
        }
        /* A wall the same all along the edge and more than a level tall
         * (the rock round a cave carved deep): one piece the edge long, cut
         * a tile a level (WallBands), not four */
        st = 1;
        if ((d == 2 ? face : blockWalls) && !(sides & VOXEL_RELIEF_THIN_DECK))
        {
            int a0 = d == 0 ? 0 : d == 1 ? n : n * R, c0 = d == 0 ? n : 0;
            int along = d == 2 ? 1 : R;
            bool same = g[a0] - o[c0] > 16;

            for (int k = 1; k <= n && same; ++k)
                same = g[a0 + k * along] == g[a0] && o[c0 + k * along] == o[c0]
                    && s[a0 + k * along] == s[a0] && os[c0 + k * along] == os[c0];
            if (same)
                st = n;
        }
        for (int k = 0; k < n; k += st)
        {
            /* ours at a and b, theirs at c and d, along the edge */
            int ia, ib, ic, id;
            float ta, tb, ba, bb, za, zb, zc, zd, xa, xb;

            if (d == 0)
            {
                ia = k * R; ib = (k + st) * R; ic = k * R + n; id = (k + st) * R + n;
            }
            else if (d == 1)
            {
                ia = k * R + n; ib = (k + st) * R + n; ic = k * R; id = (k + st) * R;
            }
            else
            {
                ia = n * R + k; ib = n * R + k + st; ic = k; id = k + st;
            }
            if (g[ia] - o[ic] <= 0 && g[ib] - o[id] <= 0)
                continue;
            /* The edge column's pixels of this stretch that the tile clears
             * are the ground beside the rock, not rock: no wall stands on
             * them (a flank's sand corner hung from the rock as a flat
             * yellow triangle) */
            if (d < 2 && !blockWalls && !((sides >> ((d == 0 ? 0u : 4u) + (unsigned)k)) & 1u))
                continue;
            ta = g[ia] / 16.0f; tb = g[ib] / 16.0f;
            ba = (g[ia] > o[ic] ? o[ic] : g[ia]) / 16.0f;
            bb = (g[ib] > o[id] ? o[id] : g[ib]) / 16.0f;
            if (sides & VOXEL_RELIEF_THIN_DECK)
            {
                ba = fmaxf(ba, ta - VOXEL_RELIEF_DECK_EDGE);
                bb = fmaxf(bb, tb - VOXEL_RELIEF_DECK_EDGE);
            }
            if (d == 2)
            {
                float ua = f0 + (f1 - f0) * k / n, ub = f0 + (f1 - f0) * (k + st) / n;
                float dva = (g1 - g0) * fminf(1.0f, ta - ba), dvb = (g1 - g0) * fminf(1.0f, tb - bb);

                xa = x + k / (float)n; xb = x + (k + st) / (float)n;
                za = y + 1 + s[ia] / 16.0f; zb = y + 1 + s[ib] / 16.0f;
                zc = y + 1 + os[ic] / 16.0f; zd = y + 1 + os[id] / 16.0f;
                if (ta - ba > 1.0f || tb - bb > 1.0f)
                {
                    const VoxelVertex p[4] = {{xa, ta, za, ua, g0, SHADE_SOUTH}, {xb, tb, zb, ub, g0, SHADE_SOUTH},
                                              {xb, bb, zd, ub, g0, SHADE_SOUTH}, {xa, ba, zc, ua, g0, SHADE_SOUTH}};

                    WallBands(b, p, fmaxf(ta, tb), g0, g1, 0, 0.0f, 0.0f);
                }
                else
                    VoxelBuilder_Quad(b, &(VoxelVertex){xa, ta, za, ua, g0, SHADE_SOUTH},
                                      &(VoxelVertex){xb, tb, zb, ub, g0, SHADE_SOUTH},
                                      &(VoxelVertex){xb, bb, zd, ub, g0 + dvb, SHADE_SOUTH},
                                      &(VoxelVertex){xa, ba, zc, ua, g0 + dva, SHADE_SOUTH});
            }
            else if (blockWalls)
            {
                /* the face down the side, as down the south, laid on the
                 * wall where it stands in the world: across it by its z,
                 * down it from the level its top is at (a taller wall
                 * stretches it). A block's side beside a face is two
                 * pieces (the rows of two cells): mapped by the world they
                 * read as the one wall they are. Seen from the east the
                 * south is on the left, so that one runs the other way. */
                float xe = (float)(d == 0 ? x : x + 1);
                float shade = d == 0 ? SHADE_WEST : SHADE_EAST;
                float zm, yt;

                za = y + k / (float)n + s[ia] / 16.0f;
                zb = y + (k + st) / (float)n + s[ib] / 16.0f;
                zc = y + k / (float)n + os[ic] / 16.0f;
                zd = y + (k + st) / (float)n + os[id] / 16.0f;
                if (walled)
                    WallClip(y + (d == 0 ? fmaxf(s[0], s[1]) : fmaxf(s[n - 1], s[n])) / 16.0f, &za, &zb, &zc, &zd);
                zm = floorf((za + zb + zc + zd) * 0.25f);
                yt = ceilf(fmaxf(ta, tb) - 0.01f);
#define WALL_U(z) (d == 0 ? f0 + (f1 - f0) * fminf(1.0f, fmaxf(0.0f, (z) - zm)) \
                          : f1 - (f1 - f0) * fminf(1.0f, fmaxf(0.0f, (z) - zm)))
#define WALL_V(h) (g0 + (g1 - g0) * fminf(1.0f, fmaxf(0.0f, yt - (h))))
                if (yt - fminf(ba, bb) > 1.0f)
                {
                    /* (facing east, its corners the other way round) */
                    const VoxelVertex p[4] = {
                        {xe, ta, za, 0.0f, 0.0f, shade}, {xe, tb, zb, 0.0f, 0.0f, shade},
                        {xe, bb, zd, 0.0f, 0.0f, shade}, {xe, ba, zc, 0.0f, 0.0f, shade}};
                    const VoxelVertex q[4] = {p[1], p[0], p[3], p[2]};

                    WallBands(b, d == 0 ? p : q, yt, g0, g1, d == 0 ? 1 : -1, f0, f1);
                }
                else if (d == 0) /* faces west */
                    VoxelBuilder_Quad(b, &(VoxelVertex){xe, ta, za, WALL_U(za), WALL_V(ta), shade},
                                      &(VoxelVertex){xe, tb, zb, WALL_U(zb), WALL_V(tb), shade},
                                      &(VoxelVertex){xe, bb, zd, WALL_U(zd), WALL_V(bb), shade},
                                      &(VoxelVertex){xe, ba, zc, WALL_U(zc), WALL_V(ba), shade});
                else        /* faces east */
                    VoxelBuilder_Quad(b, &(VoxelVertex){xe, tb, zb, WALL_U(zb), WALL_V(tb), shade},
                                      &(VoxelVertex){xe, ta, za, WALL_U(za), WALL_V(ta), shade},
                                      &(VoxelVertex){xe, ba, zc, WALL_U(zc), WALL_V(ba), shade},
                                      &(VoxelVertex){xe, bb, zd, WALL_U(zd), WALL_V(bb), shade});
#undef WALL_U
#undef WALL_V
            }
            else
            {
                /* the edge column of the drawing, half a pixel in */
                float ue = d == 0 ? o0 + (o1 - o0) / 32.0f : o1 - (o1 - o0) / 32.0f;
                float va = p0 + (p1 - p0) * k / n, vb = p0 + (p1 - p0) * (k + st) / n;
                float xe = (float)(d == 0 ? x : x + 1);

                za = y + k / (float)n + s[ia] / 16.0f;
                zb = y + (k + st) / (float)n + s[ib] / 16.0f;
                zc = y + k / (float)n + os[ic] / 16.0f;
                zd = y + (k + st) / (float)n + os[id] / 16.0f;
                if (walled)
                    WallClip(y + (d == 0 ? fmaxf(s[0], s[1]) : fmaxf(s[n - 1], s[n])) / 16.0f, &za, &zb, &zc, &zd);
                /* lit as the drawing it goes on from: its own light is in
                 * it (EmitRelief), so the outline reads as one with it */
                b->artShaded = true;
                if (d == 0) /* faces west */
                    VoxelBuilder_Quad(b, &(VoxelVertex){xe, ta, za, ue, va, SHADE_TOP},
                                      &(VoxelVertex){xe, tb, zb, ue, vb, SHADE_TOP},
                                      &(VoxelVertex){xe, bb, zd, ue, vb, SHADE_TOP},
                                      &(VoxelVertex){xe, ba, zc, ue, va, SHADE_TOP});
                else        /* faces east */
                    VoxelBuilder_Quad(b, &(VoxelVertex){xe, tb, zb, ue, vb, SHADE_TOP},
                                      &(VoxelVertex){xe, ta, za, ue, va, SHADE_TOP},
                                      &(VoxelVertex){xe, ba, zc, ue, va, SHADE_TOP},
                                      &(VoxelVertex){xe, bb, zd, ue, vb, SHADE_TOP});
                b->artShaded = false;
            }
        }
    }
}

int VoxelMesh_DraftSlot(const VoxelMapInstance *inst, const struct VoxelAtlasMap *atlas,
                        int x, int y, unsigned *uncovered)
{
    int metatile;
    unsigned slot;

    /* Straight to the world, not through the window's cache: the build that
     * may be running keeps its own. Outdoors nothing is void. */
    if (inst->indoor && VoxelWorld_ClassifyTile(x, y) == VOXEL_SHAPE_VOID)
        return -1;
    metatile = VoxelWorld_GetMetatileId(x, y);
    /* The trunk's ground, not the card of the crown drawn over it. */
    if (VoxelWorld_UsesTreeSprites(inst))
        metatile = VoxelTree_GroundMetatile(inst->secondaryTileset, metatile);
    if (metatile < 0 || metatile >= (int)VOXEL_METATILE_REAL)
        return -1;
    slot = atlas->slotOf[metatile];
    if (slot == 0 || slot == VOXEL_SLOT_PENDING)
    {
        ++*uncovered;
        return -1;
    }
    if (slot == VOXEL_SLOT_ABSENT)
        return -1;
    return (int)slot - 1;
}

void VoxelMesh_DraftCell(VoxelBuilder *b, const VoxelMapInstance *inst, int x, int y, int slot)
{
    float u0, v0, u1, v1;

    VoxelAtlas_SlotUV((unsigned)slot, &u0, &v0, &u1, &v1);
    /* At the level its relief puts it, as the ground of a modelled building
     * is: the draft is for seeing something, not for matching the slopes. */
    b->lift = VoxelRelief_CellLift(inst, x, y);
    b->shift = VoxelRelief_CellShift(inst, x, y);
    VoxelMesh_Top(b, (float)x, (float)y, 0.0f, 0.0f, u0, v0, u1, v1, SHADE_TOP);
    b->lift = 0.0f;
    b->shift = 0.0f;
}

/*
 * The ground behind a terrace's rim, run on under the cell at `foot`: under a
 * cut tile's own run on (kCutFill), and two pixels past the cell all round:
 * the floors beside it stand a pixel or two higher, and a ray between them
 * would find the clear colour. Those of cells side by side overlap: a hair
 * apart, by parity, so two drawings never share a plane.
 */
static void EmitGroundRunOn(VoxelBuilder *b, const VoxelMapInstance *inst, int x, int y,
                            float foot, float u0, float v0, float u1, float v1)
{
    b->lift = foot - (2.0f + 0.125f * (float)((x + y) & 1)) / 16.0f;
    b->shift = b->lift;
    /* sunlit as the open ground it stands for: under the terrain's own
     * heights every ray is stopped, and the fill came out as a shadowed
     * olive patch */
    b->lightingConstant = 1.0f;
    EmitRunOn(b, inst, x, y, u0, v0, u1, v1);
    b->lightingConstant = -1.0f;
    b->lift = 0.0f;
    b->shift = 0.0f;
}

/*
 * A back's north side (a top, or a cut over the terrace behind). The game's
 * camera never looks at it, but from above, past a rim, it is the gap between
 * the two: a point h pixels up stands h/16 rows south, so a back k levels up
 * leaves k rows open. The flat ground run on under the cell (EmitGroundRunOn)
 * closes one; past that, the ground of the cell north goes on flat, at its
 * own south edge's height, up to this cell's north edge, and there the
 * mountain's back stands straight up to the top: a cliff wall, a tile a
 * level of its wall (the cell's own, else the map's commonest - VoxelRelief_
 * FaceTile; the ground behind where the map has none). From the game's 45
 * degrees both lie hidden behind the top's edge, as before; from above, the
 * ground behind shows, as in the drawing; from the side, a cliff. Laid as one
 * 45-degree slope instead it was a sheet stretched down to the ground, seen
 * from any camera but the game's. Route 112's rough slope stands backs nine
 * levels over the plateau behind it. The wall stands only behind a whole top
 * (wallTile -1 otherwise: the ground alone): through a cut tile's cleared
 * pixels it showed as teeth. Where the top slopes along its edge the wall
 * goes in steps square to the north (below), and the cell's sides stop at it
 * (EmitCliffWalls).
 */
static void EmitNorthFace(VoxelBuilder *b, const VoxelMapInstance *inst, int x, int y,
                          const int16_t *g, const int16_t *s, int wallTile, int groundTile)
{
    static const int16_t kFlat[VOXEL_RELIEF_SIDE * VOXEL_RELIEF_SIDE];
    const int n = VOXEL_RELIEF_SIDE - 1;
    const int16_t *o, *os;
    float u0, v0, u1, v1, gu0, gv0, gu1, gv1;
    int lo = INT16_MAX, hi = INT16_MIN;
    bool standing, ground;

    if (!NorthFaceStands(inst, x, y, g))
        return;
    o = VoxelRelief_Cell(inst, x, y - 1);
    os = o != NULL ? VoxelRelief_Depth(inst, x, y - 1) : NULL;
    if (o == NULL)
        o = os = kFlat;
    /* (their south edge, ours north) */
    o += n * VOXEL_RELIEF_SIDE;
    os += n * VOXEL_RELIEF_SIDE;
    for (int k = 0; k <= n; ++k)
    {
        lo = o[k] < lo ? o[k] : lo;
        hi = g[k] > hi ? g[k] : hi;
    }
    standing = wallTile >= 0 && MetatileUV(b, wallTile, &u0, &v0, &u1, &v1);
    ground = groundTile >= 0 && MetatileUV(b, groundTile, &gu0, &gv0, &gu1, &gv1);
    if (!standing && !ground)
        return;
    for (int k = 0; k < n; ++k)
    {
        float xs[2] = { x + k / (float)n, x + (k + 1) / (float)n };
        float bot[2] = { o[k], o[k + 1] }, top[2] = { g[k], g[k + 1] };
        float zb[2] = { y + os[k] / 16.0f, y + os[k + 1] / 16.0f };
        float zt[2] = { y + s[k] / 16.0f, y + s[k + 1] / 16.0f };
        float ua = u1 - (u1 - u0) * k / n, ub = u1 - (u1 - u0) * (k + 1) / n;

        for (int e = 0; e < 2; ++e)
            if (top[e] < bot[e])
                top[e] = bot[e];
        /* square to the north, under the top's edge: where the top slopes
         * along the edge, the edge runs aslant (a point h up stands h rows
         * south), and a wall on it faced the camera, a fin over the lower
         * ground beside it. So a step of the wall a stretch, each at the
         * stretch's south end, the steps closed between them */
        zt[0] = zt[1] = fmaxf(zt[0], zt[1]);
        if (standing && k + 1 < n)
        {
            float zn = y + fmaxf(s[k + 1], s[k + 2]) / 16.0f;

            if (fabsf(zn - zt[1]) > 0.001f)
                WallStep(b, xs[1], fminf(zn, zt[1]), fmaxf(zn, zt[1]), o[k + 1], g[k + 1] > o[k + 1] ? g[k + 1] : o[k + 1],
                         zn < zt[1], u0, v0, u1, v1);
        }
        /* the wall, band by band of a level, the tile's top at the band's
         * top and read from behind (as VoxelMesh_North) */
        for (int band = lo; standing && band < hi; band += 16)
        {
            float h[2][2];
            bool any = false;

            for (int e = 0; e < 2; ++e)
            {
                h[e][0] = fmaxf(bot[e], (float)band);
                h[e][1] = fminf(top[e], (float)(band + 16));
                if (h[e][1] < h[e][0])
                    h[e][1] = h[e][0];
                any = any || h[e][1] > h[e][0];
            }
            if (!any)
                continue;
#define NORTH_V(hh) (v1 + (v0 - v1) * ((hh) - band) / 16.0f)
            VoxelBuilder_Quad(b, &(VoxelVertex){xs[1], h[1][0] / 16.0f, zt[1], ub, NORTH_V(h[1][0]), SHADE_NORTH},
                              &(VoxelVertex){xs[0], h[0][0] / 16.0f, zt[0], ua, NORTH_V(h[0][0]), SHADE_NORTH},
                              &(VoxelVertex){xs[0], h[0][1] / 16.0f, zt[0], ua, NORTH_V(h[0][1]), SHADE_NORTH},
                              &(VoxelVertex){xs[1], h[1][1] / 16.0f, zt[1], ub, NORTH_V(h[1][1]), SHADE_NORTH});
#undef NORTH_V
        }
        /* the ground behind, flat from the north cell's edge to the wall's
         * foot, a pixel under it (the fill run on under the cell lies two
         * under), a tile a row so that it is not stretched */
        if (!ground)
            continue;
        b->lightingConstant = 1.0f;
        /* (Brennan's doubled-ground cells - Route 111 28-30,86-87 and 29-32,119-120, Route 112's 14 - were this ground
         * and a cut tile's fill run on under the same cell from beside it (EmitRunOn: foot - 1/16 .. 1.5/16, not two
         * under as said above) in one plane: the depth test took either. Half a pixel under now, over every fill.
         * WJPC_NORTH_GROUND_OLD=1: a pixel, as before.) */
        {
            static int sOld = -1;
            if (sOld < 0)
                sOld = getenv("WJPC_NORTH_GROUND_OLD") != NULL;
            for (int e = 0; e < 2; ++e)
                bot[e] -= sOld ? 1.0f : 0.5f;
        }
        {
            float za = fminf(zb[0], zb[1]), zz = fmaxf(zt[0], zt[1]);

            for (float row = floorf(za); row < zz; row += 1.0f)
            {
                float p[2][2], hh[2];
                bool any = false;

                for (int e = 0; e < 2; ++e)
                {
                    p[e][0] = fmaxf(zb[e], row);
                    p[e][1] = fminf(zt[e], row + 1.0f);
                    if (p[e][1] < p[e][0])
                        p[e][1] = p[e][0];
                    any = any || p[e][1] > p[e][0];
                    hh[e] = bot[e] / 16.0f;
                }
                if (!any)
                    continue;
#define GROUND_V(zz_) (gv0 + (gv1 - gv0) * ((zz_) - row))
                VoxelBuilder_Quad(b, &(VoxelVertex){xs[0], hh[0], p[0][0], gu0 + (gu1 - gu0) * k / n, GROUND_V(p[0][0]), SHADE_TOP},
                                  &(VoxelVertex){xs[1], hh[1], p[1][0], gu0 + (gu1 - gu0) * (k + 1) / n, GROUND_V(p[1][0]), SHADE_TOP},
                                  &(VoxelVertex){xs[1], hh[1], p[1][1], gu0 + (gu1 - gu0) * (k + 1) / n, GROUND_V(p[1][1]), SHADE_TOP},
                                  &(VoxelVertex){xs[0], hh[0], p[0][1], gu0 + (gu1 - gu0) * k / n, GROUND_V(p[0][1]), SHADE_TOP});
#undef GROUND_V
            }
        }
        b->lightingConstant = -1.0f;
    }
}

/*
 * A tree's cell where a wood climbs a step (voxel_terraces.py, "a wood's
 * step"): its ground is the tree pass's, one flat square at its level
 * (voxel_tree.c), but the step is the relief's - the lower ground run on
 * under the cell, and its walls of the tree's own drawing. Left to the tree
 * pass alone, every step between a lifted tree and a lower one was open onto
 * the clear colour (the Safari Zone's south-west wood).
 */
static void EmitTreeStep(VoxelBuilder *b, const VoxelMapInstance *inst, int x, int y)
{
    const int16_t *relief = VoxelRelief_Cell(inst, x, y);
    float foot = 0.0f, u0, v0, u1, v1;
    int ground = -1, wall = -1;
    unsigned sides = 0xFF;
    int cut;

    if (relief == NULL)
        return;
    cut = VoxelRelief_Cut(inst, x, y, &foot, &ground, &wall, &sides);
    if (getenv("WJPC_TREESTEP_DUMP") != NULL) /* test aid */
        fprintf(stderr, "TREESTEP %d %d (local %d %d) mt %03x cut %d ground %03x wall %03x sides %02x foot %.3f rock %d\n", x, y,
                x - inst->originX, y - inst->originY, VoxelWorld_GetMetatileId(x, y), cut, ground & 0xFFF, wall & 0xFFF,
                sides, foot, (int)VoxelMesh_TreeOnRock(inst, x, y));
    if (cut == -1)
        return;
    if (cut == VOXEL_RELIEF_NO_VARIANT && ground >= 0 && MetatileUV(b, ground, &u0, &v0, &u1, &v1))
        EmitGroundRunOn(b, inst, x, y, foot, u0, v0, u1, v1);
    /* on the rock: the rock's drawing on its relief, the tree pass's
     * ground left out (VoxelMesh_TreeOnRock) */
    if (VoxelMesh_TreeOnRock(inst, x, y) && MetatileUV(b, wall, &u0, &v0, &u1, &v1))
        EmitReliefUV(b, x, y, relief, VoxelRelief_Depth(inst, x, y), u0, v0, u1, v1, false, 0, NULL);
    if (wall >= 0)
        EmitCliffWalls(b, inst, x, y, relief, VoxelRelief_Depth(inst, x, y), wall, -1, y, sides, false);
}

/*
 * DLX-26 (Wally's Journey PC, owner-approved 2026-11-13, the same skirt rule as BeltWaterSkirts): a model's cell
 * (a boulder at sea) is laid flat at its centre's height (above: "a sea ramp under a rock ... still laid flat"),
 * while the cells round it follow the relief point by point; where the sea slopes (Route 108) their edges missed by
 * a pixel or so and the clear colour showed as a light bracket round the rock. Each edge of the flat cell is joined
 * to the neighbour's real edge (SeamEdge) wherever they differ, with the cell's own drawing.
 */
static void ModelFlatSkirts(VoxelBuilder *b, const VoxelMapInstance *inst, int x, int y)
{
    static const int kDir[4][2] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
    const unsigned n = VOXEL_RELIEF_SIDE;
    const float last = (float)(VOXEL_RELIEF_SIDE - 1);
    float h = VoxelRelief_CellLift(inst, x, y), z = VoxelRelief_CellShift(inst, x, y);
    float theirH[VOXEL_RELIEF_SIDE], theirZ[VOXEL_RELIEF_SIDE];
    float u0, v0, u1, v1;
    bool uv = false;

    if (getenv("WJPC_NO_MODEL_SKIRTS"))
        return;
    for (int d = 0; d < 4; ++d)
    {
        int nx = x + kDir[d][0], ny = y + kDir[d][1];
        bool differs = false;

        if (VoxelWorld_GetInstanceAt(nx, ny) != inst || VoxelRelief_Cell(inst, nx, ny) == NULL)
            continue;
        SeamEdge(inst, nx, ny, d ^ 1, theirH, theirZ, NULL);
        for (unsigned k = 0; k < n; ++k)
            differs = differs || fabsf(theirH[k] / 16.0f - h) > 0.001f || fabsf(theirZ[k] - z) > 0.001f;
        if (!differs)
            continue;
        if (!uv)
        {
            /* a lookup only: a model cell's own art missing from the atlas must not hold the map back */
            unsigned uncovered = b->uncovered;
            uv = VoxelMesh_TileUV(b, nx, ny, &u0, &v0, &u1, &v1);
            b->uncovered = uncovered;
            if (!uv)
                continue;
        }
        for (unsigned k = 0; k + 1 < n; ++k)
        {
            float ka = k / last, kb = (k + 1) / last;
            float xa, xb, za, zb, xA, xB, zA, zB;

            if (d < 2) /* north, south edge: along x */
            {
                float e = d == 0 ? 0.0f : 1.0f;
                xa = xA = x + ka; xb = xB = x + kb;
                za = y + e + z; zb = za;
                zA = y + e + theirZ[k]; zB = y + e + theirZ[k + 1];
            }
            else /* west, east edge: along z */
            {
                float e = d == 2 ? 0.0f : 1.0f;
                xa = xb = xA = xB = x + e;
                za = y + ka + z; zb = y + kb + z;
                zA = y + ka + theirZ[k]; zB = y + kb + theirZ[k + 1];
            }
            VoxelBuilder_Quad(b,
                &(VoxelVertex){xa, h, za, u0, v0, SHADE_TOP},
                &(VoxelVertex){xb, h, zb, u1, v0, SHADE_TOP},
                &(VoxelVertex){xB, theirH[k + 1] / 16.0f, zB, u1, v1, SHADE_TOP},
                &(VoxelVertex){xA, theirH[k] / 16.0f, zA, u0, v1, SHADE_TOP});
        }
    }
}

/* One row of the ground pass over [x0,x1), already clipped to the instance. */
/*
 * Polish option 2 (Brennan 2026-11-16, "go"): a safety floor. Where two
 * pieces of ground meet a hair apart (the foot of a tree's step beside a
 * ledge on Route 118: Emerald 3DS's own engine leaves the same gaps, its
 * console's 400x240 hides them), the camera looked through to the sky under
 * the map - a light blue line. Every cell gets its own drawing (a tree cell:
 * the ground it stands on, VoxelMesh_TileUV) laid once more, flat, a pixel
 * under the lowest point of it and its four neighbours, where nothing else
 * is ever drawn: a gap now shows the ground's colour. WJPC_NO_SAFETY_FLOOR=1:
 * off, to compare.
 */
static void EmitSafetyFloor(VoxelBuilder *b, const VoxelMapInstance *inst, int x, int y)
{
    static int sOff = -1;
    static const int kDir[5][2] = { {0, 0}, {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
    float u0, v0, u1, v1, low = 1e9f;

    if (sOff < 0)
        sOff = getenv("WJPC_NO_SAFETY_FLOOR") != NULL;
    if (sOff)
        return;
    /* Brennan 2026-11-21 18:14 (Route 111, F9 - the 3D trees hidden - "leaving behind the flat 2D ... sheets on the
     * ground - and those ground sheets kept strobing"): under a tree the cell's own drawing is the 2D tree, and this
     * plate laid it flat; and it lay at exactly the height of the ground run on under a tree's step (EmitTreeStep),
     * so the two took turns on his GPU (Route 111: 333 cells of the map laid twice at one height). Under a tree it
     * is the tree's plain ground now (VoxelTree_GroundMetatile; none where that is the tree's drawing itself - the
     * tree's own ground covers the cell), and it lies 5/512 tile lower again, a height no other ground piece takes.
     * WJPC_SAFETY_FLOOR_OLD=1: as before. */
    static int sOld = -1;
    if (sOld < 0)
        sOld = getenv("WJPC_SAFETY_FLOOR_OLD") != NULL;
    if (!sOld && VoxelWorld_UsesTreeSprites(inst))
    {
        int mt = VoxelWorld_GetMetatileId(x, y);

        if (VoxelTree_Part(inst->secondaryTileset, mt) >= 0)
        {
            int ground = VoxelTree_GroundMetatile(inst->secondaryTileset, mt);

            if (ground == mt || !MetatileUV(b, ground, &u0, &v0, &u1, &v1))
                return;
            goto haveUV;
        }
    }
    if (!VoxelMesh_TileUV(b, x, y, &u0, &v0, &u1, &v1))
        return;
haveUV:
    /* POL-51 (Route 115's flight: a slab of the ground drawn through its top steps - this plate, laid at the lowest
     * of the cell and its neighbours): none in a stair notch (beside one it closed a hole: Mt Pyre), where the steps and their side walls close
     * everything. Found by turning it off (WJPC_NO_SAFETY_FLOOR=1). */
    if (VoxelRelief_InNotch(inst, x, y))
        return;
    for (int d = 0; d < 5; ++d)
    {
        int nx = x + kDir[d][0], ny = y + kDir[d][1];
        const int16_t *g;

        if (VoxelWorld_GetInstanceAt(nx, ny) != inst)
            continue;
        g = VoxelRelief_Cell(inst, nx, ny);
        if (g == NULL)
        {
            low = fminf(low, 0.0f);
            continue;
        }
        for (int i = 0; i < VOXEL_RELIEF_SIDE * VOXEL_RELIEF_SIDE; ++i)
            low = fminf(low, g[i] / 16.0f);
    }
    /* (5/512 tile lower again: the ground's levels are whole 1/16ths and EmitGroundRunOn's floors 2 or 2 1/8 pixels
     * under them - 1/64 more landed on those (Route 111 still had 261 doubled cells); a 5/512 step never can,
     * positions being kept in 1/512ths) */
    b->lift = low - 1.0f / 16.0f - (sOld ? 0.0f : 5.0f / 512.0f);
    b->shift = b->lift;
    VoxelMesh_Top(b, (float)x, (float)y, 0.0f, 0.0f, u0, v0, u1, v1, SHADE_TOP);
    b->lift = 0.0f;
    b->shift = 0.0f;
}

/*
 * POL-51 (Brennan 2026-11-19, Mt Pyre "Option C": "fill in the entire volume underneath and on the left/right sides
 * down into the terrain so they are solid 3D staircase blocks"): a flight laid out from its face
 * (voxel_relief.c LayFlight) closed on its west and east sides, from its profile down to its foot, in the map's rock
 * (its cliff face tile; else the cell's own drawing). WJPC_STAIRS_OPEN_SIDES=1: open.
 */
static void EmitFlightSides(VoxelBuilder *b, const VoxelMapInstance *inst, int x, int y)
{
    float z[65], h[65], foot, topH, u0, v0, u1, v1;
    unsigned n;
    int side, face;
    bool in;

    if (getenv("WJPC_STAIRS_OPEN_SIDES") != NULL
     || !VoxelRelief_FlightSide(inst, x, y, &side, &n, z, h, &foot, &topH, &in))
        return;
    if (getenv("WJPC_STAIRS_DUMP") != NULL)
    {
        fprintf(stderr, "FLIGHTSIDE %d %d side %d n %u foot %.3f top %.3f in %d:", x, y, side, n, foot, topH, (int)in);
        for (unsigned q = 0; q < n; ++q)
            fprintf(stderr, " %.3f/%.3f", z[q], h[q]);
        fprintf(stderr, "\n");
    }
    /* recessed (Brennan 17:23): the cliff beside the flight closed from the steps up to the ground above, facing in */
    float faceZ = 0.0f;
    bool half = !in && VoxelRelief_FlightHalf(inst, x, y, &faceZ);
    const float footOut = foot;
    if (in)
    {
        foot = topH;   /* the band from the steps up to the top: its quads wound the other way, facing in */
    }
    face = VoxelRelief_FaceTile(inst);
    for (int s = 0; s < 2; ++s)
    {
        float wx;
        if (!(side & (1 << s)))
            continue;
        /* (Brennan 18:31: the cutout's sides "bright white/light-grey" - the stair cell's own drawing: the rock of
         * the cliff beside the flight instead, the cell next to it in its row; the map's face tile, else) */
        if (!VoxelMesh_TileUV(b, s == 0 ? x - 1 : x + 1, y, &u0, &v0, &u1, &v1)
         && (face < 0 || !MetatileUV(b, face, &u0, &v0, &u1, &v1)))
            continue;
        wx = s == 0 ? (float)x : (float)x + 1.0f;
        for (unsigned q = 0; q + 1 < n; ++q)
        {
            /* (half and half, Route 112: the part behind where the face stood is cut into the cliff - closed up to the
             * top, as a recessed flight's; the part in front stands out of it - closed down to the foot) */
            if (half)
                foot = 0.5f * (z[q] + z[q + 1]) < faceZ ? topH : footOut;
            float ha = h[q], hb = h[q + 1];
            float ta = fabsf(ha - foot), tb = fabsf(hb - foot);
            if (ta <= 0.01f && tb <= 0.01f)
                continue;
            if (fabsf(z[q + 1] - z[q]) < 1e-4f)
                continue;   /* a riser: the side's outline goes straight down there, no area */
            if (s == 0) /* faces west */
                VoxelBuilder_Quad(b,
                    &(VoxelVertex){wx, foot, z[q + 1], u0, v1, SHADE_WEST},
                    &(VoxelVertex){wx, foot, z[q],     u1, v1, SHADE_WEST},
                    &(VoxelVertex){wx, ha,   z[q],     u1, v1 - (v1 - v0) * (ta < 1.0f ? ta : 1.0f), SHADE_WEST},
                    &(VoxelVertex){wx, hb,   z[q + 1], u0, v1 - (v1 - v0) * (tb < 1.0f ? tb : 1.0f), SHADE_WEST});
            else        /* faces east */
                VoxelBuilder_Quad(b,
                    &(VoxelVertex){wx, foot, z[q],     u0, v1, SHADE_EAST},
                    &(VoxelVertex){wx, foot, z[q + 1], u1, v1, SHADE_EAST},
                    &(VoxelVertex){wx, hb,   z[q + 1], u1, v1 - (v1 - v0) * (tb < 1.0f ? tb : 1.0f), SHADE_EAST},
                    &(VoxelVertex){wx, ha,   z[q],     u0, v1 - (v1 - v0) * (ta < 1.0f ? ta : 1.0f), SHADE_EAST});
        }
    }
}

/*
 * Route 119's Acro Bike rails as 3D models (Brennan 2026-11-20 17:53: "Build real 3D rail models" - thin rails with
 * posts, level with the grass banks at both ends; the hop rails kept separate, tidied). The rail cells' relief lies
 * flat at the river (tools/voxel3ds/lower_rails.py, OVER_PX 0) and is drawn as the river (RailWaterArt); on it each
 * rail cell over the water stands a bar at the bank's height - where the drawing puts it, its top the drawing's
 * white bar (horizontal: rows 12-15, columns 1-14; vertical: columns 5-9), 3 px thick - and a post down to the
 * water. The bank cells with a rail's end drawn on them (0EC, 0F0, 0F2) are the bank, and the bar meets them flush.
 * WJPC_RAIL_OLD=1: no models (and the cells drawn as they are).
 */
static bool RailsOn(const VoxelMapInstance *inst)
{
    static int sOld = -1;
    if (sOld < 0)
        sOld = getenv("WJPC_RAIL_OLD") != NULL;
    /* Brennan 2026-11-20 19:31: "Apply this exact 3D rail + dark support post model ... to every other Acro Bike rail
     * section across the entire region". By metatile behaviour (MB_*_RAIL) Hoenn has two: Route 119 (General
     * 0EC-0FD over the river) and Safari Zone South (Lilycove 312-322, the same drawing, over the grass floor). */
    return !sOld && inst != NULL && (inst->layoutId == 35 || inst->layoutId == 241);
}

/* Safari Zone South's copy of the General rail drawing, as the General's metatile (and its floor: grass) */
static int RailGeneral(const VoxelMapInstance *inst, int mt)
{
    if (inst == NULL || inst->layoutId != 241)
        return mt;
    switch (mt)
    {
    case 0x312: return 0x0F0;
    case 0x313: return 0x0F1;
    case 0x314: return 0x0F2;
    case 0x316: case 0x31E: return 0x0FB;
    case 0x31D: return 0x0FD;
    case 0x315: return 0x0F5;
    case 0x321: return 0x0F9;
    default: return -1;
    }
}

static int RailKind(int mt)   /* 1 horizontal bar, 2 vertical bar, 3 under a rail (posts, side), 0 none */
{
    switch (mt)
    {
    case 0x0ED: case 0x0F1: case 0x0FD: case 0x0FB: return 1;
    case 0x0F3: case 0x0F4: case 0x0F7: return 2;
    case 0x0F5: case 0x0F9: case 0x0FC: return 3;
    default: return 0;
    }
}

bool VoxelMesh_RailWaterArt(const VoxelMapInstance *inst, int x, int y)
{
    return RailsOn(inst) && RailKind(RailGeneral(inst, VoxelWorld_GetMetatileId(x, y))) != 0;
}

/* what is drawn under a rail: Route 119's river, Safari Zone South's grass */
int VoxelMesh_RailFloorArt(const VoxelMapInstance *inst)
{
    return inst != NULL && inst->layoutId == 241 ? 0x001 : 0x206;
}

static bool RailJoins(const VoxelMapInstance *inst, int x, int y, int kind)
{
    int mt = RailGeneral(inst, VoxelWorld_GetMetatileId(x, y));
    if (VoxelWorld_GetInstanceAt(x, y) != inst)
        return false;
    if (RailKind(mt) == kind)
        return true;
    /* the bank's cells with the rail's end drawn on them */
    return kind == 1 ? (mt == 0x0F0 || mt == 0x0F2) : (mt == 0x0EC);
}

static void RailBox(VoxelBuilder *b, float xa, float xb, float za, float zb, float ytop, float ybot,
                    float u0, float v0, float u1, float v1, bool west, bool east, bool south, bool north, float k)
{
    VoxelBuilder_Quad(b,
        &(VoxelVertex){xa, ytop, za, u0, v0, SHADE_TOP * k},
        &(VoxelVertex){xb, ytop, za, u1, v0, SHADE_TOP * k},
        &(VoxelVertex){xb, ytop, zb, u1, v1, SHADE_TOP * k},
        &(VoxelVertex){xa, ytop, zb, u0, v1, SHADE_TOP * k});
    if (south)
        VoxelBuilder_Quad(b,
            &(VoxelVertex){xa, ybot, zb, u0, v1, SHADE_SOUTH * k},
            &(VoxelVertex){xb, ybot, zb, u1, v1, SHADE_SOUTH * k},
            &(VoxelVertex){xb, ytop, zb, u1, v0, SHADE_SOUTH * k},
            &(VoxelVertex){xa, ytop, zb, u0, v0, SHADE_SOUTH * k});
    if (north)
        VoxelBuilder_Quad(b,
            &(VoxelVertex){xb, ybot, za, u1, v1, SHADE_NORTH * k},
            &(VoxelVertex){xa, ybot, za, u0, v1, SHADE_NORTH * k},
            &(VoxelVertex){xa, ytop, za, u0, v0, SHADE_NORTH * k},
            &(VoxelVertex){xb, ytop, za, u1, v0, SHADE_NORTH * k});
    if (west)
        VoxelBuilder_Quad(b,
            &(VoxelVertex){xa, ybot, za, u0, v1, SHADE_WEST * k},
            &(VoxelVertex){xa, ybot, zb, u1, v1, SHADE_WEST * k},
            &(VoxelVertex){xa, ytop, zb, u1, v0, SHADE_WEST * k},
            &(VoxelVertex){xa, ytop, za, u0, v0, SHADE_WEST * k});
    if (east)
        VoxelBuilder_Quad(b,
            &(VoxelVertex){xb, ybot, zb, u1, v1, SHADE_EAST * k},
            &(VoxelVertex){xb, ybot, za, u0, v1, SHADE_EAST * k},
            &(VoxelVertex){xb, ytop, za, u0, v0, SHADE_EAST * k},
            &(VoxelVertex){xb, ytop, zb, u1, v0, SHADE_EAST * k});
}

static void EmitRailCell(VoxelBuilder *b, const VoxelMapInstance *inst, int x, int y)
{
    const float p = 1.0f / 16.0f;
    const int16_t *grid;
    int mt, kind;
    float u0, v0, u1, v1, water, top, bot, shift;
    float pu0, pv0, pu1, pv1;

    if (!RailsOn(inst))
        return;
    mt = VoxelWorld_GetMetatileId(x, y);
    kind = RailKind(RailGeneral(inst, mt));
    if (kind != 1 && kind != 2)
        return;
    grid = VoxelRelief_Cell(inst, x, y);
    if (grid == NULL || !MetatileUV(b, mt, &u0, &v0, &u1, &v1))
        return;
    /* the posts (Brennan 18:57: "Restore the dark grey/wooden-brown color and darker underwater shading on all the
     * vertical support poles"): the drawing's own dark post grey, 0F5's left column rows 0-2 */
    if (!MetatileUV(b, inst->layoutId == 241 ? 0x315 : 0x0F5, &pu0, &pv0, &pu1, &pv1))
        pu0 = u0, pv0 = v0, pu1 = u1, pv1 = v1;
    else
        pu1 = pu0 + (pu1 - pu0) * 1.0f / 16.0f, pv1 = pv0 + (pv1 - pv0) * 3.0f / 16.0f;
    water = grid[0] / 16.0f;
    top = water + 1.0f;          /* the bank's height: a level over the river */
    bot = top - 3.0f * p;
    shift = top;                 /* as far south as it is high (the relief's rule: depth = height) */
#define RU(c) (u0 + (u1 - u0) * (c) / 16.0f)
#define RV(r) (v0 + (v1 - v0) * (r) / 16.0f)
    if (kind == 1)
    {
        bool w = RailJoins(inst, x - 1, y, 1), e = RailJoins(inst, x + 1, y, 1);
        float xa = (float)x + (w ? 0.0f : p), xb = (float)x + 1.0f - (e ? 0.0f : p);
        float za = (float)y + 12.0f * p + shift, zb = (float)y + 1.0f + shift;

        RailBox(b, xa, xb, za, zb, top, bot, RU(w ? 0 : 1), RV(12), RU(e ? 16 : 15), RV(16), !w, !e, true, true, 1.0f);
        /* a post at the cell's middle, down to the river */
        RailBox(b, (float)x + 7.0f * p, (float)x + 9.0f * p, zb - 2.0f * p, zb, bot, water,
                pu0, pv0, pu1, pv1, true, true, true, true, 0.7f);
    }
    else
    {
        bool n = RailJoins(inst, x, y - 1, 2), s2 = RailJoins(inst, x, y + 1, 2);
        float xa = (float)x + 5.0f * p, xb = (float)x + 10.0f * p;
        float za = (float)y + (n ? 0.0f : p) + shift, zb = (float)y + (s2 ? 1.0f : 15.0f * p) + shift;

        RailBox(b, xa, xb, za, zb, top, bot, RU(5), RV(n ? 0 : 1), RU(10), RV(s2 ? 16 : 15), true, true, !s2, !n, 1.0f);
        RailBox(b, (float)x + 6.5f * p, (float)x + 8.5f * p, (za + zb) * 0.5f - p, (za + zb) * 0.5f + p, bot, water,
                pu0, pv0, pu1, pv1, true, true, true, true, 0.7f);
    }
#undef RU
#undef RV
}


/*
 * The cable car in 3D (Brennan 2026-11-21 17:25, image 5: "Both the cable car tower/pylon and the diagonal cable line
 * running up Mt. Chimney are still flat 2D ... rebuild them as proper 3D models"; his choices, 2026-11-22: pylons "Match
 * the 2D T posts", cable "Slight sag", no cabin). The 3DS reference has none (its VOXEL_STATUS: "the cable car's cables
 * are flat"). The line and posts are drawn on Route 112's and Mt. Chimney's rock (Lavaridge 0x250-0x265): in 3D those
 * cells are their plain rock (CableArtSwap, each drawing's exact match without the line), and the pylons stand as T
 * posts where the posts' feet are drawn - their height the drawing's (crossbar to foot: 2.28 tiles, 36.5 px), their
 * light blue and slate the drawing's - with the cable strung through their crossbars to the stations and on past the
 * maps' edges, a little slack between them. WJPC_CABLE_CAR_FLAT=1: flat, as drawn.
 */
typedef struct { uint16_t layout; int8_t px[3], py[3]; uint8_t n; float westX, eastX; int8_t westOff, eastOff; } CableLine;
/* pylon feet (cells, the bottom of the drawn post), and the line's ends: x, and whether it goes on off the map there
 * (its slope the drawing's, 0.5 rows a column) or ends level at a station's wall */
static const CableLine kCableLines[] = {
    { 28, { 4, 12, 24 }, { 17, 21, 26 }, 3, -8.0f, 27.1f, 1, 0 },   /* Route 112: up the mountain west, the station east */
    { 136, { 28 }, { 35 }, 1, 20.0f, 47.0f, 0, 1 },                  /* Mt. Chimney: the station west, down the mountain east */
};
enum { CABLE_SWAPS = 14 };
static const uint16_t kCableArt[CABLE_SWAPS][2] = {
    { 0x250, 0x272 }, { 0x251, 0x272 }, { 0x252, 0x271 }, { 0x253, 0x271 }, { 0x254, 0x271 }, { 0x255, 0x271 },
    { 0x259, 0x272 }, { 0x25A, 0x271 }, { 0x25B, 0x271 }, { 0x25C, 0x271 }, { 0x25D, 0x271 }, { 0x261, 0x27B },
    { 0x264, 0x271 }, { 0x265, 0x271 },
};
static bool CableOn(const VoxelMapInstance *inst, const CableLine **line)
{
    static int sFlat = -1;
    if (sFlat < 0)
        sFlat = getenv("WJPC_CABLE_CAR_FLAT") != NULL;
    if (sFlat || inst == NULL)
        return false;
    for (unsigned i = 0; i < sizeof(kCableLines) / sizeof(kCableLines[0]); ++i)
        if (inst->layoutId == kCableLines[i].layout)
        {
            if (line != NULL)
                *line = &kCableLines[i];
            return true;
        }
    return false;
}

/* the plain rock under a drawn line or post (Lavaridge tile set), or the metatile as it is - for the atlas: every
 * picture of those tiles in 3D (tops, cut edges, cliff faces) is the plain rock's. The 14 ids are drawn only on Route
 * 112 and Mt. Chimney (checked over every layout on the Lavaridge set). */
int VoxelMesh_CableArtPlain(int metatile)
{
    static int sFlat = -1;
    if (sFlat < 0)
        sFlat = getenv("WJPC_CABLE_CAR_FLAT") != NULL;
    if (sFlat)
        return metatile;
    for (unsigned i = 0; i < CABLE_SWAPS; ++i)
        if (kCableArt[i][0] == metatile)
            return kCableArt[i][1];
    return metatile;
}

/* the plain rock under a drawn line or post, or the metatile as it is */
int VoxelMesh_CableArtSwap(const VoxelMapInstance *inst, int metatile)
{
    if (!CableOn(inst, NULL))
        return metatile;
    for (unsigned i = 0; i < CABLE_SWAPS; ++i)
        if (kCableArt[i][0] == metatile)
            return kCableArt[i][1];
    return metatile;
}

enum { CABLE_POST_H_16THS = 36 };   /* foot to crossbar top, px */
static void CableBox(VoxelBuilder *b, float xa, float xb, float za, float zb, float yb, float yt, VoxelSolidColor c,
                     float k)
{
    float u0, v0, u1, v1;
    VoxelAtlas_SolidUV(c, &u0, &v0, &u1, &v1);
    RailBox(b, xa, xb, za, zb, yt, yb, u0, v0, u1, v1, true, true, true, true, k);
}

/* where a pylon's foot stands: its height and its world z (as far south as it is high) */
static void CableFoot(const VoxelMapInstance *inst, int x, int y, float *h, float *z)
{
    *h = VoxelRelief_CellLift(inst, x, y);
    *z = (float)y + 0.9f + VoxelRelief_CellShift(inst, x, y);
}

static void EmitCablePylon(VoxelBuilder *b, const VoxelMapInstance *inst, int x, int y)
{
    const float p = 1.0f / 16.0f, top = CABLE_POST_H_16THS * p;
    float h, z, cx = (float)x + 0.5f;

    CableFoot(inst, x, y, &h, &z);
    /* the drawing: a foot 8 px wide a tile tall, the post 6 px wide up to the crossbar, the crossbar 14 px wide and
     * 5 px thick; the outline's slate a pixel round each */
    CableBox(b, cx - 4.0f * p, cx + 4.0f * p, z - 4.0f * p, z + 4.0f * p, h - 2.0f * p, h + 1.0f * p,
             VOXEL_SOLID_PYLON_DARK, 1.0f);
    CableBox(b, cx - 4.0f * p, cx + 4.0f * p, z - 4.0f * p, z + 4.0f * p, h + 1.0f * p, h + 16.0f * p,
             VOXEL_SOLID_PYLON, 1.0f);
    CableBox(b, cx - 3.5f * p, cx + 3.5f * p, z - 3.5f * p, z + 3.5f * p, h + 16.0f * p, h + 17.0f * p,
             VOXEL_SOLID_PYLON_DARK, 1.0f);
    CableBox(b, cx - 3.0f * p, cx + 3.0f * p, z - 3.0f * p, z + 3.0f * p, h + 17.0f * p, h + top - 5.0f * p,
             VOXEL_SOLID_PYLON, 1.0f);
    CableBox(b, cx - 7.0f * p, cx + 7.0f * p, z - 3.0f * p, z + 3.0f * p, h + top - 5.0f * p, h + top,
             VOXEL_SOLID_PYLON, 1.0f);
    CableBox(b, cx - 7.5f * p, cx + 7.5f * p, z - 3.5f * p, z + 3.5f * p, h + top - 6.0f * p, h + top - 5.0f * p,
             VOXEL_SOLID_PYLON_DARK, 1.0f);
}

/* a thin cable from a to b (world points), sagging by `sag` at its middle: two crossed ribbons, 1.5 px */
static void EmitCableSpan(VoxelBuilder *b, const float a[3], const float e[3], float sag)
{
    enum { PIECES = 12 };
    const float w = 0.75f / 16.0f;
    float u0, v0, u1, v1;
    float dx = e[0] - a[0], dz = e[2] - a[2], l = sqrtf(dx * dx + dz * dz);
    float nx = l > 0.0f ? -dz / l * w : w, nz = l > 0.0f ? dx / l * w : 0.0f;

    VoxelAtlas_SolidUV(VOXEL_SOLID_CABLE, &u0, &v0, &u1, &v1);
    for (int i = 0; i < PIECES; ++i)
    {
        float t0 = i / (float)PIECES, t1 = (i + 1) / (float)PIECES, q[2][3];
        for (int k = 0; k < 2; ++k)
        {
            float t = k ? t1 : t0;
            q[k][0] = a[0] + dx * t;
            q[k][1] = a[1] + (e[1] - a[1]) * t - sag * 4.0f * t * (1.0f - t);
            q[k][2] = a[2] + dz * t;
        }
        VoxelBuilder_Quad(b, &(VoxelVertex){q[0][0], q[0][1] - w, q[0][2], u0, v0, SHADE_SOUTH},
                          &(VoxelVertex){q[1][0], q[1][1] - w, q[1][2], u1, v0, SHADE_SOUTH},
                          &(VoxelVertex){q[1][0], q[1][1] + w, q[1][2], u1, v1, SHADE_SOUTH},
                          &(VoxelVertex){q[0][0], q[0][1] + w, q[0][2], u0, v1, SHADE_SOUTH});
        VoxelBuilder_Quad(b, &(VoxelVertex){q[0][0] - nx, q[0][1] + w, q[0][2] - nz, u0, v0, SHADE_TOP},
                          &(VoxelVertex){q[1][0] - nx, q[1][1] + w, q[1][2] - nz, u1, v0, SHADE_TOP},
                          &(VoxelVertex){q[1][0] + nx, q[1][1] + w, q[1][2] + nz, u1, v1, SHADE_TOP},
                          &(VoxelVertex){q[0][0] + nx, q[0][1] + w, q[0][2] + nz, u0, v1, SHADE_TOP});
    }
}

static void EmitCableCell(VoxelBuilder *b, const VoxelMapInstance *inst, int x, int y)
{
    const CableLine *line;
    int lx, ly;

    if (!CableOn(inst, &line))
        return;
    lx = x - inst->originX;
    ly = y - inst->originY;
    for (unsigned i = 0; i < line->n; ++i)
        if (lx == line->px[i] && ly == line->py[i])
            EmitCablePylon(b, inst, x, y);
    /* the whole line, once: with the first pylon */
    if (lx != line->px[0] || ly != line->py[0])
        return;
    {
        const float p = 1.0f / 16.0f;
        float pt[8][3];
        unsigned n = 0;

        for (unsigned i = 0; i < line->n; ++i)
        {
            float h, z;
            CableFoot(inst, inst->originX + line->px[i], inst->originY + line->py[i], &h, &z);
            pt[n][0] = (float)(inst->originX + line->px[i]) + 0.5f;
            pt[n][1] = h + (CABLE_POST_H_16THS - 2.5f) * p;
            pt[n][2] = z;
            ++n;
        }
        /* the ends: level to a station's wall, or on off the map down (east) / up (west) the mountain as the drawing
         * runs, half a row a column, a quarter level a column */
        {
            float w[3], e[3], first[3], last[3];
            memcpy(first, pt[0], sizeof first);
            memcpy(last, pt[n - 1], sizeof last);
            w[0] = (float)inst->originX + line->westX;
            e[0] = (float)inst->originX + line->eastX;
            if (line->westOff)
            {
                float dxw = first[0] - w[0];
                w[1] = first[1] + 0.25f * dxw;
                w[2] = first[2] - 0.5f * dxw + 0.25f * dxw;
            }
            else
                w[1] = first[1], w[2] = first[2];
            if (line->eastOff)
            {
                float dxe = e[0] - last[0];
                e[1] = last[1] - 0.25f * dxe;
                e[2] = last[2] + 0.5f * dxe - 0.25f * dxe;
            }
            else
                e[1] = last[1], e[2] = last[2];
            EmitCableSpan(b, w, pt[0], 0.0f);
            for (unsigned i = 0; i + 1 < n; ++i)
            {
                float dx = pt[i + 1][0] - pt[i][0], dz = pt[i + 1][2] - pt[i][2];
                EmitCableSpan(b, pt[i], pt[i + 1], 0.03f * sqrtf(dx * dx + dz * dz));
            }
            {
                float dx = e[0] - last[0], dz = e[2] - last[2];
                EmitCableSpan(b, last, e, line->eastOff ? 0.03f * sqrtf(dx * dx + dz * dz) : 0.0f);
            }
            (void)p;
        }
    }
}

/*
 * Brennan 2026-11-22 13:38 (C9, Meteor Falls: "the circular floor craters ... are still flat 2D textures - convert
 * them into proper 3D ... crater models"): a crater (Meteor Falls 0x282 0x283 / 0x28A 0x28B, a 2x2 block) has its
 * rim raised round the bowl - a ring of its own drawing, up to CRATER_CAP px, each point 0.4 of its height south as
 * the rock mounds (his pick D). WJPC_FLAT_CRATERS=1: flat.
 */
extern const struct Tileset gTileset_MeteorFalls;
static void EmitCraterRim(VoxelBuilder *b, const VoxelMapInstance *inst, int x, int y)
{
    static int sOff = -1;
    static const float kR = 12.0f, kW = 3.5f, kCap = 4.0f;
    const float p = 1.0f / 16.0f;
    float u0, v0, u1, v1, h[17][17];
    int mt, k;

    if (sOff < 0)
        sOff = getenv("WJPC_FLAT_CRATERS") != NULL;
    if (sOff || inst->secondaryTileset != &gTileset_MeteorFalls)
        return;
    mt = VoxelWorld_GetMetatileId(x, y);
    k = mt == 0x282 ? 0 : mt == 0x283 ? 1 : mt == 0x28A ? 2 : mt == 0x28B ? 3 : -1;
    if (k < 0 || !MetatileUV(b, mt, &u0, &v0, &u1, &v1))
        return;
    for (int j = 0; j <= 16; j++)
        for (int i = 0; i <= 16; i++)
        {
            float dx = (float)((k & 1) * 16 + i) - 16.0f, dy = (float)((k >> 1) * 16 + j) - 15.5f;
            float t = 1.0f - fabsf(sqrtf(dx * dx + dy * dy) - kR) / kW;
            h[j][i] = t > 0.0f ? kCap * sinf(t * 1.5707963f) * p : 0.0f;
        }
    b->lift = VoxelRelief_CellLift(inst, x, y);
    b->shift = VoxelRelief_CellShift(inst, x, y);
    for (int r = 0; r < 16; r++)
        for (int c = 0; c < 16; c++)
        {
            if (h[r][c] <= 0.0f && h[r][c + 1] <= 0.0f && h[r + 1][c] <= 0.0f && h[r + 1][c + 1] <= 0.0f)
                continue;
            float x0 = (float)x + c * p, x1 = x0 + p, z0 = (float)y + r * p, z1 = z0 + p;
            float cu0 = u0 + (u1 - u0) * (c + 0.02f) / 16.0f, cu1 = u0 + (u1 - u0) * (c + 0.98f) / 16.0f;
            float cv0 = v0 + (v1 - v0) * (r + 0.02f) / 16.0f, cv1 = v0 + (v1 - v0) * (r + 0.98f) / 16.0f;
            float nz = ((h[r][c] + h[r][c + 1]) - (h[r + 1][c] + h[r + 1][c + 1])) * 0.5f / p;
            float nx = ((h[r][c] + h[r + 1][c]) - (h[r][c + 1] + h[r + 1][c + 1])) * 0.5f / p;
            float s = 0.96f - 0.22f * nz + 0.06f * nx;
            s = s < 0.6f ? 0.6f : s > 1.0f ? 1.0f : s;
            VoxelBuilder_Quad(b,
                &(VoxelVertex){x0, h[r][c], z0 + 0.4f * h[r][c], cu0, cv0, s},
                &(VoxelVertex){x1, h[r][c + 1], z0 + 0.4f * h[r][c + 1], cu1, cv0, s},
                &(VoxelVertex){x1, h[r + 1][c + 1], z1 + 0.4f * h[r + 1][c + 1], cu1, cv1, s},
                &(VoxelVertex){x0, h[r + 1][c], z1 + 0.4f * h[r + 1][c], cu0, cv1, s});
        }
    b->lift = 0.0f;
    b->shift = 0.0f;
}

void VoxelMesh_EmitGroundRow(VoxelBuilder *builder, const VoxelMapInstance *inst,
                             int x0, int x1, int y)
{
    for (int x = x0; x < x1; ++x)
        EmitRailCell(builder, inst, x, y);
    for (int x = x0; x < x1; ++x)
        EmitCableCell(builder, inst, x, y);
    for (int x = x0; x < x1; ++x)
        EmitCraterRim(builder, inst, x, y);
    for (int x = x0; x < x1; ++x)
        EmitSafetyFloor(builder, inst, x, y);
    for (int x = x0; x < x1; ++x)
        if (x == inst->originX || x == inst->originX + inst->width - 1
         || y == inst->originY || y == inst->originY + inst->height - 1)
            EmitSeamSkirt(builder, inst, x, y);
    for (int x = x0; x < x1; ++x)
        EmitFlightSides(builder, inst, x, y);
    for (int x = x0; x < x1; ++x)
    {
        VoxelVisualShape shape = VoxelMesh_Classify(x, y);

        /* The tree pass supplies both the trunk's ground and its crown; a
         * step it stands on is drawn here. A trunk in water leaves the
         * water to this pass (VoxelTree_LaysGround). */
        if (VoxelWorld_UsesTreeSprites(inst)
         && VoxelTree_Part(inst->secondaryTileset, VoxelWorld_GetMetatileId(x, y)) >= 0
         && VoxelTree_LaysGround(inst->secondaryTileset, VoxelWorld_GetMetatileId(x, y)))
        {
            EmitTreeStep(builder, inst, x, y);
            continue;
        }
        if (shape == VOXEL_SHAPE_VOID)
            continue;
        /* A lifted cell lays its own drawing on its relief. A slope - rock,
         * stairs - is nothing but that; a level lifted cell still carries
         * whatever stands on it, drawn below with the same lift. */
        {
            const int16_t *relief = VoxelRelief_Cell(inst, x, y);

            /* A building's cells are its model's: a mountain's foot that
             * runs on under a roof is not drawn over it. Not under a model
             * lower than a level standing on a slope (Lavaridge's benches
             * and the stones of its spring along the face under the
             * boardwalk): it hides no face, and its ground laid flat at the
             * cell's centre was a plate half a level up the face, the void
             * either side of it. Only a face or a corner one level tall:
             * a sea ramp under a rock, Rustboro's slopes under its fences
             * are still laid flat. */
            float modelTop;
            /* a model's cell it stands over nothing of, its drawing on the
             * face behind: the relief, drawn less the model */
            int bare = -1;

            if (relief != NULL && VoxelBuildings_CellAt(inst, x, y, NULL, &modelTop)
             && !VoxelBuildings_BareVariant(inst, x, y, &bare))
            {
                int lo = relief[0], hi = relief[0];

                for (unsigned i = 1; i < VOXEL_RELIEF_SIDE * VOXEL_RELIEF_SIDE; ++i)
                {
                    lo = relief[i] < lo ? relief[i] : lo;
                    hi = relief[i] > hi ? relief[i] : hi;
                }
                if (!(modelTop > 0.0f && modelTop < 1.0f && hi - lo == 16))
                {
                    relief = NULL;
                    ModelFlatSkirts(builder, inst, x, y);
                }
            }
            if (relief != NULL)
            {
                /* A signpost's drawing is the sign: its relief is laid with
                 * the ground round it, and the sign stands without its own. */
                bool sign = VoxelSign_IsCell(inst, x, y);
                float foot = 0.0f, u0, v0, u1, v1;
                int ground = -1, art = -1, wall = -1;
                unsigned sides = 0xFF;
                const uint8_t *cutRows = NULL;
                int cut = sign ? -1 : VoxelRelief_Cut(inst, x, y, &foot, &ground, &wall, &sides);
                /* the ground behind its back, for its north face (not a
                 * band going on under it, a deck, a canopy) */
                const int back = cut != -1 && !(sides & (VOXEL_RELIEF_GOES_ON | VOXEL_RELIEF_GOES_ON_SOUTH
                                                         | VOXEL_RELIEF_THIN_DECK | VOXEL_RELIEF_CANOPY))
                               ? ground : -1;
                const int backWall = wall;
                /* a back's wall stands only behind a whole top: a cut tile's
                 * cleared pixels showed it through as teeth (EmitNorthFace) */
                const bool backStands = cut == VOXEL_RELIEF_NO_VARIANT;

                /* that ground: the one matched to the drawing round the sign
                 * (the cell south of it without one) */
                if (sign && VoxelSign_Ground(inst, x, y, &art)
                 && !MetatileUV(builder, art, &u0, &v0, &u1, &v1))
                {
                    /* Wally's Journey PC: the recorded ground isn't in this tileset;
                     * lay the ground of the cell south of the sign instead. */
                    art = VoxelWorld_GetMetatileId(x, y + 1);
                    if (!MetatileUV(builder, art, &u0, &v0, &u1, &v1))
                        art = -1;
                }
                /* and the cell north of a sign whose head is drawn in it
                 * (a gym sign's round top): the ground under the head, as
                 * on a map with no relief - its drawing laid on the relief
                 * painted the head flat on the ground behind the sign */
                if (!sign && cut == -1 && VoxelSign_HeadGround(inst, x, y, &art)
                 && !MetatileUV(builder, art, &u0, &v0, &u1, &v1))
                    art = -1;
                if (bare >= 0 && MetatileUV(builder, bare, &u0, &v0, &u1, &v1))
                    art = bare;

                /* a canopy cleared: its variant on its relief, over a plate
                 * of the ground under the crown; the rest as a cell with no
                 * variant (VOXEL_RELIEF_CANOPY) */
                int canopy = -1, plate = -1;

                if (cut >= 0 && cut < (int)VOXEL_CUTS && (sides & VOXEL_RELIEF_CANOPY))
                {
                    if (MetatileUV(builder, (int)(VOXEL_CUT_FIRST + (unsigned)cut), &u0, &v0, &u1, &v1))
                        canopy = (int)(VOXEL_CUT_FIRST + (unsigned)cut);
                    if (canopy >= 0 && wall >= 0 && MetatileUV(builder, wall, &u0, &v0, &u1, &v1))
                        plate = wall;
                    wall = -1;
                    cut = VOXEL_RELIEF_NO_VARIANT;
                }

                /*
                 * A cut tile (voxel_relief.h): the cell's own drawing lies
                 * flat at its foot and its relief shows the rock alone, cut
                 * where its pixels end, leaving the quads on the foot to
                 * the flat drawing. Without a slot for the variant, the
                 * tile as it is.
                 */
                /*
                 * The ground behind a terrace's rim, run on under the cell
                 * two pixels under its own level (voxel_relief.h): only ever
                 * seen from above, past the rim's edge. The cell's relief
                 * is drawn as it is.
                 */
                if (cut == VOXEL_RELIEF_NO_VARIANT)
                {
                    /* (or the one south of it, a row back: a bridge's deck over
                     * a band whose row north is another plank) */
                    const int from = (sides & VOXEL_RELIEF_GOES_ON_SOUTH) ? y + 1 : y - 1;
                    const int16_t *north = VoxelWorld_GetInstanceAt(x, from) == inst
                                         ? VoxelRelief_Cell(inst, x, from) : NULL;

                    if ((sides & (VOXEL_RELIEF_GOES_ON | VOXEL_RELIEF_GOES_ON_SOUTH)) && north != NULL)
                    {
                        /* the flank north of this top goes on under it, a row
                         * on and a pixel down, its own drawing (its cut
                         * variant if it has one) */
                        float nf, a0, b0, a1, b1;
                        int ng, nw, nart = -1;
                        unsigned ns;
                        int nc = VoxelRelief_Cut(inst, x, from, &nf, &ng, &nw, &ns);

                        const uint8_t *nrows = NULL;
                        unsigned nl, nm;

                        if ((sides & VOXEL_RELIEF_GOES_ON_FACE) && wall >= 0
                         && MetatileUV(builder, wall, &a0, &b0, &a1, &b1))
                        {
                            /* a corner cut along its rock: whole, as the
                             * band's side face, or the gap a steeper camera
                             * finds between it and this top showed the
                             * ground below */
                            nart = wall;
                        }
                        else if (nc >= 0 && nc < (int)VOXEL_CUTS
                         && MetatileUV(builder, (int)(VOXEL_CUT_FIRST + (unsigned)nc), &a0, &b0, &a1, &b1))
                        {
                            nart = (int)(VOXEL_CUT_FIRST + (unsigned)nc);
                            if (!VoxelRelief_CutVariant((unsigned)nc, &nl, &nm, &nrows))
                                nrows = NULL;
                        }
                        builder->lift = -1.0f / 16.0f;
                        builder->shift = builder->lift;
                        EmitRelief(builder, x, y, north, VoxelRelief_Depth(inst, x, from), from, nart,
                                   false, 0, nrows);
                        builder->lift = 0.0f;
                        builder->shift = 0.0f;
                    }
                    /* (its `wall` was that face, no cliff of its own) */
                    if (sides & VOXEL_RELIEF_GOES_ON_FACE)
                        wall = -1;
                    if (ground >= 0 && MetatileUV(builder, ground, &u0, &v0, &u1, &v1))
                        EmitGroundRunOn(builder, inst, x, y, foot, u0, v0, u1, v1);
                    cut = -1;
                }
                else if (cut >= 0 && cut < (int)VOXEL_CUTS
                      && MetatileUV(builder, (int)(VOXEL_CUT_FIRST + (unsigned)cut), &u0, &v0, &u1, &v1)
                      && VoxelMesh_TileUV(builder, x, y, &u0, &v0, &u1, &v1))
                    art = (int)(VOXEL_CUT_FIRST + (unsigned)cut);
                else
                    cut = -1;
                /* Brennan 2026-11-21 23:06 (Route 117's rock mound under the north-west woods: "it should just be
                 * trees no green tiles"): a cut slope (a drawing with clear pixels) on the map's north row with a wood
                 * of the belt beyond it is drawn whole in the map's rock face. Its tile is the rock with the wood's crowns
                 * painted over it (Mauville's 0x235/0x237): laid on the slope, and flat at its foot with the ground
                 * under its clear pixels (EmitCutGround and the run-on), the crowns were green wedges at the mound's
                 * back corners. The wood's 3D trees are the crowns. WJPC_EDGE_SLOPE_OWN_ART=1: as before. */
                int edgeRock = -1;
                if (art >= (int)VOXEL_CUT_FIRST && y == inst->originY && VoxelRelief_IsSlope(relief)
                 && getenv("WJPC_EDGE_SLOPE_OWN_ART") == NULL
                 && VoxelWorld_GetInstanceAt(x, y - 1) == NULL && BorderAt(x, y - 1) && BeltIsTree(x, y - 1))
                {
                    int face = VoxelRelief_FaceTile(inst);
                    float fu0, fv0, fu1, fv1;

                    if (face >= 0 && MetatileUV(builder, face, &fu0, &fv0, &fu1, &fv1))
                        edgeRock = face;
                }
                if (getenv("WJPC_EDGE_DUMP") != NULL && y == inst->originY && VoxelRelief_IsSlope(relief))
                    fprintf(stderr, "EDGESLOPE %d %d art %d cut %d rock %d\n", x, y, art, cut, edgeRock);
                if (cut >= 0 && edgeRock < 0)
                {
                    builder->lift = foot;
                    builder->shift = foot;
                    /*
                     * Its flat layer: with a ground behind it, that ground
                     * where the drawing has rock and the drawing where it is
                     * background (EmitCutGround); with none, the whole
                     * drawing. The rock the drawing carries, flat at the
                     * foot, showed as a dark wedge on the sand wherever the
                     * lifted rock did not cover it.
                     */
                    {
                        unsigned vLayout = 0, vMetatile = 0;
                        float gnd[4];

                        if (ground >= 0 && VoxelRelief_CutVariant((unsigned)cut, &vLayout, &vMetatile, &cutRows)
                         && MetatileUV(builder, ground, &gnd[0], &gnd[1], &gnd[2], &gnd[3]))
                            EmitCutGround(builder, x, y, u0, v0, u1, v1, cutRows, gnd,
                                          relief, (int)lroundf(foot * 16.0f));
                        else
                        {
                            cutRows = NULL;
                            EmitCutGround(builder, x, y, u0, v0, u1, v1, NULL, NULL, relief, 0);
                        }
                    }
                    /* and the ground behind it, run on under the rock beside
                     * it, a pixel under its own level: only ever seen through
                     * the clear background, never beside anything drawn */
                    if (ground >= 0 && MetatileUV(builder, ground, &u0, &v0, &u1, &v1))
                        for (unsigned k = 0; k < sizeof(kCutFill) / sizeof(kCutFill[0]); ++k)
                        {
                            int nx = x + kCutFill[k][0], ny = y + kCutFill[k][1];
                            const int16_t *n = VoxelWorld_GetInstanceAt(nx, ny) == inst
                                             ? VoxelRelief_Cell(inst, nx, ny) : NULL;
                            int top = INT16_MIN;

                            for (unsigned g = 0; n != NULL && g < VOXEL_RELIEF_SIDE * VOXEL_RELIEF_SIDE; ++g)
                                if (n[g] > top)
                                    top = n[g];
                            float nu0 = u0, nv0 = v0, nu1 = u1, nv1 = v1, nfoot;
                            int nground, nwall;
                            unsigned nsides;

                            if (n == NULL || top <= (int)lroundf(foot * 16.0f))
                                continue;
                            /* what runs on under the neighbour is the ground
                             * the neighbour itself stands on, if it has one of
                             * its own: this tile's sea under the next tile's
                             * sand was a bar of water across the beach */
                            if (VoxelRelief_Cut(inst, nx, ny, &nfoot, &nground, &nwall, &nsides) != -1 && nground >= 0
                             && !MetatileUV(builder, nground, &nu0, &nv0, &nu1, &nv1))
                            {
                                nu0 = u0; nv0 = v0; nu1 = u1; nv1 = v1;
                            }
                            /* a hair apart by side: two cut tiles may run
                             * their grounds on under the same cell */
                            builder->lift = foot - (1.0f + 0.125f * (float)k) / 16.0f;
                            builder->shift = builder->lift;
                            /* lit as the ground it is seen beside, this
                             * tile's own at its foot: in full sun it was a
                             * bright wedge in the shade the rock casts. A
                             * world point, the map's base counted: without
                             * it, on a raised map (Route 120) the sample lay
                             * inside the rock and the fill was a dark square
                             * on the water beside every cut tile */
                            builder->lightingConstant = 1.0f;
#if CTR_VOXEL_LIGHTING
                            if (builder->lighting)
                                builder->lightingConstant = VoxelLighting_Sample(
                                    x + 0.5f, foot + builder->base + 0.05f,
                                    VoxelBuilder_StretchZ(builder, y + 0.5f + foot));
#endif
                            EmitRunOn(builder, inst, nx, ny, nu0, nv0, nu1, nv1);
                            builder->lightingConstant = -1.0f;
                        }
                    builder->lift = 0.0f;
                    builder->shift = 0.0f;
                }
                /* a tree's cell on the rock: the rock's face, not the leaves */
                if (art < 0 && (sides & VOXEL_RELIEF_FACE_TOP) && wall >= 0
                 && MetatileUV(builder, wall, &u0, &v0, &u1, &v1))
                    art = wall;
                if (canopy >= 0)
                {
                    /* the ground the crown was drawn over, seen where its
                     * pixels are cleared */
                    if (plate >= 0)
                    {
                        /* (a pixel under it, not a pixel north: shifted
                         * too, its south edge left a hair of the void past
                         * the crown's cleared pixels there) */
                        builder->lift = -1.0f / 16.0f;
                        builder->shift = 0.0f;
                        EmitRelief(builder, x, y, relief, VoxelRelief_Depth(inst, x, y), y, plate,
                                   false, 0, NULL);
                        builder->lift = 0.0f;
                        builder->shift = 0.0f;
                    }
                    art = canopy;
                }
                /* Brennan 2026-11-21 23:06 (Route 117's rock mound under the north-west woods: "it should just be
                 * trees no green tiles"): a slope on the map's north row with a wood of the belt beyond it is drawn
                 * in the map's rock face - its tile (and its cut variant: the tile with its clear pixels cut out) is
                 * the rock with the wood's crowns painted over it (Mauville's 0x235/0x237), and laid on the slope the crowns were green wedges at the mound's back corners. The
                 * wood's 3D trees are the crowns. WJPC_EDGE_SLOPE_OWN_ART=1: its own drawing, as before. */
                if (edgeRock >= 0)
                {
                    /* (whole: the cut's clear pixels were the crowns, the ground showed through them) */
                    art = edgeRock;
                    cut = -1;
                    cutRows = NULL;
                }
                EmitRelief(builder, x, y, relief, VoxelRelief_Depth(inst, x, y),
                           sign ? y + 1 : y, art, cut >= 0, (int)lroundf(foot * 16.0f), cutRows);
                if (wall >= 0)
                    EmitCliffWalls(builder, inst, x, y, relief, VoxelRelief_Depth(inst, x, y), wall,
                                   art, sign ? y + 1 : y, sides, back >= 0 && backStands);
                if (back >= 0)
                {
                    /* its back in its own cliff wall, else the map's */
                    int face = backWall >= 0 && backWall < VOXEL_RELIEF_NO_FACE ? backWall : VoxelRelief_FaceTile(inst);

                    EmitNorthFace(builder, inst, x, y, relief, VoxelRelief_Depth(inst, x, y),
                                  backStands ? (face >= 0 ? face : back) : -1, back);
                }
                if (VoxelRelief_IsSlope(relief))
                {
                    /* A signpost at the foot of a face stands on the ground
                     * under its board, three quarters of the way down the
                     * cell (voxel_sign.c's EmitSign). */
                    const int16_t *depth = VoxelRelief_Depth(inst, x, y);
                    const int under = 3 * VOXEL_RELIEF_SIDE + 2;

                    builder->lift = relief[under] / 16.0f;
                    builder->shift = depth[under] / 16.0f;
                    VoxelSign_EmitStanding(builder, inst, x, y);
                    builder->lift = 0.0f;
                    builder->shift = 0.0f;
                    continue;
                }
                builder->lift = VoxelRelief_CellLift(inst, x, y);
                builder->shift = VoxelRelief_CellShift(inst, x, y);
                VoxelSign_EmitStanding(builder, inst, x, y);
                builder->lift = 0.0f;
                builder->shift = 0.0f;
                continue;
            }
        }
        /* A modelled building brings its own walls and roof; what is
         * left of its cells is the ground its drawing was painted over. */
        {
            int ground;
            float u0, v0, u1, v1;

            if (VoxelBuildings_CellAt(inst, x, y, &ground, NULL))
            {
                bool drawn = MetatileUV(builder, ground, &u0, &v0, &u1, &v1);

                /* A ground variant the atlas has no slot for (a full atlas)
                 * gives way to the cell's whole drawing: never a hole. */
                if (!drawn && ground >= (int)VOXEL_METATILE_REAL && ground < (int)VOXEL_METATILE_IDS
                 && builder->atlas->slotOf[ground] == VOXEL_SLOT_ABSENT)
                    drawn = MetatileUV(builder, VoxelWorld_GetMetatileId(x, y), &u0, &v0, &u1, &v1);
                if (drawn)
                {
                    /* at the level the cell's relief puts it, as the model
                     * standing on it is: Route 104's sea, a level under its
                     * land, under a rock in it */
                    builder->lift = VoxelRelief_CellLift(inst, x, y);
                    builder->shift = VoxelRelief_CellShift(inst, x, y);
                    VoxelMesh_Top(builder, (float)x, (float)y, 0.0f, 0.0f,
                                  u0, v0, u1, v1, SHADE_TOP);
                    builder->lift = 0.0f;
                    builder->shift = 0.0f;
                }
                /* a signpost in the model's cells stands in front of it
                 * (Lavaridge's gym sign on the gym's corner: the model is
                 * drawn without it); on a bridge it stands on its deck,
                 * lifted and moved south by the deck's height as its
                 * drawing is */
                if (VoxelSign_IsCell(inst, x, y))
                {
                    float deck = 0.0f;

                    if (!VoxelBuildings_DeckAt(inst, x + 0.5f, y + 0.5f, 0, &deck))
                        deck = 0.0f;
                    builder->lift = VoxelRelief_CellLift(inst, x, y) + deck;
                    builder->shift = VoxelRelief_CellShift(inst, x, y) + deck;
                    VoxelSign_EmitStanding(builder, inst, x, y);
                    builder->lift = 0.0f;
                    builder->shift = 0.0f;
                }
                continue;
            }
        }
        if (VoxelSign_EmitCell(builder, inst, x, y))
            continue;
        /* A lamp's lantern stands on its post; under it, the ground. */
        {
            int ground;
            float u0, v0, u1, v1;

            if (VoxelSign_HeadGround(inst, x, y, &ground))
            {
                if (MetatileUV(builder, ground, &u0, &v0, &u1, &v1))
                    VoxelMesh_Top(builder, (float)x, (float)y, 0.0f, 0.0f,
                                  u0, v0, u1, v1, SHADE_TOP);
                continue;
            }
        }
        EmitTile(builder, x, y, shape);
    }
}

void VoxelMesh_EmitInstance(VoxelBuilder *builder, const VoxelMapInstance *inst,
                            int x0, int y0, int x1, int y1)
{
    if (inst == NULL || builder->atlas == NULL)
        return;
    if (x0 < inst->originX) x0 = inst->originX;
    if (y0 < inst->originY) y0 = inst->originY;
    if (x1 > inst->originX + inst->width) x1 = inst->originX + inst->width;
    if (y1 > inst->originY + inst->height) y1 = inst->originY + inst->height;
    if (x1 <= x0 || y1 <= y0)
        return;

    /*
     * The ground, with the relief it is lifted on and the small things that
     * stand on it cell by cell (signs, furniture). The staging buffer is
     * finite, and this goes first so that it is never what gets refused:
     * losing the floor leaves a hole straight through the world to the clear
     * colour. Trees and modelled buildings follow in passes of their own.
     */
    for (int y = y0; y < y1; ++y)
        VoxelMesh_EmitGroundRow(builder, inst, x0, x1, y);
}

/*
 * The world past the last map.
 *
 * The window reaches beyond every instance on purpose, and until now those
 * tiles were VOID and drew nothing, so a town ended on a plate over the clear
 * colour and the horizon was a black wedge. The 2D game tiles the layout's 2x2
 * border block outwards for ever; this stands the same block up.
 *
 * Two courses, because that is what the block's own drawing measures: it is a
 * 2x2 pattern tiled, so a column through it reads one row, the other, the
 * first again - two distinct drawings, by the same rule every other column
 * here is heighted with. Laid flat instead, a border of trees reads as a leaf
 * carpet.
 *
 * Instance 0 only: the border belongs to the map the player is standing on,
 * and running it for every connection would stack four copies of it on the
 * same tiles.
 *
 * A map can say how tall its belt stands (VoxelRelief_Belt): a cave carved
 * out of the rock, its edges standing that high all round. Then the block is
 * laid as the drawing lays everything else, each of its cells at (u, h,
 * v + h): stood the map's courses and as many rows south of where the 2D game
 * tiles it, it meets the rock at the map's edges without a step or a gap -
 * stood where it is tiled, it left a trench along the north edge as wide as
 * it is tall, and doubled the rock along the south one.
 */
#define VOXEL_BORDER_COURSES 2

static bool BorderAt(int x, int y)
{
    const VoxelMapInstance *inst = VoxelWorld_GetInstanceAt(x, y);
    if (inst == NULL)
        return true;
    /* Outdoor classifiers never produce VOID inside an instance. Do not read
     * metatiles, attributes and neighbours just to discover there is no belt. */
    if (!inst->indoor)
        return false;
    return VoxelMesh_Classify(x, y) == VOXEL_SHAPE_VOID;
}

/*
 * A belt of water (Slateport's sea, Lilycove's, every route's out at sea)
 * is no wall: stood two courses up like a wood it was a step of sea standing
 * over the map's own, a raised rim round the whole of the harbour. Water lies
 * flush with the ground (ShapeHeight), and the belt's lies flush with the
 * map's: at the level of the map's edge cell nearest it when that is water
 * (a sea lowered under the land, Rustboro's, goes on at its own level), at
 * the map's ground otherwise. Over instance 0's base, as the belt is built.
 */
/* How far along a map's edge a belt of water looks for the map's sea. */
#define VOXEL_BORDER_SEA_SEARCH 12

static bool BorderWaterLevelUncached(int x, int y, float *level, float *shift);
static bool BorderWaterLevel(int x, int y, float *level, float *shift)
{
    /* (kept per cell, as BeltField) */
    enum { N = 32768 };
    typedef struct { int x, y; unsigned gen; bool ok; float l, s; } WaterMemo;
    VOXEL_TLS_TABLE(WaterMemo, sMemo, N);
    unsigned gen = VoxelWorld_CellGen(), k = ((unsigned)x * 73856093u) ^ ((unsigned)y * 19349663u);

    if (sMemo == NULL)
        return BorderWaterLevelUncached(x, y, level, shift);

    k = (k ^ (k >> 15)) & (N - 1);
    if (gen == 0 || sMemo[k].gen != gen || sMemo[k].x != x || sMemo[k].y != y)
    {
        sMemo[k].l = sMemo[k].s = 0.0f;
        sMemo[k].ok = BorderWaterLevelUncached(x, y, &sMemo[k].l, &sMemo[k].s);
        sMemo[k].x = x;
        sMemo[k].y = y;
        sMemo[k].gen = gen;
    }
    if (sMemo[k].ok)
    {
        *level = sMemo[k].l;
        if (shift != NULL)
            *shift = sMemo[k].s;
    }
    return sMemo[k].ok;
}
static bool BorderWaterLevelUncached(int x, int y, float *level, float *shift)
{
    /* POL-30: over the base of the map the cell stands with (BeltBase), at that map's sea's level - it was
     * always the current map's, so the belt beside the next map moved at the crossing. */
    const VoxelMapInstance *ring = VoxelWorld_BeltRing(x, y);
    int ex, ey;

    VoxelEdgeFill fill;

    if (ring == NULL || ring->indoor || VoxelRelief_Belt(ring) != 0
     || !VoxelWorld_BorderIsWater(x, y))
        return false;
    /* POL-51 (Brennan 2026-11-19: Mt Pyre's coast "a dark-grey/black vertical wall strip dropping into the sea"):
     * a sea fill given its own height (tools/voxel3ds/add_relief_edge.py --top) lies there, before the flat sea of
     * the map's own water - Mt Pyre outside has none, so the sea stood at its base, over the island's low south
     * edge, a wall up to it. WJPC_FILL_TOP_LATE=1: as before. */
    if (getenv("WJPC_FILL_TOP_LATE") == NULL && VoxelRelief_EdgeFillAt(ring, x, y, &fill) && fill.hasTop && fill.water)
    {
        *level = (float)fill.top / 16.0f;
        /* (drawn at its own rows, not as far south as it is low: 16 levels down it stood 16 rows south of the
         * built belt's north end, and the sky showed past it. Flat water: no edge to line up. WJPC_FILL_SEA_SHIFT=1:
         * shifted.) */
        *shift = getenv("WJPC_FILL_SEA_FLAT_ROWS") == NULL ? *level : 0.0f;   /* (with BELT_RADIUS 64 the far end is out of view again; at its rows it left the sky between it and a map's low south edge: the summit) */
        return true;
    }
    /* DLX-26 (owner's choice 2026-11-13, replaces the skirt walls): the whole belt of sea lies flat, at one
     * level - the level most of the map's own sea lies at - so no steps (and no lines) between belt cells. */
    if (getenv("WJPC_BELT_SEA_STEPS") == NULL)
    {
        /* (POL-30: one entry per map the belt stands with, not one in all) */
        static VOXEL_TLS int sKeys[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
        static VOXEL_TLS float sLevels[8], sShifts[8];
        static VOXEL_TLS unsigned sNext;
        int key = (ring->mapGroup << 8 | ring->mapNum) ^ (ring->lift << 16);
        unsigned slot;
        for (slot = 0; slot < 8 && sKeys[slot] != key; ++slot)
            ;
        if (slot == 8)
        {
            float sFlatLevel, sFlatShift;
            slot = sNext++ % 8;
            float lv[16], sh[16];
            int cnt[16], kinds = 0, best = -1;
            for (int cy = ring->originY; cy < ring->originY + ring->height; ++cy)
                for (int cx = ring->originX; cx < ring->originX + ring->width; ++cx)
                {
                    float l, h;
                    int k;
                    if (VoxelMesh_Classify(cx, cy) != VOXEL_SHAPE_WATER)
                        continue;
                    l = VoxelRelief_CellLift(ring, cx, cy);
                    h = VoxelRelief_CellShift(ring, cx, cy);
                    for (k = 0; k < kinds && (lv[k] != l || sh[k] != h); ++k)
                        ;
                    if (k == kinds && kinds < 16)
                        lv[k] = l, sh[k] = h, cnt[k] = 0, kinds++;
                    if (k < kinds)
                        cnt[k]++;
                }
            sFlatLevel = 0.0f;
            sFlatShift = 0.0f;
            for (int k = 0; k < kinds; ++k)
                if (best < 0 || cnt[k] > cnt[best])
                    best = k;
            if (best >= 0)
                sFlatLevel = lv[best], sFlatShift = sh[best];
            sKeys[slot] = key;
            sLevels[slot] = sFlatLevel;
            sShifts[slot] = sFlatShift;
        }
        *level = sLevels[slot];
        *shift = sShifts[slot];
        return true;
    }
    ex = x < ring->originX ? ring->originX
       : x >= ring->originX + ring->width ? ring->originX + ring->width - 1 : x;
    ey = y < ring->originY ? ring->originY
       : y >= ring->originY + ring->height ? ring->originY + ring->height - 1 : y;
    /* (a fill the map's fixes give a height: there, laid on from the map's
     * edge as it is drawn - shifted by its own height instead, a fill set
     * lower than the edge stood rows away from it, a trench between) */
    if (VoxelRelief_EdgeFillAt(ring, x, y, &fill) && fill.hasTop)
    {
        *level = (float)fill.top / 16.0f;
        *shift = VoxelRelief_CellShift(ring, ex, ey);
        return true;
    }
    *level = 0.0f;
    *shift = 0.0f;
    /* Where the map's edge is land, the nearest of its sea along the edge:
     * a sea filled past an edge (VoxelWorld_BorderMetatile) lies level with
     * the map's, not raised to the belt's base beside a stretch of shore. */
    if (VoxelMesh_Classify(ex, ey) != VOXEL_SHAPE_WATER)
    {
        bool alongY = x < ring->originX || x >= ring->originX + ring->width;

        for (int k = 1; k <= VOXEL_BORDER_SEA_SEARCH; ++k)
        {
            int ax = alongY ? ex : ex - k, ay = alongY ? ey - k : ey;
            int bx = alongY ? ex : ex + k, by = alongY ? ey + k : ey;

            if (VoxelWorld_GetInstanceAt(ax, ay) == ring && VoxelMesh_Classify(ax, ay) == VOXEL_SHAPE_WATER)
            {
                ex = ax, ey = ay;
                break;
            }
            if (VoxelWorld_GetInstanceAt(bx, by) == ring && VoxelMesh_Classify(bx, by) == VOXEL_SHAPE_WATER)
            {
                ex = bx, ey = by;
                break;
            }
        }
    }
    if (VoxelMesh_Classify(ex, ey) == VOXEL_SHAPE_WATER)
    {
        *level = VoxelRelief_CellLift(ring, ex, ey);
        *shift = VoxelRelief_CellShift(ring, ex, ey);
    }
    return true;
}

/*
 * A belt of land stood at the base of the map the player is on, and a map
 * whose edge is lifted over its base ended in a cliff down to it: Route
 * 120's east edge, three levels up, over a wood three levels down. It stands
 * at the level of the map's edge cell nearest it instead (the corner's past
 * a corner), as a belt of water does (BorderWaterLevel); settings.txt's
 * voxel_border_edge=0 puts it back at the base, to compare.
 */
static bool sBorderFollowsEdge = true;

void VoxelMesh_SetBorderFollowsEdge(bool on)
{
    sBorderFollowsEdge = on;
}

static bool BeltIsTree(int x, int y);
static bool BeltTreeOwn(int x, int y);
static bool BorderWaterLevel(int x, int y, float *level, float *shift);
static bool ShoreTree(int x, int y, int *wx, int *wy);
static bool SeaWood(int x, int y, int *wx, int *wy);
enum { BELT_TIER_DEPTH = 2 };
static bool TerracesOn(void);
static bool FieldOn(void);
static void BeltField(int vx, int vy, float *height, float *south);

/*
 * POL-13 / POL-40 (Brennan 2026-11-16: "the rock still shows pillars ... just make a mountain round itself out
 * instead of pillars"; "all mountains we have added in are not 3D but more so 2D blocks ... the mirrored mountains
 * meshing well with one another"): a land cell of the belt took its nearest edge cell's height, so every column of
 * the map's edge went on outward at its own height - a cliff column of the edge a long ridge, the next column's
 * another: the pillars. Now the belt's land is one mountain: its height out at distance d is the map's edge
 * averaged along the edge over a reach that widens the further out it is (+-(1 + 2d) cells, the nearer the
 * heavier, water left out) - neighbouring columns grow together - and it rounds off going out: held for
 * BELT_HOLD cells, then down a quarter cosine to the ground over BELT_FALL. With the belt's sloped tops
 * (BeltCorner) that is one rounded slope, no walls. Trees keep their own rule (POL-41). WJPC_BELT_COLUMNS=1: as
 * before (each edge column straight out).
 */
enum { BELT_HOLD = 1, BELT_FALL = 7 };

/* Is a map whose belt is laid as its drawing (Lavaridge) within `reach` cells? There the belt's land keeps its edge
 * columns' heights: the drawn belt stands its rows that far south of where it is tiled, and a rounded slope beside
 * it opened a gap of sky between the two (Lavaridge's north-east, seen 2026-11-17). */
static bool NearDrawnBelt(int x, int y, int reach)
{
    for (unsigned i = 0; i < VoxelWorld_InstanceCount(); ++i)
    {
        const VoxelMapInstance *m = VoxelWorld_Instance(i);
        int dx, dy;

        if (m == NULL || m->layout == NULL || m->indoor || VoxelRelief_Belt(m) == 0)
            continue;
        dx = x < m->originX ? m->originX - x : x >= m->originX + m->width ? x - (m->originX + m->width - 1) : 0;
        dy = y < m->originY ? m->originY - y : y >= m->originY + m->height ? y - (m->originY + m->height - 1) : 0;
        if (dx <= reach && dy <= reach)
            return true;
    }
    return false;
}

/* POL-46: the belt's terraces (so far round a drawn belt - Lavaridge's - and what of another map's belt meets it):
 * blocks and walls all round, no slopes. WJPC_BELT_SMOOTH=1: none. */
static bool TerraceBand(int x, int y)
{
    static int sOff = -1;

    if (sOff < 0)
        sOff = getenv("WJPC_BELT_SMOOTH") != NULL;
    return !sOff && (TerracesOn() || NearDrawnBelt(x, y, 24));
}

/* POL-13/40: a belt land cell `d` out from its map, its mountain `h` (shift `sh`) high: held BELT_HOLD cells, then
 * rounded down to the ground, and down to the water's level near water (BeltMountain, and the edge fills) */
static void BeltRoundOff(int x, int y, int d, float h, float sh, float *level, float *shift)
{
    float f;

    if (d <= BELT_HOLD)
        f = 1.0f;
    else if (d >= BELT_HOLD + BELT_FALL)
        f = 0.0f;
    else
        f = cosf((float)(d - BELT_HOLD) / (float)BELT_FALL * 1.5707963f);
    /* down to the ground the belt stands on (its courses under the map's zero), not under it */
    {
        const float floorLevel = -(float)VOXEL_BORDER_COURSES;
        float top = h > floorLevel ? h : floorLevel;

        *level = floorLevel + (top - floorLevel) * f;
        /* and down to the water's edge (Route 118's sea east of the mountain: the slope stood its full height
         * over the sea, a stair of tall dark walls along the shore): within BELT_SHORE cells of water - the map's
         * or the belt's - it comes down to the map's zero, the water's level, the nearer the lower */
        {
            enum { BELT_SHORE = 4 };
            int wd = BELT_SHORE + 1;
            float wl, ws;

            for (int r = 1; r <= BELT_SHORE && wd > BELT_SHORE; ++r)
                for (int j = -r; j <= r && wd > BELT_SHORE; ++j)
                    for (int i = -r; i <= r; ++i)
                    {
                        int qx = x + i, qy = y + j;

                        if (abs(i) != r && abs(j) != r)
                            continue;
                        if (VoxelWorld_GetInstanceAt(qx, qy) != NULL
                            ? VoxelMesh_Classify(qx, qy) == VOXEL_SHAPE_WATER
                            : BorderWaterLevel(qx, qy, &wl, &ws))
                        {
                            wd = r;
                            break;
                        }
                    }
            if (wd <= BELT_SHORE && *level > 0.0f)
            {
                float g = (float)(wd - 1) / (float)(BELT_SHORE - 1);

                g = sinf(g * 1.5707963f);
                *level *= g;
                top = top > 0.01f ? top : 0.01f;
            }
        }
        *shift = sh * (*level - floorLevel) / (top - floorLevel > 0.01f ? top - floorLevel : 1.0f);
    }
}

static void BeltMountain(const VoxelMapInstance *ring, int x, int y, int ex, int ey, float *level, float *shift)
{
    static int sOff = -1;
    bool alongY = x < ring->originX || x >= ring->originX + ring->width;
    bool alongX = y < ring->originY || y >= ring->originY + ring->height;
    int dx = x > ex ? x - ex : ex - x, dy = y > ey ? y - ey : ey - y;
    int d = dx > dy ? dx : dy, reach;
    float sumW = 0.0f, sumH = 0.0f, sumS = 0.0f, h, sh, f;

    if (sOff < 0)
        sOff = getenv("WJPC_BELT_COLUMNS") != NULL;
    if (sOff || BeltIsTree(x, y) || NearDrawnBelt(x, y, 8))
        return;
    reach = 1 + 2 * d;
    for (int k = -reach; k <= reach; ++k)
    {
        /* past a corner (both off the map) the reach runs along both edges' lines: the corner cell's own row */
        int cx = alongY && !alongX ? ex : ex + k, cy = alongY && !alongX ? ey + k : ey;
        float w = (float)(reach + 1 - abs(k));

        if (alongX && alongY && k != 0)
        {
            /* a corner: average the corner cell's two edges */
            cx = ex + ((k & 1) ? k / 2 : 0);
            cy = ey + ((k & 1) ? 0 : k / 2);
        }
        if (cx < ring->originX || cy < ring->originY || cx >= ring->originX + ring->width
         || cy >= ring->originY + ring->height || VoxelMesh_Classify(cx, cy) == VOXEL_SHAPE_WATER)
            continue;
        sumW += w;
        sumH += w * VoxelRelief_CellLift(ring, cx, cy);
        sumS += w * VoxelRelief_CellShift(ring, cx, cy);
    }
    if (sumW <= 0.0f)
        return;
    h = sumH / sumW;
    sh = sumS / sumW;
    BeltRoundOff(x, y, d, h, sh, level, shift);
}

/*
 * POL-46, Brennan 2026-11-17 22:07 ("generalize these exact rules into our global overworld terrain mesher so they
 * automatically apply across the entire Hoenn region"; "always generate horizontal stepped cliff ledges (fixed
 * tile-height vertical rock walls ... + flat horizontal top caps), never stretched vertical walls or organ-pipe
 * slivers"; "Continuous Mountain Massifs: When a mountain border ... or map connection meets another elevated map,
 * weld edge vertices and continue stepping tiers"). A cell of the belt's land (not trees, not water) stands as
 * the highest of every loaded map's terraces at it: each map's nearest edge cell, from its highest point (whole
 * levels, rounded up), down a level every BELT_TIER_DEPTH cells out; a drawn belt's (Lavaridge's) rings of ledges
 * climbing from its top the same way; never under the ground. Drawn as far south as it is high, as every point of
 * the drawing is, so neighbouring tops meet; walls all round (BeltCloseSeams), blocks not slopes (TerraceBand).
 * Beside a map the belt meets its edge's height (no gully, no wall over a gap); between two maps the higher's
 * terraces win, so their two mountains join. WJPC_BELT_OLD_LEVELS=1: the levels as before (edge-following,
 * rounded off).
 */
enum { BELT_TERRACE_REACH = 24, BELT_TERRACE_SPAN = 5 };
static bool TerracesOn(void)
{
    static int sOn = -1;
    if (sOn < 0)
        /* not yet by default: Route 118's north came out as tall columns (2026-11-17 22:55, WIP) */
        sOn = getenv("WJPC_BELT_TERRACES_ALL") != NULL && getenv("WJPC_BELT_SMOOTH") == NULL;
    return sOn != 0;
}

static bool BeltTerraces(int x, int y, const VoxelMapInstance *ring, float *level, float *shift)
{
    float best = -1e9f;

    if (!TerracesOn() || BeltIsTree(x, y))
        return false;
    for (unsigned i = 0; i < VoxelWorld_InstanceCount(); ++i)
    {
        const VoxelMapInstance *m = VoxelWorld_Instance(i);
        int mx, my, gx, gy, d;
        float top;

        if (m == NULL || m->layout == NULL || m->indoor)
            continue;
        mx = x < m->originX ? m->originX : x >= m->originX + m->width ? m->originX + m->width - 1 : x;
        my = y < m->originY ? m->originY : y >= m->originY + m->height ? m->originY + m->height - 1 : y;
        gx = x > mx ? x - mx : mx - x;
        gy = y > my ? y - my : my - y;
        d = gx > gy ? gx : gy;
        if (d < 1 || d > BELT_TERRACE_REACH)
            continue;
        if (VoxelRelief_Belt(m) != 0)
        {
            /* a drawn belt's rings (DrawnTierLift), its first ring its own courses */
            top = VoxelRelief_Base(m) + (float)VoxelRelief_Belt(m) + (float)((d - 1) / BELT_TIER_DEPTH);
        }
        else
        {
            float hi = -1e9f;

            if (VoxelMesh_Classify(mx, my) == VOXEL_SHAPE_WATER)
                continue;
            /* the edge's highest point over BELT_TERRACE_SPAN cells along it either way: a terrace runs on as one
             * ledge, never a column a cell (each its own height, its own way south: the organ pipes) */
            for (int k = -BELT_TERRACE_SPAN; k <= BELT_TERRACE_SPAN; ++k)
            {
                int cx = gy >= gx ? mx + k : mx, cy = gy >= gx ? my : my + k;
                const int16_t *g;
                float h;

                if (cx < m->originX || cy < m->originY || cx >= m->originX + m->width || cy >= m->originY + m->height
                 || VoxelMesh_Classify(cx, cy) == VOXEL_SHAPE_WATER)
                    continue;
                g = VoxelRelief_Cell(m, cx, cy);
                if (g != NULL)
                {
                    int t = g[0];
                    for (int q = 1; q < VOXEL_RELIEF_SIDE * VOXEL_RELIEF_SIDE; ++q)
                        t = g[q] > t ? g[q] : t;
                    h = t / 16.0f;
                }
                else
                    h = VoxelRelief_CellLift(m, cx, cy);
                hi = h > hi ? h : hi;
            }
            if (hi < -1e8f)
                continue;
            /* (south of a map the belt is in front of it, under the view: at the edge's height, never climbing) */
            top = VoxelRelief_Base(m) + ceilf(hi - 0.001f) + (float)VOXEL_BORDER_COURSES
                - (float)((d - 1) / BELT_TIER_DEPTH);
        }
        if (top > best)
            best = top;
    }
    if (best < -1e8f)
        return false;
    if (best < VoxelRelief_Base(ring))
        best = VoxelRelief_Base(ring);
    best = floorf(best + 0.001f);
    *level = best - VoxelRelief_Base(ring) - (float)VOXEL_BORDER_COURSES;
    *shift = best - VoxelRelief_Base(ring);
    return true;
}

/*
 * Brennan 2026-11-22 21:54 (the woods north of Fallarbor: "packed wall-to-wall in a tight, uniform grid ... scatter
 * them out naturally with small open patches of ground between them"): in a wood of an edge fill flagged sparse,
 * whether the tree whose top-left cell is (tx, ty) is left out - about half, fixed by place: clearings of
 * 6x3 cells and single trees. The tree emitter skips it (voxel_tree.c) and the floor there is laid in the light (EmitBorder).
 * WJPC_WOOD_DENSE=1: every tree.
 */
static uint32_t WoodHash(int x, int y)
{
    uint32_t h = (uint32_t)x * 0x8DA6B343u ^ (uint32_t)y * 0xD8163841u;
    h ^= h >> 13;
    h *= 0x85EBCA6Bu;
    h ^= h >> 16;
    return h;
}

/* whether the belt cell (x, y) is of a scattered fill (its open ground is the plateau's, not the wood's soil) */
bool VoxelMesh_WoodScatter(int x, int y)
{
    const VoxelMapInstance *ring;
    VoxelEdgeFill fill;

    return VoxelWorld_GetInstanceAt(x, y) == NULL && (ring = VoxelWorld_BeltRing(x, y)) != NULL
        && VoxelRelief_EdgeFillAt(ring, x, y, &fill) && fill.scatter;
}

bool VoxelMesh_WoodGap(int tx, int ty)
{
    static int sDense = -1;
    const VoxelMapInstance *ring;
    VoxelEdgeFill fill;
    uint32_t a, b;

    if (sDense < 0)
        sDense = getenv("WJPC_WOOD_DENSE") != NULL;
    if (sDense || VoxelWorld_GetInstanceAt(tx, ty) != NULL || (ring = VoxelWorld_BeltRing(tx, ty)) == NULL
     || !VoxelRelief_EdgeFillAt(ring, tx, ty, &fill) || !fill.sparse)
        return false;
    a = WoodHash(tx / 6, ty / 3) & 1023u;   /* clearings: a 6x3 patch of the wood */
    b = WoodHash(tx, ty) & 1023u;           /* and single trees */
    if (fill.scatter)                       /* (scattered: about one tree in six, in loose groups) */
    {
        /* none by a step between two fills (Route 114's 88 / 160 px rows): the ground slopes there and its trees
         * stood stretched down it in a line */
        /* (and none within two cells of another map's belt: where two maps' belts meet, a diagonal, the ground
         * steps and its trees stood stretched down it - Route 114's west against Route 115's north) */
        for (int dy = -2; dy <= 3; dy++)
            for (int dx = -2; dx <= 3; dx++)
            {
                VoxelEdgeFill nf;
                const VoxelMapInstance *nr;
                if (VoxelWorld_GetInstanceAt(tx + dx, ty + dy) == NULL
                 && (nr = VoxelWorld_BeltRing(tx + dx, ty + dy)) != NULL && nr != ring)
                    return true;
                if (VoxelWorld_GetInstanceAt(tx + dx, ty + dy) == NULL
                 && (nr = VoxelWorld_BeltRing(tx + dx, ty + dy)) != NULL
                 && (!VoxelRelief_EdgeFillAt(nr, tx + dx, ty + dy, &nf) || !nf.hasTop
                     || VoxelRelief_Base(nr) * 16 + nf.top != VoxelRelief_Base(ring) * 16 + fill.top))
                    return true;
            }
        return a < 600u || b < 700u;
    }
    /* not in the first rows by the map: the edge's own line of trees stays whole */
    if (ty >= ring->originY - 2 && ty < ring->originY + ring->height + 2 && tx >= ring->originX - 2
     && tx < ring->originX + ring->width + 2)
        return false;
    return a < 330u || b < 250u;
}

bool VoxelMesh_BorderLevel(int x, int y, float *level, float *shift)
{
    const VoxelMapInstance *ring = VoxelWorld_BeltRing(x, y); /* POL-30: was instance 0 (see BorderWaterLevel) */
    int ex, ey;

    VoxelEdgeFill fill;

    *level = 0.0f;
    *shift = 0.0f;
    if (ring == NULL || ring->indoor || VoxelRelief_Belt(ring) != 0
     || VoxelWorld_GetInstanceAt(x, y) != NULL)
        return false;
    ex = x < ring->originX ? ring->originX
       : x >= ring->originX + ring->width ? ring->originX + ring->width - 1 : x;
    ey = y < ring->originY ? ring->originY
       : y >= ring->originY + ring->height ? ring->originY + ring->height - 1 : y;
    /* POL-46 everywhere (Brennan 22:07: "do not hardcode this fix only for Lavaridge"): the belt's land as terraces
     * round every map (BeltTerraces) */
    if (BeltTerraces(x, y, ring, level, shift))
        return true;
    /* POL-46 rule B (Brennan 2026-11-17 22:07: "align the ground elevation beneath the trees with the adjacent cliff
     * terraces so trees sit on continuous grass/dirt plateaus rather than floating or sitting on disconnected black
     * shelves"): a wood of the belt stands on the field (BeltField) - the ledge under its middle, as far south as the
     * field there - so the trees and the terraces beside them are one ground. WJPC_BELT_TREES_OWN=1: as before. */
    if (FieldOn() && BeltIsTree(x, y) && !BeltTreeOwn(x, y))
    {
        float h = 1e9f, sh = 0.0f;
        /* the lowest of its corners (the mean stood a wood beside a map a quarter level over the map's edge, a
         * dark skirt down to it along the wood: Brennan 19:08 "thin black crack/line along the boundary") */
        for (int k = 0; k < 4; ++k)
        {
            float fh, fs;
            BeltField(x + (k & 1), y + (k >> 1), &fh, &fs);
            h = fh < h ? fh : h;
            sh += fs * 0.25f;
        }
        /* on the ledge it lies on (the field's terraces, EmitFieldCell) */
        *level = floorf(h + 1e-4f) - VoxelRelief_Base(ring);
        *shift = sh - VoxelRelief_Base(ring);
        return true;
    }
    /* a fill the map's fixes give a height stands its top there, laid on
     * from the map's edge as it is drawn (BorderWaterLevel) */
    if (VoxelRelief_EdgeFillAt(ring, x, y, &fill) && fill.hasTop && !fill.water)
    {
        *level = (float)fill.top / 16.0f - (float)VOXEL_BORDER_COURSES;
        *shift = VoxelRelief_CellShift(ring, ex, ey);
        /* Brennan 2026-11-23 08:20 (south-west of Fallarbor: trees "falling", cut by straight lines): a flat fill
         * lower than the map's edge beside it (Route 114's east, its edge climbing 2 -> 4 levels over the fill's 2)
         * took the edge's way south, its trees rows off their ground, stretched: there it goes only as far south as it
         * is high, as its ground does (FlatFillPoint). (Not everywhere - as far south as the fill's own height, Route
         * 114's west woods opened slits of sky at their far end, 2026-11-23: the thinned
         * woods - Route 114's west, Fallarbor's and Route 113's north, checked as they are - keep the edge's.)
         * WJPC_FLAT_FILL_EDGE_SHIFT=1: the edge's always, as before. */
        {
            static int sEdgeShift = -1;
            if (sEdgeShift < 0)
                sEdgeShift = getenv("WJPC_FLAT_FILL_EDGE_SHIFT") != NULL;
            if (fill.flat && !fill.sparse && !sEdgeShift && *shift > *level)
                *shift = *level;
        }
        /* POL-40 (Route 118's north: the fill stood the mountain's whole height out to the horizon, its side to
         * the sea a stair of tall dark walls): rounded off as the belt's own mountain is (WJPC_BELT_COLUMNS=1 not) */
        if (getenv("WJPC_BELT_COLUMNS") == NULL && !NearDrawnBelt(x, y, 8))
        {
            int fdx = x > ex ? x - ex : ex - x, fdy = y > ey ? y - ey : ey - y, d = fdx > fdy ? fdx : fdy;

            /* (a flat fill: level for its first FLAT_FILL_ROWS rows, then rounded off - level out to the window's
             * edge, the wood's end stood against the sky at the horizon) */
            enum { FLAT_FILL_ROWS = 24 };
            if (fill.flat)
                d = d > FLAT_FILL_ROWS ? d - FLAT_FILL_ROWS : 0;
            if (d > 0 || !fill.flat)
                BeltRoundOff(x, y, d, *level, *shift, level, shift);
        }
        return true;
    }
    if (!sBorderFollowsEdge)
        return false;
    if (VoxelMesh_Classify(ex, ey) == VOXEL_SHAPE_WATER)
        return false;
    *level = VoxelRelief_CellLift(ring, ex, ey);
    *shift = VoxelRelief_CellShift(ring, ex, ey);
    BeltMountain(ring, x, y, ex, ey, level, shift);
    /*
     * POL-41 follow-up (Brennan 2026-11-16 17:56, west of the Weather Institute: dark bands between rows of the
     * wood): a wood of the belt took exactly its nearest edge cell's level, and where the edge climbs a cliff a
     * row of crowns stood a whole level over the next with the void under it. It takes the mean of its map's edge
     * cells along the edge over +-BELT_TREE_SPAN cells (the nearer the heavier; water left out): the wood rises
     * with the edge a little a row, and its crowns hide the rise.
     */
    /* Brennan 2026-11-20 19:48 ("only the blue sea water renders behind the back row of 3D trees"): a large tree of
     * a wood on its own ground (BeltTreeOwn) that touches the belt's sea stands at the sea's level, all four cells
     * alike - the wood's ground there was a level over the sea, a green step along the shore. WJPC_SHORE_TREES_HIGH=1:
     * as before. */
    {
        int wx, wy;
        if (ShoreTree(x, y, &wx, &wy) && BorderWaterLevel(wx, wy, level, shift))
            return true;
        /* (and the rest of such a wood near the sea, the same level: Brennan 20:00, a shore row lowered alone left
         * the next row's ground standing a level over it - green steps again) */
        if (SeaWood(x, y, &wx, &wy) && BorderWaterLevel(wx, wy, level, shift))
            return true;
    }
    if (BeltIsTree(x, y) && getenv("WJPC_BELT_TREE_STEPS") == NULL)
    {
        enum { BELT_TREE_SPAN = 4 };
        bool alongY = x < ring->originX || x >= ring->originX + ring->width;
        float sumW = 0.0f, sumH = 0.0f, sumS = 0.0f;

        for (int k = -BELT_TREE_SPAN; k <= BELT_TREE_SPAN; ++k)
        {
            int cx = alongY ? ex : ex + k, cy = alongY ? ey + k : ey;
            float w = (float)(BELT_TREE_SPAN + 1 - abs(k));

            if (cx < ring->originX || cy < ring->originY || cx >= ring->originX + ring->width
             || cy >= ring->originY + ring->height || VoxelMesh_Classify(cx, cy) == VOXEL_SHAPE_WATER)
                continue;
            sumW += w;
            sumH += w * VoxelRelief_CellLift(ring, cx, cy);
            sumS += w * VoxelRelief_CellShift(ring, cx, cy);
        }
        if (sumW > 0.0f)
        {
            *level = sumH / sumW;
            *shift = sumS / sumW;
        }
    }
    /*
     * POL-30: between two maps whose edges stand at different heights (Lavaridge's north cliff and Route 112's
     * west slope) the belt took the nearer one's and stood a tall wall where the two halves met. Every loaded
     * map's nearest edge now pulls on the cell, the nearer the stronger (1 / distance cubed): beside a map the
     * belt still meets that map's edge, and between maps it goes from one height to the other in small steps.
     * WJPC_BELT_NO_BLEND=1: the nearest map's alone, to compare.
     */
    {
        static int sBlend = -1;
        float sumW = 0.0f, sumH = 0.0f, sumS = 0.0f;
        const float ownLevel = *level, ownShift = *shift;

        if (sBlend < 0)
            sBlend = getenv("WJPC_BELT_NO_BLEND") == NULL && !VoxelWorld_BeltOld();
        /* POL-41 (Brennan 2026-11-16, Route 119: "The trees in the back ground look to be higher than the
         * others ... All trees should align together to show a full forest"): a wood of the belt stood each
         * column at the blend of every map's edge (Route 118's mountains pulled the wood west of Route 119 up
         * column by column), and the steps between columns read as walls of trees. A tree cell keeps its own
         * map's edge level, unblended: the wood lies as the map's edge beside it does. WJPC_BELT_TREE_BLEND=1:
         * blended as before. */
        static int sTreeBlend = -1;
        if (sTreeBlend < 0)
            sTreeBlend = getenv("WJPC_BELT_TREE_BLEND") != NULL;
        bool blendHere = sBlend && (sTreeBlend || !BeltIsTree(x, y));
        for (unsigned i = 0; blendHere && i < VoxelWorld_InstanceCount(); ++i)
        {
            const VoxelMapInstance *m = VoxelWorld_Instance(i);
            int mx, my, dx, dy, d;
            float w, h = 0.0f, sh = 0.0f;

            if (m == NULL || m->layout == NULL || m->indoor || VoxelRelief_Belt(m) != 0)
                continue;
            mx = x < m->originX ? m->originX : x >= m->originX + m->width ? m->originX + m->width - 1 : x;
            my = y < m->originY ? m->originY : y >= m->originY + m->height ? m->originY + m->height - 1 : y;
            dx = x > mx ? x - mx : mx - x;
            dy = y > my ? y - my : my - y;
            d = dx > dy ? dx : dy;
            if (d < 1)
                d = 1;
            if (m == ring)
            {
                h = ownLevel;  /* POL-13/40: the ring's own mountain (BeltMountain), not its edge cell's height */
                sh = ownShift;
            }
            else if (VoxelMesh_Classify(mx, my) != VOXEL_SHAPE_WATER)
            {
                h = VoxelRelief_CellLift(m, mx, my);
                sh = VoxelRelief_CellShift(m, mx, my);
            }
            w = 1.0f / ((float)d * (float)d * (float)d);
            sumW += w;
            sumH += w * (h + VoxelRelief_Base(m));
            sumS += w * sh;
        }
        if (sumW > 0.0f)
        {
            *level = sumH / sumW - VoxelRelief_Base(ring);
            *shift = sumS / sumW;
        }
    }
    /*
     * POL-46 (Brennan 2026-11-17 19:42: "Never leave open vertical seams between two adjacent elevation levels";
     * "Start by fixing the Lavaridge Town to Route 112/Mt. Chimney northern mountain seam"). Beside a belt laid as
     * its drawing (Lavaridge's, its rows 4 levels up) Route 112's part of the belt lay at the ground, 2 levels
     * under it, then rose to Route 112's mountain: a pit with the sky down its stair-shaped seam. Near a drawn belt
     * the cell now starts at the drawn belt's top where the two meet and goes over to its own height over
     * BELT_DRAWN_EASE cells (eased), so the mountain rises out of the drawn rock with no step.
     * WJPC_BELT_DRAWN_PIT=1: as before.
     */
    bool inDrawnBand = false;
    {
        enum { BELT_DRAWN_EASE = 8 };
        static int sPit = -1;

        if (sPit < 0)
            sPit = getenv("WJPC_BELT_DRAWN_PIT") != NULL;
        static int sTierBand = -1;
        if (sTierBand < 0)
            sTierBand = getenv("WJPC_BELT_SMOOTH") == NULL;
        const bool tiersHere = sTierBand && !BeltIsTree(x, y);
        for (unsigned i = 0; !sPit && i < VoxelWorld_InstanceCount(); ++i)
        {
            const VoxelMapInstance *m = VoxelWorld_Instance(i);
            int dm, dr, gx, gy;
            float drawnTop, ownTop, t;

            if (m == NULL || m == ring || m->layout == NULL || m->indoor || VoxelRelief_Belt(m) == 0)
                continue;
            gx = x < m->originX ? m->originX - x : x >= m->originX + m->width ? x - (m->originX + m->width - 1) : 0;
            gy = y < m->originY ? m->originY - y : y >= m->originY + m->height ? y - (m->originY + m->height - 1) : 0;
            dm = gx > gy ? gx : gy;
            gx = x < ring->originX ? ring->originX - x : x >= ring->originX + ring->width ? x - (ring->originX + ring->width - 1) : 0;
            gy = y < ring->originY ? ring->originY - y : y >= ring->originY + ring->height ? y - (ring->originY + ring->height - 1) : 0;
            dr = gx > gy ? gx : gy;
            /* Terraces (Brennan 21:00: "horizontal stepped plateaus (terraces)—each tier must have a vertical
             * rock wall of fixed tile height (1 to 2 tiles high ...) followed by a flat horizontal top ledge ...
             * before stepping up to the next tier", as Route 112's own behind the Mart): from the drawn belt's top a
             * level every BELT_TIER_DEPTH cells, never over its own height, every row drawn as far south as the
             * drawn belt's (one row of tops, no columns each a different way south: the "organ pipes"). The map
             * edge beside it closes what is left (EdgeOverBelt). */
            if (tiersHere)
            {
                enum { BELT_TIER_REACH = 24 };
                float own;

                if (dm - dr >= BELT_TIER_REACH)
                    continue;
                drawnTop = VoxelRelief_Base(m) + (float)VoxelRelief_Belt(m);
                own = VoxelRelief_Base(ring) + (float)VOXEL_BORDER_COURSES + *level;
                /* the drawn belt's rings of ledges, gone on across the seam (DrawnTierLift); and (Brennan 22:07,
                 * "Align the elevation tiers of the forested east wall with the horizontal cliff terraces ... one
                 * continuous, uniform mountain corner") its own map's edge coming down to them the same way, a level
                 * every 2 cells out from the edge: beside the edge the belt stands at the edge's height (no wall,
                 * no shelf), and the higher of the two wins */
                ownTop = drawnTop + (float)((dm > 1 ? dm - 1 : 0) / 2);
                {
                    /* from the edge cell's highest point (its lattice, not the blend of the edge): the belt beside
                     * it never under it, so no wall of the edge stands open over a gully (22:07 red box) */
                    int ex = x < ring->originX ? ring->originX : x >= ring->originX + ring->width ? ring->originX + ring->width - 1 : x;
                    int ey = y < ring->originY ? ring->originY : y >= ring->originY + ring->height ? ring->originY + ring->height - 1 : y;
                    const int16_t *eg = VoxelRelief_Cell(ring, ex, ey);
                    float edgeTop = own;

                    if (eg != NULL)
                    {
                        int hi = eg[0];
                        for (int g = 1; g < VOXEL_RELIEF_SIDE * VOXEL_RELIEF_SIDE; ++g)
                            hi = eg[g] > hi ? eg[g] : hi;
                        if (VoxelRelief_Base(ring) + hi / 16.0f > edgeTop)
                            edgeTop = VoxelRelief_Base(ring) + hi / 16.0f;
                    }
                    float cone = ceilf(edgeTop - 0.001f) - (float)((dr > 1 ? dr - 1 : 0) / 2);
                    if (cone > ownTop)
                        ownTop = cone;
                }
                ownTop = floorf(ownTop + 0.001f);
                *level = ownTop - VoxelRelief_Base(ring) - (float)VOXEL_BORDER_COURSES;
                /* drawn as far south as it is high, as the drawing has every point (the drawn belt and the map's
                 * edge both are): one rule, so every top meets its neighbours' */
                *shift = ownTop - VoxelRelief_Base(ring);
                inDrawnBand = true;
                continue;
            }
            if (dm - dr >= BELT_DRAWN_EASE)
                continue;
            t = dm - dr <= 0 ? 0.0f : (float)(dm - dr) / (float)BELT_DRAWN_EASE;
            t = 0.5f - 0.5f * cosf(t * 3.14159265f);
            drawnTop = VoxelRelief_Base(m) + (float)VoxelRelief_Belt(m);
            ownTop = VoxelRelief_Base(ring) + (float)VOXEL_BORDER_COURSES + *level;
            ownTop = drawnTop + (ownTop - drawnTop) * t;
            /* and drawn as far south as the drawn belt's rows are (its courses, VoxelMesh_EmitBorder), going over
             * to its own: tops at one height a different way south left the stair of sky as before */
            *shift = (float)VoxelRelief_Belt(m) + (*shift - (float)VoxelRelief_Belt(m)) * t;
            *level = ownTop - VoxelRelief_Base(ring) - (float)VOXEL_BORDER_COURSES;
        }
    }
    /* POL-46, Brennan's brief ("Mountains and background hills must step inward concentrically as height
     * increases ... tiered terraces"), asked to see it beside the rounded one ("Show me both"): WJPC_BELT_TIERS=1
     * stands the belt's land in whole levels, so a slope reads as terraces each a level up and further in. */
    {
        static int sTiers = -1;

        if (sTiers < 0)
            sTiers = getenv("WJPC_BELT_TIERS_ALL") != NULL;
        if (sTiers && !BeltIsTree(x, y) && !inDrawnBand)
        {
            float q = floorf(*level + 0.5f);

            /* (and drawn a whole row south a level: a terrace is a block of the belt, its rows whole) */
            *shift = floorf(*shift + (q - *level) + 0.5f);
            *level = q;
        }
    }
    return true;
}

/* POL-30 test aid (WJPC_BELT_TABLE, voxel_gl.c): what stands at a cell of the belt - its metatile, and its top's
 * height over the world's zero in 1/16 tile (water: its surface). FALSE: not a belt cell. */
bool VoxelMesh_BeltCell(int x, int y, int *metatile, int *height16)
{
    float level, shift;

    if (!BorderAt(x, y))
        return false;
    *metatile = VoxelWorld_BorderMetatile(x, y);
    bool follows = true;

    if (!BorderWaterLevel(x, y, &level, &shift))
    {
        follows = VoxelMesh_BorderLevel(x, y, &level, &shift);
        level += (float)VOXEL_BORDER_COURSES;
    }
    if (getenv("WJPC_BELT_TABLE_Z") != NULL && !follows)
        *metatile |= 1 << 28;
    *height16 = (int)floorf((level + BeltBase(x, y)) * 16.0f + 0.5f);
    if (getenv("WJPC_BELT_TABLE_Z") != NULL) /* (and how far south it is drawn, in the metatile's high bits) */
        *metatile |= ((int)floorf((shift + BeltBase(x, y)) * 16.0f + 0.5f) & 0xFFF) << 16;
    return true;
}

/* A belt cell's top over the one north of it, seen from above: a block
 * standing at its level is drawn as far south as it is high, so a step up
 * from the cell north left the rows between the two tops open (the belt
 * beside Route 112's slope, each row a level over the last). As on the map
 * (EmitNorthFace): the north cell's top goes on flat up to this cell's north
 * edge, and there a wall stands straight up to this top, a tile a level.
 * Beside the map (west or east of it) the flat part is capped down the
 * belt's courses: the map's edge cell stands that much lower. */
static void EmitBorderNorth(VoxelBuilder *b, int x, int y, float low, float lowShift,
                            float top, float shift, float u0, float v0, float u1, float v1)
{
    const float c = (float)VOXEL_BORDER_COURSES, za = y + lowShift, zb = y + shift;

    /* POL-30 (Brennan 2026-11-15: blue saw teeth past Fallarbor): two cells at one height can still lie
     * apart north to south (a filled edge beside another map's belt: each as far south as its own map's edge
     * says), and the sky showed between. The flat part is laid then too (it was only laid under a step), at
     * the lower of the two. */
    if (zb <= za)
        return;
    if (low > top)
        low = top;
    for (float band = floorf(low); band < top; band += 1.0f)
    {
        float h0 = fmaxf(low, band), h1 = fminf(top, band + 1.0f);
        float va = v1 + (v0 - v1) * (h0 - band), vb = v1 + (v0 - v1) * (h1 - band);

        if (h1 <= h0)
            continue;
        VoxelBuilder_Quad(b, &(VoxelVertex){(float)x + 1.0f, h0, zb, u0, va, SHADE_NORTH},
                          &(VoxelVertex){(float)x, h0, zb, u1, va, SHADE_NORTH},
                          &(VoxelVertex){(float)x, h1, zb, u1, vb, SHADE_NORTH},
                          &(VoxelVertex){(float)x + 1.0f, h1, zb, u0, vb, SHADE_NORTH});
    }
    for (float row = floorf(za); row < zb; row += 1.0f)
    {
        float p0 = fmaxf(za, row), p1 = fminf(zb, row + 1.0f);

        if (p1 <= p0)
            continue;
        VoxelBuilder_Quad(b, &(VoxelVertex){(float)x, low, p0, u0, v0 + (v1 - v0) * (p0 - row), SHADE_TOP},
                          &(VoxelVertex){(float)x + 1.0f, low, p0, u1, v0 + (v1 - v0) * (p0 - row), SHADE_TOP},
                          &(VoxelVertex){(float)x + 1.0f, low, p1, u1, v0 + (v1 - v0) * (p1 - row), SHADE_TOP},
                          &(VoxelVertex){(float)x, low, p1, u0, v0 + (v1 - v0) * (p1 - row), SHADE_TOP});
    }
    for (int side = 0; side < 2; ++side)
    {
        float cx = (float)(x + side);

        if (VoxelWorld_GetInstanceAt(x + (side ? 1 : -1), y) == NULL)
            continue;
        if (side)
            VoxelBuilder_Quad(b, &(VoxelVertex){cx, low - c, za, u0, v1, SHADE_TOP},
                              &(VoxelVertex){cx, low - c, zb, u1, v1, SHADE_TOP},
                              &(VoxelVertex){cx, low, zb, u1, v0, SHADE_TOP},
                              &(VoxelVertex){cx, low, za, u0, v0, SHADE_TOP});
        else
            VoxelBuilder_Quad(b, &(VoxelVertex){cx, low - c, zb, u1, v1, SHADE_TOP},
                              &(VoxelVertex){cx, low - c, za, u0, v1, SHADE_TOP},
                              &(VoxelVertex){cx, low, za, u0, v0, SHADE_TOP},
                              &(VoxelVertex){cx, low, zb, u1, v0, SHADE_TOP});
    }
}

/* The faces of a belt cell over a lower one beside it, west, east or south:
 * its own drawing from their level up to its own (a step along the belt
 * where the map's edge steps, or down to a belt of water); north, the wall
 * up from the top there (EmitBorderNorth). */
/* A belt cell drawn as a tree (VoxelTree_EmitBorder): it stands on the level itself, with no ground laid and
 * not the belt's courses higher as every other belt cell's top is. */
/* POL-46 (Brennan 2026-11-18 19:08: "dark flat 2D tree sprites are still visible on the ground plane beneath and
 * between the 3D voxel trees ... Replace 2D tree ground tiles with plain grass"): the ground laid under a wood's
 * edge is the tree's own ground (VoxelTree_GroundMetatile: the grass it stands on), not its drawing laid flat.
 * WJPC_BELT_TREE_ART_GROUND=1: the drawing, as before. */
/* Brennan 2026-11-20 19:48 / 20:00 ("ZERO green ground/grass visible behind or sticking out around the back row of
 * standing trees ... Extend the blue sea water ground plane directly underneath that entire back row"): a large
 * tree of a wood on its own ground (BeltTreeOwn) whose ring of cells round its 2x2 touches the belt's sea is a shore
 * tree: it stands at the sea's level (VoxelMesh_BorderLevel) on the sea itself (BeltTreeGround). (wx, wy): that sea
 * cell. WJPC_SHORE_TREES_HIGH=1: as before. */
static bool ShoreTree(int x, int y, int *wx, int *wy)
{
    static const int kRing[12][2] = { {-1,-1},{0,-1},{1,-1},{2,-1},{-1,0},{2,0},{-1,1},{2,1},{-1,2},{0,2},{1,2},{2,2} };
    const VoxelMapInstance *art;
    int part, ox, oy;
    float wl, ws;

    if (getenv("WJPC_SHORE_TREES_HIGH") != NULL || !BeltIsTree(x, y) || !BeltTreeOwn(x, y))
        return false;
    art = VoxelWorld_BorderArtMap(x, y);
    part = art != NULL ? VoxelTree_Part(art->secondaryTileset, VoxelWorld_BorderMetatile(x, y)) : -1;
    if (part < 0 || part > 3)
        return false;
    ox = x - (part & 1);
    oy = y - (part >> 1);
    for (int k = 0; k < 12; ++k)
        if (VoxelWorld_GetInstanceAt(ox + kRing[k][0], oy + kRing[k][1]) == NULL
         && BorderWaterLevel(ox + kRing[k][0], oy + kRing[k][1], &wl, &ws))
        {
            *wx = ox + kRing[k][0];
            *wy = oy + kRing[k][1];
            return true;
        }
    return false;
}

/* A large tree of a wood on its own ground with the belt's sea within SEA_WOOD_REACH cells of its 2x2: (wx, wy) the
 * nearest sea cell found. WJPC_SHORE_TREES_HIGH=1: none. */
enum { SEA_WOOD_REACH = 10 };
static bool SeaWood(int x, int y, int *wx, int *wy)
{
    const VoxelMapInstance *art;
    int part, ox, oy;
    float wl, ws;

    if (getenv("WJPC_SHORE_TREES_HIGH") != NULL || !BeltIsTree(x, y) || !BeltTreeOwn(x, y))
        return false;
    art = VoxelWorld_BorderArtMap(x, y);
    part = art != NULL ? VoxelTree_Part(art->secondaryTileset, VoxelWorld_BorderMetatile(x, y)) : -1;
    if (part < 0 || part > 3)
        return false;
    ox = x - (part & 1);
    oy = y - (part >> 1);
    for (int r = 1; r <= SEA_WOOD_REACH; ++r)
        for (int dy = -r; dy <= r + 1; ++dy)
            for (int dx = -r; dx <= r + 1; ++dx)
            {
                int cx = ox + dx, cy = oy + dy;
                if (dx > -r && dx < r + 1 && dy > -r && dy < r + 1)
                    continue;   /* the ring at distance r only */
                if (VoxelWorld_GetInstanceAt(cx, cy) == NULL && BorderWaterLevel(cx, cy, &wl, &ws))
                {
                    *wx = cx;
                    *wy = cy;
                    return true;
                }
            }
    return false;
}

static int BeltTreeGround(int x, int y)
{
    const VoxelMapInstance *art = VoxelWorld_BorderArtMap(x, y);
    int mt = VoxelWorld_BorderMetatile(x, y), g;

    if (art == NULL || getenv("WJPC_BELT_TREE_ART_GROUND") != NULL)
        return mt;
    {
        int wx, wy;
        if (ShoreTree(x, y, &wx, &wy))
            return VoxelWorld_BorderMetatile(wx, wy);   /* the sea under a shore tree */
    }
    g = VoxelTree_GroundMetatile(art->secondaryTileset, mt);
    /* the General's small trees (016-017, 0C6-0C7) name no ground of their own: its plain grass, 001 */
    if (g == mt && (mt == 0x016 || mt == 0x017 || mt == 0x0C6 || mt == 0x0C7))
        g = 0x001;
    /* nor do its large trees (1D4-1FD: Route 121's woods) - the step faces and the ground at a wood's edge took the
     * tree's own drawing: the flat dark tree squares over the sea (Brennan 2026-11-20 17:59). WJPC_BELT_TREE_ART_GROUND
     * =1 above: as before. */
    if (g == mt && mt >= 0x1D4 && mt <= 0x1FD && VoxelTree_Part(art->secondaryTileset, mt) >= 0)
        g = 0x001;
    return g >= 0 ? g : mt;
}

static bool BeltIsTree(int x, int y)
{
    const VoxelMapInstance *current = VoxelWorld_BorderArtMap(x, y);

    return current != NULL && VoxelWorld_UsesTreeSprites(current)
        && VoxelTree_Part(current->secondaryTileset, VoxelWorld_BorderMetatile(x, y)) >= 0;
}

/* Brennan 2026-11-20 17:01 (Route 121's north woods: "squished, overlapped, and bunched together ... Space out and
 * rebuild those forest trees using the full, unclipped 3D tree model (matching the clean, individual 3D trees used in
 * standard Hoenn forests)"): a wood of LARGE trees (four-cell trees, the round leafy ones) stands on its own flat
 * ground at its edge's level, as the woods inside the maps do, not on the field's terraces (POL-46 rule B, which stays
 * for the small trees). WJPC_BELT_TREES_OWN=1: every wood so; WJPC_LARGE_TREES_ON_FIELD=1: large ones too on the
 * field, as before. */
static bool BeltTreeOwn(int x, int y)
{
    static int sAll = -1, sField = -1;
    const VoxelMapInstance *art;
    int part;

    if (sAll < 0)
    {
        sAll = getenv("WJPC_BELT_TREES_OWN") != NULL;
        sField = getenv("WJPC_LARGE_TREES_ON_FIELD") != NULL;
    }
    if (sAll)
        return true;
    if (sField)
        return false;
    art = VoxelWorld_BorderArtMap(x, y);
    part = art != NULL ? VoxelTree_Part(art->secondaryTileset, VoxelWorld_BorderMetatile(x, y)) : -1;
    return part >= 0 && part <= 3;
}

/*
 * POL-30 (Brennan 2026-11-15: "fix route 113"): the belt is emitted once per atlas it is drawn with - the
 * current map's, and those of loaded maps on other tile sets whose border the belt shows (voxel_gl.c's belt
 * layers). A pass only lays the cells whose metatile is of its tile sets.
 */
static VOXEL_TLS const void *sBeltPassPrimary, *sBeltPassSecondary;

void VoxelMesh_SetBeltPass(const void *primaryTileset, const void *secondaryTileset)
{
    sBeltPassPrimary = primaryTileset;
    sBeltPassSecondary = secondaryTileset;
}

static bool BeltInPass(int x, int y)
{
    const VoxelMapInstance *art;

    if (sBeltPassPrimary == NULL)
        return true; /* (no pass set: everything, as before) */
    art = VoxelWorld_BorderArtMap(x, y);
    return art == NULL || (art->primaryTileset == sBeltPassPrimary && art->secondaryTileset == sBeltPassSecondary);
}

/* The metatile of the belt at (x, wantY) for a cell at (x, y) (a standing cell reads the rows above it) - the
 * cell's own when that one is of other tile sets than this pass's. */
static int BeltMetatileFor(int x, int wantY, int y)
{
    return BeltInPass(x, wantY) ? VoxelWorld_BorderMetatile(x, wantY) : VoxelWorld_BorderMetatile(x, y);
}

/* The faces of a belt cell's step down to a lower neighbour (d: 0 south, 1 east, 2 west), from `other` up to
 * `level`, both over this cell's base. */
static void BeltStepFaces(VoxelBuilder *builder, int x, int y, int d, int nx, int ny, float other, float otherShift,
                          float level, float shift, float u0, float v0, float u1, float v1)
{
    /* POL-30: a tile a level, as EmitBorderNorth's wall - one drawing pulled over a tall step was streaks
     * (the belt rising behind Lavaridge to Route 112's slope) */
    for (float band = floorf(other); band < level; band += 1.0f)
    {
        float h0 = fmaxf(other, band), h1 = fminf(level, band + 1.0f);
        float vb = v1 + (v0 - v1) * (h0 - band), vt = v1 + (v0 - v1) * (h1 - band);

        if (h1 <= h0)
            continue;
        if (d == 0)
            VoxelMesh_South(builder, (float)x, (float)y + shift, h0, h1, 0.0f, u0, vt, u1, vb);
        else
        {
            /* POL-30 (Brennan 2026-11-15: blue saw teeth past Fallarbor): a cell is drawn as far south as
             * it is high, so the lower neighbour lies further north than this one. A side face standing
             * straight at this cell's row left a triangle of sky down to the neighbour: it leans from this
             * cell's row at the top to the neighbour's row at the foot. */
            float lowZ = otherShift + BeltBase(nx, ny) - BeltBase(x, y);
            float za = (float)y + lowZ + (shift - lowZ) * (h0 - other) / (level - other);
            float zb = (float)y + lowZ + (shift - lowZ) * (h1 - other) / (level - other);
            float fx = d == 1 ? (float)x + 1.0f : (float)x;
            uint8_t shade = d == 1 ? SHADE_EAST : SHADE_WEST;

            if (d == 1)
                VoxelBuilder_Quad(builder,
                    &(VoxelVertex){fx, h0, za + 1.0f, u0, vb, shade}, &(VoxelVertex){fx, h0, za, u1, vb, shade},
                    &(VoxelVertex){fx, h1, zb, u1, vt, shade}, &(VoxelVertex){fx, h1, zb + 1.0f, u0, vt, shade});
            else
                VoxelBuilder_Quad(builder,
                    &(VoxelVertex){fx, h0, za, u0, vb, shade}, &(VoxelVertex){fx, h0, za + 1.0f, u1, vb, shade},
                    &(VoxelVertex){fx, h1, zb + 1.0f, u1, vt, shade}, &(VoxelVertex){fx, h1, zb, u0, vt, shade});
        }
    }
}

/* POL-30: a flat belt cell (water, or a filled edge's top - BorderWaterLevel) beside one that is not flat and
 * stands lower: BeltWaterSkirts only closes flat against flat. (Fallarbor's filled edge beside Route 113's
 * trees: the blue saw teeth, Brennan 2026-11-15.) */
static void BeltFlatSteps(VoxelBuilder *builder, int x, int y, float level, float shift,
                          float u0, float v0, float u1, float v1)
{
    static const int kDir[4][2] = { {0, 1}, {1, 0}, {-1, 0}, {0, -1} };

    for (int d = 0; d < 4; ++d)
    {
        int nx = x + kDir[d][0], ny = y + kDir[d][1];
        float other, otherShift, rebase = BeltBase(nx, ny) - BeltBase(x, y);

        if (!BorderAt(nx, ny) || BorderWaterLevel(nx, ny, &other, &otherShift))
            continue;
        VoxelMesh_BorderLevel(nx, ny, &other, &otherShift);
        other += rebase + (BeltIsTree(nx, ny) ? 0.0f : (float)VOXEL_BORDER_COURSES);
        if (other >= level - 0.05f)
            continue;
        if (d < 3)
            BeltStepFaces(builder, x, y, d, nx, ny, other, otherShift, level, shift, u0, v0, u1, v1);
        else
            EmitBorderNorth(builder, x, y, other, otherShift + rebase, level, shift, u0, v0, u1, v1);
    }
}

/* The art of a belt cell's sides (its steps). Brennan 2026-11-20 17:59 (Route 121's woods over the sea: "flat 2D
 * tree tops/sprites sticking out behind the back row of 3D trees"): a wood cell's sides by the sea are its ground's
 * (the grass under the trees), not the tree's drawing. Brennan 23:18 (the same woods, inland: "flat bright-green
 * square ground tiles are sticking out" in a diagonal row - the steps where the belt's ground rises inside the wood):
 * inside a wood (no sea within SeaWood's reach) the sides are the tree's drawing again, so they read as the wood.
 * WJPC_BELT_TREE_FACE_ART=1: the tree's drawing everywhere; WJPC_BELT_TREE_FACE_GROUND=1: the ground everywhere. */
static int BeltStepArt(int x, int y)
{
    int wx, wy;

    if (!BeltIsTree(x, y) || getenv("WJPC_BELT_TREE_FACE_ART") != NULL)
        return VoxelWorld_BorderMetatile(x, y);
    if (getenv("WJPC_BELT_TREE_FACE_GROUND") != NULL || SeaWood(x, y, &wx, &wy) || ShoreTree(x, y, &wx, &wy))
        return BeltTreeGround(x, y);
    return VoxelWorld_BorderMetatile(x, y);
}

static void EmitBorderSteps(VoxelBuilder *builder, int x, int y, float level, float shift)
{
    /* POL-30 (Brennan 2026-11-15: blue saw teeth past Fallarbor, where its filled edge meets Route 113's
     * trees): a tree cell's foot is the belt's courses under a standing cell's top, and nothing closed that
     * step - one map's border was all trees or none, so the two never met before the belt took each cell from
     * the nearest map. VoxelMesh_EmitBorder stands the courses' faces beside a tree cell, and lays the ground
     * under the first of the trees. */

    static const int kDir[3][2] = { {0, 1}, {1, 0}, {-1, 0} };
    float u0, v0, u1, v1, other, otherShift;

    /* Brennan 2026-11-20 17:59 (Route 121's woods over the sea: "flat 2D tree tops/sprites sticking out behind the
     * back row of 3D trees"): a wood cell's sides are its ground's (the grass under the trees), not the tree's
     * drawing. WJPC_BELT_TREE_FACE_ART=1: as before. */
    if (!MetatileUV(builder, BeltStepArt(x, y), &u0, &v0, &u1, &v1))
        return;
    for (int d = 0; d < 3; ++d)
    {
        int nx = x + kDir[d][0], ny = y + kDir[d][1];

        if (VoxelWorld_GetInstanceAt(nx, ny) != NULL)
            continue;
        /* (a belt of water beside it lies at its own level) */
        if (!BorderWaterLevel(nx, ny, &other, &otherShift))
            VoxelMesh_BorderLevel(nx, ny, &other, &otherShift);
        other += BeltBase(nx, ny) - BeltBase(x, y); /* POL-30: over this cell's base */
        if (other >= level - 0.05f)
            continue;
        BeltStepFaces(builder, x, y, d, nx, ny, other, otherShift, level, shift, u0, v0, u1, v1);
    }
    if (VoxelWorld_GetInstanceAt(x, y - 1) == NULL)
    {
        float low;

        if (BorderWaterLevel(x, y - 1, &other, &otherShift))
            low = other;
        else
        {
            VoxelMesh_BorderLevel(x, y - 1, &other, &otherShift);
            low = other + (BeltIsTree(x, y - 1) ? 0.0f : (float)VOXEL_BORDER_COURSES);
        }

        /* POL-30: over this cell's base, and as far south */
        low += BeltBase(x, y - 1) - BeltBase(x, y);
        otherShift += BeltBase(x, y - 1) - BeltBase(x, y);
        /* Brennan 2026-11-21 01:23 (Route 121's north woods: "a dark green square tile/box sticking out" behind
         * each tree of a diagonal row): where the wood's ground steps (its cells as far south as they are high),
         * a wood cell stood a wall two courses over its own ground at its north edge, above the crowns. Between
         * two wood cells the wall reaches only this cell's ground. WJPC_BELT_WOOD_NORTH_COURSES=1: as before. */
        float top = level + (float)VOXEL_BORDER_COURSES;
        if (BeltIsTree(x, y) && BeltIsTree(x, y - 1) && getenv("WJPC_BELT_WOOD_NORTH_COURSES") == NULL)
            top = level;
        EmitBorderNorth(builder, x, y, low, otherShift, top, shift, u0, v0, u1, v1);
    }
}

/*
 * DLX-26 (Wally's Journey PC, owner-approved 2026-11-13): BorderWaterLevel gives each belt column the level of the
 * map's nearest edge cell, so two belt water cells side by side can stand a little apart, and with nothing between
 * them the clear colour showed through as a thin light line across the sea (the wide view sees far more of the belt
 * than the 3DS's). Closed as 3DS closes its other seams - "a low wall and never a gap onto the clear colour" - by a
 * skirt of the cell's own drawing from its edge to the neighbour's. East and south only, so each edge once.
 */
static void BeltWaterSkirts(VoxelBuilder *b, int x, int y, float wx, float wz, float level, float shift,
                            float u0, float v0, float u1, float v1)
{
    float nl, ns;

    /* POL-30: the neighbour's level over this cell's base (and as far south) */
    const float here = BeltBase(x, y);

    if (BorderAt(x + 1, y) && BorderWaterLevel(x + 1, y, &nl, &ns)
     && (nl += BeltBase(x + 1, y) - here, ns += BeltBase(x + 1, y) - here, nl != level || ns != shift))
    {
        float ex = wx + 1.0f;
        VoxelBuilder_Quad(b,
            &(VoxelVertex){ex, level, wz + shift, u0, v0, SHADE_TOP},
            &(VoxelVertex){ex, level, wz + shift + 1.0f, u0, v1, SHADE_TOP},
            &(VoxelVertex){ex, nl, wz + ns + 1.0f, u1, v1, SHADE_TOP},
            &(VoxelVertex){ex, nl, wz + ns, u1, v0, SHADE_TOP});
    }
    if (BorderAt(x, y + 1) && BorderWaterLevel(x, y + 1, &nl, &ns)
     && (nl += BeltBase(x, y + 1) - here, ns += BeltBase(x, y + 1) - here, nl != level || ns != shift))
    {
        VoxelBuilder_Quad(b,
            &(VoxelVertex){wx, level, wz + shift + 1.0f, u0, v0, SHADE_TOP},
            &(VoxelVertex){wx + 1.0f, level, wz + shift + 1.0f, u1, v0, SHADE_TOP},
            &(VoxelVertex){wx + 1.0f, nl, wz + 1.0f + ns, u1, v1, SHADE_TOP},
            &(VoxelVertex){wx, nl, wz + 1.0f + ns, u0, v1, SHADE_TOP});
    }
}

/*
 * POL-40 (Brennan 2026-11-16, Route 118: "the background mountains are blocks instead of like the mountain south of
 * it"): past a map's edge the belt's land takes the level of the nearest map edges, blended between maps (POL-30),
 * and every cell stood flat at its own level with a wall a tile a level down to the next: a mountain going on past
 * the edge, or the ground rising from one map's height to another's, read as a staircase of blocks with the sky
 * between them. A land cell's top now lies on its corners instead, each corner the mean of the land cells round
 * it (in the world: their bases counted), so neighbouring land cells meet edge to edge as one slope and need no
 * walls between them. Walls still stand towards water, trees and the maps. WJPC_BELT_BLOCKS=1: as before.
 */
/*
 * POL-13/40 (Brennan: the belt's mountains should look "like the mountains that appear in front of them"): the
 * belt's rounded slope was laid with the border's drawing - the rock face's vertical streaks - and read as columns
 * again. The slope is laid with the map's own mountain top instead: the metatile most of its raised, unsloped
 * relief cells show (not the face tile). -1: none (the border's drawing, as before). WJPC_BELT_FACE_TOP=1: as before.
 */
static int BeltTopTile(const VoxelMapInstance *ring)
{
    static VOXEL_TLS const VoxelMapInstance *sRing[8];
    static VOXEL_TLS int sTile[8], sNext;
    static int sOff = -1;
    int counts[1024];
    int best = -1, bestN = 0;

    if (sOff < 0)
        sOff = getenv("WJPC_BELT_FACE_TOP") != NULL;
    if (sOff || ring == NULL)
        return -1;
    for (int i = 0; i < 8; ++i)
        if (sRing[i] == ring)
            return sTile[i];
    memset(counts, 0, sizeof(counts));
    for (int y = ring->originY; y < ring->originY + ring->height; ++y)
        for (int x = ring->originX; x < ring->originX + ring->width; ++x)
        {
            const int16_t *g = VoxelRelief_Cell(ring, x, y);
            int m;

            if (g == NULL || VoxelRelief_IsSlope(g) || g[0] < 16)
                continue;
            m = VoxelWorld_GetMetatileId(x, y);
            if (m >= 0 && m < 1024 && m != VoxelRelief_FaceTile(ring))
                counts[m]++;
        }
    for (int m = 0; m < 1024; ++m)
        if (counts[m] > bestN)
            bestN = counts[m], best = m;
    sRing[sNext] = ring;
    sTile[sNext] = best;
    sNext = (sNext + 1) % 8;
    return best;
}

static int sBeltSlopes = -1;

static bool BeltLand(int x, int y, float *top, float *shift)
{
    float level, sh;

    if (!BorderAt(x, y) || BorderWaterLevel(x, y, &level, &sh) || BeltIsTree(x, y))
        return false;
    if (!VoxelMesh_BorderLevel(x, y, &level, &sh))
        return false;
    *top = level + (float)VOXEL_BORDER_COURSES + BeltBase(x, y);
    *shift = sh + BeltBase(x, y);
    return true;
}

/* The corner (cx, cy) - a cell's top-left is (x, y) - over `base`: FALSE when no land cell is round it. */
static bool BeltCorner(int cx, int cy, float base, float *top, float *shift)
{
    float sumT = 0.0f, sumS = 0.0f, t, s;
    int n = 0;

    int land = 0;

    for (int dy = -1; dy <= 0; ++dy)
        for (int dx = -1; dx <= 0; ++dx)
            if (BeltLand(cx + dx, cy + dy, &t, &s))
                sumT += t, sumS += s, ++n, ++land;
            /* POL-40: water of the belt round the corner pulls it down to its surface, so a slope meets the sea
             * at the water instead of a wall a level tall all along the shore (WJPC_BELT_COLUMNS=1: not) */
            else if (BorderAt(cx + dx, cy + dy) && getenv("WJPC_BELT_COLUMNS") == NULL
                  && BorderWaterLevel(cx + dx, cy + dy, &t, &s))
                sumT += t + BeltBase(cx + dx, cy + dy), sumS += s + BeltBase(cx + dx, cy + dy), ++n;
    if (land == 0)
        return false;
    *top = sumT / n - base;
    *shift = sumS / n - base;
    return true;
}

/* POL-40: a sloped land cell's walls - only down to water or trees beside it (a land neighbour shares its
 * edge), from its edge as the slope leaves it down to the neighbour's level. */
static void BeltSlopeSteps(VoxelBuilder *builder, int x, int y, float level, float shift)
{
    static const int kDir[4][2] = { {0, 1}, {1, 0}, {-1, 0}, {0, -1} };
    /* each side's two corners, (dx, dy) from the cell's top-left, in the order the face is wound */
    static const int kEdge[4][2][2] = { {{1, 1}, {0, 1}}, {{1, 0}, {1, 1}}, {{0, 1}, {0, 0}}, {{0, 0}, {1, 0}} };
    float u0, v0, u1, v1, other, otherShift, t[2], z[2], t0, s0;

    (void)level; (void)shift;
    if (!MetatileUV(builder, BeltStepArt(x, y), &u0, &v0, &u1, &v1))
        return;
    for (int d = 0; d < 4; ++d)
    {
        int nx = x + kDir[d][0], ny = y + kDir[d][1];
        bool ok = true;

        if (VoxelWorld_GetInstanceAt(nx, ny) != NULL || BeltLand(nx, ny, &t0, &s0))
            continue;
        if (BorderWaterLevel(nx, ny, &other, &otherShift))
            ;
        else
        {
            VoxelMesh_BorderLevel(nx, ny, &other, &otherShift);
            if (!BeltIsTree(nx, ny))
                other += (float)VOXEL_BORDER_COURSES;
        }
        other += BeltBase(nx, ny) - BeltBase(x, y);
        otherShift += BeltBase(nx, ny) - BeltBase(x, y);
        for (int k = 0; k < 2 && ok; ++k)
            ok = BeltCorner(x + kEdge[d][k][0], y + kEdge[d][k][1], BeltBase(x, y), &t[k], &z[k]);
        if (!ok || (t[0] <= other + 0.01f && t[1] <= other + 0.01f))
            continue;
        {
            /* a tile a level up the wall, as BeltStepFaces (one drawing pulled over a tall wall was streaks) */
            float xa = (float)(x + kEdge[d][0][0]), xb = (float)(x + kEdge[d][1][0]);
            float za = (float)(y + kEdge[d][0][1]), zb = (float)(y + kEdge[d][1][1]);
            float top = fmaxf(t[0], t[1]);
            uint8_t shade = d == 0 ? SHADE_SOUTH : d == 1 ? SHADE_EAST : d == 2 ? SHADE_WEST : SHADE_NORTH;

            for (float band = other; band < top - 0.01f; band += 1.0f)
            {
                float ha0 = fminf(band, t[0]), ha1 = fminf(band + 1.0f, t[0]);
                float hb0 = fminf(band, t[1]), hb1 = fminf(band + 1.0f, t[1]);
                /* along the wall, z goes from the neighbour's row at its foot to this cell's at its top */
                float fa0 = t[0] > other ? (ha0 - other) / (t[0] - other) : 1.0f;
                float fa1 = t[0] > other ? (ha1 - other) / (t[0] - other) : 1.0f;
                float fb0 = t[1] > other ? (hb0 - other) / (t[1] - other) : 1.0f;
                float fb1 = t[1] > other ? (hb1 - other) / (t[1] - other) : 1.0f;

                if (ha1 <= ha0 && hb1 <= hb0)
                    continue;
                VoxelBuilder_Quad(builder,
                    &(VoxelVertex){xa, ha1, za + otherShift + (z[0] - otherShift) * fa1, u0, v1 + (v0 - v1) * (ha1 - band), shade},
                    &(VoxelVertex){xb, hb1, zb + otherShift + (z[1] - otherShift) * fb1, u1, v1 + (v0 - v1) * (hb1 - band), shade},
                    &(VoxelVertex){xb, hb0, zb + otherShift + (z[1] - otherShift) * fb0, u1, v1 + (v0 - v1) * (hb0 - band), shade},
                    &(VoxelVertex){xa, ha0, za + otherShift + (z[0] - otherShift) * fa0, u0, v1 + (v0 - v1) * (ha0 - band), shade});
            }
        }
    }
}

/*
 * POL-41 follow-up (Brennan 2026-11-16 17:53, Route 119, picture: "where the mountain edge and trees start there's
 * a black line with blue showing ... this needs to be meshed together"): a wood of the belt beside a map stands at
 * that map's edge level, but the map's edge cell is drawn at its own heights along the edge (a cliff, a slope),
 * and nothing filled the gap between the two: the sky showed through under the first column of crowns. The wood's
 * ground (laid beside a map too, BeltLand's caller) is joined to the map's edge point by point (SeamEdge) by a
 * skirt of the cell's own drawing, as ModelFlatSkirts joins a flat model cell.
 */
static void BeltTreeSeam(VoxelBuilder *b, int x, int y, float level, float shift,
                         float u0, float v0, float u1, float v1)
{
    static const int kDir[4][2] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
    const unsigned n = VOXEL_RELIEF_SIDE;
    const float last = (float)(VOXEL_RELIEF_SIDE - 1);
    const float here = BeltBase(x, y);

    for (int d = 0; d < 4; ++d)
    {
        int nx = x + kDir[d][0], ny = y + kDir[d][1];
        const VoxelMapInstance *m = VoxelWorld_GetInstanceAt(nx, ny);
        float theirH[VOXEL_RELIEF_SIDE], theirZ[VOXEL_RELIEF_SIDE], rebase;

        if (m == NULL || m->indoor || VoxelRelief_Cell(m, nx, ny) == NULL)
            continue;
        /* the map cell's edge facing this cell: north 0, south 1, west 2, east 3 */
        SeamEdge(m, nx, ny, d ^ 1, theirH, theirZ, NULL);
        rebase = VoxelRelief_Base(m) - here;
        for (unsigned k = 0; k + 1 < n; ++k)
        {
            float ka = k / last, kb = (k + 1) / last;
            float ha = theirH[k] / 16.0f + rebase, hb = theirH[k + 1] / 16.0f + rebase;
            float xa, xb, za, zb, zA, zB;

            if (fabsf(ha - level) < 0.01f && fabsf(hb - level) < 0.01f
             && fabsf(theirZ[k] + rebase - shift) < 0.01f && fabsf(theirZ[k + 1] + rebase - shift) < 0.01f)
                continue;
            if (d < 2)
            {
                float e = d == 0 ? 0.0f : 1.0f;
                xa = x + ka; xb = x + kb;
                za = zb = y + e + shift;
                zA = y + e + theirZ[k] + rebase; zB = y + e + theirZ[k + 1] + rebase;
            }
            else
            {
                float e = d == 2 ? 0.0f : 1.0f;
                xa = xb = x + e;
                za = y + ka + shift; zb = y + kb + shift;
                zA = y + ka + theirZ[k] + rebase; zB = y + kb + theirZ[k + 1] + rebase;
            }
            VoxelBuilder_Quad(b,
                &(VoxelVertex){xa, level, za, u0, v0, SHADE_TOP},
                &(VoxelVertex){xb, level, zb, u1, v0, SHADE_TOP},
                &(VoxelVertex){xb, hb, zB, u1, v1, SHADE_TOP},
                &(VoxelVertex){xa, ha, zA, u0, v1, SHADE_TOP});
        }
    }
}

/*
 * POL-46 (Brennan 2026-11-17 19:42, the 3D brief: "Every elevated terrain mass must be a watertight, closed 3D
 * manifold ... Never leave open vertical seams between two adjacent elevation levels"; first: "the Lavaridge Town
 * to Route 112/Mt. Chimney northern mountain seam"). A cell of the belt's top, in the world: its height and how far
 * south it is drawn, as VoxelMesh_EmitBorder stands it. FALSE for no standing belt cell (a map's, water, trees).
 */
/* POL-46 terraces round a belt laid as its drawing (Lavaridge's): a level up every BELT_TIER_DEPTH cells out from
 * its map, so its rock climbs away from the town in rings of ledges (north of it, ledges along the rows, their walls
 * facing the town). WJPC_BELT_SMOOTH=1: flat as before. */
/* WJPC_TERRACE_FLAT_ROWS=1: a drawn belt's ledges not drawn further south as they rise (as the 21:00 build) */
static bool TerraceSouthOff(void)
{
    static int sOff = -1;
    if (sOff < 0)
        sOff = getenv("WJPC_TERRACE_FLAT_ROWS") != NULL;
    return sOff != 0;
}
static float DrawnTierLift(int x, int y)
{
    static int sFlat = -1;
    const VoxelMapInstance *ring = VoxelWorld_BeltRing(x, y);
    int gx, gy, d;

    if (sFlat < 0)
        sFlat = getenv("WJPC_BELT_SMOOTH") != NULL;
    if (sFlat || ring == NULL || VoxelRelief_Belt(ring) == 0)
        return 0.0f;
    gx = x < ring->originX ? ring->originX - x : x >= ring->originX + ring->width ? x - (ring->originX + ring->width - 1) : 0;
    gy = y < ring->originY ? ring->originY - y : y >= ring->originY + ring->height ? y - (ring->originY + ring->height - 1) : 0;
    d = gx > gy ? gx : gy;
    /* not south of it: there the belt is in front of the town, under the view */
    if (y >= ring->originY + ring->height || d < 1)
        return 0.0f;
    return (float)((d - 1) / BELT_TIER_DEPTH);
}

static bool BeltTopWorld(int x, int y, float *top, float *z, unsigned *drawnOut)
{
    float level, shift, edge = 0.0f, edgeShift = 0.0f;
    const VoxelMapInstance *own;
    unsigned drawn;

    if (!BorderAt(x, y) || VoxelWorld_GetInstanceAt(x, y) != NULL || BeltIsTree(x, y)
     || BorderWaterLevel(x, y, &level, &shift))
        return false;
    own = VoxelWorld_BeltRing(x, y);
    drawn = own != NULL ? VoxelRelief_Belt(own) : 0u;
    VoxelMesh_BorderLevel(x, y, &edge, &edgeShift);
    *top = BeltBase(x, y) + (float)(drawn ? drawn : VOXEL_BORDER_COURSES) + edge + (drawn ? DrawnTierLift(x, y) : 0.0f);
    *z = (float)y + (float)drawn + edgeShift + (drawn && !TerraceSouthOff() ? DrawnTierLift(x, y) : 0.0f);
    *drawnOut = drawn;
    return true;
}

/*
 * Between a cell of a belt laid as the drawing (Lavaridge's) and one stood as the map's edge (Route 112's, its
 * mountain) nothing closed the step: the two tops stood at their own heights and the sky showed down the seam in a
 * stair of blue (BREN_1841_3/4, "lavaridge background holes"). The higher cell stands a wall of its own drawing
 * down to the lower one's top, on each side where the other kind is lower. WJPC_BELT_OPEN_SEAMS=1: as before.
 */
static void BeltCloseSeams(VoxelBuilder *builder, int x, int y, unsigned drawn, float wz)
{
    static int sOpen = -1;
    static const int kDir[4][2] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
    float myTop, myZ, base = builder->base;
    unsigned myDrawn;

    if (sOpen < 0)
        sOpen = getenv("WJPC_BELT_OPEN_SEAMS") != NULL;
    static int sTiersClose = -1;
    if (sTiersClose < 0)
        sTiersClose = getenv("WJPC_BELT_TIERS_ALL") != NULL;
    if (sOpen || !BeltTopWorld(x, y, &myTop, &myZ, &myDrawn))
        return;
    for (int d = 0; d < 4; ++d)
    {
        int nx = x + kDir[d][0], ny = y + kDir[d][1];
        float top, z, u0, v0, u1, v1;
        unsigned nDrawn;

        /* (terraces, WJPC_BELT_TIERS: every step of the belt, each side - a terrace is closed all round) */
        if (!BeltTopWorld(nx, ny, &top, &z, &nDrawn) || ((nDrawn != 0) == (drawn != 0) && !sTiersClose && !TerraceBand(x, y))
         || top > myTop - 0.05f)
            continue;
        /* a level of wall a tile of the drawing, from their top up to ours */
        for (float lo = top; lo < myTop - 0.05f; lo += 1.0f)
        {
            float hi = lo + 1.0f < myTop ? lo + 1.0f : myTop;
            float yb = lo - base, yt = hi - base;

            /* the ring's cliff face, a tile a level (Brennan 21:00: "a vertical rock wall of fixed tile height ...
             * with tiled rock UVs"), its own drawing where the map has no face */
            int face = VoxelRelief_FaceTile(VoxelWorld_BeltRing(x, y));
            /* beside a drawn belt, its walls as the drawn belt's (one mountain, one rock): the cliff face of the
             * map of the nearest drawn cell along the row */
            for (int k = 1; !drawn && k <= 24; ++k)
            {
                const VoxelMapInstance *r;
                int cx = x - k;
                bool found = (r = VoxelWorld_BeltRing(cx, y)) != NULL && VoxelRelief_Belt(r) != 0 && BorderAt(cx, y);

                if (!found)
                {
                    cx = x + k;
                    found = (r = VoxelWorld_BeltRing(cx, y)) != NULL && VoxelRelief_Belt(r) != 0 && BorderAt(cx, y);
                }
                if (found)
                {
                    face = VoxelRelief_FaceTile(r);
                    break;
                }
            }
            if (!(face >= 0 && MetatileUV(builder, face, &u0, &v0, &u1, &v1))
             && !MetatileUV(builder, BeltMetatileFor(x, y, y), &u0, &v0, &u1, &v1))
                break;
            v0 = v1 - (v1 - v0) * (hi - lo);
            if (d == 0) VoxelMesh_North(builder, (float)x, wz, yb, yt, 0.0f, u0, v0, u1, v1);
            if (d == 1) VoxelMesh_South(builder, (float)x, wz, yb, yt, 0.0f, u0, v0, u1, v1);
            if (d == 2) VoxelMesh_West(builder, (float)x, wz, yb, yt, 0.0f, u0, v0, u1, v1);
            if (d == 3) VoxelMesh_East(builder, (float)x, wz, yb, yt, 0.0f, u0, v0, u1, v1);
        }
    }
}

/*
 * POL-46, Brennan 2026-11-18 13:33 - the belt's land as ONE terraced heightfield ("Treat the mountain as a unified
 * 2D heightmap grid first, then mesh it cleanly"; "Weld/share vertices between adjacent columns (X and Z
 * neighbors) so neighboring cliff columns form a single continuous hillside surface"; "Stop extruding
 * axis-aligned square boxes (90 degree corners) per tile ... chamfer/bevel diagonal corners (45 degree corner
 * slopes) so the terrace ledges curve and flow naturally around the mountain corner"; earlier, 21:00: "each tier
 * must have a vertical rock wall of fixed tile height ... followed by a flat horizontal top ledge" with the rock
 * tiled by world height, never stretched).
 *
 * BeltField: one height (world tiles) and one way south for every CORNER of the grid - a pure function of the
 * corner, so a corner shared by four cells, by two chunks or by the belt and a map's edge is the same point
 * (watertight by construction, nothing to weld afterwards). Every loaded map gives the height of its nearest edge
 * point (its relief lattice there) and, off its north, west and east sides, climbs a level every
 * BELT_FIELD_RATE cells (to BELT_FIELD_RISE levels: the mountain behind rises away from the route); off its
 * south side (in front, under the view) it stays at the edge's height. The highest map wins, so two maps'
 * mountains join into one massif. Its way south: the nearest edge point's (the map's own drawing: where the belt
 * meets the map it meets it exactly), blended with distance.
 *
 * Each cell is two triangles of that field, terraced: the part between level k and k+1 lies flat at k; where the
 * field crosses a whole level a wall a level high stands along the crossing (straight, or across the cell at 45
 * degrees where the levels turn a corner - the chamfer). WJPC_BELT_BLOCKS_OLD=1: the cell-by-cell belt as before.
 */
enum { BELT_FIELD_REACH = 16, BELT_FIELD_RATE = 2, BELT_FIELD_RISE = 6, BELT_FIELD_SOUTH = 3, BELT_FIELD_PEAK = 14, BELT_PEAK_RATE = 3 };

static bool FieldOn(void)
{
    static int sOn = -1;
    if (sOn < 0)
        sOn = getenv("WJPC_BELT_BLOCKS_OLD") == NULL;
    return sOn != 0;
}

/* smooth value noise, -2.5..+2.5 levels: two octaves on 6- and 3-cell lattices of hashed values */
static float FieldLumpAt(int gx, int gy)
{
    unsigned h = (unsigned)gx * 374761393u + (unsigned)gy * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return (float)((h ^ (h >> 16)) & 0xFFFF) / 65535.0f * 2.0f - 1.0f;
}

static float FieldLumpOctave(int vx, int vy, int cell)
{
    int gx = vx >= 0 ? vx / cell : -((-vx + cell - 1) / cell), gy = vy >= 0 ? vy / cell : -((-vy + cell - 1) / cell);
    float tx = (float)(vx - gx * cell) / (float)cell, ty = (float)(vy - gy * cell) / (float)cell;
    float a = FieldLumpAt(gx, gy), b = FieldLumpAt(gx + 1, gy), c = FieldLumpAt(gx, gy + 1), d = FieldLumpAt(gx + 1, gy + 1);
    tx = tx * tx * (3.0f - 2.0f * tx);
    ty = ty * ty * (3.0f - 2.0f * ty);
    return (a + (b - a) * tx) + ((c + (d - c) * tx) - (a + (b - a) * tx)) * ty;
}

static float FieldLumps(int vx, int vy)
{
    return 1.7f * FieldLumpOctave(vx, vy, 6) + 0.8f * FieldLumpOctave(vx + 101, vy + 57, 3);
}

static bool BorderWaterLevel(int x, int y, float *level, float *shift);
/* TRUE when a grid corner touches a wood cell of the belt and no belt water lies within 12 cells (see BeltField) */
static bool FieldCornerWood(int vx, int vy)
{
    bool wood = false;
    float wl, ws;

    for (int k = 0; k < 4 && !wood; ++k)
    {
        int cx = vx - 1 + (k & 1), cy = vy - 1 + (k >> 1);
        wood = VoxelWorld_GetInstanceAt(cx, cy) == NULL && BorderAt(cx, cy) && BeltIsTree(cx, cy);
    }
    if (!wood)
        return false;
    for (int dy = -12; dy <= 12; dy += 2)
        for (int dx = -12; dx <= 12; dx += 2)
            if (VoxelWorld_GetInstanceAt(vx + dx, vy + dy) == NULL && BorderAt(vx + dx, vy + dy)
             && BorderWaterLevel(vx + dx, vy + dy, &wl, &ws))
                return false;
    return true;
}

/* (Brennan 2026-11-20 20:15, belt stalls: BeltField walks every placed map for a point, and the belt asks it for the
 * same points many times - kept per point until the maps change, VoxelWorld_CellGen) */
/* a lattice point on map m's border (world, whole cells): its height and its way south, as BeltFieldUncached reads
 * them (the higher of the two cells meeting there) */
static bool EdgePointHeight(const VoxelMapInstance *m, int px, int py, float *h, float *s)
{
    int cx = px - (px >= m->originX + m->width ? 1 : 0), cy = py - (py >= m->originY + m->height ? 1 : 0);
    int gi = px - cx == 0 ? 0 : VOXEL_RELIEF_SIDE - 1, gj = py - cy == 0 ? 0 : VOXEL_RELIEF_SIDE - 1;
    const int16_t *g = VoxelRelief_Cell(m, cx, cy), *dg = VoxelRelief_Depth(m, cx, cy);
    float e;

    if (cx < m->originX || cy < m->originY || cx >= m->originX + m->width || cy >= m->originY + m->height)
        return false;
    e = VoxelRelief_Base(m) + (g != NULL ? g[gj * VOXEL_RELIEF_SIDE + gi] / 16.0f : 0.0f);
    if (gi == 0 && cx > m->originX)
    {
        const int16_t *g2 = VoxelRelief_Cell(m, cx - 1, cy);
        float e2 = VoxelRelief_Base(m) + (g2 != NULL ? g2[gj * VOXEL_RELIEF_SIDE + VOXEL_RELIEF_SIDE - 1] / 16.0f : 0.0f);
        e = e2 > e ? e2 : e;
    }
    if (gj == 0 && cy > m->originY)
    {
        const int16_t *g2 = VoxelRelief_Cell(m, cx, cy - 1);
        float e2 = VoxelRelief_Base(m) + (g2 != NULL ? g2[(VOXEL_RELIEF_SIDE - 1) * VOXEL_RELIEF_SIDE + gi] / 16.0f : 0.0f);
        e = e2 > e ? e2 : e;
    }
    *h = e;
    *s = VoxelRelief_Base(m) + (dg != NULL ? dg[gj * VOXEL_RELIEF_SIDE + gi] / 16.0f
                               : g != NULL ? g[gj * VOXEL_RELIEF_SIDE + gi] / 16.0f : 0.0f);
    return true;
}

enum { BELT_WOOD_EDGE_SPAN = 3 };
/* TRUE when the four cells round a grid point are all woods of the belt (no map, no water, no open field) */
static bool FieldPointDeepWood(int vx, int vy)
{
    for (int k = 0; k < 4; ++k)
    {
        int cx = vx - 1 + (k & 1), cy = vy - 1 + (k >> 1);
        if (VoxelWorld_GetInstanceAt(cx, cy) != NULL || !BorderAt(cx, cy) || !BeltIsTree(cx, cy))
            return false;
    }
    return true;
}

/* TRUE for a grid point on a map's edge whose cells off the map are all woods of the belt (two of them at least) */
static bool FieldPointEdgeWood(int vx, int vy)
{
    int woods = 0;
    for (int k = 0; k < 4; ++k)
    {
        int cx = vx - 1 + (k & 1), cy = vy - 1 + (k >> 1);
        if (VoxelWorld_GetInstanceAt(cx, cy) != NULL)
            continue;
        if (!BorderAt(cx, cy) || !BeltIsTree(cx, cy))
            return false;
        ++woods;
    }
    return woods >= 2;
}

static void BeltFieldUncached(int vx, int vy, float *height, float *south);
static void BeltField(int vx, int vy, float *height, float *south)
{
    enum { N = 32768 };
    typedef struct { int x, y; unsigned gen; float h, s; } FieldMemo;
    VOXEL_TLS_TABLE(FieldMemo, sMemo, N);
    unsigned gen = VoxelWorld_CellGen(), k = ((unsigned)vx * 73856093u) ^ ((unsigned)vy * 19349663u);

    if (sMemo == NULL)
    {
        BeltFieldUncached(vx, vy, height, south);
        return;
    }

    k = (k ^ (k >> 15)) & (N - 1);
    if (gen == 0 || sMemo[k].gen != gen || sMemo[k].x != vx || sMemo[k].y != vy)
    {
        BeltFieldUncached(vx, vy, &sMemo[k].h, &sMemo[k].s);
        sMemo[k].x = vx;
        sMemo[k].y = vy;
        sMemo[k].gen = gen;
    }
    *height = sMemo[k].h;
    *south = sMemo[k].s;
}
/* Brennan 2026-11-22 21:56 (Route 114's west: "diagonal stretched cliff ribbons ... extend the snowy mountain plateau
 * ... smoothly outward to the west at matching elevations"): the field is a blend of the maps' edges, so a fill set
 * level (flags flat) lay under it only where it stood trees. A field point whose four cells are all of one flat fill
 * (within FLAT_FILL_ROWS of the map, never on the map's edge) stands at the fill's top: the plateau runs on level.
 * WJPC_FLAT_FILL_FIELD_OFF=1: the blend, as before. */
static bool FlatFillPoint(int vx, int vy, float *height)
{
    static int sOff = -1;
    float sum = 0.0f;

    if (sOff < 0)
        sOff = getenv("WJPC_FLAT_FILL_FIELD_OFF") != NULL;
    if (sOff)
        return false;
    for (int k = 0; k < 4; ++k)
    {
        int cx = vx - 1 + (k & 1), cy = vy - 1 + (k >> 1), ex, ey, d;
        const VoxelMapInstance *ring;
        VoxelEdgeFill fill;

        if (VoxelWorld_GetInstanceAt(cx, cy) != NULL || (ring = VoxelWorld_BeltRing(cx, cy)) == NULL
         || !VoxelRelief_EdgeFillAt(ring, cx, cy, &fill) || !fill.flat || !fill.hasTop || fill.water)
            return false;
        ex = cx < ring->originX ? ring->originX - cx : cx >= ring->originX + ring->width ? cx - (ring->originX + ring->width - 1) : 0;
        ey = cy < ring->originY ? ring->originY - cy : cy >= ring->originY + ring->height ? cy - (ring->originY + ring->height - 1) : 0;
        d = ex > ey ? ex : ey;
        if (d > 24)
            return false;
        sum += VoxelRelief_Base(ring) + (float)fill.top / 16.0f - (float)VOXEL_BORDER_COURSES;
    }
    *height = sum / 4.0f;
    return true;
}

/* A belt cell of a flat fill near its map (FlatFillPoint's): its own top, world height. */
static bool FlatFillCell(int cx, int cy, float *height)
{
    const VoxelMapInstance *ring;
    VoxelEdgeFill fill;
    int ex, ey;

    if (VoxelWorld_GetInstanceAt(cx, cy) != NULL || (ring = VoxelWorld_BeltRing(cx, cy)) == NULL
     || !VoxelRelief_EdgeFillAt(ring, cx, cy, &fill) || !fill.flat || !fill.hasTop || fill.water)
        return false;
    ex = cx < ring->originX ? ring->originX - cx : cx >= ring->originX + ring->width ? cx - (ring->originX + ring->width - 1) : 0;
    ey = cy < ring->originY ? ring->originY - cy : cy >= ring->originY + ring->height ? cy - (ring->originY + ring->height - 1) : 0;
    if ((ex > ey ? ex : ey) > 24)
        return false;
    *height = VoxelRelief_Base(ring) + (float)fill.top / 16.0f - (float)VOXEL_BORDER_COURSES;
    return true;
}

/* Brennan 2026-11-23 12:18 (Route 114's west: "The slope mountain doesn't look natural it looks like it's falling
 * this needs to be horizontal"): where two flat fills of different heights meet, each cell keeps its own fill's top,
 * standing where a cell of that height stands (its way south its height, as the 2D map's rows), a wall of rock down
 * to the lower one - the field's one-cell ramp between them stood as a band of squeezed ledges. A lower cell with a
 * higher one south of it reaches under that one's edge (no hole behind the step). WJPC_FLAT_STEP_RAMP=1: the ramp,
 * as before. */
static bool FlatStepsOn(void)
{
    static int sOn = -1;
    if (sOn < 0)
        sOn = getenv("WJPC_FLAT_STEP_RAMP") == NULL;
    return sOn != 0;
}

static void BeltFieldUncached(int vx, int vy, float *height, float *south)
{
    if (FlatFillPoint(vx, vy, height))
    {
        *south = *height;
        return;
    }
    {
    /* blended over the maps by 1/d^3 (the nearest map's edge wins near it, exactly on it): the edge's height, its way
     * south, and whether the point is off a side the mountain climbs from (north, west, east) or in front (south);
     * then the climb, by the distance to the nearest map. One smooth function: no map's cone outgrowing another's
     * with distance, no cliff where a map drops out of reach (both made walls of columns, 2026-11-18). */
    float sumW = 0.0f, sumE = 0.0f, sumS = 0.0f, sumBack = 0.0f, dmin = 1e9f;
    float exactE = 0.0f, exactS = 0.0f, capMin = 1e9f, sumRise = 0.0f, nearE = 0.0f;
    bool nearSouth = false, nearDrawnSouth = false, nearDrawn = false;
    float capOther = 1e9f;
    float drawnPeak = -1e9f, lumpAdd = 0.0f;
    bool exact = false;

    for (unsigned i = 0; i < VoxelWorld_InstanceCount(); ++i)
    {
        const VoxelMapInstance *m = VoxelWorld_Instance(i);
        int ox, oy, cx, cy, gi, gj;
        float qx, qy, dx, dy, d, e, sd, w;
        const int16_t *g, *dg;

        if (m == NULL || m->layout == NULL || m->indoor)
            continue;
        ox = m->originX;
        oy = m->originY;
        if (vx > ox && vx < ox + m->width && vy > oy && vy < oy + m->height)
            continue;
        qx = vx < ox ? (float)ox : vx > ox + m->width ? (float)(ox + m->width) : (float)vx;
        qy = vy < oy ? (float)oy : vy > oy + m->height ? (float)(oy + m->height) : (float)vy;
        dx = (float)vx - qx;
        dy = (float)vy - qy;
        d = sqrtf(dx * dx + dy * dy);
        cx = (int)qx - (qx >= (float)(ox + m->width) ? 1 : 0);
        cy = (int)qy - (qy >= (float)(oy + m->height) ? 1 : 0);
        gi = (int)((qx - (float)cx) * (VOXEL_RELIEF_SIDE - 1) + 0.5f);
        gj = (int)((qy - (float)cy) * (VOXEL_RELIEF_SIDE - 1) + 0.5f);
        g = VoxelRelief_Cell(m, cx, cy);
        dg = VoxelRelief_Depth(m, cx, cy);
        e = VoxelRelief_Base(m) + (g != NULL ? g[gj * VOXEL_RELIEF_SIDE + gi] / 16.0f : 0.0f);
        /* POL-56 (Brennan 2026-11-19 14:23, Route 120 south-west: a sky triangle at the map's south edge): a point
         * on the edge between two of the map's cells is either cell's corner; a step between them (a cut tile beside
         * the grass) put the field at the lower one and left the higher cell's corner over open sky. The higher of
         * the two. WJPC_FIELD_ONE_CELL=1: the one cell, as before. */
        if (getenv("WJPC_FIELD_ONE_CELL") == NULL)
        {
            int ax = gi == 0 && cx > ox ? cx - 1 : -1, ay = gj == 0 && cy > oy ? cy - 1 : -1;
            if (ax >= 0)
            {
                const int16_t *g2 = VoxelRelief_Cell(m, ax, cy);
                float e2 = VoxelRelief_Base(m) + (g2 != NULL ? g2[gj * VOXEL_RELIEF_SIDE + VOXEL_RELIEF_SIDE - 1] / 16.0f : 0.0f);
                e = e2 > e ? e2 : e;
            }
            if (ay >= 0)
            {
                const int16_t *g2 = VoxelRelief_Cell(m, cx, ay);
                float e2 = VoxelRelief_Base(m) + (g2 != NULL ? g2[(VOXEL_RELIEF_SIDE - 1) * VOXEL_RELIEF_SIDE + gi] / 16.0f : 0.0f);
                e = e2 > e ? e2 : e;
            }
        }
        sd = VoxelRelief_Base(m) + (dg != NULL ? dg[gj * VOXEL_RELIEF_SIDE + gi] / 16.0f
                                    : g != NULL ? g[gj * VOXEL_RELIEF_SIDE + gi] / 16.0f : 0.0f);
        /* Brennan 2026-11-21 23:06 (Route 117's north-west woods, behind the small rock mound at the map's north
         * edge: "it should just be trees no green tiles on the tops of the tree"): the field took the edge's height
         * straight across, so the mound's profile (1, 2, 2, 1 over four cells) ran north under the whole wood as a
         * ridge, its steps' grass showing between the crowns. Under a deep wood (the four cells round the point all
         * woods of the belt; on the edge itself, its cells off the map all woods) the field takes the LOWEST edge point within BELT_WOOD_EDGE_SPAN cells along that edge
         * (and its way south): a bump narrower than that ends at the map, a wider rise still carries on.
         * WJPC_WOOD_FIELD_EDGE_OLD=1: as before. */
        /* (north of the map only - the woods there are behind it, the ridge ran under them; beside its west edge
         * the lowered field opened a speck of sky between the crowns: Verdanturf, 2026-11-22) */
        if (getenv("WJPC_WOOD_FIELD_EDGE_OLD") == NULL && vy <= oy
         && (d >= 1.0f ? FieldPointDeepWood(vx, vy) : FieldPointEdgeWood(vx, vy)))
        {
            bool alongX = qy == (float)oy || qy == (float)(oy + m->height);
            float keepE = e, keepS = sd;

            for (int k = -BELT_WOOD_EDGE_SPAN; k <= BELT_WOOD_EDGE_SPAN; ++k)
            {
                int px = alongX ? (int)qx + k : (int)qx, py = alongX ? (int)qy : (int)qy + k;
                float pe, ps;

                if (k == 0 || px < ox || px > ox + m->width || py < oy || py > oy + m->height)
                    continue;
                if (EdgePointHeight(m, px, py, &pe, &ps) && pe < e)
                {
                    e = pe;
                    sd = ps;
                }
            }
            /* (on the edge itself only a bump of a whole level or more: there the map's rock face closes the step
             * down to the wood - EdgeOverBelt; a gentler rise stays met point by point) */
            if (d < 1.0f && keepE - e < 1.0f)
            {
                e = keepE;
                sd = keepS;
            }
        }
        /* Brennan 2026-11-21 17:25 (image 4, Route 112's south border: "stretched walls of rock and trees are
         * shooting straight up into the sky - level/clamp them flush with the surrounding terrain"): in front of a
         * map the land took its edge point by point - Route 112's south rows are rock ledges three levels over its
         * woods, and the woods reach the edge between them, so the land in front jumped with them (walls facing the
         * camera). In front of a map (not a drawn belt's; not under a wood - levelled, a step ran through Route
         * 120's) its edge counts as the LOWEST edge point within BELT_SOUTH_SPAN cells along it; the ledges end in
         * their own rock face (EdgeOverBelt). Fed into the blend with the other maps (Brennan 2026-11-22 01:12
         * "smooth those ones too, they are too dark": set outright, the land jumped where the nearest map changed -
         * a dark wall line toward the camera by Route 111). WJPC_FIELD_SOUTH_EDGE=1: as before. */
        {
            static int sSouthEdge = -1;
            if (sSouthEdge < 0)
                sSouthEdge = getenv("WJPC_FIELD_SOUTH_EDGE") != NULL;
            if (!sSouthEdge && d >= 1.0f && vy > oy + m->height && vx >= ox && vx <= ox + m->width
             && VoxelRelief_Belt(m) == 0 && !FieldCornerWood(vx, vy))
            {
                enum { BELT_SOUTH_SPAN = 8 };
                for (int k = -BELT_SOUTH_SPAN; k <= BELT_SOUTH_SPAN; ++k)
                {
                    float pe, ps;
                    int px = (int)qx + k;
                    if (k != 0 && px >= ox && px <= ox + m->width
                     && EdgePointHeight(m, px, oy + m->height, &pe, &ps) && pe < e)
                    {
                        e = pe;
                        sd = ps;
                    }
                }
            }
        }
        /* POL-46 (Brennan 2026-11-18, Lavaridge's north-east: "Option 2 (One cliff + ledges)"): no point climbs
         * faster than a level every BELT_FIELD_RATE cells from ANY map's edge - from the low town the ledges stay
         * two squares deep; where a high edge (Route 112's) stands over that, its edge's own cliff (EdgeOverBelt,
         * rock tiled a level a tile) makes up the rest. WJPC_FIELD_NO_CAP=1: off. */
        {
            float cap = e + d / (VoxelRelief_Belt(m) != 0 && vy < oy && vx >= ox - 6 && vx <= ox + m->width + 6
                                 && getenv("WJPC_PEAK_RATE2") == NULL ? (float)BELT_PEAK_RATE : (float)BELT_FIELD_RATE);
            if (VoxelRelief_Belt(m) != 0)
                capMin = cap < capMin ? cap : capMin;
            else
                capOther = cap < capOther ? cap : capOther;
        }
        if (d < 1e-4f)
        {
            if (!exact || e > exactE)
            {
                exactE = e;
                exactS = sd;
            }
            exact = true;
            continue;
        }
        /* 1/d^2 (Brennan 19:18: Lavaridge's north-east "turns into a tall, flat vertical rock wall", Route 118's
         * mountain's west side "shoots straight up"): 1/d^3 handed one map's edge height to the next's in a few
         * cells - a tall stack of ledges; 1/d^2 spreads the change wider, the ledges deeper. WJPC_FIELD_W3=1: old */
        w = getenv("WJPC_FIELD_W3") != NULL ? 1.0f / (d * d * d) : 1.0f / (d * d);
        sumW += w;
        sumE += w * e;
        sumS += w * sd;
        sumBack += w * (vy >= oy + m->height ? 0.0f : 1.0f);
        /* POL-46 (Brennan 22:39, Lavaridge north: "Mirror/extend the stepped rocky mountain peak from the east side
         * across to the west side"): behind a map whose belt is its drawing (Lavaridge's rock) the mountain goes on
         * climbing, as Route 112's does east of it, to BELT_FIELD_PEAK levels; elsewhere BELT_FIELD_RISE (Route 118,
         * locked in 22:39, unchanged). Blended by the same weights, so no seam where one map's pull gives way. */
        sumRise += w * (float)(VoxelRelief_Belt(m) != 0 && getenv("WJPC_FIELD_NO_PEAK") == NULL ? BELT_FIELD_PEAK
                                                                                            : BELT_FIELD_RISE);
        /* the peak behind a drawn belt's map stands on its own, not halved by the blend with the map across the
         * belt from it (the bare benches north of Lavaridge, 22:39 picture: Route 111's south edge pulled it down) */
        if (VoxelRelief_Belt(m) != 0 && getenv("WJPC_FIELD_NO_PEAK") == NULL && !(vy >= oy + m->height))
        {
            /* (north of the map, a level every BELT_PEAK_RATE cells: drawn as far south as it is high, a level every 2
             * left one-row shelves - a grid of short walls - where Route 112's peak beside it has deeper ones) */
            float rate = vy < oy && vx >= ox - 6 && vx <= ox + m->width + 6 && getenv("WJPC_PEAK_RATE2") == NULL
                       ? (float)BELT_PEAK_RATE : (float)BELT_FIELD_RATE;
            float r = d / rate, pk = e + (r < (float)BELT_FIELD_PEAK ? r : (float)BELT_FIELD_PEAK);
            /* Audit batch 2 (Route 113 / Fallarbor: one flat wall of rock 15 levels tall north of the maps; Route
             * 111's east side): the peak held its full height at ANY distance, so Route 111's (6 + 14 = 20) stood
             * over the whole north of the world, a cell from Route 113's edge at 5. Past its own climb (BELT_FIELD_PEAK
             * levels at its rate) it comes down again at the same rate - a ridge, not a plateau; Lavaridge's peak
             * (within that reach) is as it was. WJPC_PEAK_NO_REACH=1: as before. */
            if (getenv("WJPC_PEAK_NO_REACH") == NULL && d > (float)BELT_FIELD_PEAK * rate)
                pk -= (d - (float)BELT_FIELD_PEAK * rate) / rate;
            /* POL-46 (Brennan 23:49: the north-west "looks too uniform and blocky - like straight rows of identical
             * stair bricks"; wants "irregular rocky terrain relief, varied ledge step widths, and broken-up cliff
             * outcrops" as the north-east peak): north of the map a smooth lumpy relief (FieldLumps), faded in
             * over 4 rows from the edge and over 6 columns past its sides (the far-west bowl stays as it is) */
            if (vy < oy && getenv("WJPC_FIELD_LUMPS") != NULL)
            {
                float fy = (float)(oy - vy) / 4.0f, fx = 1.0f;
                if (vx < ox) fx = 1.0f - (float)(ox - vx) / 6.0f;
                if (vx > ox + m->width) fx = 1.0f - (float)(vx - ox - m->width) / 6.0f;
                fy = fy > 1.0f ? 1.0f : fy;
                fx = fx < 0.0f ? 0.0f : fx;
                lumpAdd = fx * fy * FieldLumps(vx, vy);
            }
            drawnPeak = pk > drawnPeak ? pk : drawnPeak;
        }
        if (d < dmin)
        {
            nearE = e;
            nearSouth = vy >= oy + m->height && vx >= ox && vx <= ox + m->width;
            /* (Brennan 23:31: the south ridge "horizontally straight to the left edge": south of a drawn-belt map,
             * west and east of it too) */
            nearDrawnSouth = vy >= oy + m->height && VoxelRelief_Belt(m) != 0;
            nearDrawn = VoxelRelief_Belt(m) != 0;
        }
        dmin = d < dmin ? d : dmin;
    }
    /* POL-46 (Brennan 23:31: "mirror its stepped rock shape across to the left (north-west) half ... one huge,
     * continuous mountain (removing the flat bench wall in the middle)"): the other maps' caps held the peak behind
     * a drawn belt down to the low west edge of Route 112 beside it (the flat benches); where that peak stands, only
     * the drawn map's own cap holds. Exactly on a map's edge the field is still that edge. */
    /* (Audit batch 2: only where the drawn map is the nearest - north of Route 113, by Route 111's corner, Route
     * 113's own low edge holds the land down; WJPC_PEAK_NO_REACH=1: as before) */
    if (drawnPeak < -1e8f || getenv("WJPC_FIELD_PEAK_CAPPED") != NULL
     || (!nearDrawn && getenv("WJPC_PEAK_NO_REACH") == NULL))
        capMin = capOther < capMin ? capOther : capMin;
    if (getenv("WJPC_FIELD_NO_CAP") != NULL)
        capMin = 1e9f;
    if (exact)
    {
        *height = exactE < capMin ? exactE : capMin;
        *south = exactE <= capMin || getenv("WJPC_FIELD_SOUTH_OLD") != NULL ? exactS : *height;
        return;
    }
    if (sumW <= 0.0f)
    {
        *height = 0.0f;
        *south = 0.0f;
        return;
    }
    {
        float r = dmin / (float)BELT_FIELD_RATE, rise = sumRise / sumW;
        /* Audit batch 1 (Brennan 2026-11-19: the woods round Littleroot, Oldale, Routes 101/102/104 climbing as tall
         * walls of crowns - "Keep woods flat"): a corner of a wood of the belt does not climb; the rock beside it
         * still does. Route 118's wooded hill by its river (Brennan's "Wooded hill", locked in) keeps climbing: a wood
         * within 12 cells of the belt's sea. WJPC_WOODS_CLIMB=1: woods climb everywhere. */
        bool woodFlat = getenv("WJPC_WOODS_CLIMB") == NULL && FieldCornerWood(vx, vy);
        if (woodFlat)
            rise = 0.0f;
        /* (Audit batch 2: nor up a drawn map's peak where that map is not the nearest - the woods north of Route 113
         * climbed Route 111's) */
        if (woodFlat && !nearDrawn && getenv("WJPC_PEAK_NO_REACH") == NULL)
            drawnPeak = -1e9f;
        r = r < rise ? r : rise;
        *height = sumE / sumW + r * (sumBack / sumW);
        if (*height < drawnPeak)
            *height = drawnPeak;
        /* (on the land itself, whichever pull made it - added to the peak alone it changed nothing: the blend stood
         * over the peak there) */
        *height += lumpAdd;
        /* (and the drawn map's own cap with it: the peak there IS that cap - edge + d/2, rings of identical steps -
         * so the lumps were clipped flat again) */
        capMin += lumpAdd;
        /* POL-46 (Brennan 22:39, Lavaridge's south: "Make the Top of the Bottom Mountain Flush & Level Across the
         * Whole Width"; a blue triangle under the red-roofed houses): in front of a map (south of it) the land
         * never dips under the map's edge - the blend with farther, lower maps took it to 5.9 under an edge of 6.0,
         * a whole level down in places (sawtooth steps, a gap). WJPC_FIELD_SOUTH_DIP=1: as before. */
        if (nearSouth && *height < nearE && getenv("WJPC_FIELD_SOUTH_DIP") == NULL)
            *height = nearE;
        if (nearDrawnSouth && getenv("WJPC_FIELD_SOUTH_DIP") == NULL)
            *height = nearE;
        if (*height > capMin)
            *height = capMin;
        /* POL-46 (Brennan 2026-11-18 19:18: "Step the mountain down on BOTH sides toward the water"): within
         * BELT_FIELD_SHORE cells of the belt's sea the land comes down to it a level every BELT_FIELD_RATE cells - no
         * sheer wall from the hilltop into the water. WJPC_FIELD_NO_SHORE=1: off. */
        if (getenv("WJPC_FIELD_NO_SHORE") == NULL)
        {
            enum { BELT_FIELD_SHORE = 12 };  /* (5 levelled the land behind Route 118's river: "bring it back") */
            float best = 1e9f;
            for (int dy = -BELT_FIELD_SHORE; dy < BELT_FIELD_SHORE; ++dy)
                for (int dx = -BELT_FIELD_SHORE; dx < BELT_FIELD_SHORE; ++dx)
                {
                    int cx = vx + dx, cy = vy + dy;
                    float wl, ws, dd, cap;

                    if (VoxelWorld_GetInstanceAt(cx, cy) != NULL || !BorderAt(cx, cy) || !BorderWaterLevel(cx, cy, &wl, &ws))
                        continue;
                    /* the distance from the corner to the water cell's nearest point */
                    {
                        float qx = (float)vx < (float)cx ? (float)cx : (float)vx > (float)cx + 1.0f ? (float)cx + 1.0f : (float)vx;
                        float qy = (float)vy < (float)cy ? (float)cy : (float)vy > (float)cy + 1.0f ? (float)cy + 1.0f : (float)vy;
                        dd = sqrtf(((float)vx - qx) * ((float)vx - qx) + ((float)vy - qy) * ((float)vy - qy));
                    }
                    cap = BeltBase(cx, cy) + wl + dd / (float)BELT_FIELD_RATE;
                    best = cap < best ? cap : best;
                }
            if (*height > best)
                *height = best;
        }
        /* the way south only where the belt meets a map (to meet its edge exactly), gone over BELT_FIELD_SOUTH cells:
         * a mountain drawn as far south as it is high stood rows away from where it is, over the belt's sea and
         * trees beside it (which lie at their own rows), and left a hole of sky where it should have been */
        if (getenv("WJPC_FIELD_SOUTH_OLD") != NULL)
        {
            float k = 1.0f - dmin / (float)BELT_FIELD_SOUTH;
            *south = k > 0.0f ? (sumS / sumW) * k : 0.0f;
        }
        else
        {
            /* POL-46 (Lavaridge's north-east, red box: the wall between the north mountain and Route 112's hill):
             * the field drawn as far south as it is high, as every point of a map's drawing is (Route 112's slope
             * climbs a level every 2 rows, so in the world it stands far south of its rows): with its own way south
             * the field met that slope a dozen levels apart at the same place - a wall. Now one rule for both, so
             * the field's ledges run on into the map's tiers. */
            *south = *height;
        }
    }
    }
}

typedef struct { float x, y, h, s; } FieldPt;   /* grid position (cells), field height, way south */

static bool FlatFillPoint(int vx, int vy, float *height);
static bool FlatFillCell(int cx, int cy, float *height);

/* A flat fill cell's corner (vx, vy), its top `top`: on the fill (FlatFillPoint) its own top, standing where a cell of
 * that height stands (*own); on a map's edge that map's ground at the corner in this cell's own row (or column) -
 * the field's one point there is the higher of the two map cells it touches across a step in the map's edge, and
 * the cell beside the lower one stood a sliver of ledges up to it (the "spikes" by Route 114's west cliff); else the
 * field's point. (Brennan 2026-11-23 12:18, FlatStepsOn) */
/* Brennan 2026-11-23 13:00 (Route 114's west by the river: "mountain overhang"): whether corner (vx, vy) of a flat
 * fill's cell (cx, cy) lies on a map's edge whose ground there stands well over the fill (*top: the fill's, world
 * height; *edge, *edgeS: the map's ground and its way south at the corner, in this cell's own row or column). */
static bool FlatEdgeRise(int cx, int cy, int vx, int vy, float *top, float *edge, float *edgeS)
{
    static int sRamp = -1;
    float px = (float)vx + (vx > cx ? 0.01f : -0.01f), py = (float)vy + (vy > cy ? -0.01f : 0.01f);
    float qx = (float)vx + (vx > cx ? -0.01f : 0.01f), qy = (float)vy + (vy > cy ? 0.01f : -0.01f);
    float fp;

    if (sRamp < 0)
        sRamp = getenv("WJPC_FLAT_EDGE_RAMP") != NULL;
    if (sRamp || !FlatFillCell(cx, cy, top) || FlatFillPoint(vx, vy, &fp))
        return false;
    /* (only beside a map's west or east edge, where the slant showed: by a north or south edge the ramp is the
     * map's own way south - a wall there opened slits of sky by Route 115's north, 2026-11-23) */
    if (VoxelWorld_GetInstanceAt(cx - 1, cy) == NULL && VoxelWorld_GetInstanceAt(cx + 1, cy) == NULL)
        return false;
    if (VoxelWorld_GetInstanceAt((int)floorf(px), (int)floorf(py)) != NULL)
        *edge = VoxelRelief_LiftAt(px, py), *edgeS = VoxelRelief_ShiftAt(px, py);
    else if (VoxelWorld_GetInstanceAt((int)floorf(qx), (int)floorf(qy)) != NULL)
        *edge = VoxelRelief_LiftAt(qx, qy), *edgeS = VoxelRelief_ShiftAt(qx, qy);
    else
        return false;
    /* (over it by more than a quarter level: the "overhang"; under it by more than a level - the river's bank - a
     * leaning ramp too; a half level under, the plateau's own lip over the route's ground, stays a gentle ramp).
     * Judged by every map cell at the corner, so the two fill cells sharing it agree (a crack between them else). */
    for (int dq = 0; dq < 4; ++dq)
    {
        float sx = (float)vx + ((dq & 1) ? 0.01f : -0.01f), sy = (float)vy + ((dq & 2) ? 0.01f : -0.01f), l;
        if (VoxelWorld_GetInstanceAt((int)floorf(sx), (int)floorf(sy)) == NULL)
            continue;
        l = VoxelRelief_LiftAt(sx, sy);
        if (l > *top + 0.25f || l < *top - 1.0f)
            return true;
    }
    return false;
}

static FieldPt FlatCellCorner(int cx, int cy, int vx, int vy, float top, bool *own)
{
    FieldPt p;
    float fh;
    float px = (float)vx + (vx > cx ? 0.01f : -0.01f), py = (float)vy + (vy > cy ? -0.01f : 0.01f);
    float qx = (float)vx + (vx > cx ? -0.01f : 0.01f), qy = (float)vy + (vy > cy ? 0.01f : -0.01f);

    p.x = (float)vx;
    p.y = (float)vy;
    BeltField(vx, vy, &p.h, &p.s);
    *own = false;
    if (FlatFillPoint(vx, vy, &fh))
    {
        p.h = p.s = top;
        *own = true;
        return p;
    }
    /* (only a cell by a step of the fills: elsewhere the map's edge and the field's point agree, and a corner moved off
     * that point opened sky between them - Route 114's west cliff, 2026-11-23) */
    {
        static const int kN[4][2] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
        bool step = false;
        for (int d = 0; d < 4 && !step; ++d)
        {
            float t;
            step = FlatFillCell(cx + kN[d][0], cy + kN[d][1], &t) && fabsf(t - top) > 0.01f;
        }
        if (!step)
            return p;
    }
    if (VoxelWorld_GetInstanceAt((int)floorf(px), (int)floorf(py)) != NULL)
        p.h = VoxelRelief_LiftAt(px, py), p.s = VoxelRelief_ShiftAt(px, py);
    else if (VoxelWorld_GetInstanceAt((int)floorf(qx), (int)floorf(qy)) != NULL)
        p.h = VoxelRelief_LiftAt(qx, qy), p.s = VoxelRelief_ShiftAt(qx, qy);
    return p;
}

static FieldPt FieldLerp(FieldPt a, FieldPt b, float level)
{
    float t = (level - a.h) / (b.h - a.h);
    FieldPt r = { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, level, a.s + (b.s - a.s) * t };
    return r;
}

/* the part of a polygon with field height >= lo (keepAbove) or < hi (else): Sutherland-Hodgman on a plane */
static int FieldClip(const FieldPt *in, int n, FieldPt *out, float level, bool keepAbove)
{
    int m = 0;
    for (int i = 0; i < n; ++i)
    {
        FieldPt a = in[i], b = in[(i + 1) % n];
        bool ina = keepAbove ? a.h >= level : a.h < level;
        bool inb = keepAbove ? b.h >= level : b.h < level;
        if (ina)
            out[m++] = a;
        if (ina != inb)
            out[m++] = FieldLerp(a, b, level);
    }
    return m;
}

static void FieldTriangle(VoxelBuilder *b, int cellX, int cellY, FieldPt p0, FieldPt p1, FieldPt p2,
                          float tu0, float tv0, float tu1, float tv1, bool haveWall,
                          float wu0, float wv0, float wu1, float wv1)
{
    FieldPt tri[3] = { p0, p1, p2 };
    float lo = p0.h, hi = p0.h;

    for (int i = 1; i < 3; ++i)
    {
        lo = tri[i].h < lo ? tri[i].h : lo;
        hi = tri[i].h > hi ? tri[i].h : hi;
    }
    /* the ledges: each level's part, flat at that level */
    for (float k = floorf(lo); k <= floorf(hi); k += 1.0f)
    {
        FieldPt a[8], c[10];
        int n = FieldClip(tri, 3, a, k, true);
        n = FieldClip(a, n, c, k + 1.0f, false);
        if (n < 3)
            continue;
        for (int i = 1; i + 1 < n; ++i)
        {
            const FieldPt *q[3] = { &c[0], &c[i], &c[i + 1] };
            VoxelVertex v[3];
            for (int j = 0; j < 3; ++j)
            {
                v[j].x = q[j]->x;
                v[j].y = k;
                v[j].z = q[j]->y + q[j]->s;
                v[j].u = tu0 + (tu1 - tu0) * (q[j]->x - (float)cellX);
                v[j].v = tv0 + (tv1 - tv0) * (q[j]->y - (float)cellY);
                v[j].shade = SHADE_TOP;
            }
            /* wound to face up */
            VoxelBuilder_Tri(b, &v[0], &v[2], &v[1]);
        }
    }
    /* the walls: where the field crosses a whole level, a level of rock along the crossing */
    if (!haveWall)
        return;
    for (float k = floorf(lo) + 1.0f; k <= hi; k += 1.0f)
    {
        FieldPt cut[2];
        int n = 0;
        float gx, gy;

        for (int i = 0; i < 3 && n < 2; ++i)
        {
            FieldPt a = tri[i], c = tri[(i + 1) % 3];
            if ((a.h < k) != (c.h < k))
                cut[n++] = FieldLerp(a, c, k);
        }
        if (n < 2)
            continue;
        /* the downhill way (the wall faces it): the field's gradient on the triangle */
        {
            float ax = p1.x - p0.x, ay = p1.y - p0.y, ah = p1.h - p0.h;
            float bx = p2.x - p0.x, by = p2.y - p0.y, bh = p2.h - p0.h;
            float det = ax * by - ay * bx;
            if (fabsf(det) < 1e-6f)
                continue;
            gx = (ah * by - bh * ay) / det;
            gy = (bh * ax - ah * bx) / det;
        }
        {
            float ex = cut[1].x - cut[0].x, ey = cut[1].y - cut[0].y;
            /* the quad's face is (-ey, ex) for this order; it must face downhill, -grad */
            if (-ey * -gx + ex * -gy < 0.0f)
            {
                FieldPt t = cut[0];
                cut[0] = cut[1];
                cut[1] = t;
            }
        }
        {
            float nx = -(cut[1].y - cut[0].y), ny = cut[1].x - cut[0].x;
            float shade = fabsf(nx) > fabsf(ny) ? (nx < 0 ? SHADE_WEST : SHADE_EAST) : (ny > 0 ? SHADE_SOUTH : SHADE_NORTH);
            VoxelVertex q0 = { cut[0].x, k - 1.0f, cut[0].y + cut[0].s, wu0, wv1, shade };
            VoxelVertex q1 = { cut[1].x, k - 1.0f, cut[1].y + cut[1].s, wu1, wv1, shade };
            VoxelVertex q2 = { cut[1].x, k,        cut[1].y + cut[1].s, wu1, wv0, shade };
            VoxelVertex q3 = { cut[0].x, k,        cut[0].y + cut[0].s, wu0, wv0, shade };
            VoxelBuilder_Quad(b, &q0, &q1, &q2, &q3);
        }
    }
}

/* Brennan 2026-11-22 17:01/17:14 (Verdanturf's west woods: "There shouldn't be open grass/snow blocks sitting in the
 * middle of that forest corner ... Fill that entire red-box area with full-height 3D trees"): where the wood climbs, its
 * floor shows between the rows of crowns. The floor under a wood is the wood's own drawing (its crowns, the cell's
 * border metatile), so what shows between the trees is forest. WJPC_WOOD_FLOOR_GRASS=1: the grass, as before. */
/* Brennan 2026-11-22 13:39 (D2, Route 119 south of Fortree: "that row of 3D trees is flickering/strobing when
 * moving"; B4 the same on Route 119's middle-west terrace): the belt trees' ground squares (1/64 tile over their level)
 * and the wood's floor under them at that level fought in the depth buffer far from the camera - speckled dark feet
 * that change as the camera moves. Fixed by raising the squares (voxel_tree.c, 1/16); lowering the floor instead
 * opened sky at the horizon. WJPC_WOOD_FLOOR_DROP=x tiles: lower it anyway (test). */
static float WoodFloorDrop(void)
{
    static float sDrop = -1.0f;
    if (sDrop < 0.0f)
        sDrop = getenv("WJPC_WOOD_FLOOR_DROP") ? (float)atof(getenv("WJPC_WOOD_FLOOR_DROP")) : 0.0f;
    return sDrop;
}

bool VoxelMesh_BeltWater(int x, int y)
{
    float l, sh;
    return VoxelWorld_GetInstanceAt(x, y) == NULL && BorderAt(x, y) && BorderWaterLevel(x, y, &l, &sh);
}

static int WoodFloorTile(int x, int y)
{
    static int sGrass = -1;
    if (sGrass < 0)
        sGrass = getenv("WJPC_WOOD_FLOOR_GRASS") != NULL;
    return sGrass ? BeltTreeGround(x, y) : VoxelWorld_BorderMetatile(x, y);
}

static void EmitFieldCell(VoxelBuilder *b, int x, int y)
{
    FieldPt c[4];
    float tu0, tv0, tu1, tv1, wu0, wv0, wu1, wv1;
    const VoxelMapInstance *ring = VoxelWorld_BeltRing(x, y);
    int face = ring != NULL ? VoxelRelief_FaceTile(ring) : -1;
    bool wall;
    const float keepBase = b->base, keepLift = b->lift, keepShift = b->shift, keepStretch = b->stretchAdd;

    int top = BeltIsTree(x, y) ? WoodFloorTile(x, y) : BeltMetatileFor(x, y, y);

    /* POL-46 (Brennan 2026-11-19 00:12: "use the exact same rocky mountain block/voxel texture and relief as the
     * north-east mountain peak right next to it"): north of a map whose belt is its drawing (Lavaridge), the field
     * takes the rock of the nearest map beside it along the row (Route 112's peak): its top drawing and its cliff
     * face. WJPC_FIELD_OWN_ROCK=1: the cell's own. */
    if (!BeltIsTree(x, y) && getenv("WJPC_FIELD_OWN_ROCK") == NULL)
    {
        const VoxelMapInstance *drawnMap = NULL, *beside = NULL;
        int bestD = 1 << 30;
        for (unsigned i = 0; i < VoxelWorld_InstanceCount(); ++i)
        {
            const VoxelMapInstance *m = VoxelWorld_Instance(i);
            if (m != NULL && m->layout != NULL && !m->indoor && VoxelRelief_Belt(m) != 0 && y < m->originY
             && y >= m->originY - 24 && x >= m->originX - 2 && x < m->originX + m->width + 2)
                drawnMap = m;
        }
        for (unsigned i = 0; drawnMap != NULL && i < VoxelWorld_InstanceCount(); ++i)
        {
            const VoxelMapInstance *m = VoxelWorld_Instance(i);
            int dx;
            if (m == NULL || m == drawnMap || m->layout == NULL || m->indoor || VoxelRelief_Belt(m) != 0
             || y < m->originY || y >= m->originY + m->height)
                continue;
            dx = x < m->originX ? m->originX - x : x >= m->originX + m->width ? x - (m->originX + m->width - 1) : 0;
            if (dx > 0 && dx < bestD && dx <= 24)
            {
                bestD = dx;
                beside = m;
            }
        }
        if (beside != NULL)
        {
            int t = BeltTopTile(beside), f = VoxelRelief_FaceTile(beside);
            if (t >= 0)
                top = t;
            if (f >= 0)
                face = f;
        }
    }
    /* (a wood's cell: the field under its trees, in its grass - BeltTreeGround) */
    if (getenv("WJPC_FIELD_NONE") != NULL)
        return;
    if (!MetatileUV(b, top, &tu0, &tv0, &tu1, &tv1)
     && !(BeltIsTree(x, y) && MetatileUV(b, top = BeltTreeGround(x, y), &tu0, &tv0, &tu1, &tv1)))
        return; /* (a wood's drawing not in this atlas: its grass, which is) */
    /* Brennan 2026-11-21 23:06 (21q, Route 117's north-east corner: the small flat patch of rock at the foot of the
     * terraces, "Make it grass"): a ledge of the field lying flat and flush with the ground of a map beside it (the
     * field there at the map's edge height, no step) is that ground going on - it takes the map edge cell's drawing
     * (grass there), not the ring's rock. Not when that edge is trees or water. WJPC_FIELD_FLUSH_ROCK=1: as before. */
    if (!BeltIsTree(x, y) && getenv("WJPC_FIELD_FLUSH_ROCK") == NULL)
    {
        static const int kSide[4][2] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
        float h0, h1, h2, h3, s0;

        BeltField(x, y, &h0, &s0);
        BeltField(x + 1, y, &h1, &s0);
        BeltField(x, y + 1, &h2, &s0);
        BeltField(x + 1, y + 1, &h3, &s0);
        if (fabsf(h0 - h1) < 0.01f && fabsf(h0 - h2) < 0.01f && fabsf(h0 - h3) < 0.01f)
            for (int d = 0; d < 4; ++d)
            {
                int nx = x + kSide[d][0], ny = y + kSide[d][1], mt;
                const VoxelMapInstance *m = VoxelWorld_GetInstanceAt(nx, ny);
                float u0, v0, u1, v1;

                if (m == NULL || m->indoor || VoxelTree_PartAt(nx, ny) >= 0
                 || VoxelMesh_Classify(nx, ny) == VOXEL_SHAPE_WATER
                 || fabsf(VoxelRelief_LiftAt((float)nx + 0.5f, (float)ny + 0.5f) - h0) > 0.05f)
                    continue;
                mt = VoxelWorld_GetMetatileId(nx, ny);
                if (mt >= 0 && MetatileUV(b, mt, &u0, &v0, &u1, &v1))
                {
                    if (getenv("WJPC_FIELD_DUMP") != NULL)
                        fprintf(stderr, "FIELDFLUSH %d %d %03x -> %03x (map cell %d,%d)\n", x, y, top, mt, nx, ny);
                    top = mt;
                    tu0 = u0, tv0 = v0, tu1 = u1, tv1 = v1;
                    break;
                }
            }
    }
    wall = (face >= 0 && MetatileUV(b, face, &wu0, &wv0, &wu1, &wv1))
        || MetatileUV(b, BeltMetatileFor(x, y, y), &wu0, &wv0, &wu1, &wv1);
    /* Brennan 2026-11-22 01:19 (Route 112's south, 21u: the walls "textured with a bright red-and-white striped rock
     * texture that clashes ... Replace [it] with the exact earthy tan/brown rocky cliff texture shown inside the blue
     * box" - the level land in front): in front of a map (its first rows south of it) the field drops from the map's
     * edge to its level land, and those ledges were faced in the ring's cliff tile (Route 112's red stripes) over a
     * top of the border's rock. They are faced in their own top's drawing now: one rock. WJPC_FIELD_FRONT_FACE_OLD=1:
     * as before. */
    if (wall && getenv("WJPC_FIELD_FRONT_FACE_OLD") == NULL && !BeltIsTree(x, y))
    {
        bool front = false;
        for (unsigned i = 0; i < VoxelWorld_InstanceCount() && !front; ++i)
        {
            const VoxelMapInstance *m = VoxelWorld_Instance(i);
            front = m != NULL && m->layout != NULL && !m->indoor && VoxelRelief_Belt(m) == 0
                 && y >= m->originY + m->height && y < m->originY + m->height + 3
                 && x >= m->originX - 1 && x <= m->originX + m->width;
        }
        if (front)
        {
            wu0 = tu0, wv0 = tv0, wu1 = tu1, wv1 = tv1;
        }
    }
    for (int k = 0; k < 4; ++k)
    {
        int vx = x + (k & 1), vy = y + (k >> 1);
        c[k].x = (float)vx;
        c[k].y = (float)vy;
        BeltField(vx, vy, &c[k].h, &c[k].s);
    }
    /* a flat fill's cell where it meets another's of another height: its own top (FlatStepsOn, FlatCellCorner) */
    bool flatOwn[4] = { false, false, false, false };
    float ownTop = 0.0f;
    bool flatCell = FlatStepsOn() && !BeltIsTree(x, y) && FlatFillCell(x, y, &ownTop);
    if (flatCell)
        for (int k = 0; k < 4; ++k)
            c[k] = FlatCellCorner(x, y, x + (k & 1), y + (k >> 1), ownTop, &flatOwn[k]);
    /* Brennan 2026-11-23 13:00 (Route 114's west, by the river: "Looks to be some mountain overhang ... This needs
     * fixed"): where a map's ground at its edge stands well over the fill (Route 114's mountain corner, 7 levels down
     * to 4, over the plateau's 3.5), the field's one cell from the fill up to the map's edge was a steep ramp of
     * squeezed ledges leaning out over the plateau. The fill stays level to the map's edge, a wall of rock rising
     * there straight up to the map's ground (in this cell's own row). WJPC_FLAT_EDGE_RAMP=1: the ramp, as before. */
    bool edgeWall[4] = { false, false, false, false };
    float edgeTop[4] = { 0 }, edgeS[4] = { 0 };
    {
        static int sRamp = -1;
        if (sRamp < 0)
            sRamp = getenv("WJPC_FLAT_EDGE_RAMP") != NULL;
        /* (a wood's cell of the fill too: its floor - Route 114's west woods by the mountain corner) */
        float anyTop = ownTop;
        bool anyFlat = flatCell || (FlatStepsOn() && BeltIsTree(x, y) && FlatFillCell(x, y, &anyTop));
        if (anyFlat && !flatCell)
            ownTop = anyTop;
        for (int k = 0; anyFlat && !sRamp && k < 4; ++k)
        {
            float t;
            edgeTop[k] = -1e9f;
            if (FlatEdgeRise(x, y, x + (k & 1), y + (k >> 1), &t, &edgeTop[k], &edgeS[k]))
            {
                c[k].h = c[k].s = ownTop;
                edgeWall[k] = true;
            }
        }
        /* the wall on the side facing the map, where both its corners rose */
        {
            static const int kSide[4][2] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
            static const int kCorner[4][2] = { {1, 0}, {2, 3}, {0, 2}, {3, 1} };
            for (int d = 0; d < 4; ++d)
            {
                int ka = kCorner[d][0], ke = kCorner[d][1];
                if ((!edgeWall[ka] && !edgeWall[ke]) || VoxelWorld_GetInstanceAt(x + kSide[d][0], y + kSide[d][1]) == NULL)
                    continue;
                /* (a corner where the map and the fill nearly meet: the wall there from the map's ground to the field's
                 * point - or, with no map's ground found, closing to that point) */
                bool nearA = !edgeWall[ka] && edgeTop[ka] > -1e8f, nearE = !edgeWall[ke] && edgeTop[ke] > -1e8f;
                {
                    float shade = d == 0 ? SHADE_NORTH : d == 1 ? SHADE_SOUTH : d == 2 ? SHADE_WEST : SHADE_EAST;
                    float su0 = wall ? wu0 : tu0, sv0 = wall ? wv0 : tv0, su1 = wall ? wu1 : tu1, sv1 = wall ? wv1 : tv1;
                    float base0 = flatCell ? ownTop : ownTop - WoodFloorDrop();
                    float loA = edgeTop[ka] < base0 ? edgeTop[ka] : base0, hiA = edgeTop[ka] < base0 ? base0 : edgeTop[ka];
                    float loE = edgeTop[ke] < base0 ? edgeTop[ke] : base0, hiE = edgeTop[ke] < base0 ? base0 : edgeTop[ke];
                    if (!edgeWall[ka])
                        loA = nearA && edgeTop[ka] < c[ka].h ? edgeTop[ka] : c[ka].h, hiA = nearA && edgeTop[ka] > c[ka].h ? edgeTop[ka] : c[ka].h;
                    if (!edgeWall[ke])
                        loE = nearE && edgeTop[ke] < c[ke].h ? edgeTop[ke] : c[ke].h, hiE = nearE && edgeTop[ke] > c[ke].h ? edgeTop[ke] : c[ke].h;
                    /* (down to the map's lowest ground at each corner - a step in the map's own edge there, both of
                     * its sides: no slit of sky under the wall) */
                    for (int q = 0; q < 2; ++q)
                    {
                        FieldPt cp = q == 0 ? c[ka] : c[ke];
                        float *lo = q == 0 ? &loA : &loE;
                        for (int dq = 0; dq < 4; ++dq)
                        {
                            float sx = cp.x + ((dq & 1) ? 0.01f : -0.01f), sy = cp.y + ((dq & 2) ? 0.01f : -0.01f);
                            if (VoxelWorld_GetInstanceAt((int)floorf(sx), (int)floorf(sy)) != NULL)
                            {
                                float l = VoxelRelief_LiftAt(sx, sy);
                                if (l < *lo)
                                    *lo = l;
                            }
                        }
                    }
                    float bot = loA < loE ? loA : loE, topH = hiA > hiE ? hiA : hiE;
                    FieldPt a = c[ka], e = c[ke];
                    /* (each corner's wall from the lower side's point to the higher's, each where it stands: the
                     * map's ground at its own way south, the fill's at its own - a wall as the map's walls are) */
                    float slA = loA < (edgeWall[ka] ? (edgeTop[ka] < base0 ? edgeTop[ka] : ownTop) : (nearA && edgeTop[ka] < c[ka].h ? edgeTop[ka] : c[ka].h)) - 0.01f ? loA
                              : edgeWall[ka] ? (edgeTop[ka] < base0 ? edgeS[ka] : ownTop)
                              : nearA && edgeTop[ka] < c[ka].h ? edgeS[ka] : c[ka].s;
                    float shA = edgeWall[ka] ? (edgeTop[ka] < base0 ? ownTop : edgeS[ka])
                              : nearA && edgeTop[ka] > c[ka].h ? edgeS[ka] : c[ka].s;
                    float slE = loE < (edgeWall[ke] ? (edgeTop[ke] < base0 ? edgeTop[ke] : ownTop) : (nearE && edgeTop[ke] < c[ke].h ? edgeTop[ke] : c[ke].h)) - 0.01f ? loE
                              : edgeWall[ke] ? (edgeTop[ke] < base0 ? edgeS[ke] : ownTop)
                              : nearE && edgeTop[ke] < c[ke].h ? edgeS[ke] : c[ke].s;
                    float shE = edgeWall[ke] ? (edgeTop[ke] < base0 ? ownTop : edgeS[ke])
                              : nearE && edgeTop[ke] > c[ke].h ? edgeS[ke] : c[ke].s;
#define WALL_S(h, lo, hi, sl, sh) ((hi) - (lo) < 0.001f ? (sl) : (sl) + ((sh) - (sl)) * (((h) - (lo)) / ((hi) - (lo))))
                    /* (its foot an eighth of a level under the ground it meets, hidden: a 3-7 px speck of sky showed
                     * at the foot by Route 114's river bank, 2026-11-23) */
                    if (hiA - loA > 0.01f)
                        loA -= 0.125f;
                    if (hiE - loE > 0.01f)
                        loE -= 0.125f;
                    bot = loA < loE ? loA : loE;
                    for (float lo = bot; lo < topH - 0.01f; lo += 1.0f)
                    {
                        float hi = lo + 1.0f < topH ? lo + 1.0f : topH;
                        float a0 = lo < loA ? loA : lo > hiA ? hiA : lo, a1 = hi < loA ? loA : hi > hiA ? hiA : hi;
                        float e0 = lo < loE ? loE : lo > hiE ? hiE : lo, e1 = hi < loE ? loE : hi > hiE ? hiE : hi;
                        VoxelVertex q0 = { a.x, a0, a.y + WALL_S(a0, loA, hiA, slA, shA), su0, sv1, shade };
                        VoxelVertex q1 = { e.x, e0, e.y + WALL_S(e0, loE, hiE, slE, shE), su1, sv1, shade };
                        VoxelVertex q2 = { e.x, e1, e.y + WALL_S(e1, loE, hiE, slE, shE), su1, sv1 + (sv0 - sv1) * (e1 - e0), shade };
                        VoxelVertex q3 = { a.x, a1, a.y + WALL_S(a1, loA, hiA, slA, shA), su0, sv1 + (sv0 - sv1) * (a1 - a0), shade };
                        if (a1 - a0 < 0.001f && e1 - e0 < 0.001f)
                            continue;
                        VoxelBuilder_Quad(b, &q0, &q1, &q2, &q3);
                        VoxelBuilder_Quad(b, &q3, &q2, &q1, &q0);   /* (either way: it faces the camera from a side) */
                    }
#undef WALL_S
                    /* (its foot stands as far south as the map's ground is high: on the fill's ground of the rows
                     * south of this one) */
                }
            }
        }
    }
    {
        static int sDump = -1;
        if (sDump < 0)
            sDump = getenv("WJPC_FIELD_DUMP") != NULL ? 1 : 0;
        static const void *sLast0;
        if (sDump && VoxelWorld_Instance(0) != NULL && sLast0 != (const void *)VoxelWorld_Instance(0)->layout)
        {
            sLast0 = (const void *)VoxelWorld_Instance(0)->layout;
            for (unsigned i = 0; i < VoxelWorld_InstanceCount(); ++i)
            {
                const VoxelMapInstance *m = VoxelWorld_Instance(i);
                if (m != NULL)
                    fprintf(stderr, "INST %u %p origin %d,%d size %dx%d indoor %d layout %p base %.2f\n", i, (const void *)m,
                            m->originX, m->originY, m->width, m->height, (int)m->indoor, (const void *)m->layout,
                            VoxelRelief_Base(m));
            }
            int gy0 = getenv("WJPC_GRID_Y0") ? atoi(getenv("WJPC_GRID_Y0")) : -16;
            int gx0 = getenv("WJPC_GRID_X0") ? atoi(getenv("WJPC_GRID_X0")) : -8;
            for (int vy = gy0; vy <= gy0 + 18; ++vy)
            {
                fprintf(stderr, "GRID %3d:", vy);
                for (int vx = gx0; vx <= gx0 + 36; ++vx)
                {
                    float fh, fs;
                    BeltField(vx, vy, &fh, &fs);
                    fprintf(stderr, " %4.1f", fh);
                }
                fprintf(stderr, "\n");
            }
        }
        if (sDump)
            fprintf(stderr, "FIELD %d %d  %.2f/%.2f %.2f/%.2f %.2f/%.2f %.2f/%.2f  ring=%p top %03x\n", x, y, c[0].h, c[0].s,
                    c[1].h, c[1].s, c[2].h, c[2].s, c[3].h, c[3].s, (const void *)ring, top);
    }
    /* world heights and ways south, as they are */
    b->base = 0.0f;
    b->lift = 0.0f;
    b->shift = 0.0f;
    b->stretchAdd = 0.0f;
    /* Brennan 2026-11-22 13:37 (B2, Verdanturf's west woods: "grey/snow-covered cliff blocks are jutting straight
     * through the middle of the 3D trees"): under a wood climbing a slope the field's ledges stood as blocks of grass
     * between the crowns. Under a cell of the woods the field is laid as one
     * smooth slope (no ledges, no walls), in the crowns' shade; the trees on it hide it. WJPC_WOOD_FIELD_LEDGES=1: ledges, as before. */
    {
        static int sLedges = -1;
        if (sLedges < 0)
            sLedges = getenv("WJPC_WOOD_FIELD_LEDGES") != NULL;
        float lo = c[0].h, hi = c[0].h;
        for (int k = 1; k < 4; ++k)
            lo = c[k].h < lo ? c[k].h : lo, hi = c[k].h > hi ? c[k].h : hi;
        if (!sLedges && BeltIsTree(x, y))
        {
            VoxelVertex v[4];
            for (int k = 0; k < 4; ++k)
            {
                /* each corner through the trees' own levels round it (VoxelMesh_BorderLevel, over their base), so
                 * the trees stand on it - at the field's own height a tree on a ledge was buried to its crown */
                int vx = x + (k & 1), vy = y + (k >> 1), n = 0;
                float sh = 0.0f, ss = 0.0f;
                bool open = false;
                for (int q = 0; q < 4; ++q)
                {
                    int cx = vx - 1 + (q & 1), cy = vy - 1 + (q >> 1);
                    float e, es;
                    if (VoxelWorld_GetInstanceAt(cx, cy) == NULL && BorderAt(cx, cy) && BeltIsTree(cx, cy)
                     && VoxelMesh_BorderLevel(cx, cy, &e, &es))
                        sh += BeltBase(cx, cy) + e, ss += es, ++n;
                    else
                        open = true; /* the wood's edge: the field's own height there, which its neighbour meets */
                }
                if (n > 0 && !open)
                    c[k].h = sh / (float)n, c[k].s = ss / (float)n;
                if (edgeWall[k])
                    c[k].h = c[k].s = ownTop;   /* (the fill level to the map's edge: the wall above) */
                v[k].x = c[k].x;
                v[k].y = c[k].h - WoodFloorDrop();
                v[k].z = c[k].y + c[k].s;
                v[k].u = k & 1 ? tu1 : tu0;
                v[k].v = k >> 1 ? tv1 : tv0;
                {
                    static float sShade = -1.0f;
                    if (sShade < 0.0f)
                        sShade = getenv("WJPC_WOOD_FLOOR_SHADE") ? (float)atof(getenv("WJPC_WOOD_FLOOR_SHADE")) : 0.35f;
                    v[k].shade = SHADE_TOP * sShade; /* the wood's floor, in the crowns' shade */
                }
            }
            VoxelBuilder_Tri(b, &v[0], &v[3], &v[1]);
            VoxelBuilder_Tri(b, &v[0], &v[2], &v[3]);
            b->base = keepBase;
            b->lift = keepLift;
            b->shift = keepShift;
            b->stretchAdd = keepStretch;
            return;
        }
    }
    /* the diagonal along the corner that differs most from the others' mean - the chamfer runs across a turn */
    if (fabsf(c[0].h - c[3].h) <= fabsf(c[1].h - c[2].h))
    {
        FieldTriangle(b, x, y, c[0], c[1], c[3], tu0, tv0, tu1, tv1, wall, wu0, wv0, wu1, wv1);
        FieldTriangle(b, x, y, c[0], c[3], c[2], tu0, tv0, tu1, tv1, wall, wu0, wv0, wu1, wv1);
    }
    else
    {
        FieldTriangle(b, x, y, c[0], c[1], c[2], tu0, tv0, tu1, tv1, wall, wu0, wv0, wu1, wv1);
        FieldTriangle(b, x, y, c[1], c[3], c[2], tu0, tv0, tu1, tv1, wall, wu0, wv0, wu1, wv1);
    }
    /* the step down to a lower flat fill beside it: a wall of rock from that fill's ground up to this one's, where a
     * cell of this height stands, and the lower land's ground on from its own edge to the wall's foot (the higher
     * land stands further south by its height: behind a step facing away a strip of the lower land shows, under one
     * facing the camera it is hidden). Each side corner: the lower cell's point there (lo) and this one's (hi). */
    if (flatCell)
    {
        static const int kStepSide[4][2] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
        static const int kStepCorner[4][2] = { {1, 0}, {2, 3}, {0, 2}, {3, 1} };
        for (int d = 0; d < 4; ++d)
        {
            float nTop;
            int nx = x + kStepSide[d][0], ny = y + kStepSide[d][1];
            int ka = kStepCorner[d][0], ke = kStepCorner[d][1];
            bool oa, oe;
            FieldPt ha = c[ka], he = c[ke], la, le;
            if ((!flatOwn[ka] && !flatOwn[ke]) || BeltIsTree(nx, ny) || !FlatFillCell(nx, ny, &nTop) || nTop >= ownTop - 0.01f)
                continue;
            la = FlatCellCorner(nx, ny, (int)ha.x, (int)ha.y, nTop, &oa);
            le = FlatCellCorner(nx, ny, (int)he.x, (int)he.y, nTop, &oe);
            if (la.h > ha.h) la.h = ha.h;
            if (le.h > he.h) le.h = he.h;
            {
                float shade = d == 0 ? SHADE_NORTH : d == 1 ? SHADE_SOUTH : d == 2 ? SHADE_WEST : SHADE_EAST;
                float su0 = wall ? wu0 : tu0, sv0 = wall ? wv0 : tv0, su1 = wall ? wu1 : tu1, sv1 = wall ? wv1 : tv1;
                float bottom = la.h < le.h ? la.h : le.h, topH = ha.h > he.h ? ha.h : he.h;
                /* the wall, a course at a time, at this cell's corners (each corner clamped to its own lo..hi) */
                for (float lo = bottom; lo < topH - 0.01f; lo += 1.0f)
                {
                    float hi = lo + 1.0f < topH ? lo + 1.0f : topH;
                    float a0 = lo < la.h ? la.h : lo > ha.h ? ha.h : lo, a1 = hi < la.h ? la.h : hi > ha.h ? ha.h : hi;
                    float e0 = lo < le.h ? le.h : lo > he.h ? he.h : lo, e1 = hi < le.h ? le.h : hi > he.h ? he.h : hi;
                    if (a1 - a0 < 0.001f && e1 - e0 < 0.001f)
                        continue;
                    {
                        VoxelVertex q0 = { ha.x, a0, ha.y + ha.s, su0, sv1, shade };
                        VoxelVertex q1 = { he.x, e0, he.y + he.s, su1, sv1, shade };
                        VoxelVertex q2 = { he.x, e1, he.y + he.s, su1, sv1 + (sv0 - sv1) * (e1 - e0), shade };
                        VoxelVertex q3 = { ha.x, a1, ha.y + ha.s, su0, sv1 + (sv0 - sv1) * (a1 - a0), shade };
                        VoxelBuilder_Quad(b, &q0, &q1, &q2, &q3);
                    }
                }
                /* the lower land's strip from its edge to the wall's foot (north or south sides), a cell at a time */
                if (d <= 1)
                {
                    float za0 = la.y + la.s, za1 = ha.y + ha.s, ze0 = le.y + le.s, ze1 = he.y + he.s;
                    float len = fabsf(za1 - za0) > fabsf(ze1 - ze0) ? fabsf(za1 - za0) : fabsf(ze1 - ze0);
                    int steps = (int)ceilf(len - 0.01f);
                    for (int st = 0; st < steps; ++st)
                    {
                        float t0 = (float)st / (float)steps, t1 = (float)(st + 1) / (float)steps;
                        float va = tv0 + (tv1 - tv0) * (len / (float)steps);
                        VoxelVertex f0 = { ha.x, la.h, za0 + (za1 - za0) * t0, tu0, tv0, SHADE_TOP };
                        VoxelVertex f1 = { he.x, le.h, ze0 + (ze1 - ze0) * t0, tu1, tv0, SHADE_TOP };
                        VoxelVertex f2 = { he.x, le.h, ze0 + (ze1 - ze0) * t1, tu1, va, SHADE_TOP };
                        VoxelVertex f3 = { ha.x, la.h, za0 + (za1 - za0) * t1, tu0, va, SHADE_TOP };
                        VoxelBuilder_Quad(b, &f0, &f1, &f2, &f3);
                        VoxelBuilder_Quad(b, &f3, &f2, &f1, &f0);   /* (either way up: it may be seen from below) */
                    }
                }
            }
        }
    }
    /* its sides on what is not the field (the belt's sea and trees, a map): a wall of rock from the ground up to
     * the ledges along that side, each stretch of the side at its whole level - the field's edge stood open over
     * the sea beside it, a hole of sky down the shore (Route 118's north, 2026-11-18) */
    if (wall)
    {
        static const int kSide[4][2] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
        /* each side's corners, in the order that faces it outward (see VoxelMesh_North/South/West/East) */
        static const int kCorner[4][2] = { {1, 0}, {2, 3}, {0, 2}, {3, 1} };

        for (int d = 0; d < 4; ++d)
        {
            int nx = x + kSide[d][0], ny = y + kSide[d][1];
            float floorH, wl, ws;
            FieldPt a = c[kCorner[d][0]], e = c[kCorner[d][1]], cut[8];
            int n = 0;
            float shade = d == 0 ? SHADE_NORTH : d == 1 ? SHADE_SOUTH : d == 2 ? SHADE_WEST : SHADE_EAST;

            if (VoxelWorld_GetInstanceAt(nx, ny) != NULL || !BorderAt(nx, ny))
                continue;   /* a map: its edge closes it (EdgeOverBelt, EmitSeamSkirt) */
            if (BorderWaterLevel(nx, ny, &wl, &ws) ? false
                : !BeltIsTree(nx, ny) || !BeltTreeOwn(nx, ny))
                continue;   /* the field (woods lie on it too): the same corners, nothing open */
            floorH = BeltBase(nx, ny) - 1.0f;
            /* the side, cut where the field crosses a whole level: each stretch one level of ledge */
            cut[n++] = a;
            {
                float lo = a.h < e.h ? a.h : e.h, hi = a.h < e.h ? e.h : a.h;
                for (float k = floorf(lo) + 1.0f; k <= hi && n < 7; k += 1.0f)
                    cut[n++] = FieldLerp(a, e, k);
                if (a.h > e.h)
                    for (int i = 1, j = n - 1; i < j; ++i, --j)
                    {
                        FieldPt t = cut[i];
                        cut[i] = cut[j];
                        cut[j] = t;
                    }
            }
            cut[n++] = e;
            for (int i = 0; i + 1 < n; ++i)
            {
                FieldPt p = cut[i], q = cut[i + 1];
                float top = floorf((p.h + q.h) * 0.5f + 1e-4f);
                for (float lo = floorH; lo < top; lo += 1.0f)
                {
                    float hi = lo + 1.0f < top ? lo + 1.0f : top;
                    float vb = wv1, vt = wv1 + (wv0 - wv1) * (hi - lo);
                    VoxelVertex q0 = { p.x, lo, p.y + p.s, wu0, vb, shade };
                    VoxelVertex q1 = { q.x, lo, q.y + q.s, wu1, vb, shade };
                    VoxelVertex q2 = { q.x, hi, q.y + q.s, wu1, vt, shade };
                    VoxelVertex q3 = { p.x, hi, p.y + p.s, wu0, vt, shade };
                    VoxelBuilder_Quad(b, &q0, &q1, &q2, &q3);
                }
            }
        }
    }
    b->base = keepBase;
    b->lift = keepLift;
    b->shift = keepShift;
    b->stretchAdd = keepStretch;
}

/*
 * POL-46 (Brennan 2026-11-18 19:18: "Place trees on the flat horizontal terrace shelves where a real hillside
 * forest would grow, avoiding steep cliff faces"; asked where: "Near existing woods"): a land cell of the field
 * whose four corners lie on one ledge, within BELT_LEDGE_WOOD cells of a wood (the belt's or a map's trees),
 * carries a small tree (two in three, by its place, so the edge of the wood thins out instead of a straight block).
 * Its level and way south: the ledge's. WJPC_NO_LEDGE_TREES=1: none.
 */
/* POL-46: TRUE when a wood cell of the belt lies across a wall of the field (its corners on two ledges): its tree
 * would straddle the drop, so it stands none (its grass is the field's). */
bool VoxelMesh_TreeOverDrop(int x, int y)
{
    float lo = 1e9f, hi = -1e9f;
    static int sEdge = -1;

    if (!FieldOn() || BeltTreeOwn(x, y) || getenv("WJPC_TREES_ON_DROPS") != NULL)
        return false;
    for (int k = 0; k < 4; ++k)
    {
        float fh, fs;
        BeltField(x + (k & 1), y + (k >> 1), &fh, &fs);
        fh = floorf(fh + 1e-4f);
        lo = fh < lo ? fh : lo;
        hi = fh > hi ? fh : hi;
    }
    if (hi == lo)
        return false;
    /* Brennan 2026-11-21 17:25 (image 7, Route 117's north-west woods: "a strip of raised bright-green grass cuts
     * straight up through the woods instead of trees - fill that strip with full 3D trees"): the rock mound at the
     * map's north edge (cells 33-35) raises the field behind it into a ridge two cells wide running north, and every
     * wood cell either side of it straddled a step - none stood, two lines of bare grass. Deep in a wood (every cell round it
     * that is the belt's is its wood; a map's cells beside it do not count) the tree stands anyway, on the lower ledge (VoxelMesh_BorderLevel: the lowest
     * corner), its crown over the step; at a wood's edge, beside open terraces, still none (POL-46: no trees on the
     * cliff faces). WJPC_NO_TREES_ON_INNER_DROPS=1: none, as before. */
    if (sEdge < 0)
        sEdge = getenv("WJPC_NO_TREES_ON_INNER_DROPS") != NULL;
    if (!sEdge)
    {
        bool inner = true;
        for (int dy = -1; dy <= 1 && inner; ++dy)
            for (int dx = -1; dx <= 1 && inner; ++dx)
                if ((dx != 0 || dy != 0) && VoxelWorld_GetInstanceAt(x + dx, y + dy) == NULL)
                    inner = BorderAt(x + dx, y + dy) && BeltIsTree(x + dx, y + dy);
        if (getenv("WJPC_TREE_DUMP") != NULL)
            fprintf(stderr, "DROPTREE %d %d drop %.0f inner %d\n", x, y, hi - lo, (int)inner);
        if (inner)
            return false;
    }
    return true;
}

enum { BELT_LEDGE_WOOD = 2 };
/* a well-mixed 0..99 per cell (the scatter of single trees: no columns or rows) */
static unsigned TreeScatter(int x, int y)
{
    unsigned h = (unsigned)x * 0x9E3779B1u ^ ((unsigned)y + 0x7F4A7C15u) * 0x85EBCA77u;
    h ^= h >> 15;
    h *= 0x2C1B3C6Du;
    h ^= h >> 12;
    return h % 100u;
}
/*
 * Brennan 2026-11-23 06:28 (Route 114's west plateau: "Scatter some trees, let me see what it looks like"; 07:39 "match
 * the trees with the surrounding ones ... I like the grouping"): on a flat fill of plain ground flagged scatter
 * (relief.bin, add_relief_edge.py --scatter) trees stand in loose groups - the map's own lone tree where its tileset
 * has one (Fallarbor's, two cells wide: voxel_tree.c), else the General's small tree. Whether (x, y) is such a tree's
 * left cell: 4x3-cell patches, about two in five of them wooded; never within two cells of another map's belt or a fill
 * of another height (the seam's ground steps: a wood fill made stretched trees down it, 2026-11-23). VoxelMesh_LedgeTree
 * keeps them off steps. WJPC_NO_PLATEAU_TREES=1: none.
 */
bool VoxelMesh_PlateauTree(int x, int y)
{
    static int sOff = -1;
    VoxelEdgeFill f;
    const VoxelMapInstance *r;

    if (sOff < 0)
        sOff = getenv("WJPC_NO_PLATEAU_TREES") != NULL;
    if (sOff || (x & 1) != 0 || VoxelWorld_GetInstanceAt(x, y) != NULL || (r = VoxelWorld_BeltRing(x, y)) == NULL
     || !VoxelRelief_EdgeFillAt(r, x, y, &f) || !f.scatter || !f.flat || !f.hasTop || f.water)
        return false;
    if (WoodHash(x >> 2, y / 3) % 5u >= 2u || TreeScatter(x, y) >= 50u)
        return false;
    for (int dy = -2; dy <= 2; ++dy)
        for (int dx = -2; dx <= 3; ++dx)
        {
            const VoxelMapInstance *nr;
            VoxelEdgeFill nf;
            if (VoxelWorld_GetInstanceAt(x + dx, y + dy) != NULL)
                continue;
            nr = VoxelWorld_BeltRing(x + dx, y + dy);
            if (nr != r || !VoxelRelief_EdgeFillAt(nr, x + dx, y + dy, &nf) || !nf.flat || nf.top != f.top)
                return false;
        }
    return true;
}

bool VoxelMesh_LedgeTree(int x, int y, float *level, float *shift)
{
    float lo = 1e9f, hi = -1e9f, sh = 0.0f, wl, ws;
    const VoxelMapInstance *ring;
    bool wood = false;

    if (!FieldOn() || getenv("WJPC_NO_LEDGE_TREES") != NULL || !BorderAt(x, y)
     || VoxelWorld_GetInstanceAt(x, y) != NULL || BeltIsTree(x, y) || BorderWaterLevel(x, y, &wl, &ws))
        return false;
    if ((((unsigned)(x * 73856093) ^ (unsigned)(y * 19349663)) % 3u) == 0u)
        return false;
    /* Brennan 2026-11-22 02:10 (Route 112's south, red box: "a small cluster of 3D trees sticking down onto the bare
     * rocky mountainside below the grass forest line ... Remove those trees ... so the forest ends cleanly at the
     * grass edge and only the bare rock mountainside remains below it"): no ledge tree in front of a map (a map cell
     * within 4 rows north of it) - that land is the bare rock the map's edge drops to. WJPC_LEDGE_TREES_SOUTH=1: as
     * before. */
    if (getenv("WJPC_LEDGE_TREES_SOUTH") == NULL)
        for (int k = 1; k <= 4; ++k)
            if (VoxelWorld_GetInstanceAt(x, y - k) != NULL)
                return false;
    for (int dy = -BELT_LEDGE_WOOD; dy <= BELT_LEDGE_WOOD && !wood; ++dy)
        for (int dx = -BELT_LEDGE_WOOD; dx <= BELT_LEDGE_WOOD && !wood; ++dx)
        {
            int nx = x + dx, ny = y + dy;
            wood = VoxelWorld_GetInstanceAt(nx, ny) != NULL ? VoxelTree_PartAt(nx, ny) >= 0
                 : BorderAt(nx, ny) && BeltIsTree(nx, ny);
        }
    /* POL-46 (Brennan 22:39: "Mirror the tree placement onto the western shelves too"): on the mountain behind a map
     * whose belt is its drawing (Lavaridge), its shelves two levels or more over the map's rock carry trees too */
    if (!wood && getenv("WJPC_FIELD_NO_PEAK") == NULL)
    {
        /* (the drawn-belt map north of which it stands - not the cell's ring, which up there is the nearer map
         * across the belt: no trees came, 23:31 picture) */
        const VoxelMapInstance *r = NULL;
        float fh, fs;
        for (unsigned i = 0; i < VoxelWorld_InstanceCount() && r == NULL; ++i)
        {
            const VoxelMapInstance *m = VoxelWorld_Instance(i);
            if (m != NULL && m->layout != NULL && !m->indoor && VoxelRelief_Belt(m) != 0 && y < m->originY
             && y >= m->originY - 24 && x >= m->originX - 2 && x < m->originX + m->width + 2)
                r = m;
        }
        BeltField(x, y, &fh, &fs);
        /* in clumps, as the east slope's (a tree on every shelf read as an orchard): 3x3-cell patches, about half
         * of them wooded */
        unsigned patch = ((unsigned)((x >> 2) * 83492791) ^ (unsigned)((y >> 1) * 2971215073u)) % 5u;
        /* (Brennan 23:31: none on the far-west curved terraces - "a bare volcanic rock mountain"; on the mountain
         * north of the town "only a few scattered trees on each step ... not neat rows") */
        wood = r != NULL && VoxelRelief_Belt(r) != 0 && y < r->originY
            && x >= r->originX - 2 && x < r->originX + r->width + 2
            && fh >= VoxelRelief_Base(r) + (float)VoxelRelief_Belt(r) + 2.0f
            && (TreeScatter(x, y) < 12u || getenv("WJPC_PEAK_TREES_ALL") != NULL);
    }
    /* Brennan 2026-11-23 06:28 (Route 114's west plateau: "Scatter some trees") - VoxelMesh_PlateauTree */
    bool plateau = !wood && VoxelMesh_PlateauTree(x, y);
    wood = wood || plateau;
    if (!wood)
        return false;
    /* Brennan 2026-11-23 12:18 (Route 114: "a few trees that weren't changed. Change these to the surrounding trees"):
     * where the map's own lone tree stands instead (VoxelTree_OwnLoneLedge: two cells wide) a tree starts only on an
     * even column, the shelf flat under both its cells */
    bool wide = plateau || VoxelTree_OwnLoneLedge(x, y);
    if (wide && (x & 1) != 0)
        return false;
    /* the shelf all round it flat too (Brennan: "Snap tree positions onto the flat centers of the terrace shelves"
     * - a tree on a shelf's edge cell hung its crown over the drop) */
    /* (its own corners and the next row's south of it: the shelf in front of the trunk, where the crown leans;
     * a 3x3 flat patch was too much for 2-square shelves - no trees at all on them) */
    for (int vy = y; vy <= y + 1; ++vy)
        for (int vx = x; vx <= x + (wide ? 2 : 1); ++vx)
        {
            float fh, fs;
            BeltField(vx, vy, &fh, &fs);
            fh = floorf(fh + 1e-4f);
            lo = fh < lo ? fh : lo;
            hi = fh > hi ? fh : hi;
            if (vx >= x && vx <= x + 1 && vy >= y && vy <= y + 1)
                sh += fs * 0.25f;
        }
    if (hi != lo)
        return false;   /* a wall crosses it or its neighbours: no tree on or over a cliff face */
    ring = VoxelWorld_BeltRing(x, y);
    *level = lo - (ring != NULL ? VoxelRelief_Base(ring) : 0.0f);
    *shift = sh - (ring != NULL ? VoxelRelief_Base(ring) : 0.0f);
    return true;
}

void VoxelMesh_EmitBorder(VoxelBuilder *builder, int x0, int y0, int x1, int y1)
{
    float u0, v0, u1, v1;
    unsigned drawn, courses;

    if (builder->atlas == NULL)
        return;
    /* Indoors the border is the black filler outside the rooms: stood up it
     * was a black wall two tiles tall round the room, and the south one hid
     * the doorway. The 2D game shows black there, and so does the clear. */
    if (VoxelWorld_Instance(0) != NULL && VoxelWorld_Instance(0)->indoor)
        return;
    /* A belt laid as the drawing (above): each cell stands `drawn` courses,
     * that many rows south. Its cells are the window's; their blocks may
     * stand past it. */
    /*
     * POL-24/30/43 (Brennan 2026-11-16: "when going from route 112 to lavaridge the backgrounds disappear in the red
     * boxed areas"): whether the belt is laid as the drawing (a map's VoxelRelief_Belt, Lavaridge's) was the map the
     * player stood on's: crossing from Route 112 into Lavaridge switched the WHOLE belt - Route 112's side too -
     * from standing blocks to Lavaridge's drawn rock, and back. Each cell now goes with its own map (BeltRing), as
     * its art, height and base already do. WJPC_BELT_DRAWN_GLOBAL=1: as before.
     */
    static int sDrawnGlobal = -1;
    if (sDrawnGlobal < 0)
        sDrawnGlobal = getenv("WJPC_BELT_DRAWN_GLOBAL") != NULL;
    const unsigned drawnHere = VoxelWorld_Instance(0) != NULL ? VoxelRelief_Belt(VoxelWorld_Instance(0)) : 0u;
    const float callerBase = builder->base;
    for (int y = y0; y < y1; ++y)
        for (int x = x0; x < x1; ++x, builder->base = callerBase)
        {
            const VoxelMapInstance *own = sDrawnGlobal ? NULL : VoxelWorld_BeltRing(x, y);

            drawn = sDrawnGlobal ? drawnHere : own != NULL ? VoxelRelief_Belt(own) : 0u;
            courses = drawn ? drawn : VOXEL_BORDER_COURSES;
            float wx = (float)x, wz = (float)(y + (int)drawn);
            /* Only where the border meets something else: the inside of a
             * belt that reaches the whole way to the horizon is never seen,
             * and drawing it is most of the tiles in the window. */
            bool south, east, west;
            float level, shift, lift = 0.0f;

            if (!BorderAt(x, y) || !BeltInPass(x, y))
                continue;
            /* POL-30: each cell over the base of the map it stands with, not the current map's (a drawn cell too,
             * now that it goes with its own map) */
            if (!drawn || !sDrawnGlobal)
                builder->base = BeltBase(x, y);
            if (BorderWaterLevel(x, y, &level, &shift))
            {
                if (getenv("WJPC_FIELD_DUMP") != NULL)
                    fprintf(stderr, "WATER %d %d %.2f %.2f base %.2f mt %d uv %d pass %p\n", x, y, level, shift, builder->base,
                            VoxelWorld_BorderMetatile(x, y),
                            (int)MetatileUV(builder, VoxelWorld_BorderMetatile(x, y), &u0, &v0, &u1, &v1),
                            (const void *)sBeltPassPrimary);
                if (getenv("WJPC_FIELD_DUMP") != NULL)
                {
                    const VoxelMapInstance *am = VoxelWorld_BorderArtMap(x, y), *rg = VoxelWorld_BeltRing(x, y);
                    fprintf(stderr, "ART %d %d art %p (%d,%d) ring %p (%d,%d) uv %.4f %.4f %.4f %.4f\n", x, y, (const void *)am,
                            am ? am->originX : 0, am ? am->originY : 0, (const void *)rg, rg ? rg->originX : 0,
                            rg ? rg->originY : 0, u0, v0, u1, v1);
                }
                /* flat, and no faces: the map's own edge closes any step */
                if (MetatileUV(builder, VoxelWorld_BorderMetatile(x, y), &u0, &v0, &u1, &v1))
                {
                    VoxelMesh_Top(builder, wx, wz + shift, level, 0.0f,
                                  u0, v0, u1, v1, SHADE_TOP);
                    BeltWaterSkirts(builder, x, y, wx, wz, level, shift, u0, v0, u1, v1);
                    BeltFlatSteps(builder, x, y, level, shift, u0, v0, u1, v1);
                }
                continue;
            }
            if (FieldOn() && !BeltIsTree(x, y))
            {
                EmitFieldCell(builder, x, y);
                continue;
            }
            /* Brennan 2026-11-22 13:37 (B2, Verdanturf's west woods: "grey/snow-covered cliff blocks are jutting
             * straight through the middle of the 3D trees"): a wood of large trees stood on its own flat ground at
             * its edge's level, a block a cell with step faces - on a slope (the woods climbing north-west of
             * Verdanturf) the blocks stood up between the crowns. Its floor is the field's now, laid as one smooth
             * slope in the crowns' shade (EmitFieldCell); the trees keep their own level. WJPC_WOOD_BLOCKS=1: blocks. */
            {
                static int sBlocks = -1;
                if (sBlocks < 0)
                    sBlocks = getenv("WJPC_WOOD_BLOCKS") != NULL;
                if (!sBlocks && !drawn && FieldOn() && BeltIsTree(x, y) && BeltTreeOwn(x, y))
                {
                    /* Brennan 2026-11-22 (C7, the woods north of Fallarbor: a slit of sky down the wood's floor): a point
                     * stands as far south as it is high, the map's base counted (VoxelBuilder_StretchZ adds the cell's
                     * base to z) - two wood cells over different maps' bases (Fallarbor's 2, the belt west of it 0) put
                     * their shared corner rows apart. Each corner's way south is over this cell's base now, as its
                     * height already was. WJPC_WOOD_Z_OLD=1: as before. */
                    static int sWoodZOld = -1;
                    if (sWoodZOld < 0)
                        sWoodZOld = getenv("WJPC_WOOD_Z_OLD") != NULL;
#define ZBase(cx, cy) (sWoodZOld ? 0.0f : BeltBase(cx, cy) - builder->base)
                    /* each corner the mean of the trees' levels round it (as they stand: VoxelMesh_BorderLevel) */
                    float ch[4], cs[4];
                    bool ok = true;
                    for (int k = 0; k < 4 && ok; ++k)
                    {
                        int vx = x + (k & 1), vy = y + (k >> 1), n = 0;
                        float sh = 0.0f, ss = 0.0f;
                        for (int q = 0; q < 4; ++q)
                        {
                            int cx = vx - 1 + (q & 1), cy = vy - 1 + (q >> 1);
                            float e, es;
                            /* (every wood cell round it, over its own base: the field's woods' floor meets it) */
                            if (VoxelWorld_GetInstanceAt(cx, cy) == NULL && BorderAt(cx, cy) && BeltIsTree(cx, cy)
                             && VoxelMesh_BorderLevel(cx, cy, &e, &es))
                                sh += BeltBase(cx, cy) - builder->base + e, ss += es + ZBase(cx, cy), ++n;
                        }
                        ok = n > 0;
                        if (ok)
                            ch[k] = sh / (float)n, cs[k] = ss / (float)n;
                        if (ok && n < 4)
                        {
                            /* the wood's edge: the field's own height there, which its neighbour meets */
                            float fh, fs;
                            BeltField(vx, vy, &fh, &fs);
                            ch[k] = fh - builder->base;
                            cs[k] = fs - (sWoodZOld ? 0.0f : builder->base);
                        }
                    }
                    float u0f, v0f, u1f, v1f;
                    /* (the tree on this cell left out: VoxelMesh_WoodGap, by the tree's left cell - own woods' trees
                     * are two cells wide, a trunk's half each) */
                    bool gapHere = false;
                    {
                        const VoxelMapInstance *ga = VoxelWorld_BorderArtMap(x, y);
                        int gp = ga != NULL ? VoxelTree_Part(ga->secondaryTileset, VoxelWorld_BorderMetatile(x, y)) : -1;
                        gapHere = gp >= 2 && gp <= 3 && VoxelMesh_WoodGap(x - (gp & 1), y);
                    }
                    int gapGround = -1;
                    if (gapHere)
                    {
                        const VoxelMapInstance *ga = VoxelWorld_BorderArtMap(x, y);
                        gapGround = ga != NULL ? VoxelTree_OwnWoodGround(ga->secondaryTileset, VoxelWorld_BorderMetatile(x, y),
                                                                         VoxelMesh_WoodScatter(x, y)) : -1;
                    }
                    if (ok && ((gapGround >= 0 && MetatileUV(builder, gapGround, &u0f, &v0f, &u1f, &v1f))
                            || MetatileUV(builder, WoodFloorTile(x, y), &u0f, &v0f, &u1f, &v1f)
                            || MetatileUV(builder, BeltTreeGround(x, y), &u0f, &v0f, &u1f, &v1f)))
                    {
                        VoxelVertex v[4];
                        for (int k = 0; k < 4; ++k)
                        {
                            v[k].x = (float)(x + (k & 1));
                            v[k].y = ch[k] - WoodFloorDrop(); /* (under the trees' own ground: no fighting) */
                            v[k].z = (float)(y + (k >> 1)) + cs[k];
                            v[k].u = k & 1 ? u1f : u0f;
                            v[k].v = k >> 1 ? v1f : v0f;
                            static float sWoodShade = -1.0f;
                            if (sWoodShade < 0.0f)
                                sWoodShade = getenv("WJPC_WOOD_FLOOR_SHADE") ? (float)atof(getenv("WJPC_WOOD_FLOOR_SHADE")) : 0.35f;
                            v[k].shade = SHADE_TOP * sWoodShade; /* the floor under a wood, in its shade (also with Gamma lighting) */
                            if (gapHere)
                                v[k].shade = SHADE_TOP * 0.85f; /* (an open patch of a thinned wood: in the light) */
                        }
                        if (getenv("WJPC_WOOD_DUMP") != NULL) /* test aid: the wood floor's corners */
                            fprintf(stderr, "WOODFLOOR %d %d gap %d base %.3f y %.3f %.3f %.3f %.3f z %.3f %.3f %.3f %.3f\n", x, y,
                                    (int)gapHere, builder->base, v[0].y, v[1].y, v[2].y, v[3].y, v[0].z, v[1].z, v[2].z, v[3].z);
                        VoxelBuilder_Tri(builder, &v[0], &v[3], &v[1]);
                        VoxelBuilder_Tri(builder, &v[0], &v[2], &v[3]);
#undef ZBase
                        continue;
                    }
                }
            }
            {
                float edge, edgeShift;

                if (sBeltSlopes < 0)
                    sBeltSlopes = getenv("WJPC_BELT_BLOCKS") == NULL && getenv("WJPC_BELT_TIERS_ALL") == NULL;
                if (VoxelMesh_BorderLevel(x, y, &edge, &edgeShift)
                 /* POL-46: a wood on the field (its ground eased with the field's) stood step faces of its own
                  * tree drawing between neighbours a fraction of a level apart: the dark flat tree squares
                  * between the crowns west of Route 119's Weather Institute. The crowns cover the rise. */
                 && !(FieldOn() && BeltIsTree(x, y) && !BeltTreeOwn(x, y)))
                {
                    if (!(sBeltSlopes && !drawn && !BeltIsTree(x, y) && !TerraceBand(x, y)))
                        EmitBorderSteps(builder, x, y, edge, edgeShift);
                    else
                        BeltSlopeSteps(builder, x, y, edge, edgeShift);
                }
                wz += edgeShift;
                lift = edge;
                if (drawn)
                {
                    /* (a ledge up is drawn a row further south, as every point of the drawing) */
                    float tier = DrawnTierLift(x, y);
                    lift += tier;
                    if (!TerraceSouthOff())
                        wz += tier;
                }
            }
            if (BeltIsTree(x, y))
            {
                /* POL-30: at the edge of the trees (a cell of another map's belt beside it) the sky showed under
                 * the leaning crowns: the cell's own drawing laid as ground there. */
                bool besideMap = VoxelWorld_GetInstanceAt(x - 1, y) || VoxelWorld_GetInstanceAt(x + 1, y)
                              || VoxelWorld_GetInstanceAt(x, y - 1) || VoxelWorld_GetInstanceAt(x, y + 1);

                /* POL-46: a wood on the field lies on the field itself (EmitFieldCell, in its grass): one ground
                 * with no gaps between cells - flat quads at eased heights showed the sky between them */
                if (!drawn && FieldOn() && !BeltTreeOwn(x, y))
                {
                    EmitFieldCell(builder, x, y);
                    continue;
                }
                /* Brennan 2026-11-20 23:18 (Route 121's west woods: "a light-blue skybox sliver is bleeding
                 * through the seam between the western trees"): inside a wood (trees all round) the cells had no
                 * ground - the crowns were meant to cover it, and where two columns of trees stand a little apart
                 * the sky showed between them in thin lines. Every wood cell has its ground now, shaded as the
                 * floor under a wood (darker than open grass). WJPC_BELT_WOOD_NO_FLOOR=1: as before. */
                static int sNoFloor = -1;
                bool inner = !besideMap && BeltIsTree(x - 1, y) && BeltIsTree(x + 1, y) && BeltIsTree(x, y - 1)
                          && BeltIsTree(x, y + 1);
                if (sNoFloor < 0)
                    sNoFloor = getenv("WJPC_BELT_WOOD_NO_FLOOR") != NULL;
                if (!drawn && inner && !sNoFloor && MetatileUV(builder, BeltTreeGround(x, y), &u0, &v0, &u1, &v1))
                    VoxelMesh_Top(builder, wx, wz, lift, 0.0f, u0, v0, u1, v1, VOXEL_FACE_SHADE(0.55f));
                else if (!drawn && (besideMap
                               || !BeltIsTree(x - 1, y) || !BeltIsTree(x + 1, y) || !BeltIsTree(x, y - 1)
                               || !BeltIsTree(x, y + 1))
                 && MetatileUV(builder, BeltTreeGround(x, y), &u0, &v0, &u1, &v1))
                {
                    if (getenv("WJPC_FIELD_DUMP") != NULL)
                        fprintf(stderr, "TREEGROUND %d %d besideMap %d n %d%d%d%d\n", x, y, (int)besideMap,
                                (int)BeltIsTree(x - 1, y), (int)BeltIsTree(x + 1, y), (int)BeltIsTree(x, y - 1),
                                (int)BeltIsTree(x, y + 1));
                    VoxelMesh_Top(builder, wx, wz, lift, 0.0f, u0, v0, u1, v1, SHADE_TOP);
                    if (besideMap)
                        BeltTreeSeam(builder, x, y, lift, wz - (float)y, u0, v0, u1, v1);
                }
                continue; /* flat trunks and tilted crowns are appended later */
            }
            /* a standing cell beside a flat one of water shows its face too */
            /* (POL-30: and beside a tree cell, which stands the courses lower - EmitBorderSteps) */
            south = !BorderAt(x, y + 1) || BorderWaterLevel(x, y + 1, &level, &shift) || BeltIsTree(x, y + 1);
            east = !BorderAt(x + 1, y) || BorderWaterLevel(x + 1, y, &level, &shift) || BeltIsTree(x + 1, y);
            west = !BorderAt(x - 1, y) || BorderWaterLevel(x - 1, y, &level, &shift) || BeltIsTree(x - 1, y);
            /* Laid as drawn, a cell's top is its own drawing; stood where it
             * is tiled, the column reads the rows above it. */
            bool slopedTop = false;
            if (MetatileUV(builder, BeltMetatileFor(x, drawn ? y : y - (int)courses + 1, y),
                           &u0, &v0, &u1, &v1))
            {
                float ct[4], cz[4];
                const int kc[4][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 1} };
                bool sloped = sBeltSlopes && !drawn && !TerraceBand(x, y);

                for (int k = 0; k < 4 && sloped; ++k)
                    sloped = BeltCorner(x + kc[k][0], y + kc[k][1], builder->base, &ct[k], &cz[k]);
                slopedTop = sloped;
                if (sloped)
                {
                    int topTile = BeltTopTile(VoxelWorld_BeltRing(x, y));

                    float tu0, tv0, tu1, tv1;

                    if (topTile >= 0 && MetatileUV(builder, topTile, &tu0, &tv0, &tu1, &tv1))
                        u0 = tu0, v0 = tv0, u1 = tu1, v1 = tv1;
                    /* a steep slope is cut a level a piece, each with the whole drawing (one drawing pulled
                     * over a tall slope was streaks, as EmitBorderNorth's wall) */
                    float lo = fminf(fminf(ct[0], ct[1]), fminf(ct[2], ct[3]));
                    float hi = fmaxf(fmaxf(ct[0], ct[1]), fmaxf(ct[2], ct[3]));
                    int n = (int)ceilf(hi - lo - 0.25f);

                    n = n < 1 ? 1 : n > 12 ? 12 : n;
                    for (int j = 0; j < n; ++j)
                        for (int i = 0; i < n; ++i)
                        {
                            VoxelVertex q[4];
                            const int di[4] = { 0, 1, 1, 0 }, dj[4] = { 0, 0, 1, 1 };

                            for (int k = 0; k < 4; ++k)
                            {
                                float a = (float)(i + di[k]) / n, c = (float)(j + dj[k]) / n;
                                float t = (ct[0] * (1 - a) + ct[1] * a) * (1 - c) + (ct[3] * (1 - a) + ct[2] * a) * c;
                                float zz = (cz[0] * (1 - a) + cz[1] * a) * (1 - c) + (cz[3] * (1 - a) + cz[2] * a) * c;

                                q[k] = (VoxelVertex){ wx + a, t, (float)y + c + zz, di[k] ? u1 : u0, dj[k] ? v1 : v0,
                                                      SHADE_TOP };
                            }
                            VoxelBuilder_Quad(builder, &q[0], &q[1], &q[2], &q[3]);
                        }
                }
                else
                    VoxelMesh_Top(builder, wx, wz, (float)courses + lift, 0.0f,
                                  u0, v0, u1, v1, SHADE_TOP);
            }
            /* Laid as drawn it meets the map at its own height: the map's
             * faces close every step there, and a wall of the belt's would
             * stand in the same place as the cave's north wall. */
            /* Wally's Journey PC: the wider view sees between a drawn belt's
             * rows of tops, so it keeps its faces too. */
            if (slopedTop)
            {
                /* POL-40: water and trees beside it have their walls in BeltSlopeSteps */
                south = !BorderAt(x, y + 1);
                east = !BorderAt(x + 1, y);
                west = !BorderAt(x - 1, y);
            }
            BeltCloseSeams(builder, x, y, drawn, wz);
            if (!south && !east && !west)
                continue;
            for (int h = 0; h < (int)courses; ++h)
            {
                float yBottom = (float)h + lift, yTop = (float)(h + 1) + lift;

                if (!MetatileUV(builder, BeltMetatileFor(x, drawn ? y : y - h, y),
                                &u0, &v0, &u1, &v1))
                    continue;
                if (south) VoxelMesh_South(builder, wx, wz, yBottom, yTop, 0.0f, u0, v0, u1, v1);
                if (east) VoxelMesh_East(builder, wx, wz, yBottom, yTop, 0.0f, u0, v0, u1, v1);
                if (west) VoxelMesh_West(builder, wx, wz, yBottom, yTop, 0.0f, u0, v0, u1, v1);
            }
        }
}
