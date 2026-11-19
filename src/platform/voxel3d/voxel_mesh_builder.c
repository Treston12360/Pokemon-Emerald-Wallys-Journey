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

static int sWindowX0, sWindowY0, sWindowW, sWindowH;

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
static uint8_t sShapes[VOXEL_WINDOW_MAX * VOXEL_WINDOW_MAX];

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

static void EmitRelief(VoxelBuilder *b, int x, int y, const int16_t *g, const int16_t *s,
                       int artY, int metatile, bool onFloor, int floor, const uint8_t *rows)
{
    float u0, v0, u1, v1;

    if (metatile >= 0 ? !MetatileUV(b, metatile, &u0, &v0, &u1, &v1)
                      : !VoxelMesh_TileUV(b, x, artY, &u0, &v0, &u1, &v1))
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

/*
 * POL-46 (Brennan 2026-11-17 21:00: "Completely eliminate the blue triangle gap at the seam"; "tile the rock texture
 * cleanly by world-space height ... instead of stretching UVs"). A map's west, east or north edge standing over the
 * belt beside it (Route 112's mountain over the belt north of Lavaridge) was open down to the belt: only an edge
 * under the belt had a wall. It stands a wall of the map's cliff face from the belt's top up to the edge, point by
 * point of the edge, a tile of the face a level (by world height, never stretched). WJPC_EDGE_OPEN=1: as before.
 */
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

    if (sOpen < 0)
        sOpen = getenv("WJPC_EDGE_OPEN") != NULL;
    if (sOpen || grid == NULL || d == 1 || !BeltTopWorld(nx, ny, &beltTop, &beltZ, &drawn))
        return;
    /* down to the belt's ground: the belt cell beside the wall's foot is drawn rows away (each stands as far south
     * as it is high), so the wall goes to the ground and the belt in front hides what is under its tops */
    beltTop = BeltBase(nx, ny) - base;
    for (unsigned k = 0; k < VOXEL_RELIEF_SIDE; ++k)
    {
        unsigned i = d == 0 ? k : k * VOXEL_RELIEF_SIDE + (d == 2 ? 0 : last);
        h[k] = grid[i] / 16.0f;
        z[k] = (float)y + (d == 0 ? 0.0f : k / (float)last) + (depth ? depth[i] : grid[i]) / 16.0f;
        if (h[k] > top)
            top = h[k];
    }
    if (top <= beltTop + 0.05f)
        return;
    face = VoxelRelief_FaceTile(inst);
    /* beside the drawn belt's terraces, its rock (one mountain): the drawn map's cliff face */
    for (unsigned i = 0; i < VoxelWorld_InstanceCount(); ++i)
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
    if (face < 0 || !MetatileUV(b, face, &u0, &v0, &u1, &v1))
    {
        if (!VoxelMesh_TileUV(b, x, y, &u0, &v0, &u1, &v1))
            return;
    }
    for (unsigned k = 0; k < last; ++k)
    {
        float xa = d == 0 ? (float)x + k / (float)last : d == 2 ? (float)x : (float)x + 1.0f;
        float xb = d == 0 ? (float)x + (k + 1) / (float)last : xa;
        float hi = h[k] > h[k + 1] ? h[k] : h[k + 1];
        float shade = d == 0 ? SHADE_NORTH : d == 2 ? SHADE_WEST : SHADE_EAST;
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
        if (other == NULL && !inst->indoor && d != 1)
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

            if (ring == NULL || VoxelRelief_Belt(ring) != 0)
                continue;
            /* a belt of water lies at its own level (BorderWaterLevel), a
             * belt of land at its edge's (VoxelMesh_BorderLevel) */
            if (BorderWaterLevel(nx, ny, &level, &shift) || VoxelMesh_BorderLevel(nx, ny, &level, &shift))
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
            if (!uv && !(uv = VoxelMesh_TileUV(b, x, y, &u0, &v0, &u1, &v1)))
                return;
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
                        &(VoxelVertex){xb, belt,         (float)y + beltZ, ub, v0,              SHADE_SOUTH},
                        &(VoxelVertex){xa, belt,         (float)y + beltZ, ua, v0,              SHADE_SOUTH});
                }
                else
                {
                    float za = (float)y + along[k] + beltZ, zb = (float)y + along[k + 1] + beltZ;
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
            continue;
        }
        if (other == NULL && d == 1 && !inst->indoor && TreeBeltAt(nx, ny))
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
            const float beltZ = (float)y + 1.0f + belt / 16.0f;
            float level, shift;

            if (oursH == NULL || VoxelRelief_Belt(ring) != 0)
                continue;
            if (VoxelMesh_BorderLevel(nx, ny, &level, &shift))
                belt += level * 16.0f;
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
            static int16_t under[VOXEL_RELIEF_SIDE * VOXEL_RELIEF_SIDE];
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
        for (int e = 0; e < 2; ++e)
            bot[e] -= 1.0f;
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
    if (sOff || !VoxelMesh_TileUV(b, x, y, &u0, &v0, &u1, &v1))
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
    b->lift = low - 1.0f / 16.0f;
    b->shift = b->lift;
    VoxelMesh_Top(b, (float)x, (float)y, 0.0f, 0.0f, u0, v0, u1, v1, SHADE_TOP);
    b->lift = 0.0f;
    b->shift = 0.0f;
}

