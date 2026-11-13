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
    X(void, glUniform3fv, (GLint, GLsizei, const GLfloat *)) \
    X(void, glUniform1fv, (GLint, GLsizei, const GLfloat *)) \
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
static GLint sUniTime, sUniGamma, sUniSunDir, sUniSunLight, sUniSkyTint, sUniSh, sUniSat, sUniExposure;

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
    "varying vec3 vWorld;\n"
    "void main() {\n"
    "  vec3 p = pos + origin;\n"
    "  vWorld = p;\n"
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
    "uniform vec3 cam;\n"
    "uniform float gammaLight;\n"
    // Gamma Emerald lighting, values from docs/GAMMA_LIGHTING_VALUES.md (all read from its files)
    "uniform vec3 sunDir;\n"     // towards the sun
    "uniform vec3 sunLight;\n"   // colour x intensity / pi
    "uniform vec3 skyTint;\n"    // sky light colour x intensity
    "uniform vec3 sh[9];\n"      // sky light cubemap irradiance / pi, order-2 SH, our axes (x east, y up, z south)
    "uniform float saturation;\n"
    "uniform float exposure;\n"
    "uniform float time;\n"
    "varying vec2 vUv;\n"
    "varying float vShade;\n"
    "varying float vDist;\n"
    "varying vec3 vWorld;\n"
    "vec3 toLinear(vec3 c) { return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), step(0.04045, c)); }\n"
    "vec3 toSrgb(vec3 c) { c = max(c, 0.0); return mix(c * 12.92, 1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055, step(0.0031308, c)); }\n"
    "vec3 skyIrr(vec3 n) {\n"
    "  return sh[0] + sh[1]*n.y + sh[2]*n.z + sh[3]*n.x + sh[4]*n.x*n.y + sh[5]*n.y*n.z\n"
    "       + sh[6]*(3.0*n.z*n.z - 1.0) + sh[7]*n.x*n.z + sh[8]*(n.x*n.x - n.y*n.y);\n"
    "}\n"
    "float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }\n"
    "float vnoise(vec2 p) { vec2 i = floor(p), f = fract(p); f = f*f*(3.0-2.0*f);\n"
    "  return mix(mix(hash(i), hash(i+vec2(1,0)), f.x), mix(hash(i+vec2(0,1)), hash(i+vec2(1,1)), f.x), f.y); }\n"
    "float fbm(vec2 p) { float a = 0.5, s = 0.0; for (int k = 0; k < 4; k++) { s += a * vnoise(p); p *= 2.03; a *= 0.5; } return s / 0.9375; }\n"
    // M_SunCloudShadows_LF1 with Gamma's numbers; the two noise textures are stand-ins (theirs are art assets)
    "uniform sampler2D cloudTex, roughTex;\n"
    "float clouds(vec3 w) {\n"
    "  vec2 P = vec2(-w.z, w.x) * 64.0;\n" // Gamma X (north), Y (east), in cm
    "  float nn = texture2D(roughTex, P / 888.25 + time * vec2(-0.0010, -0.0007)).r;\n"
    "  float cc = texture2D(cloudTex, (P + nn * 40.0) / 5000.0 + time * vec2(-0.0021, -0.0030)).r;\n"
    "  return 1.0 - clamp((cc - 0.164) * 3.8417, 0.0, 1.0) * 0.75;\n"
    "}\n"
    "void main() {\n"
    "  vec4 c = texture2D(tex, vUv);\n"
    "  if (c.a < alphaMin) discard;\n"
    "  vec3 rgb;\n"
    "  if (gammaLight > 0.5) {\n"
    "    vec3 n = normalize(cross(dFdx(vWorld), dFdy(vWorld)));\n"
    "    if (dot(n, cam - vWorld) < 0.0) n = -n;\n"
    "    vec3 albedo = toLinear(c.rgb);\n"
    "    vec3 light = sunLight * max(dot(n, sunDir), 0.0) * vShade * clouds(vWorld) + skyTint * max(skyIrr(n), 0.0);\n"
    "    rgb = albedo * light;\n"
    // haze (M_StylizedFogPP, day): distances in Gamma units, 64 per tile
    "    float d = length(vWorld - cam) * 64.0;\n"
    "    float amt = clamp((d - 10.0) / 39990.0, 0.0, 1.0) * 0.5;\n"
    "    vec3 v = normalize(cam - vWorld);\n"
    "    amt = min(amt, clamp(1.0 - 0.4 * (dot(sunDir, v) - 0.5), 0.0, 1.0));\n"
    "    rgb = mix(rgb, vec3(0.2, 0.467, 1.0), amt);\n"
    "    if (mode > 1.5) { gl_FragColor = vec4(albedo * (sunLight * 0.643 + skyTint * skyIrr(vec3(0,1,0))), 0.45); return; }\n"
    "    if (mode > 0.5) { gl_FragColor = vec4(0.0, 0.0, 0.0, vShade); return; }\n"
    "    gl_FragColor = vec4(rgb, 1.0);\n"
    "    return;\n"
    "  } else {\n"
    "    rgb = c.rgb * vShade;\n"
    "    float f = clamp((vDist - fog.x) / max(fog.y - fog.x, 0.001), 0.0, 1.0);\n"
    "    rgb = mix(rgb, fogColor, f * f);\n"
    "  }\n"
    "  rgb = mix(rgb, fade.rgb, fade.a);\n"
    "  gl_FragColor = vec4(rgb, 1.0);\n"
    "  if (mode > 0.5 && mode < 1.5) gl_FragColor = vec4(0.0, 0.0, 0.0, vShade);\n"
    "  if (mode > 1.5) gl_FragColor = vec4(c.rgb, 0.45);\n"
    "  if (alphaMin < -5.0) gl_FragColor = vec4(fract(vUv.x * 32.0), fract(vUv.y * 16.0), 0.0, 1.0);\n"
    "}\n";


/* Gamma Emerald's outdoor daytime light, every number from docs/GAMMA_LIGHTING_VALUES.md.
 * Our axes: x east, y up, z south. Gamma's camera looks along Unreal +X (screen up = north here), so
 * Unreal (X, Y, Z) maps to ours as (east = Y, up = Z, south = -X). */
static uint32_t sFrame;
static GLuint sCloudTex[2];
static const float sGammaSh[9][3] = {
    {0.38100f, 0.88990f, 1.51087f}, {0.05944f, 0.15420f, 0.27341f}, {-0.02007f, -0.03913f, -0.03543f},
    {-0.01342f, -0.02757f, -0.04645f}, {-0.00478f, -0.00471f, -0.00461f}, {-0.01140f, -0.01572f, -0.01706f},
    {0.00710f, 0.02091f, 0.02727f}, {-0.00126f, -0.00245f, -0.00232f}, {0.00685f, 0.02464f, 0.03052f},
};

static void GammaLightUniforms(void)
{
    const float pi = 3.14159265f;
    float sunI = 3.0f / pi;         /* sun 3 lux, Lambert */
    float skyI = 0.32f;             /* sky light intensity */
    wj_glUniform1f(sUniGamma, gVoxelGammaLight ? 1.0f : 0.0f);
    if (!gVoxelGammaLight)
        return;
    wj_glUniform3f(sUniSunDir, 0.1747f, 0.6428f, 0.7459f);
    wj_glUniform3f(sUniSunLight, 0.788f * sunI, 0.738f * sunI, 0.700f * sunI);
    wj_glUniform3f(sUniSkyTint, 1.0f * skyI, 0.701f * skyI, 0.271f * skyI);
    wj_glUniform3fv(sUniSh, 9, &sGammaSh[0][0]);
    wj_glUniform1f(sUniSat, 1.25f); /* camera colour saturation */
    wj_glUniform1f(sUniTime, (float)sFrame / 60.0f);
    if (sCloudTex[0] != 0)
    {
        wj_glUniform1i(wj_glGetUniformLocation(sProgram, "cloudTex"), 6);
        wj_glUniform1i(wj_glGetUniformLocation(sProgram, "roughTex"), 7);
        wj_glActiveTexture(GL_TEXTURE0 + 6);
        wj_glBindTexture(GL_TEXTURE_2D, sCloudTex[0]);
        wj_glActiveTexture(GL_TEXTURE0 + 7);
        wj_glBindTexture(GL_TEXTURE_2D, sCloudTex[1]);
        wj_glActiveTexture(GL_TEXTURE0);
    }
    wj_glUniform1f(sUniExposure, getenv("WJPC_3D_EXPOSURE") ? (float)atof(getenv("WJPC_3D_EXPOSURE")) : 1.0f);
}

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
    sUniGamma = wj_glGetUniformLocation(sProgram, "gammaLight");
    sUniTime = wj_glGetUniformLocation(sProgram, "time");
    sUniSunDir = wj_glGetUniformLocation(sProgram, "sunDir");
    sUniSunLight = wj_glGetUniformLocation(sProgram, "sunLight");
    sUniSkyTint = wj_glGetUniformLocation(sProgram, "skyTint");
    sUniSh = wj_glGetUniformLocation(sProgram, "sh");
    sUniSat = wj_glGetUniformLocation(sProgram, "saturation");
    sUniExposure = wj_glGetUniformLocation(sProgram, "exposure");
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
    uint32_t season; // VoxelWorld_SeasonKey() it was baked for (DLX-17)
    bool valid;
} Atlas;

