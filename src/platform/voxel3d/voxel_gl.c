// Wally's Journey PC: the 3D overworld drawn with OpenGL.
//
// The world itself - which cells are cliffs, trees and buildings, their meshes, the sprites as
// cards, the camera - is Emerald 3DS's voxel code (src/platform/voxel3d/voxel_*.c, CC BY-NC-SA
// 4.0, see NOTICE.md). Their console backend (ctr_voxel.c, for the 3DS GPU) is replaced by this
// file: no streaming or memory budgets, a PC has room for the whole neighbourhood at once.
//
// Called from VDraw while the game thread waits, so the game's state holds still.
#include <SDL.h>
#include <SDL_opengl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "fieldmap.h"
#include "palette.h"
#include "voxel_world.h"
#include "voxel_atlas.h"
#include "voxel_mesh_builder.h"
#include "voxel_camera.h"
#include "voxel_entities.h"
#include "voxel_building.h"
#include "voxel_tree.h"
#include "voxel_relief.h"
#include "voxel_regions.h"
#include "voxel_sign.h"
#include "voxel_grade.h"
#include "voxel_lighting.h"
#include "voxel_sprite_slots.h"
#include "voxel_gl.h"

void Voxel3D_Log(const char *fmt, ...);

// ---------------------------------------------------------------------------
// OpenGL functions, all looked up through SDL (no link-time GL dependency).

#ifndef APIENTRY
#define APIENTRY
#endif
#define GLFUNCS \
    X(void, glGenTextures, (GLsizei, GLuint *)) \
    X(void, glDeleteTextures, (GLsizei, const GLuint *)) \
    X(void, glBindTexture, (GLenum, GLuint)) \
    X(void, glTexImage2D, (GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *)) \
    X(void, glTexSubImage2D, (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void *)) \
    X(void, glTexParameteri, (GLenum, GLenum, GLint)) \
    X(void, glViewport, (GLint, GLint, GLsizei, GLsizei)) \
    X(void, glClearColor, (GLfloat, GLfloat, GLfloat, GLfloat)) \
    X(void, glClear, (GLbitfield)) \
    X(void, glEnable, (GLenum)) \
    X(void, glDisable, (GLenum)) \
    X(GLboolean, glIsEnabled, (GLenum)) \
    X(void, glDepthFunc, (GLenum)) \
    X(void, glDepthMask, (GLboolean)) \
    X(void, glBlendFunc, (GLenum, GLenum)) \
    X(void, glBlendFuncSeparate, (GLenum, GLenum, GLenum, GLenum)) \
    X(void, glDrawArrays, (GLenum, GLint, GLsizei)) \
    X(void, glGetIntegerv, (GLenum, GLint *)) \
    X(void, glGetFloatv, (GLenum, GLfloat *)) \
    X(void, glPixelStorei, (GLenum, GLint)) \
    X(void, glReadPixels, (GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void *)) \
    X(GLenum, glGetError, (void)) \
    X(void, glActiveTexture, (GLenum)) \
    X(GLuint, glCreateShader, (GLenum)) \
    X(void, glShaderSource, (GLuint, GLsizei, const GLchar *const *, const GLint *)) \
    X(void, glCompileShader, (GLuint)) \
    X(void, glGetShaderiv, (GLuint, GLenum, GLint *)) \
    X(void, glGetShaderInfoLog, (GLuint, GLsizei, GLsizei *, GLchar *)) \
    X(GLuint, glCreateProgram, (void)) \
    X(void, glAttachShader, (GLuint, GLuint)) \
    X(void, glBindAttribLocation, (GLuint, GLuint, const GLchar *)) \
    X(void, glLinkProgram, (GLuint)) \
    X(void, glGetProgramiv, (GLuint, GLenum, GLint *)) \
    X(void, glGetProgramInfoLog, (GLuint, GLsizei, GLsizei *, GLchar *)) \
    X(void, glUseProgram, (GLuint)) \
    X(GLint, glGetUniformLocation, (GLuint, const GLchar *)) \
    X(void, glUniformMatrix4fv, (GLint, GLsizei, GLboolean, const GLfloat *)) \
    X(void, glUniform1i, (GLint, GLint)) \
    X(void, glUniform1f, (GLint, GLfloat)) \
    X(void, glUniform2f, (GLint, GLfloat, GLfloat)) \
    X(void, glUniform3f, (GLint, GLfloat, GLfloat, GLfloat)) \
    X(void, glUniform4f, (GLint, GLfloat, GLfloat, GLfloat, GLfloat)) \
    X(void, glGenBuffers, (GLsizei, GLuint *)) \
    X(void, glDeleteBuffers, (GLsizei, const GLuint *)) \
    X(void, glBindBuffer, (GLenum, GLuint)) \
    X(void, glBufferData, (GLenum, GLsizeiptr, const void *, GLenum)) \
    X(void, glEnableVertexAttribArray, (GLuint)) \
    X(void, glDisableVertexAttribArray, (GLuint)) \
    X(void, glVertexAttribPointer, (GLuint, GLint, GLenum, GLboolean, GLsizei, const void *)) \
    X(void, glGenFramebuffers, (GLsizei, GLuint *)) \
    X(void, glBindFramebuffer, (GLenum, GLuint)) \
    X(void, glFramebufferTexture2D, (GLenum, GLenum, GLenum, GLuint, GLint)) \
    X(void, glGenRenderbuffers, (GLsizei, GLuint *)) \
    X(void, glBindRenderbuffer, (GLenum, GLuint)) \
    X(void, glRenderbufferStorage, (GLenum, GLenum, GLsizei, GLsizei)) \
    X(void, glFramebufferRenderbuffer, (GLenum, GLenum, GLenum, GLuint)) \
    X(GLenum, glCheckFramebufferStatus, (GLenum)) \
    X(void, glDeleteRenderbuffers, (GLsizei, const GLuint *))

#define X(ret, name, args) typedef ret (APIENTRY *PFN_wj_##name) args; static PFN_wj_##name wj_##name;
GLFUNCS
#undef X