void VoxelMesh_EmitGroundRow(VoxelBuilder *builder, const VoxelMapInstance *inst,
                             int x0, int x1, int y)
{
    for (int x = x0; x < x1; ++x)
        EmitSafetyFloor(builder, inst, x, y);
    for (int x = x0; x < x1; ++x)
        if (x == inst->originX || x == inst->originX + inst->width - 1
         || y == inst->originY || y == inst->originY + inst->height - 1)
            EmitSeamSkirt(builder, inst, x, y);
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
                if (cut >= 0)
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

static bool BorderWaterLevel(int x, int y, float *level, float *shift)
{
    /* POL-30: over the base of the map the cell stands with (BeltBase), at that map's sea's level - it was
     * always the current map's, so the belt beside the next map moved at the crossing. */
    const VoxelMapInstance *ring = VoxelWorld_BeltRing(x, y);
    int ex, ey;

    VoxelEdgeFill fill;

    if (ring == NULL || ring->indoor || VoxelRelief_Belt(ring) != 0
     || !VoxelWorld_BorderIsWater(x, y))
        return false;
    /* DLX-26 (owner's choice 2026-11-13, replaces the skirt walls): the whole belt of sea lies flat, at one
     * level - the level most of the map's own sea lies at - so no steps (and no lines) between belt cells. */
    if (getenv("WJPC_BELT_SEA_STEPS") == NULL)
    {
        /* (POL-30: one entry per map the belt stands with, not one in all) */
        static int sKeys[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
        static float sLevels[8], sShifts[8];
        static unsigned sNext;
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
    if (FieldOn() && BeltIsTree(x, y) && getenv("WJPC_BELT_TREES_OWN") == NULL)
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
        /* POL-40 (Route 118's north: the fill stood the mountain's whole height out to the horizon, its side to
         * the sea a stair of tall dark walls): rounded off as the belt's own mountain is (WJPC_BELT_COLUMNS=1 not) */
        if (getenv("WJPC_BELT_COLUMNS") == NULL && !NearDrawnBelt(x, y, 8))
        {
            int fdx = x > ex ? x - ex : ex - x, fdy = y > ey ? y - ey : ey - y;

            BeltRoundOff(x, y, fdx > fdy ? fdx : fdy, *level, *shift, level, shift);
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
static int BeltTreeGround(int x, int y)
{
    const VoxelMapInstance *art = VoxelWorld_BorderArtMap(x, y);
    int mt = VoxelWorld_BorderMetatile(x, y), g;

    if (art == NULL || getenv("WJPC_BELT_TREE_ART_GROUND") != NULL)
        return mt;
    g = VoxelTree_GroundMetatile(art->secondaryTileset, mt);
    /* the General's small trees (016-017, 0C6-0C7) name no ground of their own: its plain grass, 001 */
    if (g == mt && (mt == 0x016 || mt == 0x017 || mt == 0x0C6 || mt == 0x0C7))
        g = 0x001;
    return g >= 0 ? g : mt;
}

static bool BeltIsTree(int x, int y)
{
    const VoxelMapInstance *current = VoxelWorld_BorderArtMap(x, y);

    return current != NULL && VoxelWorld_UsesTreeSprites(current)
        && VoxelTree_Part(current->secondaryTileset, VoxelWorld_BorderMetatile(x, y)) >= 0;
}

/*
 * POL-30 (Brennan 2026-11-15: "fix route 113"): the belt is emitted once per atlas it is drawn with - the
 * current map's, and those of loaded maps on other tile sets whose border the belt shows (voxel_gl.c's belt
 * layers). A pass only lays the cells whose metatile is of its tile sets.
 */
static const void *sBeltPassPrimary, *sBeltPassSecondary;

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

static void EmitBorderSteps(VoxelBuilder *builder, int x, int y, float level, float shift)
{
    /* POL-30 (Brennan 2026-11-15: blue saw teeth past Fallarbor, where its filled edge meets Route 113's
     * trees): a tree cell's foot is the belt's courses under a standing cell's top, and nothing closed that
     * step - one map's border was all trees or none, so the two never met before the belt took each cell from
     * the nearest map. VoxelMesh_EmitBorder stands the courses' faces beside a tree cell, and lays the ground
     * under the first of the trees. */

    static const int kDir[3][2] = { {0, 1}, {1, 0}, {-1, 0} };
    float u0, v0, u1, v1, other, otherShift;

    if (!MetatileUV(builder, VoxelWorld_BorderMetatile(x, y), &u0, &v0, &u1, &v1))
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
        EmitBorderNorth(builder, x, y, low, otherShift, level + (float)VOXEL_BORDER_COURSES, shift,
                        u0, v0, u1, v1);
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
    static const VoxelMapInstance *sRing[8];
    static int sTile[8], sNext;
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
    if (!MetatileUV(builder, VoxelWorld_BorderMetatile(x, y), &u0, &v0, &u1, &v1))
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
enum { BELT_FIELD_REACH = 16, BELT_FIELD_RATE = 2, BELT_FIELD_RISE = 6, BELT_FIELD_SOUTH = 3, BELT_FIELD_PEAK = 14 };

static bool FieldOn(void)
{
    static int sOn = -1;
    if (sOn < 0)
        sOn = getenv("WJPC_BELT_BLOCKS_OLD") == NULL;
    return sOn != 0;
}

static void BeltField(int vx, int vy, float *height, float *south)
{
    /* blended over the maps by 1/d^3 (the nearest map's edge wins near it, exactly on it): the edge's height, its way
     * south, and whether the point is off a side the mountain climbs from (north, west, east) or in front (south);
     * then the climb, by the distance to the nearest map. One smooth function: no map's cone outgrowing another's
     * with distance, no cliff where a map drops out of reach (both made walls of columns, 2026-11-18). */
    float sumW = 0.0f, sumE = 0.0f, sumS = 0.0f, sumBack = 0.0f, dmin = 1e9f;
    float exactE = 0.0f, exactS = 0.0f, capMin = 1e9f, sumRise = 0.0f, nearE = 0.0f;
    bool nearSouth = false;
    float drawnPeak = -1e9f;
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
        sd = VoxelRelief_Base(m) + (dg != NULL ? dg[gj * VOXEL_RELIEF_SIDE + gi] / 16.0f
                                    : g != NULL ? g[gj * VOXEL_RELIEF_SIDE + gi] / 16.0f : 0.0f);
        /* POL-46 (Brennan 2026-11-18, Lavaridge's north-east: "Option 2 (One cliff + ledges)"): no point climbs
         * faster than a level every BELT_FIELD_RATE cells from ANY map's edge - from the low town the ledges stay
         * two squares deep; where a high edge (Route 112's) stands over that, its edge's own cliff (EdgeOverBelt,
         * rock tiled a level a tile) makes up the rest. WJPC_FIELD_NO_CAP=1: off. */
        {
            float cap = e + d / (float)BELT_FIELD_RATE;
            capMin = cap < capMin ? cap : capMin;
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
            float r = d / (float)BELT_FIELD_RATE, pk = e + (r < (float)BELT_FIELD_PEAK ? r : (float)BELT_FIELD_PEAK);
            drawnPeak = pk > drawnPeak ? pk : drawnPeak;
        }
        if (d < dmin)
        {
            nearE = e;
            nearSouth = vy >= oy + m->height && vx >= ox && vx <= ox + m->width;
        }
        dmin = d < dmin ? d : dmin;
    }
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
        r = r < rise ? r : rise;
        *height = sumE / sumW + r * (sumBack / sumW);
        if (*height < drawnPeak)
            *height = drawnPeak;
        /* POL-46 (Brennan 22:39, Lavaridge's south: "Make the Top of the Bottom Mountain Flush & Level Across the
         * Whole Width"; a blue triangle under the red-roofed houses): in front of a map (south of it) the land
         * never dips under the map's edge - the blend with farther, lower maps took it to 5.9 under an edge of 6.0,
         * a whole level down in places (sawtooth steps, a gap). WJPC_FIELD_SOUTH_DIP=1: as before. */
        if (nearSouth && *height < nearE && getenv("WJPC_FIELD_SOUTH_DIP") == NULL)
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

typedef struct { float x, y, h, s; } FieldPt;   /* grid position (cells), field height, way south */

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

static void EmitFieldCell(VoxelBuilder *b, int x, int y)
{
    FieldPt c[4];
    float tu0, tv0, tu1, tv1, wu0, wv0, wu1, wv1;
    const VoxelMapInstance *ring = VoxelWorld_BeltRing(x, y);
    int face = ring != NULL ? VoxelRelief_FaceTile(ring) : -1;
    bool wall;
    const float keepBase = b->base, keepLift = b->lift, keepShift = b->shift, keepStretch = b->stretchAdd;

    /* (a wood's cell: the field under its trees, in its grass - BeltTreeGround) */
    if (getenv("WJPC_FIELD_NONE") != NULL
     || !MetatileUV(b, BeltIsTree(x, y) ? BeltTreeGround(x, y) : BeltMetatileFor(x, y, y), &tu0, &tv0, &tu1, &tv1))
        return;
    wall = (face >= 0 && MetatileUV(b, face, &wu0, &wv0, &wu1, &wv1))
        || MetatileUV(b, BeltMetatileFor(x, y, y), &wu0, &wv0, &wu1, &wv1);
    for (int k = 0; k < 4; ++k)
    {
        int vx = x + (k & 1), vy = y + (k >> 1);
        c[k].x = (float)vx;
        c[k].y = (float)vy;
        BeltField(vx, vy, &c[k].h, &c[k].s);
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
            fprintf(stderr, "FIELD %d %d  %.2f/%.2f %.2f/%.2f %.2f/%.2f %.2f/%.2f  ring=%p\n", x, y, c[0].h, c[0].s,
                    c[1].h, c[1].s, c[2].h, c[2].s, c[3].h, c[3].s, (const void *)ring);
    }
    /* world heights and ways south, as they are */
    b->base = 0.0f;
    b->lift = 0.0f;
    b->shift = 0.0f;
    b->stretchAdd = 0.0f;
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
                : !BeltIsTree(nx, ny) || getenv("WJPC_BELT_TREES_OWN") == NULL)
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
    if (!FieldOn() || getenv("WJPC_BELT_TREES_OWN") != NULL || getenv("WJPC_TREES_ON_DROPS") != NULL)
        return false;
    for (int k = 0; k < 4; ++k)
    {
        float fh, fs;
        BeltField(x + (k & 1), y + (k >> 1), &fh, &fs);
        fh = floorf(fh + 1e-4f);
        lo = fh < lo ? fh : lo;
        hi = fh > hi ? fh : hi;
    }
    return hi != lo;
}

enum { BELT_LEDGE_WOOD = 2 };
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
        const VoxelMapInstance *r = VoxelWorld_BeltRing(x, y);
        float fh, fs;
        BeltField(x, y, &fh, &fs);
        wood = r != NULL && VoxelRelief_Belt(r) != 0 && y < r->originY + r->height   /* (not in front of it) */
            && fh >= VoxelRelief_Base(r) + (float)VoxelRelief_Belt(r) + 2.0f;
    }
    if (!wood)
        return false;
    /* the shelf all round it flat too (Brennan: "Snap tree positions onto the flat centers of the terrace shelves"
     * - a tree on a shelf's edge cell hung its crown over the drop) */
    /* (its own corners and the next row's south of it: the shelf in front of the trunk, where the crown leans;
     * a 3x3 flat patch was too much for 2-square shelves - no trees at all on them) */
    for (int vy = y; vy <= y + 2; ++vy)
        for (int vx = x; vx <= x + 1; ++vx)
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
            {
                float edge, edgeShift;

                if (sBeltSlopes < 0)
                    sBeltSlopes = getenv("WJPC_BELT_BLOCKS") == NULL && getenv("WJPC_BELT_TIERS_ALL") == NULL;
                if (VoxelMesh_BorderLevel(x, y, &edge, &edgeShift)
                 /* POL-46: a wood on the field (its ground eased with the field's) stood step faces of its own
                  * tree drawing between neighbours a fraction of a level apart: the dark flat tree squares
                  * between the crowns west of Route 119's Weather Institute. The crowns cover the rise. */
                 && !(FieldOn() && BeltIsTree(x, y) && getenv("WJPC_BELT_TREES_OWN") == NULL))
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
                if (!drawn && FieldOn() && getenv("WJPC_BELT_TREES_OWN") == NULL)
                {
                    EmitFieldCell(builder, x, y);
                    continue;
                }
                if (!drawn && (besideMap
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