static Atlas sAtlases[MAX_ATLASES];

// The season key, asked for once a frame (it reads the clock; asked per atlas per map it was most of a frame).
static uint32_t sSeasonKey, sSeasonFrame = 0xFFFFFFFFu;
static uint32_t sFrame;
static uint32_t SeasonNow(void)
{
    if (sSeasonFrame != sFrame)
    {
        sSeasonKey = VoxelWorld_SeasonKey();
        sSeasonFrame = sFrame;
    }
    return sSeasonKey;
}
static VoxelAtlasJob sAtlasJob;
static uint16_t *sAtlasStaging;

static bool AtlasReady(const VoxelMapInstance *inst)
{
    for (int i = 0; i < MAX_ATLASES; i++)
        if (sAtlases[i].valid && sAtlases[i].primary == inst->primaryTileset && sAtlases[i].secondary == inst->secondaryTileset
            && sAtlases[i].season == SeasonNow())
            return true;
    return false;
}

// DLX-18: an atlas is composed a page at a time; for a map not underfoot (`deadline` set) the work is spread
// over frames, one job at a time, and NULL is returned until it is done. With no deadline it is finished now
// (a job for another map's tile sets is dropped for it). The slot the job fills is invalid until then.
static Atlas *sJobSlot;
static const void *sJobPrimary, *sJobSecondary; // the job's own copies are cleared when it ends

static Atlas *FindAtlas(const VoxelMapInstance *inst)
{
    for (int i = 0; i < MAX_ATLASES; i++)
        if (sAtlases[i].valid && sAtlases[i].primary == inst->primaryTileset && sAtlases[i].secondary == inst->secondaryTileset
            && sAtlases[i].season == SeasonNow())
        {
            sAtlases[i].lastUse = sFrame;
            return &sAtlases[i];
        }
    return NULL;
}