static bool LoadGl(void)
{
    bool ok = true;
#define X(ret, name, args) \
    wj_##name = (PFN_wj_##name)SDL_GL_GetProcAddress(#name); \
    if (wj_##name == NULL) { \
        char ext[64]; \
        snprintf(ext, sizeof(ext), "%sEXT", #name); \
        wj_##name = (PFN_wj_##name)SDL_GL_GetProcAddress(ext); \
    } \
    if (wj_##name == NULL) { Voxel3D_Log("[3D] missing GL function %s", #name); ok = false; }
    GLFUNCS
#undef X
    return ok;
}

#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER 0x8D40
#define GL_RENDERBUFFER 0x8D41
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_FRAMEBUFFER_BINDING 0x8CA6
#define GL_DEPTH_COMPONENT24 0x81A6
#endif
#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#define GL_STREAM_DRAW 0x88E0
#define GL_ARRAY_BUFFER_BINDING 0x8894
#endif
#ifndef GL_CURRENT_PROGRAM
#define GL_CURRENT_PROGRAM 0x8B8D
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#endif
#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#define GL_ACTIVE_TEXTURE 0x84E0
#endif
#ifndef GL_UNSIGNED_SHORT_5_5_5_1
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

// ---------------------------------------------------------------------------
// Textures: the voxel modules lay every texture out in the 3DS GPU's tiled order
// (CtrVideo_Texel); here it becomes rows, row 0 = v 0.

uint32_t CtrVideo_Texel(unsigned x, unsigned y, unsigned width);

static uint16_t *sUntile;
static size_t sUntileSize;

static GLuint UploadTiled(GLuint tex, const uint16_t *tiled, unsigned w, unsigned h)
{
    if (sUntileSize < (size_t)w * h)
    {
        free(sUntile);
        sUntileSize = (size_t)w * h;
        sUntile = malloc(sUntileSize * sizeof(uint16_t));
    }
    for (unsigned y = 0; y < h; y++)
        for (unsigned x = 0; x < w; x++)
            sUntile[y * w + x] = tiled[CtrVideo_Texel(x, y, w)];
    if (tex == 0)
        wj_glGenTextures(1, &tex);
    wj_glBindTexture(GL_TEXTURE_2D, tex);
    wj_glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    wj_glPixelStorei(0x0CF2 /* GL_UNPACK_ROW_LENGTH: SDL leaves its own pitch set */, 0);
    wj_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, sUntile);
    wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return tex;
}

// ---------------------------------------------------------------------------
// Shader: texture x the vertex's shade, alpha tested; a palette fade and distance haze on top.

static GLuint sProgram;
static GLint sUniMode, sUniAlphaMin, sUniMvp, sUniOrigin, sUniTex, sUniFade, sUniFog, sUniFogColor, sUniCam;

static const char *sVertexSrc =
    "#version 120\n"
    "uniform mat4 mvp;\n"
    "uniform vec3 origin;\n"
    "uniform vec3 cam;\n"
    "attribute vec3 pos;\n"
    "attribute vec2 uv;\n"
    "attribute float shade;\n"
    "varying vec2 vUv;\n"
    "varying float vShade;\n"
    "varying float vDist;\n"
    "void main() {\n"
    "  vec3 p = pos + origin;\n"
    "  vUv = vec2(uv.x, 1.0 - uv.y);\n" // PICA textures count v from the bottom row

    "  vShade = shade;\n"
    "  vDist = length(p - cam);\n"
    "  gl_Position = mvp * vec4(p, 1.0);\n"
    "}\n";

static const char *sFragmentSrc =
    "#version 120\n"
    "uniform sampler2D tex;\n"
    "uniform float alphaMin;\n"
    "uniform float mode;\n" // 0 solid, 1 shadow (shade = darkness), 2 reflection (half see-through)

    "uniform vec4 fade;\n"
    "uniform vec2 fog;\n"
    "uniform vec3 fogColor;\n"
    "varying vec2 vUv;\n"
    "varying float vShade;\n"
    "varying float vDist;\n"
    "void main() {\n"
    "  vec4 c = texture2D(tex, vUv);\n"
    "  if (c.a < alphaMin) discard;\n"
    "  vec3 rgb = c.rgb * vShade;\n"
    "  float f = clamp((vDist - fog.x) / max(fog.y - fog.x, 0.001), 0.0, 1.0);\n"
    "  rgb = mix(rgb, fogColor, f * f);\n"
    "  rgb = mix(rgb, fade.rgb, fade.a);\n"
    "  gl_FragColor = vec4(rgb, 1.0);\n"
    "  if (mode > 0.5 && mode < 1.5) gl_FragColor = vec4(0.0, 0.0, 0.0, vShade);\n"
    "  if (mode > 1.5) gl_FragColor = vec4(c.rgb, 0.45);\n"
    "  if (alphaMin < -5.0) gl_FragColor = vec4(fract(vUv.x * 32.0), fract(vUv.y * 16.0), 0.0, 1.0);\n"
    "}\n";

static GLuint Compile(GLenum type, const char *src)
{
    GLuint s = wj_glCreateShader(type);
    GLint ok = 0;
    wj_glShaderSource(s, 1, &src, NULL);
    wj_glCompileShader(s);
    wj_glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        wj_glGetShaderInfoLog(s, sizeof(log), NULL, log);
        Voxel3D_Log("[3D] shader: %s", log);
        return 0;
    }
    return s;
}

static bool MakeProgram(void)
{
    GLuint vs = Compile(GL_VERTEX_SHADER, sVertexSrc), fs = Compile(GL_FRAGMENT_SHADER, sFragmentSrc);
    GLint ok = 0;

    if (vs == 0 || fs == 0)
        return false;
    sProgram = wj_glCreateProgram();
    wj_glAttachShader(sProgram, vs);
    wj_glAttachShader(sProgram, fs);
    wj_glBindAttribLocation(sProgram, 0, "pos");
    wj_glBindAttribLocation(sProgram, 1, "uv");
    wj_glBindAttribLocation(sProgram, 2, "shade");
    wj_glLinkProgram(sProgram);
    wj_glGetProgramiv(sProgram, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        wj_glGetProgramInfoLog(sProgram, sizeof(log), NULL, log);
        Voxel3D_Log("[3D] link: %s", log);
        return false;
    }
    sUniMvp = wj_glGetUniformLocation(sProgram, "mvp");
    sUniOrigin = wj_glGetUniformLocation(sProgram, "origin");
    sUniTex = wj_glGetUniformLocation(sProgram, "tex");
    sUniFade = wj_glGetUniformLocation(sProgram, "fade");
    sUniFog = wj_glGetUniformLocation(sProgram, "fog");
    sUniFogColor = wj_glGetUniformLocation(sProgram, "fogColor");
    sUniMode = wj_glGetUniformLocation(sProgram, "mode");
    sUniCam = wj_glGetUniformLocation(sProgram, "cam");
    sUniAlphaMin = wj_glGetUniformLocation(sProgram, "alphaMin");
    return true;
}

// ---------------------------------------------------------------------------
// Vertex buffers.

typedef struct
{
    GLuint vbo;
    unsigned count;
} Mesh;

static void MeshSet(Mesh *m, const VoxelVertex *v, unsigned count, GLenum usage)
{
    if (m->vbo == 0)
        wj_glGenBuffers(1, &m->vbo);
    wj_glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    wj_glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)count * sizeof(VoxelVertex), v, usage);
    m->count = count;
}

static void MeshFree(Mesh *m)
{
    if (m->vbo != 0)
        wj_glDeleteBuffers(1, &m->vbo);
    m->vbo = 0;
    m->count = 0;
}

static void MeshDraw(const Mesh *m, GLuint tex, float ox, float oy, float oz)
{
    if (m->count == 0 || tex == 0)
        return;
    wj_glBindTexture(GL_TEXTURE_2D, tex);
    wj_glUniform3f(sUniOrigin, ox, oy, oz);
    wj_glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    wj_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void *)offsetof(VoxelVertex, x));
    wj_glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void *)offsetof(VoxelVertex, u));
    wj_glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void *)offsetof(VoxelVertex, shade));
    wj_glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m->count);
}

// ---------------------------------------------------------------------------
// Map tile atlases, one per tileset pair, every page composed at once.

#define MAX_ATLASES 12

typedef struct
{
    const void *primary, *secondary;
    VoxelAtlasMap map;
    GLuint tex[VOXEL_ATLAS_PAGES];
    unsigned pages;
    uint32_t lastUse;
    bool valid;
} Atlas;

static Atlas sAtlases[MAX_ATLASES];
static VoxelAtlasJob sAtlasJob;
static uint16_t *sAtlasStaging;
static uint32_t sFrame;

static Atlas *GetAtlas(const VoxelMapInstance *inst)
{
    Atlas *slot = NULL;

    for (int i = 0; i < MAX_ATLASES; i++)
        if (sAtlases[i].valid && sAtlases[i].primary == inst->primaryTileset && sAtlases[i].secondary == inst->secondaryTileset)
        {
            sAtlases[i].lastUse = sFrame;
            return &sAtlases[i];
        }
    for (int i = 0; i < MAX_ATLASES; i++)
        if (slot == NULL || !sAtlases[i].valid || (slot->valid && sAtlases[i].lastUse < slot->lastUse))
            slot = &sAtlases[i];
    if (slot->valid)
    {
        wj_glDeleteTextures(VOXEL_ATLAS_PAGES, slot->tex);
        memset(slot->tex, 0, sizeof(slot->tex));
    }
    slot->valid = false;
    if (sAtlasStaging == NULL)
        sAtlasStaging = malloc(VOXEL_ATLAS_PIXELS * sizeof(uint16_t));
    if (!VoxelAtlas_JobBegin(&sAtlasJob, inst, sAtlasStaging, NULL))
        return NULL;
    slot->pages = 0;
    for (;;)
    {
        while (!VoxelAtlas_JobStep(&sAtlasJob, 4096))
            ;
        if (!sAtlasJob.ok)
        {
            VoxelAtlas_JobCancel(&sAtlasJob);
            Voxel3D_Log("[3D] atlas for %d:%d failed", inst->mapGroup, inst->mapNum);
            return NULL;
        }
        VoxelGrade_Texels(sAtlasStaging, VOXEL_ATLAS_PIXELS);
        slot->tex[sAtlasJob.page] = UploadTiled(0, sAtlasStaging, VOXEL_ATLAS_W, VOXEL_ATLAS_H);
        if (getenv("WJPC_VOXEL_DUMP") != NULL && sAtlasJob.page == 0)
        {
            char name[64];
            FILE *f;
            snprintf(name, sizeof(name), "atlas_%d_%d.ppm", inst->mapGroup, inst->mapNum);
            f = fopen(name, "wb");
            if (f != NULL)
            {
                fprintf(f, "P6 %u %u 255\n", VOXEL_ATLAS_W, VOXEL_ATLAS_H);
                for (unsigned i = 0; i < VOXEL_ATLAS_PIXELS; i++)
                {
                    uint16_t t = sUntile[i];
                    unsigned char rgb[3] = {(unsigned char)(((t >> 11) & 31) << 3), (unsigned char)(((t >> 6) & 31) << 3),
                                            (unsigned char)((t & 1) ? ((t >> 1) & 31) << 3 : 255)};
                    fwrite(rgb, 1, 3, f);
                }
                fclose(f);
            }
        }
        if (sAtlasJob.page + 1 > slot->pages)
            slot->pages = sAtlasJob.page + 1;
        if (!sAtlasJob.active)
            break;
        VoxelAtlas_JobNextPage(&sAtlasJob);
    }
    slot->map = sAtlasJob.map;
    slot->primary = inst->primaryTileset;
    slot->secondary = inst->secondaryTileset;
    slot->valid = true;
    slot->lastUse = sFrame;
    return slot;
}

// ---------------------------------------------------------------------------
// Building pages and the big static models.

#define MAX_PAGES 512
static GLuint sPageTex[MAX_PAGES];
static uint16_t *sPageScratch;
static size_t sPageScratchSize;

static GLuint BuildingPage(int page)
{
    unsigned w, h, th;

    if (page < 0 || page >= MAX_PAGES)
        return 0;
    if (sPageTex[page] != 0)
        return sPageTex[page];
    if (!VoxelBuildings_PageSize((unsigned)page, &w, &h, &th))
        return 0;
    if (sPageScratchSize < (size_t)w * th)
    {
        free(sPageScratch);
        sPageScratchSize = (size_t)w * th;
        sPageScratch = malloc(sPageScratchSize * sizeof(uint16_t));
    }
    // The page is w x h; its texture is padded to th rows. ReadPage grades the colours itself.
    memset(sPageScratch, 0, (size_t)w * th * sizeof(uint16_t));
    if (!VoxelBuildings_ReadPage((unsigned)page, 0, w * h, sPageScratch))
        return 0;
    sPageTex[page] = UploadTiled(0, sPageScratch, w, th);
    return sPageTex[page];
}

typedef struct
{
    unsigned pageModel;
    bool lit, used;
    Mesh mesh;
} StaticMesh;

#define MAX_STATICS 1024
static StaticMesh sStatics[MAX_STATICS];

static const Mesh *StaticModel(unsigned pageModel, bool lit)
{
    for (int i = 0; i < MAX_STATICS; i++)
        if (sStatics[i].used && sStatics[i].pageModel == pageModel && sStatics[i].lit == lit)
            return &sStatics[i].mesh;
    for (int i = 0; i < MAX_STATICS; i++)
        if (!sStatics[i].used)
        {
            const VoxelVertex *v;
            unsigned count;
            float ox, oy, su, sv;
            if (!VoxelBuildings_StaticModel(pageModel, &v, &count, &ox, &oy, &su, &sv))
                return NULL;
            VoxelVertex *t = malloc((size_t)count * sizeof(VoxelVertex));
            if (t == NULL)
                return NULL;
            // The model's texture place and scale, as ctr_voxel.c applies them.
            for (unsigned k = 0; k < count; k++)
            {
                t[k] = v[k];
                t[k].u = (ox + v[k].u) * su;
                t[k].v = 1.0f - (oy + v[k].v) * sv;
            }
            sStatics[i].used = true;
            sStatics[i].pageModel = pageModel;
            sStatics[i].lit = lit;
            MeshSet(&sStatics[i].mesh, t, count, GL_STATIC_DRAW);
            free(t);
            return &sStatics[i].mesh;
        }
    return NULL;
}