static Atlas *PrepareAtlas(const VoxelMapInstance *inst, Uint64 deadline)
{
    Atlas *slot = FindAtlas(inst);

    if (slot != NULL)
        return slot;
    if (sJobSlot != NULL && (sJobPrimary != inst->primaryTileset || sJobSecondary != inst->secondaryTileset
                             || sJobSlot->season != SeasonNow()))
    {
        if (deadline == 0)
        {
            VoxelAtlas_JobCancel(&sAtlasJob);
            sJobSlot = NULL;
        }
        // else: another map's is under way; it is carried on (and this one asked for again after it)
    }
    if (sJobSlot == NULL)
    {
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
        slot->season = SeasonNow();
        sJobSlot = slot;
        sJobPrimary = inst->primaryTileset;
        sJobSecondary = inst->secondaryTileset;
        Voxel3D_Log("[3D] atlas job begins for %d:%d (slot %d)", inst->mapGroup, inst->mapNum, (int)(slot - sAtlases));
    }
    slot = sJobSlot;
    for (;;)
    {
        while (!VoxelAtlas_JobStep(&sAtlasJob, deadline != 0 ? 256 : 4096))
            if (deadline != 0 && SDL_GetPerformanceCounter() >= deadline)
                return NULL;
        if (!sAtlasJob.ok)
        {
            VoxelAtlas_JobCancel(&sAtlasJob);
            sJobSlot = NULL;
            Voxel3D_Log("[3D] atlas for %d:%d failed", sAtlasJob.mapGroup, sAtlasJob.mapNum);
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
        if (deadline != 0 && SDL_GetPerformanceCounter() >= deadline)
            return NULL;
    }
    sJobSlot = NULL;
    slot->map = sAtlasJob.map;
    slot->primary = sJobPrimary;
    slot->secondary = sJobSecondary;
    slot->valid = true;
    slot->lastUse = sFrame;
    return slot->primary == inst->primaryTileset && slot->secondary == inst->secondaryTileset ? slot : NULL;
}

static Atlas *GetAtlas(const VoxelMapInstance *inst)
{
    return PrepareAtlas(inst, 0);
}

// ---------------------------------------------------------------------------
// Building pages and the big static models.

#define MAX_PAGES 1024 // data generated from our own maps has more than 512 pages
static GLuint sPageTex[MAX_PAGES];
static uint16_t *sPageScratch;
static size_t sPageScratchSize;

static GLuint LoadBuildingPage(int page, bool reload);
static GLuint BuildingPage(int page)
{
    return LoadBuildingPage(page, false);
}

// reload: read the page again into the texture it already has (DLX-17: season change), so meshes that keep
// its handle stay right.
static GLuint LoadBuildingPage(int page, bool reload)
{
    unsigned w, h, th;

    if (page < 0 || page >= MAX_PAGES)
        return 0;
    if (sPageTex[page] != 0 && !reload)
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
    VoxelWorld_SeasonTexels(sPageScratch, (size_t)w * h); // DLX-17 (after ReadPage's grading)
    sPageTex[page] = UploadTiled(sPageTex[page], sPageScratch, w, th);
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

// DLX-18: more slots than maps on screen; a map gone from the view keeps its mesh until the slot is
// needed, so walking back that way finds it built.
#define MAX_INSTANCES 32
#define MAX_INST_STATICS 256

typedef struct
{
    bool used, keep;
    bool shown;               // one of this frame's maps (drawn)
    int mapGroup, mapNum, originX, originY, lift;
    uint32_t signature, lastUse;
    const void *primary, *secondary; // the tile sets its atlas must still hold
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

// DLX-18: a map's mesh is built a chunk at a time. The player's own map is built at once when it has to be;
// any other one over as many frames as it takes (a few ms each), its old mesh, if any, shown meanwhile.
// Crossing into a map now never builds it: it was built while it was a neighbour.
typedef struct
{
    bool active;
    int mapGroup, mapNum, originX, originY, lift;
    uint32_t signature;
    Atlas *atlas;
    int nextX, nextY;            // the next chunk, cells into the map
    VoxelVertex *v[3];           // ground, trees, models
    unsigned n[3], cap[3];
    VoxelStaticDraw statics[MAX_INST_STATICS];
    unsigned staticCount, frames;
    double ms;
} InstanceBuild;

static InstanceBuild sBackBuild, sNowBuild;

static void BuildPush(InstanceBuild *pb, int which, const VoxelVertex *v, unsigned count)
{
    if (count == 0)
        return;
    if (pb->n[which] + count > pb->cap[which])
    {
        unsigned cap = pb->cap[which] ? pb->cap[which] : 65536u;
        VoxelVertex *nv;

        while (cap < pb->n[which] + count)
            cap *= 2;
        nv = realloc(pb->v[which], cap * sizeof(VoxelVertex));
        if (nv == NULL)
            return;
        pb->v[which] = nv;
        pb->cap[which] = cap;
    }
    memcpy(pb->v[which] + pb->n[which], v, count * sizeof(VoxelVertex));
    pb->n[which] += count;
}

static void BuildBegin(InstanceBuild *pb, const VoxelMapInstance *inst, Atlas *atlas, uint32_t sig)
{
    pb->active = true;
    pb->mapGroup = inst->mapGroup;
    pb->mapNum = inst->mapNum;
    pb->originX = inst->originX;
    pb->originY = inst->originY;
    pb->lift = inst->lift;
    pb->signature = sig;
    pb->atlas = atlas;
    pb->nextX = pb->nextY = 0;
    pb->n[0] = pb->n[1] = pb->n[2] = 0;
    pb->staticCount = 0;
    pb->frames = 0;
    pb->ms = 0.0;
}

// The same map, where it was when the build began (the chunks' world lookups use its place).
static bool BuildIsFor(const InstanceBuild *pb, const VoxelMapInstance *inst)
{
    return pb->active && pb->mapGroup == inst->mapGroup && pb->mapNum == inst->mapNum
        && pb->originX == inst->originX && pb->originY == inst->originY && pb->lift == inst->lift;
}

// Chunks until `deadline` (0: to the end), at least one. TRUE when the whole map is done.
static bool BuildSteps(InstanceBuild *pb, const VoxelMapInstance *inst, Uint64 deadline)
{
    static VoxelVertex *chunkVerts;
    Uint64 t0 = SDL_GetPerformanceCounter();
    bool first = true;
    VoxelBuilder b;

    if (chunkVerts == NULL)
        chunkVerts = malloc(CHUNK_VERTS * sizeof(VoxelVertex));
    if (chunkVerts == NULL)
        return true;
    pb->frames++;
    while (pb->nextY < inst->height)
    {
        int x0 = inst->originX + pb->nextX, y0 = inst->originY + pb->nextY;
        int x1 = x0 + CHUNK, y1 = y0 + CHUNK;
        unsigned terrainEnd, treeEnd;

        if (!first && deadline != 0 && SDL_GetPerformanceCounter() >= deadline)
        {
            pb->ms += (SDL_GetPerformanceCounter() - t0) * 1000.0 / SDL_GetPerformanceFrequency();
            return false;
        }
        first = false;
        if (x1 > inst->originX + inst->width)
            x1 = inst->originX + inst->width;
        if (y1 > inst->originY + inst->height)
            y1 = inst->originY + inst->height;
        VoxelBuilder_Init(&b, chunkVerts, CHUNK_VERTS);
        b.statics = sChunkStatics;
        b.staticCapacity = ARRAY_COUNT(sChunkStatics);
        VoxelBuilder_SetAtlas(&b, &pb->atlas->map);
        // In the map's own frame: crossing a border moves the maps, not their meshes.
        VoxelBuilder_SetOrigin(&b, inst->originX, inst->originY);
        b.base = VoxelRelief_Base(inst);
        if (getenv("WJPC_3D_NOSTRETCH") == NULL)
            VoxelBuilder_SetStretch(&b, inst);
        if (getenv("WJPC_3D_LOGBASE") && x0 == inst->originX && y0 == inst->originY)
            Voxel3D_Log("[3D] %d:%d base %.2f stretch %.2f from %.2f to %.2f", inst->mapGroup, inst->mapNum, b.base, b.stretchAdd, b.stretchFrom, b.stretchTo);
        b.lighting = !inst->indoor;
        b.lightingRefine = true;
        VoxelMesh_BeginWindow(x0 - 1, y0 - VOXEL_CHUNK_MARGIN_NORTH, x1 + 1, y1 + 1);
        for (int y = y0; y < y1; y++)
            VoxelMesh_EmitGroundRow(&b, inst, x0, x1, y);
        terrainEnd = b.count;
        VoxelTree_EmitInstance(&b, inst, x0, y0, x1, y1);
        treeEnd = b.count;
        VoxelBuildings_EmitInstance(&b, inst, x0, y0, x1, y1);
        BuildPush(pb, 0, b.vertices, terrainEnd);
        BuildPush(pb, 1, b.vertices + terrainEnd, treeEnd - terrainEnd);
        BuildPush(pb, 2, b.vertices + treeEnd, b.count - treeEnd);
        for (unsigned st = 0; st < b.staticCount && pb->staticCount < MAX_INST_STATICS; st++)
        {
            if (getenv("WJPC_3D_LOGBASE"))
                Voxel3D_Log("[3D] %d:%d static model %u at %.1f %.2f %.1f size %.1f x %.1f top %.2f", inst->mapGroup, inst->mapNum,
                            b.statics[st].pageModel, b.statics[st].x, b.statics[st].y, b.statics[st].z, b.statics[st].w, b.statics[st].d, b.statics[st].top);
            pb->statics[pb->staticCount++] = b.statics[st];
        }
        pb->nextX += CHUNK;
        if (pb->nextX >= inst->width)
        {
            pb->nextX = 0;
            pb->nextY += CHUNK;
        }
    }
    pb->ms += (SDL_GetPerformanceCounter() - t0) * 1000.0 / SDL_GetPerformanceFrequency();
    return true;
}

// The finished build into `m` (freed first if it held a mesh), which is then one of this frame's maps.
static void BuildFinish(InstanceBuild *pb, const VoxelMapInstance *inst, InstanceMesh *m)
{
    unsigned ground = pb->n[0] < SCRATCH_VERTS ? pb->n[0] : SCRATCH_VERTS;

    if (m->used)
        InstanceFree(m);
    // Terrain split by atlas page (u = 2 * page + u in page).
    for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
    {
        unsigned n = 0;
        for (unsigned i = 0; i + 2 < ground; i += 3)
        {
            unsigned page = (unsigned)(pb->v[0][i].u * 0.5f);
            if (page != p)
                continue;
            for (int k = 0; k < 3; k++)
            {
                sSorted[n] = pb->v[0][i + k];
                sSorted[n].u -= 2.0f * p;
                n++;
            }
        }
        if (n != 0)
            MeshSet(&m->terrain[p], sSorted, n, GL_STATIC_DRAW);
        else
            MeshFree(&m->terrain[p]);
    }
    MeshSet(&m->trees, pb->v[1], pb->n[1], GL_STATIC_DRAW);
    MeshSet(&m->buildings, pb->v[2], pb->n[2], GL_STATIC_DRAW);
    memcpy(m->statics, pb->statics, pb->staticCount * sizeof(VoxelStaticDraw));
    m->staticCount = pb->staticCount;
    m->buildingPage = VoxelBuildings_PageOf(inst);
    m->atlas = pb->atlas;
    m->primary = pb->atlas->primary;
    m->secondary = pb->atlas->secondary;
    m->used = true;
    m->shown = true;
    m->mapGroup = inst->mapGroup;
    m->mapNum = inst->mapNum;
    m->originX = inst->originX;
    m->originY = inst->originY;
    m->lift = inst->lift;
    m->signature = pb->signature;
    m->lastUse = sFrame;
    pb->active = false;
    Voxel3D_Log("[3D] built %d:%d (%dx%d): %u ground, %u tree, %u model vertices, %u big models; %.1f ms over %u frame%s",
                inst->mapGroup, inst->mapNum, inst->width, inst->height, pb->n[0], pb->n[1], pb->n[2],
                m->staticCount, pb->ms, pb->frames, pb->frames == 1 ? "" : "s");
}

// ---------------------------------------------------------------------------
// The belt: filler (sea, forest) round the maps, the world's edge.

static Mesh sBeltTerrain[VOXEL_ATLAS_PAGES], sBeltTrees;
static Atlas *sBeltAtlas;
static int sBeltX = 0x7FFFFFFF, sBeltY;
// DLX-18: after a crossing the old belt is drawn moved into the new map's coordinates until the new one is
// ready; sBeltRedo asks for that (all pieces again, a few a frame).
static int sBeltShiftX, sBeltShiftZ;
static bool sBeltRedo;
static Atlas *sBeltCacheAtlas;
#define BELT_RADIUS 40

// The belt is made of CHUNK x CHUNK pieces in world cells (origin 0,0), so a piece is the same wherever the
// player stands. Pieces are kept in a cache: walking on only builds the new ones, a few a frame, and the
// belt swaps to the new position once all its pieces are ready (DLX-7: no 5-12 ms hitch every 2 chunks).
// The cache is emptied when what a piece depends on changes: the map (its border, base and lighting),
// the atlas or the world (VoxelWorld_LiveDigest); those still build the whole belt at once, as before.
typedef struct
{
    int x0, y0;
    VoxelVertex *v;
    unsigned terrainN, treeN, cap, lastUse;
    bool valid;
} BeltChunk;

#define BELT_CACHE 256
#define BELT_CHUNKS_PER_FRAME 10
static BeltChunk sBeltCache[BELT_CACHE];
static unsigned sBeltUse;
static int sBeltWantX = 0x7FFFFFFF, sBeltWantY;

static void BeltCache_Flush(void)
{
    for (int i = 0; i < BELT_CACHE; i++)
        sBeltCache[i].valid = false;
    sBeltWantX = 0x7FFFFFFF;
}

static BeltChunk *BeltCache_Find(int x0, int y0)
{
    for (int i = 0; i < BELT_CACHE; i++)
        if (sBeltCache[i].valid && sBeltCache[i].x0 == x0 && sBeltCache[i].y0 == y0)
            return &sBeltCache[i];
    return NULL;
}

static bool InBeltWindow(int x0, int y0, int cx, int cy)
{
    return x0 >= cx - BELT_RADIUS && x0 < cx + BELT_RADIUS && y0 >= cy - BELT_RADIUS && y0 < cy + BELT_RADIUS;
}

// Emit one piece and keep it. Its slot: a free one, else the least recently used piece outside the window.
static void BeltCache_Build(const VoxelMapInstance *inst0, Atlas *atlas, int x0, int y0, int cx, int cy)
{
    static VoxelVertex *chunkVerts;
    VoxelBuilder b;
    BeltChunk *slot = NULL;
    unsigned terrainEnd;

    for (int i = 0; i < BELT_CACHE; i++)
    {
        BeltChunk *c = &sBeltCache[i];
        if (!c->valid)
        {
            slot = c;
            break;
        }
        if (!InBeltWindow(c->x0, c->y0, cx, cy) && (slot == NULL || c->lastUse < slot->lastUse))
            slot = c;
    }
    if (slot == NULL)
        return; // cannot happen: the window holds 100 pieces, the cache 256
    if (chunkVerts == NULL)
        chunkVerts = malloc(CHUNK_VERTS * sizeof(VoxelVertex));
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
    if (slot->cap < b.count)
    {
        free(slot->v);
        slot->cap = b.count;
        slot->v = malloc(b.count * sizeof(VoxelVertex));
    }
    if (b.count != 0)
        memcpy(slot->v, b.vertices, b.count * sizeof(VoxelVertex));
    slot->x0 = x0;
    slot->y0 = y0;
    slot->terrainN = terrainEnd;
    slot->treeN = b.count - terrainEnd;
    slot->lastUse = sBeltUse;
    slot->valid = true;
}

// Build up to `budget` missing pieces of the window round (cx, cy) (budget < 0: all of them).
// TRUE when every piece is ready (with a budget: and none had to be built this time).
static bool BeltBuildStep(const VoxelMapInstance *inst0, Atlas *atlas, int cx, int cy, int budget)
{
    bool ready = true;

    for (int y0 = cy - BELT_RADIUS; y0 < cy + BELT_RADIUS; y0 += CHUNK)
        for (int x0 = cx - BELT_RADIUS; x0 < cx + BELT_RADIUS; x0 += CHUNK)
        {
            if (BeltCache_Find(x0, y0) != NULL)
                continue;
            if (budget == 0)
            {
                ready = false;
                continue;
            }
            BeltCache_Build(inst0, atlas, x0, y0, cx, cy);
            if (budget > 0)
            {
                budget--;
                ready = false; // put it together next frame, so building and uploading never share one
            }
        }
    return ready;
}

// Put the window's pieces together and upload them (all pieces must be in the cache).
// Ground triangles go straight into their atlas page's run of sSorted (count, then place: one pass each).
static void BeltAssemble(Atlas *atlas, int cx, int cy)
{
    unsigned count[VOXEL_ATLAS_PAGES] = {0}, at[VOXEL_ATLAS_PAGES], total = 0;

    sTerrainCount = sTreeCount = sBuildCount = 0;
    sBeltUse++;
    for (int pass = 0; pass < 2; pass++)
    {
        if (pass == 1)
            for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
            {
                at[p] = total;
                total += count[p];
            }
        for (int y0 = cy - BELT_RADIUS; y0 < cy + BELT_RADIUS; y0 += CHUNK)
            for (int x0 = cx - BELT_RADIUS; x0 < cx + BELT_RADIUS; x0 += CHUNK)
            {
                BeltChunk *c = BeltCache_Find(x0, y0);
                if (c == NULL)
                    continue;
                for (unsigned i = 0; i + 2 < c->terrainN; i += 3)
                {
                    unsigned page = (unsigned)(c->v[i].u * 0.5f);
                    if (page >= VOXEL_ATLAS_PAGES)
                        continue;
                    if (pass == 0)
                    {
                        count[page] += 3;
                        continue;
                    }
                    for (int k = 0; k < 3; k++)
                    {
                        sSorted[at[page]] = c->v[i + k];
                        sSorted[at[page]].u -= 2.0f * page;
                        at[page]++;
                    }
                }
                if (pass == 1)
                {
                    c->lastUse = sBeltUse;
                    for (unsigned i = c->terrainN; i < c->terrainN + c->treeN && sTreeCount < SCRATCH_VERTS; i++)
                        sTreeBuf[sTreeCount++] = c->v[i];
                }
            }
        if (pass == 0)
        {
            unsigned sum = 0;
            for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
                sum += count[p];
            if (sum > SCRATCH_VERTS) // far beyond any real belt; keep it safe anyway
            {
                memset(count, 0, sizeof(count));
                break;
            }
        }
    }
    total = 0;
    for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
    {
        MeshSet(&sBeltTerrain[p], sSorted + total, count[p], GL_STATIC_DRAW);
        total += count[p];
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
// DLX-5: the battle scenery. The field camera is kept aside while a battle is drawn and put back after.
#include "voxel_battle.h"
static bool sBattleMode, sBattleCamOn;
static VoxelCamera sFieldCamera;
static float sStageDX, sStageDZ, sStageGround;

void Voxel3D_SetBattle(bool on)
{
    sBattleMode = on;
}
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
    VoxelWorld_SeasonTexels(texels, VOXEL_TREE_TEXTURE_DIM * VOXEL_TREE_TEXTURE_DIM); // DLX-17
    VoxelGrade_Texels(texels, VOXEL_TREE_TEXTURE_DIM * VOXEL_TREE_TEXTURE_DIM);
    sTreeTex = UploadTiled(sTreeTex, texels, VOXEL_TREE_TEXTURE_DIM, VOXEL_TREE_TEXTURE_DIM);
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


/* ---------------------------------------------------------------------------
 * Gamma Emerald's screen pass (docs/GAMMA_LIGHTING_VALUES.md): the scene is drawn in linear light to a
 * half-float target, then: bloom (BP_StylizedSky post process: intensity 6, threshold 0.4, sizes 4..128 x 15,
 * UE's default tints), depth of field (player camera: f/1.744, sensor 450, Gamma's 100-degree lens,
 * focused on the player), exposure locked at 1, saturation 1.25 (camera) in AP1, and UE's filmic curve
 * (slope 1, toe 0.3, shoulder 1, black clip 0, white clip 0) - the ACES-based FilmToneMap. */

/* Gamma's camera gives 2^1 x 0.18 / 1 = 0.36; our GBA art is drawn fully lit, so the multiplier is the one
 * that returns sunlit ground to its own colours (fitted through this exact curve): 2.06. */
static float sExposure = 2.06f;
static GLuint sBlurFbo[6], sBlurTex[6];
static GLuint sPostProg, sQuadVbo, sHdrFbo, sHdrTex, sHdrDepth, sBloomFbo[7], sBloomTex[7];
static int sHdrW, sHdrH, sBloomW[7], sBloomH[7];
static GLint sPU_mode, sPU_src, sPU_src2, sPU_depth, sPU_texel, sPU_dir, sPU_radius, sPU_misc, sPU_fade, sPU_tints, sPU_coc;

static const char *sPostVs =
    "#version 120\n"
    "attribute vec2 p;\n"
    "varying vec2 uv;\n"
    "void main() { uv = p * 0.5 + 0.5; gl_Position = vec4(p, 0.0, 1.0); }\n";

static const char *sPostFs =
    "#version 120\n"
    "uniform float mode;\n"          // 0 bright pass, 1 blur, 2 final
    "uniform sampler2D src, src2, depthTex, b1, b2, b3, b4, b5;\n"
    "uniform float tint[6];\n"
    "uniform vec2 texel, dir;\n"
    "uniform float radius;\n"
    "uniform vec4 misc;\n"           // x exposure, y saturation, z bloom intensity, w threshold
    "uniform vec4 fade;\n"
    "uniform vec4 coc;\n"            // x A*f/(S-f)/sensor (per mm of depth), y focus (mm), z near, w far
    "varying vec2 uv;\n"
    "const mat3 sRGB_2_AP1 = mat3(0.61319, 0.07021, 0.02062, 0.33951, 0.91634, 0.10957, 0.04737, 0.01345, 0.86961);\n"
    "const mat3 AP1_2_sRGB = mat3(1.70505, -0.13026, -0.02400, -0.62179, 1.14080, -0.12897, -0.08326, -0.01055, 1.15297);\n"
    "const mat3 sRGB_2_AP0 = mat3(0.4397010, 0.0897923, 0.0175440, 0.3829780, 0.8134230, 0.1115440, 0.1773350, 0.0967616, 0.8707040);\n"
    "const mat3 AP0_2_AP1 = mat3(1.4514393161, -0.0765537734, 0.0083161484, -0.2365107469, 1.1762296998, -0.0060324498, -0.2149285693, -0.0996759264, 0.9977163014);\n"
    "const vec3 AP1_Y = vec3(0.2722287168, 0.6740817658, 0.0536895174);\n"
    "float sat3(vec3 c) { float mi = min(c.r, min(c.g, c.b)), ma = max(c.r, max(c.g, c.b)); return (max(ma, 1e-10) - max(mi, 1e-10)) / max(ma, 1e-2); }\n"
    "float yc(vec3 c) { float ch = sqrt(max(c.b*(c.b-c.g) + c.g*(c.g-c.r) + c.r*(c.r-c.b), 0.0)); return (c.b + c.g + c.r + 1.75*ch) / 3.0; }\n"
    "float sigmoid(float x) { float t = max(1.0 - abs(0.5*x), 0.0); return (1.0 + sign(x)*(1.0 - t*t)) * 0.5; }\n"
    "float glow(float y, float g, float m) { if (y <= 2.0/3.0*m) return g; if (y >= 2.0*m) return 0.0; return g*(m/y - 0.5); }\n"
    "float hue(vec3 c) { if (c.r == c.g && c.g == c.b) return 0.0; float h = degrees(atan(sqrt(3.0)*(c.g-c.b), 2.0*c.r-c.g-c.b)); return h < 0.0 ? h + 360.0 : h; }\n"
    "vec3 film(vec3 lin) {\n"   // UE FilmToneMap, returns AP1
    "  const float slope = 1.0, toe = 0.3, shoulder = 1.0, black = 0.0, white = 0.0;\n"
    "  vec3 aces = sRGB_2_AP0 * lin;\n"
    "  float s = sat3(aces);\n"
    "  aces *= 1.0 + glow(yc(aces), 0.05 * sigmoid((s - 0.4) / 0.2), 0.08);\n"
    "  float h = hue(aces); if (h > 180.0) h -= 360.0;\n"
    "  float hw = smoothstep(0.0, 1.0, 1.0 - abs(2.0 * h / 135.0)); hw *= hw;\n"
    "  aces.r += hw * s * (0.03 - aces.r) * (1.0 - 0.82);\n"
    "  vec3 w = clamp(AP0_2_AP1 * aces, 0.0, 65504.0);\n"
    "  w = mix(vec3(dot(w, AP1_Y)), w, 0.96);\n"
    "  float toeScale = 1.0 + black - toe, shScale = 1.0 + white - shoulder;\n"
    "  float bt = (0.18 + black) / toeScale - 1.0;\n"
    "  float toeMatch = log(0.18)/log(10.0) - 0.5 * log((1.0 + bt) / (1.0 - bt)) * (toeScale / slope);\n"
    "  float straightMatch = (1.0 - toe) / slope - toeMatch;\n"
    "  float shMatch = shoulder / slope - straightMatch;\n"
    "  vec3 L = log(max(w, 1e-10)) / log(10.0);\n"
    "  vec3 st = slope * (L + straightMatch);\n"
    "  vec3 tc = -black + (2.0 * toeScale) / (1.0 + exp((-2.0 * slope / toeScale) * (L - toeMatch)));\n"
    "  vec3 sc = (1.0 + white) - (2.0 * shScale) / (1.0 + exp((2.0 * slope / max(shScale, 1e-6)) * (L - shMatch)));\n"
    "  tc = mix(st, tc, vec3(lessThan(L, vec3(toeMatch))));\n"
    "  sc = mix(st, sc, vec3(greaterThan(L, vec3(shMatch))));\n"
    "  vec3 t = clamp((L - toeMatch) / (shMatch - toeMatch), 0.0, 1.0);\n"
    "  if (shMatch < toeMatch) t = 1.0 - t;\n"
    "  t = (3.0 - 2.0 * t) * t * t;\n"
    "  vec3 tone = mix(tc, sc, t);\n"
    "  tone = mix(vec3(dot(tone, AP1_Y)), tone, 0.93);\n"
    "  return max(tone, 0.0);\n"
    "}\n"
    "vec3 toSrgb(vec3 c) { c = clamp(c, 0.0, 1.0); return mix(c * 12.92, 1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055, step(0.0031308, c)); }\n"
    "float viewDepth(vec2 t) { float z = texture2D(depthTex, t).r * 2.0 - 1.0; return 2.0 * coc.z * coc.w / (coc.w + coc.z - z * (coc.w - coc.z)); }\n"
    "void main() {\n"
    "  if (mode < 0.5) {\n" // bright pass (UE BloomSetup): colour * saturate((lum*exposure - threshold) * 0.5)
    "    vec3 c = texture2D(src, uv).rgb;\n"
    "    float l = dot(c, vec3(0.212639, 0.715169, 0.072192)) * misc.x;\n"
    "    gl_FragColor = vec4(c * clamp((l - misc.w) * 0.5, 0.0, 1.0), 1.0); return;\n"
    "  }\n"
    "  if (mode < 1.5) {\n" // separable gaussian, radius in texels
    "    vec3 a = vec3(0.0); float wsum = 0.0;\n"
    "    for (int i = -12; i <= 12; i++) { float x = float(i) / 12.0 * radius; float wt = exp(-0.5 * (x*x) / max(radius*radius/9.0, 1e-4));\n"
    "      a += texture2D(src, uv + dir * texel * x).rgb * wt; wsum += wt; }\n"
    "    gl_FragColor = vec4(a / wsum, 1.0); return;\n"
    "  }\n"
    // final: depth of field (gather), bloom, exposure, grading, film curve
    "  float D = viewDepth(uv) * 640.0;\n"   // tiles -> mm (64 cm a tile)
    "  float r = min(abs(D - coc.y) / max(D, 1.0) * coc.x * 0.5, 24.0);\n" // radius in pixels
    "  vec3 c = vec3(0.0); float wsum = 0.0;\n"
    "  for (int i = 0; i < 24; i++) { float a = float(i) * 2.39996; float rr = sqrt((float(i) + 0.5) / 24.0) * r;\n"
    "    vec2 o = vec2(cos(a), sin(a)) * rr * texel; float Dn = viewDepth(uv + o) * 640.0;\n"
    "    float rn = min(abs(Dn - coc.y) / max(Dn, 1.0) * coc.x * 0.5, 24.0);\n"
    "    float wt = (Dn < D - 1.0) ? clamp(rn - rr + 1.0, 0.0, 1.0) : 1.0;\n"
    "    c += texture2D(src, uv + o).rgb * wt; wsum += wt; }\n"
    "  c /= max(wsum, 1e-4);\n"
    "  c += (texture2D(src2, uv).rgb * tint[0] + texture2D(b1, uv).rgb * tint[1] + texture2D(b2, uv).rgb * tint[2]\n"
    "       + texture2D(b3, uv).rgb * tint[3] + texture2D(b4, uv).rgb * tint[4] + texture2D(b5, uv).rgb * tint[5]) * misc.z;\n"
    "  c *= misc.x;\n"
    "  vec3 ap = sRGB_2_AP1 * c;\n"
    "  ap = max(mix(vec3(dot(ap, AP1_Y)), ap, misc.y), 0.0);\n"
    "  vec3 outc = toSrgb(AP1_2_sRGB * film(AP1_2_sRGB * ap));\n"
    "  gl_FragColor = vec4(mix(outc, fade.rgb, fade.a), 1.0);\n"
    "}\n";

static bool PostInit(void)
{
    static const float quad[8] = {-1, -1, 1, -1, -1, 1, 1, 1};
    GLuint vs, fs;
    GLint ok = 0;
    if (sPostProg != 0)
        return true;
    vs = Compile(GL_VERTEX_SHADER, sPostVs);
    fs = Compile(GL_FRAGMENT_SHADER, sPostFs);
    if (vs == 0 || fs == 0)
        return false;
    sPostProg = wj_glCreateProgram();
    wj_glAttachShader(sPostProg, vs);
    wj_glAttachShader(sPostProg, fs);
    wj_glBindAttribLocation(sPostProg, 0, "p");
    wj_glLinkProgram(sPostProg);
    wj_glGetProgramiv(sPostProg, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        wj_glGetProgramInfoLog(sPostProg, sizeof(log), NULL, log);
        Voxel3D_Log("[3D] post link: %s", log);
        sPostProg = 0;
        return false;
    }
    sPU_mode = wj_glGetUniformLocation(sPostProg, "mode");
    sPU_src = wj_glGetUniformLocation(sPostProg, "src");
    sPU_src2 = wj_glGetUniformLocation(sPostProg, "src2");
    sPU_depth = wj_glGetUniformLocation(sPostProg, "depthTex");
    sPU_texel = wj_glGetUniformLocation(sPostProg, "texel");
    sPU_dir = wj_glGetUniformLocation(sPostProg, "dir");
    sPU_radius = wj_glGetUniformLocation(sPostProg, "radius");
    sPU_misc = wj_glGetUniformLocation(sPostProg, "misc");
    sPU_fade = wj_glGetUniformLocation(sPostProg, "fade");
    sPU_coc = wj_glGetUniformLocation(sPostProg, "coc");
    wj_glGenBuffers(1, &sQuadVbo);
    wj_glBindBuffer(GL_ARRAY_BUFFER, sQuadVbo);
    wj_glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    return true;
}

static GLuint MakeTarget(GLuint *tex, int w, int h, GLenum fmt, GLenum type, GLenum attach)
{
    GLuint fbo = 0;
    if (*tex == 0)
        wj_glGenTextures(1, tex);
    wj_glBindTexture(GL_TEXTURE_2D, *tex);
    wj_glTexImage2D(GL_TEXTURE_2D, 0, fmt == 0x1902 ? 0x81A6 /* DEPTH24 */ : 0x881A /* RGBA16F */, w, h, 0, fmt, type, NULL);
    wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F);
    wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F);
    (void)attach;
    return fbo;
}

/* Bind the half-float scene target (with a depth texture for the blur). */
static bool PostBeginScene(int w, int h)
{
    if (!PostInit())
        return false;
    if (sHdrW != w || sHdrH != h)
    {
        MakeTarget(&sHdrTex, w, h, GL_RGBA, 0x140B, 0);
        MakeTarget(&sHdrDepth, w, h, 0x1902, GL_UNSIGNED_INT, 0);
        for (int i = 0; i < 7; i++)
        {
            sBloomW[i] = w >> (i + 1) > 1 ? w >> (i + 1) : 1;
            sBloomH[i] = h >> (i + 1) > 1 ? h >> (i + 1) : 1;
            MakeTarget(&sBloomTex[i], sBloomW[i], sBloomH[i], GL_RGBA, 0x140B, 0);
            if (sBloomFbo[i] == 0)
                wj_glGenFramebuffers(1, &sBloomFbo[i]);
            wj_glBindFramebuffer(GL_FRAMEBUFFER, sBloomFbo[i]);
            wj_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sBloomTex[i], 0);
            if (i < 6)
            {
                MakeTarget(&sBlurTex[i], sBloomW[i], sBloomH[i], GL_RGBA, 0x140B, 0);
                if (sBlurFbo[i] == 0)
                    wj_glGenFramebuffers(1, &sBlurFbo[i]);
                wj_glBindFramebuffer(GL_FRAMEBUFFER, sBlurFbo[i]);
                wj_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sBlurTex[i], 0);
            }
        }
        if (sHdrFbo == 0)
            wj_glGenFramebuffers(1, &sHdrFbo);
        wj_glBindFramebuffer(GL_FRAMEBUFFER, sHdrFbo);
        wj_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sHdrTex, 0);
        wj_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sHdrDepth, 0);
        sHdrW = w;
        sHdrH = h;
    }
    wj_glBindFramebuffer(GL_FRAMEBUFFER, sHdrFbo);
    return wj_glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