// ---------------------------------------------------------------------------
// The maps on screen: each one's whole geometry, built once, split by texture.

#define MAX_INSTANCES 16
#define MAX_INST_STATICS 256

typedef struct
{
    bool used, keep;
    int mapGroup, mapNum, originX, originY;
    uint32_t signature;
    Atlas *atlas;
    Mesh terrain[VOXEL_ATLAS_PAGES];
    Mesh trees, buildings;
    int buildingPage;
    VoxelStaticDraw statics[MAX_INST_STATICS];
    unsigned staticCount;
} InstanceMesh;

static InstanceMesh sInstanceMeshes[MAX_INSTANCES];

#define SCRATCH_VERTS (1u << 21)
static VoxelVertex *sScratch;
static VoxelVertex *sSorted;
static VoxelStaticDraw sChunkStatics[64];

static void InstanceFree(InstanceMesh *m)
{
    for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
        MeshFree(&m->terrain[p]);
    MeshFree(&m->trees);
    MeshFree(&m->buildings);
    m->used = false;
}

// One chunk (VOXEL_CHUNK cells square) into the builder, its ground / trees / models sorted
// into the three running lists.
static unsigned sTerrainCount, sTreeCount, sBuildCount;
static VoxelVertex *sTerrainBuf, *sTreeBuf, *sBuildBuf;

static void AppendChunk(VoxelBuilder *b, unsigned terrainEnd, unsigned treeEnd)
{
    unsigned i;

    for (i = 0; i < terrainEnd && sTerrainCount < SCRATCH_VERTS; i++)
        sTerrainBuf[sTerrainCount++] = b->vertices[i];
    for (; i < treeEnd && sTreeCount < SCRATCH_VERTS; i++)
        sTreeBuf[sTreeCount++] = b->vertices[i];
    for (; i < b->count && sBuildCount < SCRATCH_VERTS; i++)
        sBuildBuf[sBuildCount++] = b->vertices[i];
}

#define CHUNK 8
#define CHUNK_VERTS 65536u

static void BuildInstance(InstanceMesh *m, const VoxelMapInstance *inst, Atlas *atlas)
{
    static VoxelVertex *chunkVerts;
    VoxelBuilder b;

    if (chunkVerts == NULL)
        chunkVerts = malloc(CHUNK_VERTS * sizeof(VoxelVertex));
    unsigned uncovered = 0;
    sTerrainCount = sTreeCount = sBuildCount = 0;
    m->staticCount = 0;
    for (int y0 = inst->originY; y0 < inst->originY + inst->height; y0 += CHUNK)
        for (int x0 = inst->originX; x0 < inst->originX + inst->width; x0 += CHUNK)
        {
            int x1 = x0 + CHUNK, y1 = y0 + CHUNK;
            unsigned terrainEnd, treeEnd;

            if (x1 > inst->originX + inst->width)
                x1 = inst->originX + inst->width;
            if (y1 > inst->originY + inst->height)
                y1 = inst->originY + inst->height;
            VoxelBuilder_Init(&b, chunkVerts, CHUNK_VERTS);
            b.statics = sChunkStatics;
            b.staticCapacity = ARRAY_COUNT(sChunkStatics);
            VoxelBuilder_SetAtlas(&b, &atlas->map);
            VoxelBuilder_SetOrigin(&b, 0, 0);
            b.base = VoxelRelief_Base(inst);
            VoxelBuilder_SetStretch(&b, inst);
            b.lighting = !inst->indoor;
            b.lightingRefine = true;
            VoxelMesh_BeginWindow(x0 - 1, y0 - VOXEL_CHUNK_MARGIN_NORTH, x1 + 1, y1 + 1);
            for (int y = y0; y < y1; y++)
                VoxelMesh_EmitGroundRow(&b, inst, x0, x1, y);
            terrainEnd = b.count;
            VoxelTree_EmitInstance(&b, inst, x0, y0, x1, y1);
            treeEnd = b.count;
            VoxelBuildings_EmitInstance(&b, inst, x0, y0, x1, y1);
            AppendChunk(&b, terrainEnd, treeEnd);
            uncovered += b.uncovered;
            for (unsigned s = 0; s < b.staticCount && m->staticCount < MAX_INST_STATICS; s++)
                m->statics[m->staticCount++] = b.statics[s];
        }
    // Terrain split by atlas page (u = 2 * page + u in page).
    for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
    {
        unsigned n = 0;
        for (unsigned i = 0; i + 2 < sTerrainCount; i += 3)
        {
            unsigned page = (unsigned)(sTerrainBuf[i].u * 0.5f);
            if (page != p)
                continue;
            for (int k = 0; k < 3; k++)
            {
                sSorted[n] = sTerrainBuf[i + k];
                sSorted[n].u -= 2.0f * p;
                n++;
            }
        }
        if (n != 0)
            MeshSet(&m->terrain[p], sSorted, n, GL_STATIC_DRAW);
        else
            MeshFree(&m->terrain[p]);
    }
    MeshSet(&m->trees, sTreeBuf, sTreeCount, GL_STATIC_DRAW);
    MeshSet(&m->buildings, sBuildBuf, sBuildCount, GL_STATIC_DRAW);
    m->buildingPage = VoxelBuildings_PageOf(inst);
    m->atlas = atlas;
    Voxel3D_Log("[3D] %d:%d layout %d origin %d,%d sign(10,10)=%d sign(11,16)=%d", inst->mapGroup, inst->mapNum, inst->layoutId, inst->originX, inst->originY, VoxelSign_IsCell(inst, inst->originX + 10, inst->originY + 10), VoxelSign_IsCell(inst, inst->originX + 11, inst->originY + 16));
    Voxel3D_Log("[3D] built %d:%d (%dx%d): %u ground, %u tree, %u model vertices, %u big models",
                inst->mapGroup, inst->mapNum, inst->width, inst->height, sTerrainCount, sTreeCount, sBuildCount,
                m->staticCount);
}

// ---------------------------------------------------------------------------
// The belt: filler (sea, forest) round the maps, the world's edge.

static Mesh sBeltTerrain[VOXEL_ATLAS_PAGES], sBeltTrees;
static Atlas *sBeltAtlas;
static int sBeltX = 0x7FFFFFFF, sBeltY, sBeltGroup, sBeltNum;
#define BELT_RADIUS 40

static void BuildBelt(const VoxelMapInstance *inst0, Atlas *atlas, int cx, int cy)
{
    static VoxelVertex *chunkVerts;
    VoxelBuilder b;

    if (chunkVerts == NULL)
        chunkVerts = malloc(CHUNK_VERTS * sizeof(VoxelVertex));
    sTerrainCount = sTreeCount = sBuildCount = 0;
    for (int y0 = cy - BELT_RADIUS; y0 < cy + BELT_RADIUS; y0 += CHUNK)
        for (int x0 = cx - BELT_RADIUS; x0 < cx + BELT_RADIUS; x0 += CHUNK)
        {
            unsigned terrainEnd;
            VoxelBuilder_Init(&b, chunkVerts, CHUNK_VERTS);
            VoxelBuilder_SetAtlas(&b, &atlas->map);
            VoxelBuilder_SetOrigin(&b, 0, 0);
            b.base = VoxelRelief_Base(inst0);
            b.lighting = !inst0->indoor;
            b.lightingRefine = false;
            VoxelMesh_BeginWindow(x0 - 1, y0 - VOXEL_CHUNK_MARGIN_NORTH, x0 + CHUNK + 1, y0 + CHUNK + 1);
            for (int y = y0; y < y0 + CHUNK; y++)
                for (int x = x0; x < x0 + CHUNK; x++)
                    VoxelMesh_EmitBorder(&b, x, y, x + 1, y + 1);
            terrainEnd = b.count;
            for (int y = y0; y < y0 + CHUNK; y++)
                for (int x = x0; x < x0 + CHUNK; x++)
                    VoxelTree_EmitBorder(&b, x, y, x + 1, y + 1);
            AppendChunk(&b, terrainEnd, b.count);
        }
    for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
    {
        unsigned n = 0;
        for (unsigned i = 0; i + 2 < sTerrainCount; i += 3)
        {
            unsigned page = (unsigned)(sTerrainBuf[i].u * 0.5f);
            if (page != p)
                continue;
            for (int k = 0; k < 3; k++)
            {
                sSorted[n] = sTerrainBuf[i + k];
                sSorted[n].u -= 2.0f * p;
                n++;
            }
        }
        MeshSet(&sBeltTerrain[p], sSorted, n, GL_STATIC_DRAW);
    }
    MeshSet(&sBeltTrees, sTreeBuf, sTreeCount, GL_STATIC_DRAW);
    sBeltAtlas = atlas;
}

// ---------------------------------------------------------------------------
// Sprites (people, Pokemon, field effects) as cards, rebuilt every frame.

#define SPRITE_VERTS 8192u
static VoxelVertex *sSpriteVerts, *sShadowVerts, *sReflectVerts;
static uint16_t *sSpriteAtlas;
static GLuint sSpriteTex;
static Mesh sSpriteMesh, sShadowMesh, sReflectMesh;

// ---------------------------------------------------------------------------

static bool sInited, sFailed;
static GLuint sTreeTex;
static VoxelCamera sCamera;
static bool sCameraValid;
static int sLastGroup = -1, sLastNum = -1;
static uint32_t sLastDigest;
static GLuint sFbo, sDepth;
static int sDepthW, sDepthH;
static char sDataDir[512];

static bool LoadTrees(void)
{
    char path[600];
    uint16_t *texels = malloc(VOXEL_TREE_TEXTURE_DIM * VOXEL_TREE_TEXTURE_DIM * sizeof(uint16_t));
    FILE *f;
    size_t n;

    snprintf(path, sizeof(path), "%s/trees.rgba5551", sDataDir);
    f = fopen(path, "rb");
    if (f == NULL)
    {
        free(texels);
        return false;
    }
    n = fread(texels, 1, VOXEL_TREE_TEXTURE_DIM * VOXEL_TREE_TEXTURE_DIM * sizeof(uint16_t), f);
    fclose(f);
    VoxelGrade_Texels(texels, VOXEL_TREE_TEXTURE_DIM * VOXEL_TREE_TEXTURE_DIM);
    sTreeTex = UploadTiled(0, texels, VOXEL_TREE_TEXTURE_DIM, VOXEL_TREE_TEXTURE_DIM);
    free(texels);
    return n == VOXEL_TREE_TEXTURE_DIM * VOXEL_TREE_TEXTURE_DIM * sizeof(uint16_t);
}

// "voxel/x.bin" -> <data folder>/x.bin
FILE *Voxel3D_OpenData(const char *path)
{
    char full[700];
    const char *name = strrchr(path, '/');

    snprintf(full, sizeof(full), "%s/%s", sDataDir, name != NULL ? name + 1 : path);
    return fopen(full, "rb");
}

bool Voxel3D_Init(const char *dataDir)
{
    if (sInited || sFailed)
        return sInited;
    sFailed = true;
    snprintf(sDataDir, sizeof(sDataDir), "%s", dataDir);
    if (!LoadGl() || !MakeProgram())
        return false;
    // The voxel modules open "voxel/...": run from the folder that holds it.
    VoxelRegions_Init();
    VoxelRelief_Init();
    VoxelSign_Init();
    VoxelGrade_Init();
    VoxelCamera_Init(&sCamera);
    VoxelEntities_Reset();
    VoxelBuildings_SetDrawings(VoxelWorld_LayoutDrawing);
    if (!VoxelBuildings_Init())
        Voxel3D_Log("[3D] no building models (voxel/buildings.bin)");
    if (!LoadTrees())
        Voxel3D_Log("[3D] no tree texture (voxel/trees.rgba5551)");
    sScratch = malloc(SCRATCH_VERTS * sizeof(VoxelVertex) * 4);
    sTerrainBuf = sScratch;
    sTreeBuf = sScratch + SCRATCH_VERTS;
    sBuildBuf = sScratch + 2 * SCRATCH_VERTS;
    sSorted = sScratch + 3 * SCRATCH_VERTS;
    sSpriteVerts = malloc(SPRITE_VERTS * sizeof(VoxelVertex) * 3);
    sShadowVerts = sSpriteVerts + SPRITE_VERTS;
    sReflectVerts = sSpriteVerts + 2 * SPRITE_VERTS;
    sSpriteAtlas = calloc(VOXEL_SPRITE_PIXELS, sizeof(uint16_t));
    if (sScratch == NULL || sSpriteVerts == NULL || sSpriteAtlas == NULL)
        return false;
    sFailed = false;
    sInited = true;
    Voxel3D_Log("[3D] ready");
    return true;
}

// ---------------------------------------------------------------------------
// Matrices.

static void Perspective(float *m, float fovYDeg, float aspect, float zn, float zf)
{
    float f = 1.0f / tanf(fovYDeg * 0.5f * 3.14159265f / 180.0f);
    memset(m, 0, 16 * sizeof(float));
    m[0] = f / aspect;
    m[5] = f;
    m[10] = (zf + zn) / (zn - zf);
    m[11] = -1.0f;
    m[14] = 2.0f * zf * zn / (zn - zf);
}