static void PostQuad(void)
{
    wj_glBindBuffer(GL_ARRAY_BUFFER, sQuadVbo);
    wj_glDisableVertexAttribArray(1);
    wj_glDisableVertexAttribArray(2);
    wj_glEnableVertexAttribArray(0);
    wj_glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    wj_glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void PostBlur(GLuint fbo, GLuint srcTex, int w, int h, float dx, float dy, float radius)
{
    wj_glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    wj_glViewport(0, 0, w, h);
    wj_glActiveTexture(GL_TEXTURE0);
    wj_glBindTexture(GL_TEXTURE_2D, srcTex);
    wj_glUniform1f(sPU_mode, 1.0f);
    wj_glUniform2f(sPU_texel, 1.0f / w, 1.0f / h);
    wj_glUniform2f(sPU_dir, dx, dy);
    wj_glUniform1f(sPU_radius, radius);
    PostQuad();
}

/* Scene is in sHdrTex: bloom, then the final pass into the caller's framebuffer. */
static void PostFinish(GLuint outFbo, int w, int h, const float fade[4], float playerDist)
{
    /* UE default bloom tints; sizes are % of screen width x BloomSizeScale (Gamma: 4..128 x 15). */
    static const float tint[6] = {0.3465f, 0.138f, 0.1176f, 0.066f, 0.066f, 0.061f};
    wj_glDisable(GL_DEPTH_TEST);
    wj_glDisable(GL_BLEND);
    wj_glUseProgram(sPostProg);
    wj_glUniform1i(sPU_src, 0);
    wj_glUniform1i(sPU_src2, 1);
    wj_glUniform1i(sPU_depth, 2);
    wj_glUniform4f(sPU_misc, sExposure, 1.25f, 6.0f, 0.4f);
    /* bright pass at 1/4 size into level 0 */
    wj_glBindFramebuffer(GL_FRAMEBUFFER, sBloomFbo[0]);
    wj_glViewport(0, 0, sBloomW[0], sBloomH[0]);
    wj_glActiveTexture(GL_TEXTURE0);
    wj_glBindTexture(GL_TEXTURE_2D, sHdrTex);
    wj_glUniform1f(sPU_mode, 0.0f);
    PostQuad();
    /* Six stages at 1/2 .. 1/64 size (bloom stage k on downsample k). Gamma's sizes (4..128 x 15 % of the
     * width) are far beyond the 32-sample limit (r.Filter.LoopMode 0, read from the game's program), so
     * every stage's radius is clamped to 31 texels of its own resolution. */
    {
        for (int i = 1; i < 6; i++)
            PostBlur(sBloomFbo[i], sBloomTex[i - 1], sBloomW[i], sBloomH[i], 0.0f, 0.0f, 0.0f);
        for (int i = 0; i < 6; i++)
        {
            PostBlur(sBlurFbo[i], sBloomTex[i], sBloomW[i], sBloomH[i], 1.0f, 0.0f, 31.0f);
            PostBlur(sBloomFbo[i], sBlurTex[i], sBloomW[i], sBloomH[i], 0.0f, 1.0f, 31.0f);
        }
        int lv = 0;
        wj_glUseProgram(sPostProg);
        wj_glUniform4f(sPU_misc, sExposure, 1.25f, getenv("WJPC_3D_NOBLOOM") ? 0.0f : 6.0f, 0.4f);
        wj_glUniform1fv(wj_glGetUniformLocation(sPostProg, "tint"), 6, tint);
        for (int i = 1; i < 6; i++)
        {
            char nm[4] = {'b', (char)('0' + i), 0, 0};
            wj_glUniform1i(wj_glGetUniformLocation(sPostProg, nm), 2 + i);
            wj_glActiveTexture(GL_TEXTURE2 + i);
            wj_glBindTexture(GL_TEXTURE_2D, sBloomTex[i]);
        }
        /* final */
        wj_glBindFramebuffer(GL_FRAMEBUFFER, outFbo);
        wj_glViewport(0, 0, w, h);
        wj_glActiveTexture(GL_TEXTURE2);
        wj_glBindTexture(GL_TEXTURE_2D, sHdrDepth);
        wj_glTexParameteri(GL_TEXTURE_2D, 0x884C /* COMPARE_MODE */, 0);
        wj_glActiveTexture(GL_TEXTURE1);
        wj_glBindTexture(GL_TEXTURE_2D, sBloomTex[lv]);
        wj_glActiveTexture(GL_TEXTURE0);
        wj_glBindTexture(GL_TEXTURE_2D, sHdrTex);
        wj_glUniform2f(sPU_texel, 1.0f / w, 1.0f / h);
        wj_glUniform4f(sPU_fade, fade[0], fade[1], fade[2], fade[3]);
        {
            /* Gamma's lens: 100-degree FOV on a 450 mm sensor -> f = 188.8 mm, f/1.744 -> A = 108.2 mm.
             * CoC (fraction of sensor) = A f |D - S| / (D (S - f)) / 450; pixels = fraction x width. */
            const float f = 450.0f / (2.0f * 1.19175359f), A = f / 1.7442f;
            float S = playerDist * 640.0f;
            float k = A * f / ((S - f) > 1.0f ? (S - f) : 1.0f) / 450.0f * (float)w;
            wj_glUniform4f(sPU_coc, k, S, 0.5f, 400.0f);
        }
        wj_glUniform1f(sPU_mode, 2.0f);
        PostQuad();
        wj_glActiveTexture(GL_TEXTURE0);
    }
}

bool Voxel3D_Init(const char *dataDir)
{
    if (sInited || sFailed)
        return sInited;
    sFailed = true;
    snprintf(sDataDir, sizeof(sDataDir), "%s", dataDir);
    if (!LoadGl() || !MakeProgram())
        return false;
    if (getenv("WJPC_3D_GAMMALIGHT") != NULL && atoi(getenv("WJPC_3D_GAMMALIGHT")) != 0)
    {
        /* Gamma's sun: 40 degrees up, behind the viewer and slightly right, as (-DX, 1, -DZ). */
        gVoxelGammaLight = 1;
        VoxelLighting_SetSun(-0.2718f, -1.1604f, 1.0f, 0x6A33u);
        {   /* Gamma's cloud-shadow textures (T_NoisePuddles, T_NoiseRough), 1024x1024 greyscale each */
            FILE *cf = Voxel3D_OpenData("voxel/gamma_clouds.bin");
            unsigned char *px = malloc(2u * 1024 * 1024);
            if (cf != NULL && px != NULL && fread(px, 1, 2u * 1024 * 1024, cf) == 2u * 1024 * 1024)
                for (int t = 0; t < 2; t++)
                {
                    wj_glGenTextures(1, &sCloudTex[t]);
                    wj_glBindTexture(GL_TEXTURE_2D, sCloudTex[t]);
                    wj_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                    wj_glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 1024, 1024, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, px + t * 1024 * 1024);
                    wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                }
            if (cf != NULL)
                fclose(cf);
            free(px);
        }
    }
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

static InstanceMesh *MeshOf(int mapGroup, int mapNum)
{
    for (int i = 0; i < MAX_INSTANCES; i++)
        if (sInstanceMeshes[i].used && sInstanceMeshes[i].mapGroup == mapGroup && sInstanceMeshes[i].mapNum == mapNum)
            return &sInstanceMeshes[i];
    return NULL;
}

// A slot for that map's mesh: its own, a free one, or the one longest out of the view (NULL: all on screen).
static InstanceMesh *MeshSlotFor(int mapGroup, int mapNum)
{
    InstanceMesh *pick = MeshOf(mapGroup, mapNum);

    if (pick != NULL)
        return pick;
    for (int i = 0; i < MAX_INSTANCES; i++)
    {
        InstanceMesh *m = &sInstanceMeshes[i];
        if (!m->used)
            return m;
        if (!m->shown && (pick == NULL || m->lastUse < pick->lastUse))
            pick = m;
    }
    return pick;
}

// Its atlas still holds its tile sets (a slot can be taken for another pair once the map is out of sight).
static bool MeshAtlasOk(const InstanceMesh *m)
{
    return m->atlas != NULL && m->atlas->valid && m->atlas->primary == m->primary && m->atlas->secondary == m->secondary;
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

    // DLX-17: the trees' and building models' baked textures follow the season too; reload them when it changes.
    {
        static uint32_t sTexSeason = 0xFFFFFFFFu;
        uint32_t season = SeasonNow();
        if (season != sTexSeason)
        {
            if (sTreeTex != 0)
                LoadTrees();
            for (int p = 0; p < MAX_PAGES; p++)
                if (sPageTex[p] != 0)
                    LoadBuildingPage(p, true); // same texture, new colours
            sTexSeason = season;
        }
    }
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
        // DLX-18: walked across a seam (the new map was on screen, its place known in last frame's
        // coordinates): everything moves by that much, so the camera and the belt are moved with it rather
        // than started again (a jump of the camera and a whole belt built on the step that crossed).
        InstanceMesh *was = MeshOf(group, num);
        bool crossed = was != NULL && was->shown && !inst0->indoor && sCameraValid && !sBattleMode
                    && getenv("WJPC_3D_OLD_CROSSING") == NULL;

        VoxelBuildings_MapChanged();
        VoxelLighting_Reset();
        if (crossed)
        {
            VoxelCamera_Shift(&sCamera, (float)-was->originX, (float)-was->originY);
            sBeltShiftX -= was->originX;
            sBeltShiftZ -= was->originY;
            BeltCache_Flush(); // its pieces were the old map's (border, base); built again a few a frame
            sBeltRedo = true;
        }
        else
        {
            sCameraValid = false;
            sBeltX = 0x7FFFFFFF;
        }
        sLastGroup = group;
        sLastNum = num;
        VoxelEntities_Reset();
    }
    else if (digest != sLastDigest)
    {
        VoxelBuildings_WorldChanged();
        for (int i = 0; i < MAX_INSTANCES; i++)
            if (sInstanceMeshes[i].used)
                sInstanceMeshes[i].signature ^= 1; // rebuild below
        sBackBuild.active = false;
        sBeltX = 0x7FFFFFFF;
    }
    sLastDigest = digest;

    SaveGl(&saved);
    Uint64 workStart = SDL_GetPerformanceCounter();

    // Maps (DLX-18). The player's own is built at once if it has to be; the others in the background, a few
    // ms a frame, nearest first, old meshes shown meanwhile. While the screen is dark (a warp's fade) the
    // budget is a frame's worth. Meshes of maps that left the view are kept for when they come back.
    {
        Uint64 freq = SDL_GetPerformanceFrequency();
        float dark = 0.0f, rgb[3];
        double budget = VoxelWorld_ScreenFade(&dark, rgb) && dark >= 0.5f ? 40.0 : 3.0;
        Uint64 deadline = SDL_GetPerformanceCounter() + (Uint64)(budget * freq / 1000.0);
        const VoxelMapInstance *backInst = NULL;
        bool asked = false;

        if (getenv("WJPC_3D_BUILD_MS") != NULL)
            deadline = SDL_GetPerformanceCounter() + (Uint64)(atof(getenv("WJPC_3D_BUILD_MS")) * freq / 1000.0);
        for (int i = 0; i < MAX_INSTANCES; i++)
            sInstanceMeshes[i].shown = false;
        for (unsigned i = 0; i < count; i++)
        {
            const VoxelMapInstance *inst = VoxelWorld_Instance(i);
            InstanceMesh *m = MeshOf(inst->mapGroup, inst->mapNum);
            uint32_t sig = VoxelBuildings_LiveSignature(inst);
            Atlas *atlas = i == 0 ? GetAtlas(inst) : FindAtlas(inst);
            bool stale = m == NULL || m->signature != sig || m->lift != inst->lift || atlas == NULL
                      || m->atlas != atlas || !MeshAtlasOk(m);

            if (BuildIsFor(&sBackBuild, inst))
            {
                if (sBackBuild.signature != sig || (atlas != NULL && sBackBuild.atlas != atlas) || !sBackBuild.atlas->valid)
                    sBackBuild.active = false; // what it was building for has changed
                else
                    backInst = inst;
            }
            if (i == 0 && stale && atlas != NULL)
            {
                // Underfoot: now. A background build of it is finished rather than begun again.
                InstanceBuild *pb = backInst == inst ? &sBackBuild : &sNowBuild;

                if (pb == &sNowBuild)
                    BuildBegin(pb, inst, atlas, sig);
                BuildSteps(pb, inst, 0);
                m = MeshSlotFor(inst->mapGroup, inst->mapNum);
                if (m != NULL)
                    BuildFinish(pb, inst, m);
                pb->active = false;
                if (pb == &sBackBuild)
                    backInst = NULL;
            }
            else if (stale && !sBackBuild.active && !asked)
            {
                // The first one in need: its tile sheet (over frames), then its mesh.
                asked = true;
                atlas = PrepareAtlas(inst, deadline);
                if (atlas != NULL)
                {
                    BuildBegin(&sBackBuild, inst, atlas, sig);
                    backInst = inst;
                }
            }
            if (m != NULL && MeshAtlasOk(m))
            {
                m->shown = true;
                m->originX = inst->originX;
                m->originY = inst->originY;
                m->lastUse = sFrame;
            }
        }
        if (backInst == NULL)
            sBackBuild.active = false; // its map left the view or moved: dropped
        else if (BuildSteps(&sBackBuild, backInst, deadline))
        {
            InstanceMesh *m = MeshSlotFor(backInst->mapGroup, backInst->mapNum);
            if (m != NULL)
                BuildFinish(&sBackBuild, backInst, m);
            sBackBuild.active = false;
        }
    }

    Uint64 workMaps = SDL_GetPerformanceCounter();
    // Camera follows the player.
    VoxelWorld_GetPlayerWorldCoords(&playerX, &playerZ);
    VoxelEntities_GetPlayerWorldPos(&smoothX, &smoothZ);
    if (sBattleMode)
    {
        // Battle: a low camera on the spot the battle started, looking on past it (Emerald 3DS's angles).
        if (!sBattleCamOn)
        {
            sFieldCamera = sCamera;
            sBattleCamOn = true;
            VoxelBattle_PickStage(&sStageDX, &sStageDZ, &sStageGround); // once per battle
            Voxel3D_Log("[3D] battle stage at %+.0f,%+.0f from the player", sStageDX, sStageDZ);
        }
        sCamera.fov = VOXEL_BATTLE_FOV;
        VoxelCamera_Frame(&sCamera, smoothX + sStageDX, smoothZ + sStageDZ, sStageGround,
                          VOXEL_BATTLE_PITCH, VOXEL_BATTLE_DISTANCE);
    }
    else if (sBattleCamOn)
    {
        sCamera = sFieldCamera;
        sBattleCamOn = false;
        sCameraValid = false;
    }
    if (sBattleMode)
        ;
    else if (!sCameraValid)
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

    Uint64 workCam = SDL_GetPerformanceCounter();
    // The belt round the maps, rebuilt when the player has moved a chunk.
    {
        int cx = ((int)floorf(playerX) / CHUNK) * CHUNK, cy = ((int)floorf(playerZ) / CHUNK) * CHUNK;
        Atlas *atlas0 = GetAtlas(inst0);
        if (atlas0 != NULL && !inst0->indoor)
        {
            Uint64 t0 = SDL_GetPerformanceCounter();
            bool full = sBeltX == 0x7FFFFFFF || sBeltAtlas == NULL;

            if (!full && atlas0 != sBeltCacheAtlas)
            {
                // Into a map with other tile sets: its pieces again, a few a frame, the old belt up meanwhile.
                BeltCache_Flush();
                sBeltRedo = true;
            }
            sBeltCacheAtlas = atlas0;
            if (sBeltRedo && !full)
            {
                sBeltWantX = cx;
                sBeltWantY = cy;
                sBeltRedo = false;
            }
            if (full)
            {
                // New map, new world or new atlas: start again and build it all now (as before).
                BeltCache_Flush();
                BeltBuildStep(inst0, atlas0, cx, cy, -1);
                BeltAssemble(atlas0, cx, cy);
                sBeltX = cx;
                sBeltY = cy;
                sBeltShiftX = sBeltShiftZ = 0;
                sBeltRedo = false;
            }
            else
            {
                if (abs(cx - sBeltX) >= CHUNK * 2 || abs(cy - sBeltY) >= CHUNK * 2)
                {
                    sBeltWantX = cx;
                    sBeltWantY = cy;
                }
                if (sBeltWantX != 0x7FFFFFFF && BeltBuildStep(inst0, atlas0, sBeltWantX, sBeltWantY, BELT_CHUNKS_PER_FRAME))
                {
                    BeltAssemble(atlas0, sBeltWantX, sBeltWantY);
                    sBeltX = sBeltWantX;
                    sBeltY = sBeltWantY;
                    sBeltWantX = 0x7FFFFFFF;
                    sBeltShiftX = sBeltShiftZ = 0;
                }
            }
            double ms = (SDL_GetPerformanceCounter() - t0) * 1000.0 / SDL_GetPerformanceFrequency();
            if (ms > 0.5)
                Voxel3D_Log("[3D] belt %s %.1f ms", full ? "full" : "step", ms);
        }
    }

    {
        // Test aid: frames whose map and belt work took long (the hitches DLX-18 is about).
        double ms = (SDL_GetPerformanceCounter() - workStart) * 1000.0 / SDL_GetPerformanceFrequency();
        if (ms > 4.0 && getenv("WJPC_VOXEL_LOG") != NULL)
            Voxel3D_Log("[3D] slow frame %u: %.1f ms of map and belt work (maps %.1f, camera %.1f)", sFrame, ms,
                        (workMaps - workStart) * 1000.0 / SDL_GetPerformanceFrequency(),
                        (workCam - workMaps) * 1000.0 / SDL_GetPerformanceFrequency());
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
        unsigned changed = 0;
        if (!sBattleMode) // in a battle the people are not there (the battle draws its own)
            changed = VoxelEntities_Emit(&sprites, sSpriteAtlas, &sCamera, &shadows, &reflections);
        if (changed != 0 || sSpriteTex == 0) // 1024x512 now: not every frame when nothing in it changed
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

    if (gVoxelGammaLight && !PostBeginScene(w, h))
        gVoxelGammaLight = 0;
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
        if (gVoxelGammaLight)
        {   /* backdrop in linear light, so the screen pass returns it unchanged in look */
            r = r * r / sExposure; g = g * g / sExposure; b = b * b / sExposure;
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
    if (VoxelWorld_ScreenFade(&fadeAmount, fadeRgb) && !gVoxelGammaLight)
        wj_glUniform4f(sUniFade, fadeRgb[0], fadeRgb[1], fadeRgb[2], fadeAmount);
    else
        wj_glUniform4f(sUniFade, 0, 0, 0, 0);
    // Haze, outdoors only: starts well past the player, the sky's colour.
    if (inst0->indoor)
        wj_glUniform2f(sUniFog, 1.0e6f, 2.0e6f);
    else
        wj_glUniform2f(sUniFog, sFogNear, sFogFar);
    wj_glUniform3f(sUniFogColor, 0.55f, 0.75f, 0.95f);
    GammaLightUniforms();

    // Ground, trees and models of every map.
    for (int i = 0; i < MAX_INSTANCES; i++)
    {
        InstanceMesh *m = &sInstanceMeshes[i];
        if (!m->used || !m->shown)
            continue;
        for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
            MeshDraw(&m->terrain[p], m->atlas->tex[p], (float)m->originX, 0, (float)m->originY);
        MeshDraw(&m->trees, sTreeTex, (float)m->originX, 0, (float)m->originY);
        if (m->buildingPage >= 0)
        {
            GLuint page = BuildingPage(m->buildingPage);
            if (getenv("WJPC_VOXEL_LOG") != NULL && (sFrame % 120) == 1)
                Voxel3D_Log("[3D]   map %d:%d buildings page %d tex %u, %u chunk verts, %u statics, first model %p", m->mapGroup, m->mapNum, m->buildingPage, page, m->buildings.count, m->staticCount, m->staticCount ? (const void *)StaticModel(m->statics[0].pageModel, m->statics[0].lit) : NULL);
            MeshDraw(&m->buildings, page, (float)m->originX, 0, (float)m->originY);
            for (unsigned s = 0; s < m->staticCount; s++)
            {
                const VoxelStaticDraw *d = &m->statics[s];
                const Mesh *mesh = StaticModel(d->pageModel, d->lit);
                if (mesh != NULL)
                    MeshDraw(mesh, page, d->x + (float)m->originX, d->y, d->z + (float)m->originY);
            }
        }
    }
    if (sBeltAtlas != NULL && !inst0->indoor)
    {
        for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
            MeshDraw(&sBeltTerrain[p], sBeltAtlas->tex[p], (float)sBeltShiftX, 0, (float)sBeltShiftZ);
        MeshDraw(&sBeltTrees, sTreeTex, (float)sBeltShiftX, 0, (float)sBeltShiftZ);
    }

    // Shadows and reflections under the cards, then the cards.
    // As on the 3DS: blended, without writing depth.
    wj_glEnable(GL_BLEND);
    wj_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    wj_glDepthMask(GL_FALSE);
    wj_glUniform1f(sUniMode, 2.0f);
    if (getenv("WJPC_3D_NOREFLECT") == NULL)
        MeshDraw(&sReflectMesh, sSpriteTex, 0, 0, 0);
    wj_glUniform1f(sUniMode, 1.0f);
    if (getenv("WJPC_3D_NOSHADOW") == NULL)
        MeshDraw(&sShadowMesh, sSpriteTex, 0, 0, 0);
    wj_glDepthMask(GL_TRUE);
    wj_glDisable(GL_BLEND);
    wj_glUniform1f(sUniMode, 0.0f);
    MeshDraw(&sSpriteMesh, sSpriteTex, 0, 0, 0);

    if (gVoxelGammaLight)
    {
        float fd[4] = {fadeRgb[0], fadeRgb[1], fadeRgb[2], fadeAmount};
        float dx = sCamera.x - sCamera.targetX, dy = sCamera.y - sCamera.targetY, dz = sCamera.z - sCamera.targetZ;
        PostFinish(sFbo, w, h, fd, sqrtf(dx * dx + dy * dy + dz * dz));
    }

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