static void LookAt(float *m, float ex, float ey, float ez, float cx, float cy, float cz)
{
    float fx = cx - ex, fy = cy - ey, fz = cz - ez, l = sqrtf(fx * fx + fy * fy + fz * fz);
    float sx, sy, sz, ux, uy, uz;
    fx /= l; fy /= l; fz /= l;
    // s = f x up(0,1,0)
    sx = -fz; sy = 0; sz = fx;
    l = sqrtf(sx * sx + sz * sz);
    sx /= l; sz /= l;
    // u = s x f
    ux = sy * fz - sz * fy; uy = sz * fx - sx * fz; uz = sx * fy - sy * fx;
    memset(m, 0, 16 * sizeof(float));
    m[0] = sx; m[4] = sy; m[8] = sz;
    m[1] = ux; m[5] = uy; m[9] = uz;
    m[2] = -fx; m[6] = -fy; m[10] = -fz;
    m[12] = -(sx * ex + sy * ey + sz * ez);
    m[13] = -(ux * ex + uy * ey + uz * ez);
    m[14] = (fx * ex + fy * ey + fz * ez);
    m[15] = 1.0f;
}

static void Multiply(float *out, const float *a, const float *b)
{
    float r[16];
    for (int c = 0; c < 4; c++)
        for (int rr = 0; rr < 4; rr++)
            r[c * 4 + rr] = a[0 * 4 + rr] * b[c * 4 + 0] + a[1 * 4 + rr] * b[c * 4 + 1]
                          + a[2 * 4 + rr] * b[c * 4 + 2] + a[3 * 4 + rr] * b[c * 4 + 3];
    memcpy(out, r, sizeof(r));
}

// ---------------------------------------------------------------------------
// Saved GL state: SDL's renderer shares the context and expects its own back.

typedef struct
{
    GLint fbo, program, buffer, texture, active, viewport[4], blend4[4];
    GLboolean depth, blend, cull, scissor;
    GLfloat clear[4];
} SavedGl;

static void SaveGl(SavedGl *s)
{
    wj_glGetIntegerv(GL_FRAMEBUFFER_BINDING, &s->fbo);
    wj_glGetIntegerv(GL_CURRENT_PROGRAM, &s->program);
    wj_glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &s->buffer);
    wj_glGetIntegerv(GL_ACTIVE_TEXTURE, &s->active);
    wj_glActiveTexture(GL_TEXTURE0);
    wj_glGetIntegerv(GL_TEXTURE_BINDING_2D, &s->texture);
    wj_glGetIntegerv(GL_VIEWPORT, s->viewport);
    wj_glGetIntegerv(0x80C9 /* GL_BLEND_SRC_RGB */, &s->blend4[0]);
    wj_glGetIntegerv(0x80C8 /* GL_BLEND_DST_RGB */, &s->blend4[1]);
    wj_glGetIntegerv(0x80CB /* GL_BLEND_SRC_ALPHA */, &s->blend4[2]);
    wj_glGetIntegerv(0x80CA /* GL_BLEND_DST_ALPHA */, &s->blend4[3]);
    wj_glGetFloatv(GL_COLOR_CLEAR_VALUE, s->clear);
    s->depth = wj_glIsEnabled(GL_DEPTH_TEST);
    s->blend = wj_glIsEnabled(GL_BLEND);
    s->cull = wj_glIsEnabled(GL_CULL_FACE);
    s->scissor = wj_glIsEnabled(GL_SCISSOR_TEST);
}

static void RestoreGl(const SavedGl *s)
{
    wj_glDisableVertexAttribArray(0);
    wj_glDisableVertexAttribArray(1);
    wj_glDisableVertexAttribArray(2);
    wj_glBindFramebuffer(GL_FRAMEBUFFER, s->fbo);
    wj_glUseProgram(s->program);
    wj_glBindBuffer(GL_ARRAY_BUFFER, s->buffer);
    wj_glBindTexture(GL_TEXTURE_2D, s->texture);
    wj_glActiveTexture(s->active);
    if (s->fbo == 0 && SDL_GL_GetCurrentWindow() != NULL)
    {
        // SDL may not have sent the window's viewport since a resize: leave the whole window.
        int dw = 0, dh = 0;
        SDL_GL_GetDrawableSize(SDL_GL_GetCurrentWindow(), &dw, &dh);
        wj_glViewport(0, 0, dw, dh);
    }
    else
        wj_glViewport(s->viewport[0], s->viewport[1], s->viewport[2], s->viewport[3]);
    wj_glClearColor(s->clear[0], s->clear[1], s->clear[2], s->clear[3]);
    (s->depth ? wj_glEnable : wj_glDisable)(GL_DEPTH_TEST);
    (s->blend ? wj_glEnable : wj_glDisable)(GL_BLEND);
    (s->cull ? wj_glEnable : wj_glDisable)(GL_CULL_FACE);
    (s->scissor ? wj_glEnable : wj_glDisable)(GL_SCISSOR_TEST);
    wj_glDepthMask(GL_TRUE);
    wj_glBlendFuncSeparate(s->blend4[0], s->blend4[1], s->blend4[2], s->blend4[3]);
}

// ---------------------------------------------------------------------------

static InstanceMesh *FindInstanceMesh(const VoxelMapInstance *inst)
{
    InstanceMesh *free = NULL;

    for (int i = 0; i < MAX_INSTANCES; i++)
    {
        InstanceMesh *m = &sInstanceMeshes[i];
        if (m->used && m->mapGroup == inst->mapGroup && m->mapNum == inst->mapNum)
            return m;
        if (!m->used && free == NULL)
            free = m;
    }
    return free;
}

static float sFogNear = 40.0f, sFogFar = 90.0f;

bool Voxel3D_Render(unsigned colorTexture, int w, int h)
{
    SavedGl saved;
    unsigned count;
    const VoxelMapInstance *inst0;
    float playerX, playerZ, smoothX, smoothZ;
    int group, num;
    uint32_t digest;
    float proj[16], view[16], mvp[16];
    float fadeAmount = 0.0f, fadeRgb[3] = {0, 0, 0};
    uint16_t backdrop;

    if (!sInited)
        return false;
    sFrame++;
    VoxelWorld_BeginBatch();
    VoxelWorld_BuildInstances();
    count = VoxelWorld_InstanceCount();
    if (count == 0)
        return false;
    inst0 = VoxelWorld_Instance(0);

    VoxelWorld_GetLocation(&group, &num);
    digest = VoxelWorld_LiveDigest();
    if (group != sLastGroup || num != sLastNum)
    {
        VoxelBuildings_MapChanged();
        VoxelLighting_Reset();
        sCameraValid = false;
        sLastGroup = group;
        sLastNum = num;
        sBeltX = 0x7FFFFFFF;
        VoxelEntities_Reset();
    }
    else if (digest != sLastDigest)
    {
        VoxelBuildings_WorldChanged();
        for (int i = 0; i < MAX_INSTANCES; i++)
            if (sInstanceMeshes[i].used)
                sInstanceMeshes[i].signature ^= 1; // rebuild below
        sBeltX = 0x7FFFFFFF;
    }
    sLastDigest = digest;

    SaveGl(&saved);

    // Maps: build what is new, forget what is gone.
    for (int i = 0; i < MAX_INSTANCES; i++)
        sInstanceMeshes[i].keep = false;
    for (unsigned i = 0; i < count; i++)
    {
        const VoxelMapInstance *inst = VoxelWorld_Instance(i);
        InstanceMesh *m = FindInstanceMesh(inst);
        Atlas *atlas;
        uint32_t sig = VoxelBuildings_LiveSignature(inst) ^ (uint32_t)inst->originX * 7919u ^ (uint32_t)inst->originY * 104729u;

        if (m == NULL)
            continue;
        atlas = GetAtlas(inst);
        if (atlas == NULL)
            continue;
        if (!m->used || m->signature != sig || m->atlas != atlas || m->originX != inst->originX || m->originY != inst->originY)
        {
            if (m->used)
                InstanceFree(m);
            BuildInstance(m, inst, atlas);
            m->used = true;
            m->mapGroup = inst->mapGroup;
            m->mapNum = inst->mapNum;
            m->originX = inst->originX;
            m->originY = inst->originY;
            m->signature = sig;
        }
        m->keep = true;
    }
    for (int i = 0; i < MAX_INSTANCES; i++)
        if (sInstanceMeshes[i].used && !sInstanceMeshes[i].keep)
            InstanceFree(&sInstanceMeshes[i]);

    // Camera follows the player.
    VoxelWorld_GetPlayerWorldCoords(&playerX, &playerZ);
    VoxelEntities_GetPlayerWorldPos(&smoothX, &smoothZ);
    if (!sCameraValid)
    {
        VoxelCamera_SetGround(&sCamera, VoxelEntities_PlayerGround(), 1);
        VoxelCamera_Snap(&sCamera, smoothX, smoothZ);
        sCameraValid = true;
    }
    else
    {
        VoxelCamera_SetGround(&sCamera, VoxelEntities_PlayerGround(), 0);
        VoxelCamera_Update(&sCamera, smoothX, smoothZ);
    }

    // The belt round the maps, rebuilt when the player has moved a chunk.
    {
        int cx = ((int)floorf(playerX) / CHUNK) * CHUNK, cy = ((int)floorf(playerZ) / CHUNK) * CHUNK;
        Atlas *atlas0 = GetAtlas(inst0);
        if (atlas0 != NULL && !inst0->indoor && (abs(cx - sBeltX) >= CHUNK * 2 || abs(cy - sBeltY) >= CHUNK * 2 || atlas0 != sBeltAtlas))
        {
            BuildBelt(inst0, atlas0, cx, cy);
            sBeltX = cx;
            sBeltY = cy;
        }
    }

    // Sprites.
    {
        VoxelBuilder sprites, shadows, reflections;
        VoxelBuilder_Init(&sprites, sSpriteVerts, SPRITE_VERTS);
        VoxelBuilder_SetOrigin(&sprites, 0, 0);
        VoxelBuilder_Init(&shadows, sShadowVerts, SPRITE_VERTS);
        VoxelBuilder_SetOrigin(&shadows, 0, 0);
        VoxelBuilder_Init(&reflections, sReflectVerts, SPRITE_VERTS);
        VoxelBuilder_SetOrigin(&reflections, 0, 0);
        VoxelEntities_Emit(&sprites, sSpriteAtlas, &sCamera, &shadows, &reflections);
        sSpriteTex = UploadTiled(sSpriteTex, sSpriteAtlas, VOXEL_SPRITE_ATLAS_W, VOXEL_SPRITE_ATLAS_H);
        MeshSet(&sSpriteMesh, sSpriteVerts, sprites.count, GL_STREAM_DRAW);
        MeshSet(&sShadowMesh, sShadowVerts, shadows.count, GL_STREAM_DRAW);
        MeshSet(&sReflectMesh, sReflectVerts, reflections.count, GL_STREAM_DRAW);
    }

    // Target: the caller's texture, with a depth buffer of our own.
    if (sFbo == 0)
        wj_glGenFramebuffers(1, &sFbo);
    if (sDepth == 0 || sDepthW != w || sDepthH != h)
    {
        if (sDepth != 0)
            wj_glDeleteRenderbuffers(1, &sDepth);
        wj_glGenRenderbuffers(1, &sDepth);
        wj_glBindRenderbuffer(GL_RENDERBUFFER, sDepth);
        wj_glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
        sDepthW = w;
        sDepthH = h;
    }
    wj_glBindFramebuffer(GL_FRAMEBUFFER, sFbo);
    wj_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    wj_glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sDepth);
    if (wj_glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Voxel3D_Log("[3D] framebuffer incomplete");
        RestoreGl(&saved);
        return false;
    }

    wj_glViewport(0, 0, w, h);
    wj_glDepthMask(GL_TRUE);
    if (getenv("WJPC_VOXEL_LOG") != NULL && (sFrame % 120) == 1)
        Voxel3D_Log("[3D] saved viewport %d %d %d %d fbo %d", saved.viewport[0], saved.viewport[1], saved.viewport[2], saved.viewport[3], saved.fbo);
    if (getenv("WJPC_VOXEL_LOG") != NULL && (sFrame % 120) == 1)
    {
        const InstanceMesh *m = NULL;
        for (int i = 0; i < MAX_INSTANCES; i++)
            if (sInstanceMeshes[i].used) { m = &sInstanceMeshes[i]; break; }
        Voxel3D_Log("[3D] frame %u %dx%d tex %u cam (%.1f %.1f %.1f) -> (%.1f %.1f %.1f) fov %.1f player %.1f,%.1f",
                    sFrame, w, h, colorTexture, sCamera.x, sCamera.y, sCamera.z, sCamera.targetX, sCamera.targetY,
                    sCamera.targetZ, sCamera.fov, playerX, playerZ);
        if (m != NULL)
        {
            for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
                if (m->terrain[p].count)
                    Voxel3D_Log("[3D]   map %d:%d page %u: %u verts, tex %u", m->mapGroup, m->mapNum, p, m->terrain[p].count, m->atlas->tex[p]);
        }
        Voxel3D_Log("[3D]   ground sample (%.2f %.2f %.2f) uv %.3f %.3f shade %.2f; sprites %u",
                    sTerrainBuf[0].x, sTerrainBuf[0].y, sTerrainBuf[0].z, sTerrainBuf[0].u, sTerrainBuf[0].v, sTerrainBuf[0].shade, sSpriteMesh.count);
    }
    backdrop = *(const uint16_t *)PLTT;
    {
        float r = (backdrop & 31) / 31.0f, g = ((backdrop >> 5) & 31) / 31.0f, b = ((backdrop >> 10) & 31) / 31.0f;
        if (!inst0->indoor)
        {
            // Outdoors: a sky colour, not the GBA backdrop.
            r = 0.55f; g = 0.75f; b = 0.95f;
        }
        wj_glClearColor(r, g, b, 1.0f);
    }
    wj_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    wj_glEnable(GL_DEPTH_TEST);
    wj_glDepthFunc(GL_LEQUAL);
    wj_glDisable(GL_BLEND);
    wj_glDisable(GL_CULL_FACE);
    wj_glDisable(GL_SCISSOR_TEST);
    wj_glUseProgram(sProgram);
    wj_glUniform1i(sUniTex, 0);
    wj_glUniform1f(sUniMode, 0.0f);
    wj_glUniform1f(sUniAlphaMin, getenv("WJPC_3D_UV") ? -10.0f : getenv("WJPC_3D_NOALPHA") ? -1.0f : 0.5f);
    wj_glActiveTexture(GL_TEXTURE0);
    wj_glEnableVertexAttribArray(0);
    wj_glEnableVertexAttribArray(1);
    wj_glEnableVertexAttribArray(2);

    Perspective(proj, sCamera.fov, (float)w / (float)h, 0.5f, 400.0f);
    // SDL shows render targets as they are stored: draw upside down so it comes out right.
    proj[1] = -proj[1]; proj[5] = -proj[5]; proj[9] = -proj[9]; proj[13] = -proj[13];
    LookAt(view, sCamera.x, sCamera.y, sCamera.z, sCamera.targetX, sCamera.targetY, sCamera.targetZ);
    Multiply(mvp, proj, view);
    wj_glUniformMatrix4fv(sUniMvp, 1, GL_FALSE, mvp);
    wj_glUniform3f(sUniCam, sCamera.x, sCamera.y, sCamera.z);
    if (VoxelWorld_ScreenFade(&fadeAmount, fadeRgb))
        wj_glUniform4f(sUniFade, fadeRgb[0], fadeRgb[1], fadeRgb[2], fadeAmount);
    else
        wj_glUniform4f(sUniFade, 0, 0, 0, 0);
    // Haze, outdoors only: starts well past the player, the sky's colour.
    if (inst0->indoor)
        wj_glUniform2f(sUniFog, 1.0e6f, 2.0e6f);
    else
        wj_glUniform2f(sUniFog, sFogNear, sFogFar);
    wj_glUniform3f(sUniFogColor, 0.55f, 0.75f, 0.95f);

    // Ground, trees and models of every map.
    for (int i = 0; i < MAX_INSTANCES; i++)
    {
        InstanceMesh *m = &sInstanceMeshes[i];
        if (!m->used)
            continue;
        for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
            MeshDraw(&m->terrain[p], m->atlas->tex[p], 0, 0, 0);
        MeshDraw(&m->trees, sTreeTex, 0, 0, 0);
        if (m->buildingPage >= 0)
        {
            GLuint page = BuildingPage(m->buildingPage);
            if (getenv("WJPC_VOXEL_LOG") != NULL && (sFrame % 120) == 1)
                Voxel3D_Log("[3D]   map %d:%d buildings page %d tex %u, %u chunk verts, %u statics, first model %p", m->mapGroup, m->mapNum, m->buildingPage, page, m->buildings.count, m->staticCount, m->staticCount ? (const void *)StaticModel(m->statics[0].pageModel, m->statics[0].lit) : NULL);
            MeshDraw(&m->buildings, page, 0, 0, 0);
            for (unsigned s = 0; s < m->staticCount; s++)
            {
                const VoxelStaticDraw *d = &m->statics[s];
                const Mesh *mesh = StaticModel(d->pageModel, d->lit);
                if (mesh != NULL)
                    MeshDraw(mesh, page, d->x, d->y, d->z);
            }
        }
    }
    if (sBeltAtlas != NULL && !inst0->indoor)
    {
        for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
            MeshDraw(&sBeltTerrain[p], sBeltAtlas->tex[p], 0, 0, 0);
        MeshDraw(&sBeltTrees, sTreeTex, 0, 0, 0);
    }

    // Shadows and reflections under the cards, then the cards.
    // As on the 3DS: blended, without writing depth.
    wj_glEnable(GL_BLEND);
    wj_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    wj_glDepthMask(GL_FALSE);
    wj_glUniform1f(sUniMode, 2.0f);
    MeshDraw(&sReflectMesh, sSpriteTex, 0, 0, 0);
    wj_glUniform1f(sUniMode, 1.0f);
    MeshDraw(&sShadowMesh, sSpriteTex, 0, 0, 0);
    wj_glDepthMask(GL_TRUE);
    wj_glDisable(GL_BLEND);
    wj_glUniform1f(sUniMode, 0.0f);
    MeshDraw(&sSpriteMesh, sSpriteTex, 0, 0, 0);

    if (getenv("WJPC_VOXEL_LOG") != NULL && (sFrame % 120) == 1)
    {
        unsigned char px[4] = {0};
        GLint tw = 0, th = 0, target = 0;
        wj_glReadPixels(w / 2, h / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
        Voxel3D_Log("[3D]   centre pixel %u %u %u, GL error 0x%x", px[0], px[1], px[2], wj_glGetError());
        if (getenv("WJPC_VOXEL_DUMP") != NULL)
        {
            unsigned char *all = malloc((size_t)w * h * 3);
            FILE *f = fopen("fbo.ppm", "wb");
            wj_glPixelStorei(GL_PACK_ALIGNMENT, 1);
            wj_glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, all);
            if (f) { fprintf(f, "P6 %d %d 255\n", w, h); fwrite(all, 1, (size_t)w * h * 3, f); fclose(f); }
            free(all);
        }
    }
    RestoreGl(&saved);
    return true;
}

// The GL name of an SDL texture (SDL's OpenGL renderer).
unsigned Voxel3D_TextureId(void *sdlTexture)
{
    float tw, th;
    GLint id = 0;

    if (!sInited || SDL_GL_BindTexture((SDL_Texture *)sdlTexture, &tw, &th) != 0)
        return 0;
    wj_glGetIntegerv(GL_TEXTURE_BINDING_2D, &id);
    SDL_GL_UnbindTexture((SDL_Texture *)sdlTexture);
    return (unsigned)id;
}
