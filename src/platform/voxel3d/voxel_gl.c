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
#include "wally_journey.h"
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
#include "voxel_env.h" // cached test switches (after the includes)

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
    X(void, glCopyTexSubImage2D, (GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei)) \
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

/*
 * Brennan 2026-11-21 02:12 (240 fps; audit docs/polish_reports/PERF_AUDIT_2026-11-21.md step 2): the animated tiles
 * re-graded and re-uploaded whole atlas pages every 16 game frames (~4 ms of "maps" standing still). Only the rows of
 * slots that changed (bit r: slots 32r..32r+31, VOXEL_ATLAS_ROW_BYTES of the tiled page each) are uploaded: the
 * texture rows that hold them, in runs, with glTexSubImage2D. `tiled` need only hold those rows.
 */
static void UploadTiledRows(GLuint tex, const uint16_t *tiled, unsigned w, unsigned h, unsigned rows)
{
    const unsigned chunk = VOXEL_ATLAS_ROW_BYTES / 2u; /* texels in a row of slots */
    int run = -1;

    if (sUntileSize < (size_t)w * h)
    {
        free(sUntile);
        sUntileSize = (size_t)w * h;
        sUntile = malloc(sUntileSize * sizeof(uint16_t));
    }
    wj_glBindTexture(GL_TEXTURE_2D, tex);
    wj_glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    wj_glPixelStorei(0x0CF2 /* GL_UNPACK_ROW_LENGTH */, 0);
    for (unsigned y = 0; y <= h; y++)
    {
        bool in = y < h && ((rows >> (CtrVideo_Texel(0, y, w) / chunk)) & 1u);

        if (in)
        {
            for (unsigned x = 0; x < w; x++)
                sUntile[y * w + x] = tiled[CtrVideo_Texel(x, y, w)];
            if (run < 0)
                run = (int)y;
        }
        else if (run >= 0)
        {
            wj_glTexSubImage2D(GL_TEXTURE_2D, 0, 0, run, w, y - (unsigned)run, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1,
                               &sUntile[(unsigned)run * w]);
            run = -1;
        }
    }
}

// ---------------------------------------------------------------------------
// Shader: texture x the vertex's shade, alpha tested; a palette fade and distance haze on top.

static GLuint sProgram;
static GLint sUniUnfade; // POL-16
static GLint sUniTintMul, sUniTintAdd, sUniHazeMul; // POL-26b
static GLint sUniGlowStrength, sUniGlowLin; // POL-26b: lit windows
static GLint sUniFlatLight; // #29: a cave's models, lit as its floor
static GLint sUniLamp; // POL-26b: night lamps
#define GLOW_UNIT 1 // texture unit of a building page's window mask
static float sTintMul[3] = {1, 1, 1}, sTintAdd[3]; // POL-26b: this frame's time-of-day colour (world)
static GLint sUniMode, sUniAlphaMin, sUniMvp, sUniOrigin, sUniTex, sUniFade, sUniFog, sUniFogColor, sUniCam;
static GLint sUniChunky, sUniAtlasTexel, sUniTime, sUniGamma, sUniSunDir, sUniSunLight, sUniSkyTint, sUniSh, sUniSat, sUniExposure;

static const char *sVertexSrc =
    "#version 120\n"
    "uniform mat4 mvp;\n"
    "uniform vec3 origin;\n"
    "uniform vec3 cam;\n"
    "attribute vec3 pos;\n"
    "attribute vec2 uv;\n"
    "attribute float shade;\n"
    "centroid varying vec2 vUv;\n" // (option 5: with anti-aliasing an edge pixel reads its uv inside its own face)
    "varying float vShade;\n"
    "varying float vFlat;\n"
    "varying float vDist;\n"
    "varying vec3 vWorld;\n"
    "void main() {\n"
    "  vec3 p = pos + origin;\n"
    "  vWorld = p;\n"
    "  vUv = vec2(uv.x, 1.0 - uv.y);\n" // PICA textures count v from the bottom row

    // #29: a shade over 3 marks a cave's model (voxel_building.c CaveFlatShade): lit as the floor, shade - 4
    "  vFlat = shade > 3.0 ? 1.0 : 0.0;\n"
    "  vShade = shade > 3.0 ? shade - 4.0 : shade;\n"
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
    "centroid varying vec2 vUv;\n" // (option 5: with anti-aliasing an edge pixel reads its uv inside its own face)
    "varying float vShade;\n"
    "varying float vFlat;\n"
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
    "uniform float cloudsOn;\n"
    "uniform vec2 steady;\n" // the world origin in a frame that does not jump at a crossed connection

    "float clouds(vec3 w) {\n"
    "  if (cloudsOn < 0.5) return 1.0;\n"
    // a light function on the directional light is cast along the light: every point takes the cloud
    // over the ground spot its sun ray comes from, so walls and roofs carry the ground's pattern instead of
    // repeating one ground sample all the way up (that drew vertical streaks over buildings)
    "  vec2 g = w.xz + steady - sunDir.xz * (w.y / max(sunDir.y, 0.05));\n"
    "  vec2 P = vec2(-g.y, g.x) * 64.0;\n" // Gamma X (north), Y (east), in cm
    "  float nn = texture2D(roughTex, P / 888.25 + time * vec2(-0.0010, -0.0007)).r;\n"
    "  float cc = texture2D(cloudTex, (P + nn * 40.0) / 5000.0 + time * vec2(-0.0021, -0.0030)).r;\n"
    "  return 1.0 - clamp((cc - 0.164) * 3.8417, 0.0, 1.0) * 0.75;\n"
    "}\n"
    // DLX-8 chunky people: the card is drawn as stacked slices; a slice keeps a pixel only where the
    // sprite is thick enough there (1 = even blocks, 2 = rounded: thicker away from the row's edges)
    "uniform vec4 unfade;\n"
    "uniform vec3 tintMul, tintAdd, hazeMul;\n"
    // POL-26b: a building page's window-glass mask (buildings.bin L8 glow), lit at night as Emerald 3DS does
    // (ctr_voxel.c BuildingGlowEnv: after the light, the texel goes towards warm light 255,216,160 by
    // mask x strength). glowLin is that colour in the Gamma path's linear light (see Voxel3D_Render).
    "uniform sampler2D glowTex;\n"
    "uniform float glowStrength;\n"
    "uniform float flatLight;\n"
    "uniform vec3 glowLin;\n"
    // POL-26b lamps: mode 3 a night-light sprite, as the GBA blends it (OBJ blend 7/16 over the scene x 12/16:
    // out = c x 7/16, alpha 12/16, blended ONE, SRC_ALPHA; Gamma path: c in its linear light, c x c x lamp.w);
    // mode 4 a lamp's pool (Emerald 3DS LampTexEnv: the ground x (1 + falloff x lamp.rgb), blended DST_COLOR, ONE)
    "uniform vec4 lamp;\n" // POL-26b: the 2D time-of-day colour (world), the haze's too
    "uniform vec3 chunky;\n" // x mode, y slice depth in sprite pixels (signed), z unused
    "uniform vec2 atlasTexel;\n"
    "void main() {\n"
    "  vec4 c = texture2D(tex, vUv);\n"
    "  if (c.a < alphaMin) discard;\n"
    // POL-16: a palette fade taken back off a person's colours (Gamma lighting: the screen pass fades them)
    "  if (unfade.a > 0.0) c.rgb = clamp((c.rgb - unfade.a * unfade.rgb) / max(1.0 - unfade.a, 0.004), 0.0, 1.0);\n"
    "  c.rgb = clamp(c.rgb * tintMul + tintAdd, 0.0, 1.0);\n" // POL-26b: as the 2D palettes are moved (night)
    "  if (mode > 4.5) { vec3 g = gammaLight > 0.5 ? lamp.rgb * lamp.rgb * lamp.w : lamp.rgb; gl_FragColor = vec4(g, 220.0 / 255.0); return; }\n"
    "  if (mode > 3.5) { gl_FragColor = vec4(c.r * lamp.rgb, 1.0); return; }\n"
    "  if (mode > 2.5) { vec3 g = gammaLight > 0.5 ? c.rgb * c.rgb * lamp.w : c.rgb; gl_FragColor = vec4(g * (7.0 / 16.0), 12.0 / 16.0); return; }\n"
    "  if (chunky.x > 0.5) {\n"
    "    float half_ = 2.0;\n"
    "    if (chunky.x > 1.5) {\n"
    "      float e = 4.0;\n"
    "      for (int k = 1; k <= 4; k++) {\n"
    "        if (texture2D(tex, vUv + vec2(float(k), 0.0) * atlasTexel).a < 0.5 || texture2D(tex, vUv - vec2(float(k), 0.0) * atlasTexel).a < 0.5) { e = min(e, float(k) - 1.0); }\n"
    "      }\n"
    "      half_ = (1.0 + 2.0 * e) * 0.5;\n"
    "    }\n"
    "    if (abs(chunky.y) > half_) discard;\n"
    "  }\n"
    "  vec3 rgb;\n"
    "  if (gammaLight > 0.5) {\n"
    "    vec3 n = normalize(cross(dFdx(vWorld), dFdy(vWorld)));\n"
    "    if (dot(n, cam - vWorld) < 0.0) n = -n;\n"
    // #29: a cave's models lit as the floor round them - no sun in a cave; by their facing, their sides turned
    // from it took only the blue sky light and came out grey-green against the warm floor
    "    if (flatLight > 0.5 || vFlat > 0.5) n = vec3(0.0, 1.0, 0.0);\n"
    "    vec3 albedo = toLinear(c.rgb);\n"
    "    vec3 light = sunLight * max(dot(n, sunDir), 0.0) * vShade * clouds(vWorld) + skyTint * max(skyIrr(n), 0.0);\n"
    "    rgb = albedo * light;\n"
    "    if (glowStrength > 0.0) rgb = mix(rgb, glowLin, texture2D(glowTex, vUv).r * glowStrength);\n"
    "    if (gammaLight > 1.5 && gammaLight < 2.5) { gl_FragColor = vec4(vec3(vShade) * 0.1, 1.0); return; }\n"
    "    if (gammaLight > 4.5) { vec2 f = fract(vWorld.xz / 8.0); vec2 g = fract(vWorld.xz); gl_FragColor = vec4(min(f.x, f.y) < 0.015 ? 0.1 : 0.0, min(g.x, g.y) < 0.03 ? 0.03 : 0.0, 0.02, 1.0); return; }\n"
    "    if (gammaLight > 3.5) { gl_FragColor = vec4(vec3(clouds(vWorld)) * 0.1, 1.0); return; }\n"
    "    if (gammaLight > 2.5) { gl_FragColor = vec4(n * 0.5 + 0.5, 1.0); return; }\n"
    // haze (M_StylizedFogPP, day): distances in Gamma units, 64 per tile
    "    float d = length(vWorld - cam) * 64.0;\n"
    /* Gamma's day haze tops out at 0.5; owner asked for it a little weaker (2026-11-13): 0.35 */
    "    float amt = clamp((d - 10.0) / 39990.0, 0.0, 1.0) * 0.35;\n"
    "    vec3 v = normalize(cam - vWorld);\n"
    "    amt = min(amt, clamp(1.0 - 0.4 * (dot(sunDir, v) - 0.5), 0.0, 1.0));\n"
    "    rgb = mix(rgb, vec3(0.2, 0.467, 1.0) * hazeMul, amt);\n"
    "    if (mode > 1.5) { gl_FragColor = vec4(albedo * (sunLight * 0.643 + skyTint * skyIrr(vec3(0,1,0))), 0.45); return; }\n"
    "    if (mode > 0.5) { gl_FragColor = vec4(0.0, 0.0, 0.0, vShade); return; }\n"
    "    gl_FragColor = vec4(rgb, 1.0);\n"
    "    return;\n"
    "  } else {\n"
    "    rgb = c.rgb * vShade;\n"
    "    if (glowStrength > 0.0) rgb = mix(rgb, vec3(1.0, 0.847, 0.627), texture2D(glowTex, vUv).r * glowStrength);\n"
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
/* Gamma's sky light: irradiance / pi of its captured sky cubemap (cube16, UE axes mapped to ours), fitted by
 * least squares to this basis over 400 directions (2026-11-13 recheck: the first table was ~3x too strong and
 * had the up term's sign wrong; this one matches the 2D path's value straight up, 0.13/0.31/0.54). */
static const float sGammaSh[9][3] = {
    {0.11698f, 0.36169f, 0.59448f}, {0.01342f, -0.03871f, -0.03525f}, {-0.00665f, -0.01860f, -0.01580f},
    {-0.01248f, -0.02280f, -0.03298f}, {-0.00062f, 0.00374f, 0.00487f}, {-0.00539f, -0.01375f, -0.01947f},
    {0.00176f, 0.00795f, 0.01170f}, {-0.00673f, -0.00932f, -0.00613f}, {-0.00502f, 0.00715f, 0.01298f},
};

static void GammaLightUniforms(void)
{
    const float pi = 3.14159265f;
    float sunI = 3.0f / pi;         /* sun 3 lux, Lambert */
    float skyI = 0.32f;             /* sky light intensity */
    wj_glUniform1f(sUniGamma, gVoxelGammaLight ? (getenv("WJPC_3D_GDEBUG") ? (float)atof(getenv("WJPC_3D_GDEBUG")) : 1.0f) : 0.0f);
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
    else
    {
        /* no voxel/gamma_clouds.bin: a black texel (no cloud anywhere), never whatever else is bound there */
        static GLuint none;
        if (none == 0)
        {
            static const unsigned char zero[4] = {0, 0, 0, 255};
            wj_glGenTextures(1, &none);
            wj_glBindTexture(GL_TEXTURE_2D, none);
            wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            wj_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, zero);
        }
        wj_glUniform1i(wj_glGetUniformLocation(sProgram, "cloudTex"), 6);
        wj_glUniform1i(wj_glGetUniformLocation(sProgram, "roughTex"), 7);
        wj_glActiveTexture(GL_TEXTURE0 + 6);
        wj_glBindTexture(GL_TEXTURE_2D, none);
        wj_glActiveTexture(GL_TEXTURE0 + 7);
        wj_glBindTexture(GL_TEXTURE_2D, none);
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
    sUniUnfade = wj_glGetUniformLocation(sProgram, "unfade");
    sUniTintMul = wj_glGetUniformLocation(sProgram, "tintMul");
    sUniTintAdd = wj_glGetUniformLocation(sProgram, "tintAdd");
    sUniHazeMul = wj_glGetUniformLocation(sProgram, "hazeMul");
    sUniGlowStrength = wj_glGetUniformLocation(sProgram, "glowStrength");
    sUniFlatLight = wj_glGetUniformLocation(sProgram, "flatLight");
    sUniGlowLin = wj_glGetUniformLocation(sProgram, "glowLin");
    sUniLamp = wj_glGetUniformLocation(sProgram, "lamp");
    wj_glUseProgram(sProgram); // uniforms start at 0: no tint is 1, 0
    wj_glUniform3f(sUniTintMul, 1, 1, 1);
    wj_glUniform3f(sUniTintAdd, 0, 0, 0);
    wj_glUniform3f(sUniHazeMul, 1, 1, 1);
    wj_glUniform1i(wj_glGetUniformLocation(sProgram, "glowTex"), GLOW_UNIT);
    wj_glUniform1f(sUniGlowStrength, 0);
    wj_glUniform1f(sUniFlatLight, 0);
    sUniFog = wj_glGetUniformLocation(sProgram, "fog");
    sUniFogColor = wj_glGetUniformLocation(sProgram, "fogColor");
    sUniMode = wj_glGetUniformLocation(sProgram, "mode");
    sUniCam = wj_glGetUniformLocation(sProgram, "cam");
    sUniAlphaMin = wj_glGetUniformLocation(sProgram, "alphaMin");
    sUniGamma = wj_glGetUniformLocation(sProgram, "gammaLight");
    sUniTime = wj_glGetUniformLocation(sProgram, "time");
    sUniChunky = wj_glGetUniformLocation(sProgram, "chunky");
    sUniAtlasTexel = wj_glGetUniformLocation(sProgram, "atlasTexel");
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

/*
 * DLX-22, ported from Emerald 3DS (ctr_voxel.c, GPU vertex format + Quantise): positions go to the GPU in 1/512
 * tile, measured from the draw's origin (always a whole tile), rounded half up, never half away from zero, "so two
 * chunks quantise a shared edge to exactly the same place". Raw builder floats let vertices that should meet differ
 * in the last bit, and real GPUs showed the background through as thin dark or light lines (Granite Cave, the sea).
 * Kept as floats: k/512 is exact for any position a map can have.
 */
#define VOXEL_POS_SCALE 512.0f
static float QuantisePos(float value)
{
    return floorf(value * VOXEL_POS_SCALE + 0.5f) / VOXEL_POS_SCALE;
}

static void MeshSetQuantised(Mesh *m, const VoxelVertex *v, unsigned count, GLenum usage)
{
    static VoxelVertex *buf;
    static unsigned cap;
    if (count > cap)
    {
        VoxelVertex *grown = realloc(buf, (size_t)count * sizeof(VoxelVertex));
        if (grown == NULL)
        {
            MeshSet(m, v, count, usage);
            return;
        }
        buf = grown;
        cap = count;
    }
    for (unsigned i = 0; i < count; i++)
    {
        buf[i] = v[i];
        buf[i].x = QuantisePos(v[i].x);
        buf[i].y = QuantisePos(v[i].y);
        buf[i].z = QuantisePos(v[i].z);
    }
    MeshSet(m, buf, count, usage);
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
    uint32_t gen;    // bumped at every (re)build: meshes laid on an older one are laid again
    uint32_t layoutGen; // POL-15: the build its slots were last placed anew; growing keeps them (MeshAtlasOk)
    bool grow;          // POL-15: a loaded map uses metatiles it has no slot for: grown (PrepareAtlas)
    bool valid;
    uint16_t *raw[VOXEL_ATLAS_PAGES]; // POL-12: each page as composed, before VoxelGrade_Texels (tiled order)
    /* Audit step 2b (Brennan 2026-11-21 02:12; Ever Grande's waterfalls: ~180 ms in the GL driver every few frames
     * on the test box): changing a texture the last frame drew with makes the driver wait for that frame. The
     * animated tiles go into another texture of the page, the one drawn with longest ago (a ring of three: tex, then
     * spare[0] last frame's, spare[1] the one before - a driver may still be drawing the last frame), which is then
     * drawn with; spareStale: the rows of slots each lacks (~0: all). */
    GLuint spare[VOXEL_ATLAS_PAGES][2];
    unsigned spareStale[VOXEL_ATLAS_PAGES][2];
} Atlas;

static uint32_t sAtlasGen;

/*
 * An atlas is made for the maps loaded when it is made. Another map on the same tile sets later (Granite Cave's
 * floors, a town and its routes) can use metatiles it never got - drawn as nothing, the sky through the gap
 * (the blue squares). Emerald 3DS grows its atlas then (extendPending); here: a map whose cells use a metatile
 * the atlas has no slot for has it made again with every loaded map's metatiles. Checked once per map and build.
 */
/*
 * POL-27 (Brennan: "Route 113 ash on the floor after walking through it shows blue boxes"): stepping in ash turns
 * the cell into another metatile (Fallarbor ash grass -> normal grass) that no map had when the atlas was made, so
 * it was drawn as nothing - the sky through the gap. The check was kept per map and build only, so it never saw a
 * cell a script or field effect changed later. For the map underfoot (`live`, read off the live grid) it is now
 * also kept per state of the live grid (VoxelWorld_LiveDigest, taken once a frame in Voxel3D_Render): the grid
 * changes, the map is checked again, and a new metatile has the atlas made again as above.
 */
static uint32_t sAtlasLiveDigest;

static bool AtlasCovers(const Atlas *a, const VoxelMapInstance *inst)
{
    static struct { const Atlas *atlas; uint32_t gen, digest; uint16_t group, num; } seen[32];
    static unsigned next;
    uint32_t digest = inst->live ? sAtlasLiveDigest : 0;
    for (unsigned i = 0; i < 32; i++)
        if (seen[i].atlas == a && seen[i].gen == a->gen && seen[i].group == inst->mapGroup && seen[i].num == inst->mapNum
            && seen[i].digest == digest)
            return true;
    for (int y = 0; y < inst->height; y++)
        for (int x = 0; x < inst->width; x++)
        {
            int id = VoxelWorld_GetMetatileId(inst->originX + x, inst->originY + y);
            if (VoxelWorld_UsesTreeSprites(inst))
                id = VoxelTree_GroundMetatile(inst->secondaryTileset, id);
            if (id >= 0 && id < VOXEL_METATILE_REAL && a->map.slotOf[id] == 0)
            {
                Voxel3D_Log("[3D] atlas lacks metatile %x of %d:%d: grown", id, inst->mapGroup, inst->mapNum);
                return false;
            }
        }
    seen[next].atlas = a;
    seen[next].gen = a->gen;
    seen[next].digest = digest;
    seen[next].group = (uint16_t)inst->mapGroup;
    seen[next].num = (uint16_t)inst->mapNum;
    next = (next + 1) % 32;
    return true;
}

static Atlas sAtlases[MAX_ATLASES];
static void LoadGammaClouds(void);

static void AtlasFreeRaw(Atlas *a)
{
    for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
    {
        free(a->raw[p]);
        a->raw[p] = NULL;
    }
}

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
/* Brennan 2026-11-22 (13:37 list B1, answer "Tone it down"): Rusturf Tunnel's green is the original game's (its
 * palettes byte for byte pret's pokeemerald), but in 3D it read neon. Its atlas is drawn less saturated and a little
 * darker: each texel 45% of the way to its grey, then 88% bright. WJPC_RUSTURF_NEON=1: as before. */
extern const struct Tileset gTileset_RusturfTunnel;
static void CaveToneDown(uint16_t *texels, unsigned count)
{
    static int sOff = -1;
    if (sOff < 0)
        sOff = getenv("WJPC_RUSTURF_NEON") != NULL;
    if (sOff)
        return;
    for (unsigned i = 0; i < count; ++i)
    {
        uint16_t t = texels[i];
        float r = (float)((t >> 11) & 31), g = (float)((t >> 6) & 31), b = (float)((t >> 1) & 31);
        float y = 0.299f * r + 0.587f * g + 0.114f * b;
        r = (r + (y - r) * 0.45f) * 0.88f;
        g = (g + (y - g) * 0.45f) * 0.88f;
        b = (b + (y - b) * 0.45f) * 0.88f;
        unsigned R = (unsigned)lroundf(r), G = (unsigned)lroundf(g), B = (unsigned)lroundf(b);
        texels[i] = (uint16_t)((R > 31 ? 31 : R) << 11 | (G > 31 ? 31 : G) << 6 | (B > 31 ? 31 : B) << 1 | (t & 1u));
    }
}
static const void *sJobPrimary, *sJobSecondary; // the job's own copies are cleared when it ends

static Atlas *FindAtlas(const VoxelMapInstance *inst)
{
    for (int i = 0; i < MAX_ATLASES; i++)
        if (sAtlases[i].valid && sAtlases[i].primary == inst->primaryTileset && sAtlases[i].secondary == inst->secondaryTileset
            && sAtlases[i].season == SeasonNow())
        {
            /*
             * POL-15 (Brennan 2026-11-15, video: from Route 110 into Mauville the whole of Route 110 went blue for
             * over a second, then the game lagged): a map that came into reach used metatiles the atlas lacked,
             * and the atlas was thrown away and made again - every map on its tile sets (Route 110, Mauville,
             * 111, 117, 118 and Verdanturf share one) lost its mesh until it was laid again, one map at a time.
             * Now, as Emerald 3DS does (ctr_voxel.c extendPending; VoxelAtlas_JobBegin's `base`), the atlas is
             * grown instead: its slots stay where they are, so every mesh on it is still drawn while the new
             * metatiles are added (PrepareAtlas) and the maps are laid again behind it.
             */
            if (!sAtlases[i].grow && !AtlasCovers(&sAtlases[i], inst))
                sAtlases[i].grow = true;
            sAtlases[i].lastUse = sFrame;
            return &sAtlases[i];
        }
    return NULL;
}

static bool sJobGrow; // POL-15: the job grows sJobSlot (its slots kept) rather than making it anew

static Atlas *PrepareAtlas(const VoxelMapInstance *inst, Uint64 deadline)
{
    Atlas *slot = FindAtlas(inst);

    if (slot != NULL && (!slot->grow || (sJobSlot != NULL && sJobSlot != slot)))
        return slot; // ready (or to be grown once the job under way is done: drawn as it is meanwhile)
    if (slot != NULL && sJobSlot == NULL)
    {
        // POL-15: grow it - the same slots, the new metatiles added, every page composed again
        if (sAtlasStaging == NULL)
            sAtlasStaging = malloc(VOXEL_ATLAS_PIXELS * sizeof(uint16_t));
        if (sAtlasStaging == NULL || !VoxelAtlas_JobBegin(&sAtlasJob, inst, sAtlasStaging, &slot->map))
        {
            slot->grow = false; // nothing new to add after all (or no memory: drawn as it is)
            return slot;
        }
        sJobSlot = slot;
        sJobGrow = true;
        sJobPrimary = inst->primaryTileset;
        sJobSecondary = inst->secondaryTileset;
        Voxel3D_Log("[3D] atlas grown for %d:%d (slot %d)", inst->mapGroup, inst->mapNum, (int)(slot - sAtlases));
    }
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
            wj_glDeleteTextures(2 * VOXEL_ATLAS_PAGES, &slot->spare[0][0]);
            memset(slot->spare, 0, sizeof(slot->spare));
            AtlasFreeRaw(slot);
        }
        slot->valid = false;
        if (sAtlasStaging == NULL)
            sAtlasStaging = malloc(VOXEL_ATLAS_PIXELS * sizeof(uint16_t));
        if (!VoxelAtlas_JobBegin(&sAtlasJob, inst, sAtlasStaging, NULL))
            return NULL;
        slot->pages = 0;
        slot->season = SeasonNow();
        slot->grow = false;
        sJobSlot = slot;
        sJobGrow = false;
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
        // POL-12: keep the page as composed, so the animated tiles can be composed into it again later
        if (slot->raw[sAtlasJob.page] == NULL)
            slot->raw[sAtlasJob.page] = malloc(VOXEL_ATLAS_PIXELS * sizeof(uint16_t));
        if (slot->raw[sAtlasJob.page] != NULL)
            memcpy(slot->raw[sAtlasJob.page], sAtlasStaging, VOXEL_ATLAS_PIXELS * sizeof(uint16_t));
        VoxelGrade_Texels(sAtlasStaging, VOXEL_ATLAS_PIXELS);
        if (sJobSecondary == (const void *)&gTileset_RusturfTunnel)
            CaveToneDown(sAtlasStaging, VOXEL_ATLAS_PIXELS);
        // (grown: into the page's own texture, the meshes drawn from it see the same slots)
        slot->tex[sAtlasJob.page] = UploadTiled(sJobGrow ? slot->tex[sAtlasJob.page] : 0, sAtlasStaging, VOXEL_ATLAS_W, VOXEL_ATLAS_H);
        slot->spareStale[sAtlasJob.page][0] = slot->spareStale[sAtlasJob.page][1] = ~0u; // (its other textures: out of date)
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
    slot->gen = ++sAtlasGen;
    if (!sJobGrow)
        slot->layoutGen = slot->gen;
    slot->grow = false;
    sJobGrow = false;
    slot->lastUse = sFrame;
    return slot->primary == inst->primaryTileset && slot->secondary == inst->secondaryTileset ? slot : NULL;
}

static Atlas *GetAtlas(const VoxelMapInstance *inst)
{
    return PrepareAtlas(inst, 0);
}

/*
 * POL-12 (Brennan: "Water is not moving in 3D mode it is still"): the animated tiles - the sea and its sandy,
 * grassy and waterfall edges, the flowers - redrawn in the atlases, as Emerald 3DS does (ctr_voxel.c
 * AnimateTiles / AnimBegin, "The animated tiles (water, flowers) of the atlases in view"). The game writes its
 * animation frames into BG VRAM (TransferTilesetAnimsBuffer) on frames 0-4 of every 16. Here the tiles that
 * changed are found by comparing VRAM with last frame's copy; once a frame passes with no change (3DS's
 * VOXEL_ANIM_SETTLE: wait for the writes to finish), every slot drawn from a changed tile is composed again
 * (VoxelAtlas_RefreshAnimatedPageRows, ported from 3DS) in the current map's atlas, and in the atlases of maps
 * beside it that share its primary tile set (only the primary tiles' changes: VRAM holds just the current map's
 * secondary set - 3DS's AnimShares), then those pages are graded and uploaded again.
 */
#define ANIM_VRAM_BYTES 0x8000 // both tile sets, 1024 tiles (VoxelAtlas_RefreshAnimatedPage)
static uint8_t sAnimVram[ANIM_VRAM_BYTES];
static uint8_t sAnimDirty[128];
static bool sAnimHaveVram, sAnimAnyDirty;
static const void *sAnimPrimary, *sAnimSecondary;

static Uint64 sAnimComposeTicks, sAnimUploadTicks; // (the log: where an animation pass's time goes)
static void AnimateAtlasTiles(const VoxelMapInstance *inst)
{
    const uint8_t *vram = VRAM_; // BG char block 0: the overworld's two tile sets (3DS: CtrVideo_GetBgVram)
    bool wrote = false;

    if (vram == NULL || inst == NULL || getenv("WJPC_3D_NO_TILE_ANIM") != NULL)
        return;
    if (!sAnimHaveVram || inst->primaryTileset != sAnimPrimary || inst->secondaryTileset != sAnimSecondary)
    {
        // first frame, or new tile sets in VRAM (a new map): start comparing from here
        memcpy(sAnimVram, vram, ANIM_VRAM_BYTES);
        memset(sAnimDirty, 0, sizeof(sAnimDirty));
        sAnimHaveVram = true;
        sAnimAnyDirty = false;
        sAnimPrimary = inst->primaryTileset;
        sAnimSecondary = inst->secondaryTileset;
        return;
    }
    for (unsigned t = 0; t < ANIM_VRAM_BYTES / 32; t++)
        if (memcmp(sAnimVram + t * 32, vram + t * 32, 32) != 0)
        {
            sAnimDirty[t >> 3] |= (uint8_t)(1u << (t & 7));
            memcpy(sAnimVram + t * 32, vram + t * 32, 32);
            wrote = true;
        }
    if (wrote)
    {
        static unsigned sLoggedW;
        if (sLoggedW++ < 10)
            Voxel3D_Log("[3D] animated tiles: VRAM tiles written (frame %u)", sFrame);
        sAnimAnyDirty = true;
        return; // still writing: wait for a quiet frame
    }
    if (!sAnimAnyDirty)
        return;
    {
        unsigned primaryTiles = Voxel_PrimaryTiles(inst->primaryTileset);
        uint8_t primaryOnly[128];

        memcpy(primaryOnly, sAnimDirty, sizeof(primaryOnly));
        for (unsigned t = primaryTiles; t < 1024; t++)
            primaryOnly[t >> 3] &= (uint8_t)~(1u << (t & 7));
        for (int i = 0; i < MAX_ATLASES; i++)
        {
            Atlas *a = &sAtlases[i];
            VoxelMapInstance pair;
            bool same;

            if (!a->valid || a->primary != inst->primaryTileset || a->season != SeasonNow())
                continue;
            same = a->secondary == inst->secondaryTileset;
            memset(&pair, 0, sizeof(pair));
            pair.primaryTileset = a->primary;
            pair.secondaryTileset = a->secondary;
            for (unsigned p = 0; p < a->pages && p < VOXEL_ATLAS_PAGES; p++)
            {
                unsigned rows = 0;
                if (a->raw[p] == NULL || a->tex[p] == 0)
                    continue;
                Uint64 tc0 = SDL_GetPerformanceCounter();
                if (VoxelAtlas_RefreshAnimatedPageRows(&pair, &a->map, p, a->raw[p], vram,
                                                       same ? sAnimDirty : primaryOnly, &rows) == 0)
                    continue;
                sAnimComposeTicks += SDL_GetPerformanceCounter() - tc0;
                Uint64 tu0 = SDL_GetPerformanceCounter();
                if (sAtlasStaging == NULL)
                    sAtlasStaging = malloc(VOXEL_ATLAS_PIXELS * sizeof(uint16_t));
                if (sAtlasStaging == NULL || (sJobSlot != NULL && sAtlasJob.active))
                    continue; // the staging page is a job's right now: this page waits for the next change
                {
                    static int sWhole = -1;
                    if (sWhole < 0)
                        sWhole = getenv("WJPC_ANIM_WHOLE_PAGES") != NULL; // (as before: the whole page)
                    if (sWhole || rows == 0)
                    {
                        memcpy(sAtlasStaging, a->raw[p], VOXEL_ATLAS_PIXELS * sizeof(uint16_t));
                        VoxelGrade_Texels(sAtlasStaging, VOXEL_ATLAS_PIXELS);
                        UploadTiled(a->tex[p], sAtlasStaging, VOXEL_ATLAS_W, VOXEL_ATLAS_H);
                    }
                    else
                    {
                        const unsigned chunk = VOXEL_ATLAS_ROW_BYTES / 2u;
                        static int sOneTexture = -1;
                        if (sOneTexture < 0)
                            sOneTexture = getenv("WJPC_ANIM_ONE_TEXTURE") != NULL; // (as before: into the drawn one)
                        unsigned want = sOneTexture ? rows : rows | a->spareStale[p][1];

                        for (unsigned r = 0; r < 32; r++)
                            if ((want >> r) & 1u && (r + 1) * chunk <= VOXEL_ATLAS_PIXELS)
                            {
                                memcpy(sAtlasStaging + r * chunk, a->raw[p] + r * chunk, chunk * sizeof(uint16_t));
                                VoxelGrade_Texels(sAtlasStaging + r * chunk, chunk);
                            }
                        if (sOneTexture)
                            UploadTiledRows(a->tex[p], sAtlasStaging, VOXEL_ATLAS_W, VOXEL_ATLAS_H, rows);
                        else
                        {
                            GLuint oldest = a->spare[p][1];

                            if (oldest == 0)
                            {
                                /* first time: made whole (every row graded above: want = ~0) */
                                oldest = UploadTiled(0, sAtlasStaging, VOXEL_ATLAS_W, VOXEL_ATLAS_H);
                            }
                            else
                                UploadTiledRows(oldest, sAtlasStaging, VOXEL_ATLAS_W, VOXEL_ATLAS_H, want);
                            /* the ring turns: the oldest is drawn now; the others lack this change */
                            a->spare[p][1] = a->spare[p][0];
                            a->spareStale[p][1] = a->spareStale[p][0] | rows;
                            a->spare[p][0] = a->tex[p];
                            a->spareStale[p][0] = rows;
                            a->tex[p] = oldest;
                        }
                    }
                }
                sAnimUploadTicks += SDL_GetPerformanceCounter() - tu0;
                {
                    static unsigned sLogged;
                    if (sLogged++ < 20)
                        Voxel3D_Log("[3D] animated tiles: atlas %d page %u rows %x redrawn (frame %u)", i, p, rows, sFrame);
                }
            }
        }
    }
    memset(sAnimDirty, 0, sizeof(sAnimDirty));
    sAnimAnyDirty = false;
}

// ---------------------------------------------------------------------------
// Building pages and the big static models.

#define MAX_PAGES 1024 // data generated from our own maps has more than 512 pages
static GLuint sPageTex[MAX_PAGES];
static uint16_t *sPageScratch;
static size_t sPageScratchSize;

static GLuint LoadBuildingPage(int page, bool reload);
/* #29 (Brennan 2026-11-23: "all caves should be 3D models"): the pages of a cave's models. The floor's
 * colours are graded twice (the atlas's colour table, VoxelAtlas BuildColorTable, and the page as composed); a
 * model page once - so a cave's rocks came out paler and greyer than the same rock painted on the floor beside
 * them (Victory Road, Desert Underpass, Artisan Cave). A cave's page is graded as its floor.
 * WJPC_CAVE_PAGES_ONE_GRADE: once, as before. */
static bool sPageCave[2048];
static bool CavePagesOneGrade(void)
{
    static int sOld = -1;
    if (sOld < 0)
        sOld = getenv("WJPC_CAVE_PAGES_ONE_GRADE") != NULL;
    return sOld != 0;
}
/*
 * Brennan 2026-11-22 13:37 (B6) and 13:38 (C2): in winter "keep the front walls, doors, and windows 100% their
 * original non-winter colors across all seasons. Only the tops/roofs of houses should have white snow on them" (Mr.
 * Briney's cottage and the Cycling Road gatehouse were washed white - the snow-cover season turns light earth tones
 * white, as 2D does to the tile sets' palettes; Petalburg's red roofs had no snow). A building page is uploaded three
 * ways: as drawn (no season: the walls), seasoned (the ground pieces at a model's foot, as the ground round them),
 * and seasoned with snow laid over in winter (the roofs and tops). A model's faces are split by their facing: upright
 * ones (walls, doors, windows) and the rest (roofs, tops; ground pieces lying on the ground keep the seasoned page).
 * WJPC_WINTER_WALLS_OLD=1: one seasoned page for all, as before; WJPC_ROOF_SNOW=x: how white the snow (0..1).
 */
enum { FACE_TOP, FACE_WALL, FACE_GROUND, FACE_KINDS };
static GLuint sPageTexRaw[1024], sPageTexRoof[1024];
static bool WinterWallsOld(void)
{
    static int sOld = -1;
    if (sOld < 0)
        sOld = getenv("WJPC_WINTER_WALLS_OLD") != NULL;
    return sOld;
}
static int FaceKind(const VoxelVertex *t)
{
    float ax = t[1].x - t[0].x, ay = t[1].y - t[0].y, az = t[1].z - t[0].z;
    float bx = t[2].x - t[0].x, by = t[2].y - t[0].y, bz = t[2].z - t[0].z;
    float nx = ay * bz - az * by, ny = az * bx - ax * bz, nz = ax * by - ay * bx;
    float len = sqrtf(nx * nx + ny * ny + nz * nz);
    if (len < 1e-9f)
        return FACE_TOP;
    if (fabsf(ny) / len < 0.35f)
        return FACE_WALL;
    if (t[0].y < 0.13f && t[1].y < 0.13f && t[2].y < 0.13f)
        return FACE_GROUND;
    return FACE_TOP;
}
/* the triangles of v (count) of one kind into out (the caller's room for count): their number */
static unsigned FacesOfKind(const VoxelVertex *v, unsigned count, int kind, VoxelVertex *out)
{
    unsigned n = 0;
    for (unsigned t = 0; t + 2 < count; t += 3)
        if (FaceKind(&v[t]) == kind)
        {
            out[n++] = v[t];
            out[n++] = v[t + 1];
            out[n++] = v[t + 2];
        }
    return n;
}


/* POL-26b: a page's window mask (w x h bytes, tiled like the page), as a one-channel texture; 0 = none. */
static GLuint sGlowTex[MAX_PAGES];
static bool sGlowTried[MAX_PAGES];
static GLuint GlowPage(int page)
{
    unsigned w, h, th;
    uint8_t *tiled, *flat;

    if (page < 0 || page >= MAX_PAGES)
        return 0;
    if (sGlowTex[page] != 0 || sGlowTried[page])
        return sGlowTex[page];
    sGlowTried[page] = true;
    if (!VoxelBuildings_PageHasGlow((unsigned)page) || !VoxelBuildings_PageSize((unsigned)page, &w, &h, &th))
        return 0;
    tiled = malloc((size_t)w * th);
    flat = calloc((size_t)w * th, 1);
    if (tiled != NULL && flat != NULL && VoxelBuildings_ReadGlow((unsigned)page, 0, w * h, tiled))
    {
        for (unsigned y = 0; y < h; y++)
            for (unsigned x = 0; x < w; x++)
                flat[y * w + x] = tiled[CtrVideo_Texel(x, y, w)];
        wj_glGenTextures(1, &sGlowTex[page]);
        wj_glBindTexture(GL_TEXTURE_2D, sGlowTex[page]);
        wj_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        wj_glPixelStorei(0x0CF2 /* GL_UNPACK_ROW_LENGTH */, 0);
        wj_glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w, th, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, flat);
        wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    free(tiled);
    free(flat);
    return sGlowTex[page];
}
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
    // DLX-17 seasons, then the grading. POL-34 (Brennan 2026-11-15, winter night in Mauville: "All the houses now
    // lost their color ... appear to be pale white"): the season was applied AFTER the grading (in ReadPage), so
    // the formula saw brightened colours - in winter (snow cover: light earth tones turn white) most of a house's
    // walls passed its brightness test and turned white, where 2D, which recolours the tile sets' own palette
    // colours, keeps them. Now the formula sees the page's own colours, as 2D does (and as the trees already did).
    VoxelWorld_SeasonTexels(sPageScratch, (size_t)w * h);
    VoxelGrade_Texels(sPageScratch, (unsigned)(w * h));
    if (page < 2048 && sPageCave[page])
        VoxelGrade_Texels(sPageScratch, (unsigned)(w * h));
    if (getenv("WJPC_DUMP_PAGES") != NULL) /* test aid: <dir>/page<N>_<w>x<th>.raw, the texels as uploaded (RGBA5551, row 0 first) */
    {
        char name[512];
        FILE *pf;
        snprintf(name, sizeof(name), "%s\\page%d_%ux%u.raw", getenv("WJPC_DUMP_PAGES"), page, w, th);
        if ((pf = fopen(name, "wb")) != NULL)
        {
            for (unsigned y = 0; y < th; y++)
                for (unsigned x = 0; x < w; x++)
                    fwrite(&sPageScratch[CtrVideo_Texel(x, y, w)], sizeof(uint16_t), 1, pf);
            fclose(pf);
        }
    }
    extern unsigned gVoxel3DPages;
    if (sPageTex[page] == 0)
        gVoxel3DPages++; // (stall log)
    sPageTex[page] = UploadTiled(sPageTex[page], sPageScratch, w, th);
    if (!WinterWallsOld())
    {
        /* the roofs' page: the seasoned one with snow over it in winter (B6) */
        uint32_t key = WallyJourney_FoliageKey();
        static float sSnow = -1.0f;
        if (sSnow < 0.0f)
            sSnow = getenv("WJPC_ROOF_SNOW") ? (float)atof(getenv("WJPC_ROOF_SNOW")) : 0.85f; /* Brennan: "B: heavy (white)" */
        if (((key >> 4) & 0xF) == WALLY_SEASON_WINTER && sSnow > 0.0f)
            for (size_t i = 0; i < (size_t)w * th; i++)
            {
                uint16_t t = sPageScratch[i];
                if (!(t & 1))
                    continue;
                float r = (float)((t >> 11) & 31), g = (float)((t >> 6) & 31), b = (float)((t >> 1) & 31);
                float y = 0.3f * r + 0.59f * g + 0.11f * b;
                /* towards a cool white, keeping a little of the drawing's light and dark (its tiles' lines) */
                float sr = 26.0f + (y - 16.0f) * 0.3f, sg = 28.0f + (y - 16.0f) * 0.3f, sb = 31.0f + (y - 16.0f) * 0.2f;
                r += (sr - r) * sSnow, g += (sg - g) * sSnow, b += (sb - b) * sSnow;
                unsigned R = (unsigned)fmaxf(0.0f, fminf(31.0f, r + 0.5f)), G = (unsigned)fmaxf(0.0f, fminf(31.0f, g + 0.5f));
                unsigned B = (unsigned)fmaxf(0.0f, fminf(31.0f, b + 0.5f));
                sPageScratch[i] = (uint16_t)(R << 11 | G << 6 | B << 1 | 1u);
            }
        sPageTexRoof[page] = UploadTiled(sPageTexRoof[page], sPageScratch, w, th);
        /* the walls' page: as drawn, no season */
        memset(sPageScratch, 0, (size_t)w * th * sizeof(uint16_t));
        if (VoxelBuildings_ReadPage((unsigned)page, 0, w * h, sPageScratch))
        {
            VoxelGrade_Texels(sPageScratch, (unsigned)(w * h));
            sPageTexRaw[page] = UploadTiled(sPageTexRaw[page], sPageScratch, w, th);
        }
    }
    return sPageTex[page];
}

typedef struct
{
    unsigned pageModel;
    bool lit, used;
    Mesh mesh;
    Mesh kind[FACE_KINDS]; /* (B6: its faces by facing; mesh holds them all, for WJPC_WINTER_WALLS_OLD) */
} StaticMesh;

#define MAX_STATICS 1024
static StaticMesh sStatics[MAX_STATICS];

static const Mesh *StaticModel(unsigned pageModel, bool lit);
static const StaticMesh *StaticModelParts(unsigned pageModel, bool lit)
{
    const Mesh *m = StaticModel(pageModel, lit);
    for (int i = 0; m != NULL && i < MAX_STATICS; i++)
        if (&sStatics[i].mesh == m)
            return &sStatics[i];
    return NULL;
}
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
                t[k] = v[k];
            // POL-8: faces' uv 1/32 pixel in from their edges (VoxelBuildings_InsetTriUV)
            for (unsigned k = 0; k + 2 < count; k += 3)
                VoxelBuildings_InsetTriUV(&t[k]);
            for (unsigned k = 0; k < count; k++)
            {
                t[k].u = (ox + t[k].u) * su;
                t[k].v = 1.0f - (oy + t[k].v) * sv;
            }
            sStatics[i].used = true;
            sStatics[i].pageModel = pageModel;
            sStatics[i].lit = lit;
            MeshSetQuantised(&sStatics[i].mesh, t, count, GL_STATIC_DRAW);
            {
                VoxelVertex *k = malloc((size_t)count * sizeof(VoxelVertex));
                if (k != NULL)
                    for (int kind = 0; kind < FACE_KINDS; kind++)
                        MeshSetQuantised(&sStatics[i].kind[kind], k, FacesOfKind(t, count, kind, k), GL_STATIC_DRAW);
                free(k);
            }
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
    uint32_t atlasGen;        // the atlas build it was laid on
    Mesh terrain[VOXEL_ATLAS_PAGES];
    Mesh trees, buildings;
    Mesh buildingKind[FACE_KINDS]; /* (B6) */
    int buildingPage;
    VoxelStaticDraw statics[MAX_INST_STATICS];
    unsigned staticCount;
} InstanceMesh;

static InstanceMesh sInstanceMeshes[MAX_INSTANCES];

#define SCRATCH_VERTS (1u << 22) // (BELT_RADIUS 64)
static VoxelVertex *sScratch;
static VoxelVertex *sSorted;
static VOXEL_TLS VoxelStaticDraw sChunkStatics[64]; // (per thread: maps are built on the worker too)

static void InstanceFree(InstanceMesh *m)
{
    for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
        MeshFree(&m->terrain[p]);
    MeshFree(&m->trees);
    MeshFree(&m->buildings);
    for (int kind = 0; kind < FACE_KINDS; kind++)
        MeshFree(&m->buildingKind[kind]);
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
    uint32_t atlasGen;              // the atlas's generation when the build began (what the mesh is laid on)
    const VoxelAtlasMap *map;       // the slots it is laid with: the atlas's own, or the worker's copy of it
    /* (step 4c) the ground split by atlas page, and every vertex quantised (QuantisePos), ready to upload: done
     * on the worker after the build (BuildPrepare), so the main thread only uploads */
    VoxelVertex *pageV[VOXEL_ATLAS_PAGES];
    unsigned pageN[VOXEL_ATLAS_PAGES], pageCap[VOXEL_ATLAS_PAGES];
    bool prepared;
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
    pb->atlasGen = atlas->gen;
    pb->map = &atlas->map;
    pb->nextX = pb->nextY = 0;
    pb->prepared = false;
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
    static VOXEL_TLS VoxelVertex *chunkVerts;
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
        VoxelBuilder_SetAtlas(&b, pb->map);
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

static void QuantiseInPlace(VoxelVertex *v, unsigned count)
{
    for (unsigned i = 0; i < count; i++)
    {
        v[i].x = QuantisePos(v[i].x);
        v[i].y = QuantisePos(v[i].y);
        v[i].z = QuantisePos(v[i].z);
    }
}

/* Audit step 4c: the finished build made ready to upload - the ground split by atlas page (u = 2 * page + u in
 * page) and every position quantised (DLX-22, as MeshSetQuantised did at upload). CPU only: run on the worker. */
static void ModelsOffFlush(InstanceBuild *pb);
static void BuildPrepare(InstanceBuild *pb)
{
    unsigned ground = pb->n[0] < SCRATCH_VERTS ? pb->n[0] : SCRATCH_VERTS;
    const VoxelVertex *groundV = pb->v[0];

    for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
        pb->pageN[p] = 0;
    for (unsigned i = 0; i + 2 < ground; i += 3)
    {
        unsigned p = (unsigned)(groundV[i].u * 0.5f);

        if (p >= VOXEL_ATLAS_PAGES)
            continue;
        if (pb->pageN[p] + 3 > pb->pageCap[p])
        {
            unsigned cap = pb->pageCap[p] ? pb->pageCap[p] * 2 : 65536u;
            VoxelVertex *nv = realloc(pb->pageV[p], cap * sizeof(VoxelVertex));

            if (nv == NULL)
                continue;
            pb->pageV[p] = nv;
            pb->pageCap[p] = cap;
        }
        for (int k = 0; k < 3; k++)
        {
            VoxelVertex *d = &pb->pageV[p][pb->pageN[p]++];

            *d = groundV[i + k];
            d->u -= 2.0f * p;
        }
    }
    for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
        QuantiseInPlace(pb->pageV[p], pb->pageN[p]);
    QuantiseInPlace(pb->v[1], pb->n[1]);
    QuantiseInPlace(pb->v[2], pb->n[2]);
    /* (here, on the build thread: up to 13 ms a map) */
    ModelsOffFlush(pb);
    pb->prepared = true;
}

/*
 * Brennan 2026-11-21 02:12 (audit step 4, docs/polish_reports/PERF_AUDIT_2026-11-21.md: "Parse and build all map
 * vertex/index data completely on a background worker thread"): the background build (sBackBuild) runs on a worker
 * thread. The main thread picks the map and readies its atlas as before, then hands the worker a snapshot of the
 * world (VoxelWorld_SnapshotTake: the maps placed and the live map grid) and a copy of the atlas's slots; the worker
 * lays the whole map from those (its caches are its own, voxel_thread.h) and says when it is done; the main thread
 * then sorts and uploads it (BuildFinish). While it works, the old mesh, if any, is drawn. A build whose map changed
 * or left the view meanwhile is thrown away when it comes back. WJPC_BUILD_THREAD_OFF=1: on the main thread, a few
 * ms a frame, as before.
 */
static struct
{
    SDL_Thread *thread;
    SDL_sem *go;
    SDL_atomic_t busy;        // 1 from the hand-over until the worker has finished
    VoxelWorldState *snap;
    unsigned index;           // the map's place in the snapshot's instances
    VoxelAtlasMap map;        // the atlas's slots when the build began
    bool failed;
} sWorker;

static int WorkerMain(void *arg)
{
    (void)arg;
    for (;;)
    {
        SDL_SemWait(sWorker.go);
        VoxelWorld_UseSnapshot(sWorker.snap);
        VoxelWorld_BeginBatch();
        {
            const VoxelMapInstance *inst = VoxelWorld_Instance(sWorker.index);

            if (inst != NULL)
            {
                BuildSteps(&sBackBuild, inst, 0);
                BuildPrepare(&sBackBuild);
            }
        }
        SDL_AtomicSet(&sWorker.busy, 0);
    }
    return 0;
}

static bool WorkerOn(void)
{
    static int sOff = -1;

    if (sOff < 0)
        sOff = getenv("WJPC_BUILD_THREAD_OFF") != NULL;
    if (sOff || sWorker.failed)
        return false;
    if (sWorker.thread == NULL)
    {
        sWorker.go = SDL_CreateSemaphore(0);
        sWorker.snap = VoxelWorld_SnapshotNew();
        if (sWorker.go != NULL && sWorker.snap != NULL)
            sWorker.thread = SDL_CreateThreadWithStackSize(WorkerMain, "voxel-build", 16u << 20, NULL);
        if (sWorker.thread == NULL)
        {
            sWorker.failed = true;
            Voxel3D_Log("[3D] map-building thread not started: maps built on the main thread");
            return false;
        }
    }
    return true;
}

static bool WorkerBusy(void)
{
    return sWorker.thread != NULL && SDL_AtomicGet(&sWorker.busy) != 0;
}

/* Hands sBackBuild (begun for `inst`, the `index`th map) to the worker. */
static void WorkerStart(unsigned index)
{
    VoxelWorld_SnapshotTake(sWorker.snap);
    sWorker.index = index;
    sWorker.map = *sBackBuild.map;
    sBackBuild.map = &sWorker.map;
    SDL_AtomicSet(&sWorker.busy, 1);
    SDL_SemPost(sWorker.go);
}

/* test aid: WJPC_CULL_BOX="group*100+num x0 x1 z0 z1" - that map's triangles with their centre in the box (map
 * cells) are not drawn, to find which map draws a face */
static void CullBox(const VoxelMapInstance *inst, VoxelVertex *v, unsigned n)
{
    const char *e = getenv("WJPC_CULL_BOX");
    int map;
    float x0, x1, z0, z1;

    if (e == NULL || sscanf(e, "%d %f %f %f %f", &map, &x0, &x1, &z0, &z1) != 5
     || inst->mapGroup != map / 100 || inst->mapNum != map % 100)
        return;
    for (unsigned t = 0; t + 2 < n; t += 3)
    {
        float cx = (v[t].x + v[t + 1].x + v[t + 2].x) / 3.0f, cz = (v[t].z + v[t + 1].z + v[t + 2].z) / 3.0f;
        if (cx >= x0 && cx <= x1 && cz >= z0 && cz <= z1)
            v[t + 1] = v[t + 2] = v[t];
    }
}

/*
 * Brennan 2026-11-21 17:25 (image 3, Route 111's Trainer Hill: "the top middle edge of the door frame
 * flickers/Z-fights with the surrounding wall. Give the door frame a clean depth offset so it sits solid"): the door
 * model's front lay in the very plane of the cliff face behind it (z 116, x 30-33), and its frame's sides in the
 * planes of the cliff's sides - the depth test took either, frame to frame. A model face lying flush on a terrain face
 * (same plane, same way, overlapping) is moved 1/128 tile out along its facing, with every model corner on that
 * plane within two cells of it, so the faces beside it stretch to meet it (no crack). Route 111: the door's 6 and 2
 * more; Route 117: none. WJPC_MODEL_FLUSH_OLD=1: as before.
 */
typedef struct { float n[3], d, cx, cy, cz; const VoxelVertex *v; } FlushTri;

static bool FlushPlane(const VoxelVertex *v, float n[3], float *d)
{
    float ux = v[1].x - v[0].x, uy = v[1].y - v[0].y, uz = v[1].z - v[0].z;
    float wx = v[2].x - v[0].x, wy = v[2].y - v[0].y, wz = v[2].z - v[0].z;
    float l;

    n[0] = uy * wz - uz * wy;
    n[1] = uz * wx - ux * wz;
    n[2] = ux * wy - uy * wx;
    l = sqrtf(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
    if (l < 1e-9f)
        return false;
    n[0] /= l, n[1] /= l, n[2] /= l;
    *d = n[0] * v[0].x + n[1] * v[0].y + n[2] * v[0].z;
    return true;
}

/* is (px,py,pz) strictly inside triangle v, seen along its normal's main axis */
static bool FlushInside(const VoxelVertex *v, const float n[3], float px, float py, float pz)
{
    int ax = fabsf(n[0]) >= fabsf(n[1]) && fabsf(n[0]) >= fabsf(n[2]) ? 0 : fabsf(n[1]) >= fabsf(n[2]) ? 1 : 2;
    float a[3][2], p[2], s[3];

    for (int i = 0; i < 3; ++i)
    {
        float q[3] = { v[i].x, v[i].y, v[i].z };
        a[i][0] = q[ax == 0 ? 1 : 0];
        a[i][1] = q[ax == 2 ? 1 : 2];
    }
    {
        float q[3] = { px, py, pz };
        p[0] = q[ax == 0 ? 1 : 0];
        p[1] = q[ax == 2 ? 1 : 2];
    }
    for (int i = 0; i < 3; ++i)
    {
        const float *e0 = a[i], *e1 = a[(i + 1) % 3];
        s[i] = (e1[0] - e0[0]) * (p[1] - e0[1]) - (e1[1] - e0[1]) * (p[0] - e0[0]);
    }
    return (s[0] > 1e-6f && s[1] > 1e-6f && s[2] > 1e-6f) || (s[0] < -1e-6f && s[1] < -1e-6f && s[2] < -1e-6f);
}

static void ModelsOffFlush(InstanceBuild *pb)
{
    static int sOld = -1;
    VoxelVertex *mv = pb->v[2];
    unsigned mn = pb->n[2], hits = 0;
    float x0 = 1e9f, x1 = -1e9f, z0 = 1e9f, z1 = -1e9f;
    FlushTri *terr = NULL, *hit = NULL;
    unsigned tn = 0, tcap = 0, hn = 0, hcap = 0;
    Uint64 t0 = SDL_GetPerformanceCounter();

    if (sOld < 0)
        sOld = getenv("WJPC_MODEL_FLUSH_OLD") != NULL;
    if (sOld || mv == NULL || mn < 3)
        return;
    for (unsigned i = 0; i < mn; ++i)
    {
        x0 = fminf(x0, mv[i].x), x1 = fmaxf(x1, mv[i].x);
        z0 = fminf(z0, mv[i].z), z1 = fmaxf(z1, mv[i].z);
    }
    /* the terrain's triangles about the models */
    for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; ++p)
        for (unsigned t = 0; t + 2 < pb->pageN[p]; t += 3)
        {
            const VoxelVertex *v = &pb->pageV[p][t];
            FlushTri f;

            f.cx = (v[0].x + v[1].x + v[2].x) / 3.0f;
            f.cy = (v[0].y + v[1].y + v[2].y) / 3.0f;
            f.cz = (v[0].z + v[1].z + v[2].z) / 3.0f;
            if (f.cx < x0 - 1.0f || f.cx > x1 + 1.0f || f.cz < z0 - 1.0f || f.cz > z1 + 1.0f || !FlushPlane(v, f.n, &f.d))
                continue;
            f.v = v;
            if (tn == tcap)
            {
                FlushTri *g = realloc(terr, (tcap = tcap ? tcap * 2 : 4096) * sizeof(*g));
                if (g == NULL)
                    goto done;
                terr = g;
            }
            terr[tn++] = f;
        }
    /* the model faces flush on one of them: the terrain sorted into a grid of cells (counting sort), each model face
     * checked against its own cell and the eight round it */
    {
        int gx0 = (int)floorf(x0) - 2, gz0 = (int)floorf(z0) - 2;
        int gw = (int)floorf(x1) + 3 - gx0, gh = (int)floorf(z1) + 3 - gz0;
        unsigned *start = calloc((size_t)gw * gh + 1, sizeof(unsigned));
        unsigned *order = malloc((tn ? tn : 1) * sizeof(unsigned));

        if (start == NULL || order == NULL || gw <= 0 || gh <= 0)
        {
            free(start);
            free(order);
            goto done;
        }
#define FLUSH_CELL(fx, fz) ((int)floorf(fz) - gz0) * gw + ((int)floorf(fx) - gx0)
        for (unsigned k = 0; k < tn; ++k)
            start[FLUSH_CELL(terr[k].cx, terr[k].cz) + 1]++;
        for (int c = 0; c < gw * gh; ++c)
            start[c + 1] += start[c];
        {
            unsigned *fill = malloc(((size_t)gw * gh + 1) * sizeof(unsigned));
            if (fill == NULL)
            {
                free(start);
                free(order);
                goto done;
            }
            memcpy(fill, start, ((size_t)gw * gh + 1) * sizeof(unsigned));
            for (unsigned k = 0; k < tn; ++k)
                order[fill[FLUSH_CELL(terr[k].cx, terr[k].cz)]++] = k;
            free(fill);
        }
        for (unsigned t = 0; t + 2 < mn; t += 3)
        {
            FlushTri m;
            bool found = false;
            int mcx, mcz;

            if (!FlushPlane(&mv[t], m.n, &m.d))
                continue;
            m.cx = (mv[t].x + mv[t + 1].x + mv[t + 2].x) / 3.0f;
            m.cy = (mv[t].y + mv[t + 1].y + mv[t + 2].y) / 3.0f;
            m.cz = (mv[t].z + mv[t + 1].z + mv[t + 2].z) / 3.0f;
            mcx = (int)floorf(m.cx) - gx0;
            mcz = (int)floorf(m.cz) - gz0;
            for (int dz = -1; dz <= 1 && !found; ++dz)
                for (int dx = -1; dx <= 1 && !found; ++dx)
                {
                    int c = (mcz + dz) * gw + (mcx + dx);
                    if (mcx + dx < 0 || mcx + dx >= gw || mcz + dz < 0 || mcz + dz >= gh)
                        continue;
                    for (unsigned q = start[c]; q < start[c + 1] && !found; ++q)
                    {
                        const FlushTri *f = &terr[order[q]];

                        float dot = m.n[0] * f->n[0] + m.n[1] * f->n[1] + m.n[2] * f->n[2];

                        /* (either way round: faces are drawn from both sides) */
                        if (fabsf(dot) < 0.999f || fabsf(m.d - (dot > 0.0f ? f->d : -f->d)) > 1.0f / 256.0f)
                            continue;
                        found = FlushInside(f->v, f->n, m.cx, m.cy, m.cz) || FlushInside(&mv[t], m.n, f->cx, f->cy, f->cz);
                    }
                }
            if (found)
            {
                if (hn == hcap)
                {
                    FlushTri *g = realloc(hit, (hcap = hcap ? hcap * 2 : 64) * sizeof(*g));
                    if (g == NULL)
                        break;
                    hit = g;
                }
                hit[hn++] = m;
            }
        }
#undef FLUSH_CELL
        free(start);
        free(order);
    }
    /* which way out: toward the camera (up, south) for a face it looks at; an upright face across it (east or west)
     * away from the model's own body there */
    for (unsigned h = 0; h < hn; ++h)
    {
        FlushTri *m = &hit[h];
        float toCam = m->n[1] + m->n[2], side;

        if (fabsf(toCam) < 0.05f)
        {
            float sx = 0.0f, sy = 0.0f, sz = 0.0f;
            unsigned c = 0;
            for (unsigned i = 0; i < mn; ++i)
                if (fabsf(mv[i].x - m->cx) <= 1.5f && fabsf(mv[i].y - m->cy) <= 1.5f && fabsf(mv[i].z - m->cz) <= 1.5f)
                    sx += mv[i].x, sy += mv[i].y, sz += mv[i].z, ++c;
            side = c ? m->n[0] * (m->cx - sx / c) + m->n[1] * (m->cy - sy / c) + m->n[2] * (m->cz - sz / c) : 1.0f;
        }
        else
            side = toCam;
        if (side < 0.0f)
        {
            m->n[0] = -m->n[0], m->n[1] = -m->n[1], m->n[2] = -m->n[2];
            m->d = -m->d;
        }
    }
    /* every model corner on such a plane near it, out that way */
    for (unsigned i = 0; i < mn; ++i)
        for (unsigned h = 0; h < hn; ++h)
        {
            const FlushTri *m = &hit[h];
            if (fabsf(m->n[0] * mv[i].x + m->n[1] * mv[i].y + m->n[2] * mv[i].z - m->d) < 1.0f / 1024.0f
             && fabsf(mv[i].x - m->cx) <= 2.0f && fabsf(mv[i].y - m->cy) <= 2.0f && fabsf(mv[i].z - m->cz) <= 2.0f)
            {
                mv[i].x += m->n[0] / 128.0f;
                mv[i].y += m->n[1] / 128.0f;
                mv[i].z += m->n[2] / 128.0f;
                ++hits;
                break;
            }
        }
    if (getenv("WJPC_MODEL_FLUSH_DUMP") != NULL)
        fprintf(stderr, "MODELFLUSH %u faces, %u corners moved, %u terrain, %u model tris, %.2f ms\n", hn, hits, tn,
                mn / 3, (SDL_GetPerformanceCounter() - t0) * 1000.0 / (double)SDL_GetPerformanceFrequency());
done:
    free(terr);
    free(hit);
}

// The finished build into `m` (freed first if it held a mesh), which is then one of this frame's maps.
unsigned gVoxel3DMapBuilds; // maps laid (built and sent), counted for the stall log (Brennan 2026-11-21 15:33: shadow blocks strobing)
static void BuildFinish(InstanceBuild *pb, const VoxelMapInstance *inst, InstanceMesh *m)
{
    Uint64 finish0 = SDL_GetPerformanceCounter();

    gVoxel3DMapBuilds++;

    if (!pb->prepared)
        BuildPrepare(pb); // (a build on the main thread: here; the worker's is ready)
    if (m->used)
        InstanceFree(m);
    /* Brennan 2026-11-24 12:39 (the caves' rock raised into walls: "fix the dark shadows that show and keep the
     * textures the same"): a cave's ground and walls are lit as its floor, whatever way they face - their sides,
     * lit by their facing (no sun in a cave: only the blue sky light) came out darker than the drawing. Marked by
     * shade + 4 (the shader's vFlat, as the cave's models: voxel_building.c CaveFlatShade).
     * WJPC_CAVE_TERRAIN_NORMAL_LIT=1: by their facing, as before. */
    {
        static int sNormalLit = -1;
        if (sNormalLit < 0)
            sNormalLit = getenv("WJPC_CAVE_TERRAIN_NORMAL_LIT") != NULL;
        if (inst != NULL && inst->underground && !sNormalLit)
        {
            /* (and all in the light: under Gamma's lighting a ground vertex's shade is how much of the sun it sees,
             * 0 for a slope turned from it - a dark blur where a rock top sloped down beside a face) */
            /* Brennan 2026-11-24 14:28 ("Naval rock looks 2D"): all at the floor's light the raised rock read as
             * flat - the cave's walls as bright as its floor (Navel Rock's pale rock above all). An upright face a
             * little darker than the floor (by its slope: 0.8 for a wall), never black. WJPC_CAVE_FACES_FLAT=1: no. */
            static int sFacesFlat = -1;
            if (sFacesFlat < 0)
                sFacesFlat = getenv("WJPC_CAVE_FACES_FLAT") != NULL;
            for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
                for (unsigned k = 0; k + 2 < pb->pageN[p]; k += 3)
                {
                    VoxelVertex *t = &pb->pageV[p][k];
                    float ax = t[1].x - t[0].x, ay = t[1].y - t[0].y, az = t[1].z - t[0].z;
                    float bx = t[2].x - t[0].x, by = t[2].y - t[0].y, bz = t[2].z - t[0].z;
                    float nx = ay * bz - az * by, ny = az * bx - ax * bz, nz = ax * by - ay * bx;
                    float len = sqrtf(nx * nx + ny * ny + nz * nz);
                    float f = sFacesFlat || len <= 0.0f ? 1.0f : 0.8f + 0.2f * fabsf(ny) / len;

                    for (int j = 0; j < 3; j++)
                        if (t[j].shade < 3.0f)
                            t[j].shade = gVoxelGammaLight ? 4.0f + f : t[j].shade * f + 4.0f;
                }
        }
    }
    // Terrain split by atlas page (BuildPrepare), positions already quantised: uploaded as they are.
    for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
    {
        unsigned n = pb->pageN[p];

        if (n != 0)
        {
            CullBox(inst, pb->pageV[p], n);
            MeshSet(&m->terrain[p], pb->pageV[p], n, GL_STATIC_DRAW);
            /* (test aid, 2026-11-23 region audit: WJPC_DUMP_TERRAIN_ALL=DIR writes every map's terrain as it is
             * built, DIR/<group>_<num>.<page>, and its trees DIR/<group>_<num>.trees - tools/voxel3ds/coplanar_check.py) */
            if (getenv("WJPC_DUMP_TERRAIN_ALL"))
            {
                char name[256];
                FILE *f;
                snprintf(name, sizeof(name), "%s/%u_%u.%u", getenv("WJPC_DUMP_TERRAIN_ALL"), inst->mapGroup, inst->mapNum, p);
                if ((f = fopen(name, "wb")) != NULL)
                {
                    fwrite(pb->pageV[p], sizeof(VoxelVertex), n, f);
                    fclose(f);
                }
            }
            if (getenv("WJPC_DUMP_TERRAIN") && inst->mapGroup == atoi(getenv("WJPC_DUMP_TERRAIN")) / 100
             && inst->mapNum == atoi(getenv("WJPC_DUMP_TERRAIN")) % 100)
            {
                char name[256];
                FILE *f;
                snprintf(name, sizeof(name), "%s.%u", getenv("WJPC_DUMP_TERRAIN_FILE") ? getenv("WJPC_DUMP_TERRAIN_FILE") : "terrain.bin", p);
                if ((f = fopen(name, "wb")) != NULL)
                {
                    fwrite(pb->pageV[p], sizeof(VoxelVertex), n, f);
                    fclose(f);
                }
            }
        }
        else
            MeshFree(&m->terrain[p]);
    }
    CullBox(inst, pb->v[1], pb->n[1]);
    CullBox(inst, pb->v[2], pb->n[2]);
    MeshSet(&m->trees, pb->v[1], pb->n[1], GL_STATIC_DRAW);
    MeshSet(&m->buildings, pb->v[2], pb->n[2], GL_STATIC_DRAW);
    if (getenv("WJPC_DUMP_TERRAIN_ALL"))
    {
        char name[256];
        FILE *f;
        snprintf(name, sizeof(name), "%s/%u_%u.trees", getenv("WJPC_DUMP_TERRAIN_ALL"), inst->mapGroup, inst->mapNum);
        if ((f = fopen(name, "wb")) != NULL)
        {
            fwrite(pb->v[1], sizeof(VoxelVertex), pb->n[1], f);
            fclose(f);
        }
        snprintf(name, sizeof(name), "%s/%u_%u.models", getenv("WJPC_DUMP_TERRAIN_ALL"), inst->mapGroup, inst->mapNum);
        if ((f = fopen(name, "wb")) != NULL)
        {
            fwrite(pb->v[2], sizeof(VoxelVertex), pb->n[2], f);
            fclose(f);
        }
        /* where the map is drawn (MeshDraw's offset), for tools/pc/gapscan/pick.py with WJPC_DUMP_MVP */
        snprintf(name, sizeof(name), "%s/%u_%u.origin", getenv("WJPC_DUMP_TERRAIN_ALL"), inst->mapGroup, inst->mapNum);
        if ((f = fopen(name, "w")) != NULL)
        {
            fprintf(f, "%d %d\n", inst->originX, inst->originY);
            fclose(f);
        }
    }
    {
        VoxelVertex *k = pb->n[2] ? malloc((size_t)pb->n[2] * sizeof(VoxelVertex)) : NULL;
        for (int kind = 0; kind < FACE_KINDS; kind++)
            MeshSet(&m->buildingKind[kind], k, k != NULL ? FacesOfKind(pb->v[2], pb->n[2], kind, k) : 0, GL_STATIC_DRAW);
        free(k);
    }
    if (getenv("WJPC_DUMP_TERRAIN") && inst->mapGroup == atoi(getenv("WJPC_DUMP_TERRAIN")) / 100
     && inst->mapNum == atoi(getenv("WJPC_DUMP_TERRAIN")) % 100)
    {
        /* (test aid: the models too - <file>.models: the small models' vertices as built (VoxelVertex, map
         * cells); <file>.statics: per big model "pageModel x y z" then its vertices as text) */
        char name[256];
        FILE *f;
        snprintf(name, sizeof(name), "%s.models", getenv("WJPC_DUMP_TERRAIN_FILE") ? getenv("WJPC_DUMP_TERRAIN_FILE") : "terrain.bin");
        if ((f = fopen(name, "wb")) != NULL)
        {
            fwrite(pb->v[2], sizeof(VoxelVertex), pb->n[2], f);
            fclose(f);
        }
        snprintf(name, sizeof(name), "%s.trees", getenv("WJPC_DUMP_TERRAIN_FILE") ? getenv("WJPC_DUMP_TERRAIN_FILE") : "terrain.bin");
        if ((f = fopen(name, "wb")) != NULL)
        {
            fwrite(pb->v[1], sizeof(VoxelVertex), pb->n[1], f);
            fclose(f);
        }
        snprintf(name, sizeof(name), "%s.statics", getenv("WJPC_DUMP_TERRAIN_FILE") ? getenv("WJPC_DUMP_TERRAIN_FILE") : "terrain.bin");
        if ((f = fopen(name, "w")) != NULL)
        {
            for (unsigned i = 0; i < pb->staticCount; i++)
            {
                const VoxelStaticDraw *d = &pb->statics[i];
                const VoxelVertex *v;
                unsigned count;
                float ox, oy, su, sv;
                fprintf(f, "static %u %g %g %g\n", d->pageModel, d->x, d->y, d->z);
                if (VoxelBuildings_StaticModel(d->pageModel, &v, &count, &ox, &oy, &su, &sv))
                    for (unsigned k = 0; k < count; k++)
                        fprintf(f, "v %g %g %g %g %g %g\n", v[k].x, v[k].y, v[k].z, v[k].u, v[k].v, v[k].shade);
            }
            fclose(f);
        }
    }
    memcpy(m->statics, pb->statics, pb->staticCount * sizeof(VoxelStaticDraw));
    m->staticCount = pb->staticCount;
    m->buildingPage = VoxelBuildings_PageOf(inst);
    m->atlas = pb->atlas;
    m->atlasGen = pb->atlasGen; // (grown meanwhile: laid again, its new metatiles were not in this build)
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
    Voxel3D_Log("[3D] built %d:%d (%dx%d): %u ground, %u tree, %u model vertices, %u big models; %.1f ms over %u frame%s"
                " (+ %.1f ms sorting and uploading)",
                inst->mapGroup, inst->mapNum, inst->width, inst->height, pb->n[0], pb->n[1], pb->n[2],
                m->staticCount, pb->ms, pb->frames, pb->frames == 1 ? "" : "s",
                (SDL_GetPerformanceCounter() - finish0) * 1000.0 / SDL_GetPerformanceFrequency());
}

// ---------------------------------------------------------------------------
// The belt: filler (sea, forest) round the maps, the world's edge.

// POL-30 (Brennan 2026-11-15: "fix route 113"): the belt in layers, one per atlas it is drawn with. Layer 0 is
// the current map's atlas; the others are the atlases of loaded maps on other tile sets, whose border the belt
// shows beside them (Route 113's ash trees seen from Route 111/112). A ground vertex of layer L carries
// 2 * VOXEL_ATLAS_PAGES * L more in u until BeltAssemble sorts it into its layer's page.
#define BELT_LAYERS 16  /* POL-46: was 4 - a fifth tile set's belt was never drawn (a hole of sky north of Route 118) */
static Mesh sBeltTerrain[BELT_LAYERS][VOXEL_ATLAS_PAGES], sBeltTrees;
static Atlas *sBeltAtlas;
static Atlas *sBeltLayer[BELT_LAYERS];        // what the pieces are built with now ([0] unused: atlas0)
static uint32_t sBeltLayerGen[BELT_LAYERS];
static unsigned sBeltLayers = 1;
static Atlas *sBeltDrawn[BELT_LAYERS];        // what the uploaded belt was built with
static uint32_t sBeltDrawnGen[BELT_LAYERS];
static int sBeltX = 0x7FFFFFFF, sBeltY;
/* Brennan 2026-11-22 13:42 (stall log: 166-308 ms of "belt" in one frame entering Verdanturf, Littleroot, Oldale,
 * Petalburg Woods): after a warp (a new map, not a walk across a seam) the whole belt was built in one frame. Now it
 * is built a few ms a frame (WJPC_BELT_WARP_MS, default 8) while the screen fades in, nearest the player first, and
 * what is built is shown every third frame. WJPC_BELT_FULL_ON_WARP=1: all in one frame, as before. */
static unsigned sBeltResetGen, sBeltWarpGen = ~0u;
static bool sBeltWarpPending, sBeltWarpShown;
static int sBeltWarpX, sBeltWarpY;
static double sBeltBudgetOverrideMs;
// DLX-18: after a crossing the old belt is drawn moved into the new map's coordinates until the new one is
// ready; sBeltRedo asks for that (all pieces again, a few a frame).
static int sBeltShiftX, sBeltShiftZ;
static bool sBeltRedo;
static Atlas *sBeltCacheAtlas;
static uint32_t sBorderRemadeKey; // POL-30, see the belt in Voxel3D_Render
static uint32_t sBeltCacheGen;
// Brennan 2026-11-20 20:15 (stall log: "belt" 150-1680 ms a frame biking and crossing maps round Lavaridge): the
// 64-cell belt (POL-51, for Mt Pyre's sea 16 levels down) is 2.56 times the pieces of the 40-cell one. 64 only on the
// maps that need it (Mt Pyre outside and summit), 40 elsewhere; chosen when a map's belt is built whole.
// WJPC_BELT_RADIUS=n: that, everywhere.
// #25 (Brennan 2026-11-23 21:03: "build the background further out, so that no sky is shown"): 56 elsewhere (was
// 40). At 40 the far top corner showed sky running east on Route 121 (up to 11,000 px a frame at 1920x1080) and Ever
// Grande's north view; at 56 none. The belt is built a few pieces a frame, so no frame took longer on the test box
// (worst belt frame 23 ms at 56 vs 36 ms at 40); it takes about twice as many frames to finish.
// Ever Grande (layout 9, its cliffs 11 levels over the sea: the view from the top reaches further) 64 as Mt Pyre:
// at 56 the far top-right corner still showed sky from (30,50) - 537 px at 1920x1080, 951 at 2560x1440; at 64 none.
static int sBeltRadius = 56;
#define BELT_RADIUS sBeltRadius

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

#define BELT_CACHE 640 // (BELT_RADIUS 64: 256 pieces in view)
#define BELT_CHUNKS_PER_FRAME 10
static BeltChunk sBeltCache[BELT_CACHE];
static unsigned sBeltUse;
static int sBeltWantX = 0x7FFFFFFF, sBeltWantY;

extern unsigned gVoxel3DBeltFlushes;
static void BeltCache_Flush(void)
{
    gVoxel3DBeltFlushes++;
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
    for (unsigned layer = 0; layer < sBeltLayers; layer++)
    {
        Atlas *a = layer == 0 ? atlas : sBeltLayer[layer];
        unsigned first = b.count;

        VoxelBuilder_SetAtlas(&b, &a->map);
        VoxelMesh_SetBeltPass(a->primary, a->secondary);
        for (int y = y0; y < y0 + CHUNK; y++)
            for (int x = x0; x < x0 + CHUNK; x++)
                VoxelMesh_EmitBorder(&b, x, y, x + 1, y + 1);
        for (unsigned i = first; layer != 0 && i < b.count; i++)
            b.vertices[i].u += 2.0f * VOXEL_ATLAS_PAGES * layer;
    }
    VoxelMesh_SetBeltPass(NULL, NULL);
    VoxelBuilder_SetAtlas(&b, &atlas->map);
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
static unsigned sBeltBuilt; // pieces built this frame (the log)
extern volatile int gVoxelProfileOn; // sdl2.c WJPC_PROFILE
static bool BeltBuildStep(const VoxelMapInstance *inst0, Atlas *atlas, int cx, int cy, int budget)
{
    bool ready = true;
    static double sBudgetMs = -1.0;
    Uint64 deadline;

    if (sBudgetMs < 0.0)
        sBudgetMs = getenv("WJPC_BELT_BUDGET_MS") ? atof(getenv("WJPC_BELT_BUDGET_MS")) : 1.0;
    deadline = SDL_GetPerformanceCounter()
             + (Uint64)((sBeltBudgetOverrideMs > 0.0 ? sBeltBudgetOverrideMs : sBudgetMs) * SDL_GetPerformanceFrequency() / 1000.0);

    // (after a warp: nearest the player first, so the belt shown while it is built is the part in view)
    static int order[4096][2];
    int n = 0;
    for (int y0 = cy - BELT_RADIUS; y0 < cy + BELT_RADIUS; y0 += CHUNK)
        for (int x0 = cx - BELT_RADIUS; x0 < cx + BELT_RADIUS && n < 4096; x0 += CHUNK)
        {
            order[n][0] = x0;
            order[n][1] = y0;
            n++;
        }
    if (sBeltBudgetOverrideMs > 0.0)
        for (int i = 1; i < n; i++)
        {
            int x = order[i][0], y = order[i][1], j = i;
            int d = abs(x + CHUNK / 2 - cx) + abs(y + CHUNK / 2 - cy);
            while (j > 0 && abs(order[j - 1][0] + CHUNK / 2 - cx) + abs(order[j - 1][1] + CHUNK / 2 - cy) > d)
            {
                order[j][0] = order[j - 1][0];
                order[j][1] = order[j - 1][1];
                j--;
            }
            order[j][0] = x;
            order[j][1] = y;
        }
    for (int i = 0; i < n; i++)
        {
            int x0 = order[i][0], y0 = order[i][1];
            if (BeltCache_Find(x0, y0) != NULL)
                continue;
            if (budget == 0)
            {
                ready = false;
                continue;
            }
            gVoxelProfileOn = 1;
            BeltCache_Build(inst0, atlas, x0, y0, cx, cy);
            gVoxelProfileOn = 0;
            sBeltBuilt++;
            if (budget > 0)
            {
                budget--;
                ready = false; // put it together next frame, so building and uploading never share one
                /* (Brennan 20:15: no more than a few ms of pieces a frame - the old belt stays up meanwhile.
                 * WJPC_BELT_BUDGET_MS=n, default 1) */
                if (SDL_GetPerformanceCounter() >= deadline)
                    budget = 0;
            }
        }
    return ready;
}

// Put the window's pieces together and upload them (all pieces must be in the cache).
// Ground triangles go straight into their atlas page's run of sSorted (count, then place: one pass each).
static void BeltAssemble(Atlas *atlas, int cx, int cy)
{
#define BELT_BINS (BELT_LAYERS * VOXEL_ATLAS_PAGES)
    unsigned count[BELT_BINS] = {0}, at[BELT_BINS], total = 0;

    sTerrainCount = sTreeCount = sBuildCount = 0;
    sBeltUse++;
    for (int pass = 0; pass < 2; pass++)
    {
        if (pass == 1)
            for (unsigned p = 0; p < BELT_BINS; p++)
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
                    if (page >= BELT_BINS)
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
            for (unsigned p = 0; p < BELT_BINS; p++)
                sum += count[p];
            if (sum > SCRATCH_VERTS) // far beyond any real belt; keep it safe anyway
            {
                memset(count, 0, sizeof(count));
                break;
            }
        }
    }
    total = 0;
    for (unsigned p = 0; p < BELT_BINS; p++)
    {
        MeshSetQuantised(&sBeltTerrain[p / VOXEL_ATLAS_PAGES][p % VOXEL_ATLAS_PAGES], sSorted + total, count[p], GL_STATIC_DRAW);
        if (getenv("WJPC_DUMP_BELT"))
        {
            FILE *f = fopen(getenv("WJPC_DUMP_BELT"), p == 0 ? "wb" : "ab");
            if (f != NULL)
            {
                fwrite(sSorted + total, sizeof(VoxelVertex), count[p], f);
                fclose(f);
            }
        }
        total += count[p];
    }
    MeshSetQuantised(&sBeltTrees, sTreeBuf, sTreeCount, GL_STATIC_DRAW);
    if (getenv("WJPC_DUMP_BELT_TREES")) /* test aid: every assembly's belt trees appended: count, then the vertices */
    {
        FILE *f = fopen(getenv("WJPC_DUMP_BELT_TREES"), "ab");
        if (f != NULL)
        {
            uint32_t n = sTreeCount;
            fwrite(&n, 4, 1, f);
            fwrite(sTreeBuf, sizeof(VoxelVertex), sTreeCount, f);
            fclose(f);
        }
    }
    sBeltAtlas = atlas;
    for (unsigned l = 1; l < BELT_LAYERS; l++)
    {
        sBeltDrawn[l] = l < sBeltLayers ? sBeltLayer[l] : NULL;
        sBeltDrawnGen[l] = l < sBeltLayers ? sBeltLayerGen[l] : 0;
    }
}

// ---------------------------------------------------------------------------
// Sprites (people, Pokemon, field effects) as cards, rebuilt every frame.

#define SPRITE_VERTS 65536u // (was 8192: Brennan 2026-11-20 23:18, berry plants as voxel models)
static VoxelVertex *sSpriteVerts, *sShadowVerts, *sReflectVerts;
static uint16_t *sSpriteAtlas;
static GLuint sSpriteTex;
static Mesh sSpriteMesh, sShadowMesh, sReflectMesh;
// POL-26b: the night-light sprites' cards (blended as the GBA does) and the lamps' pools of light (Emerald 3DS)
#define LAMP_VERTS 4096u
static VoxelVertex *sGlowVerts, *sPoolVerts, *sGlassVerts;
static Mesh sGlowMesh, sPoolMesh, sGlassMesh;
static GLuint sPoolTex;

// ---------------------------------------------------------------------------

static bool sInited, sFailed;
static GLuint sTreeTex;
static VoxelCamera sCamera;
static bool sCameraValid;
// DLX-5: the battle scenery. The field camera is kept aside while a battle is drawn and put back after.
#include "voxel_battle.h"
static bool sBattleMode;

void Voxel3D_SetBattle(bool on)
{
    sBattleMode = on;
}

/*
 * Emerald 3DS's battle camera (ctr_voxel.c, "The 3D battle"), as it is there: the stage searched a slice a
 * frame (voxel_battle.c), then the glide from where the field left the camera - a crane, a dolly zoom, a
 * rush in, a push and a short quake on landing - while the intro would slide the scenery in; BG3's scroll
 * (a move shaking the scenery) moves it. The effects drawn over it come from sBattle.intro.
 */
#define CTR_GAME_WIDTH 400
#define CTR_GAME_HEIGHT 240
#define CTR_BATTLE_ZOOM 1.4f
#define VOXEL_BATTLE_GLIDE_FRAMES 100u
#define VOXEL_BATTLE_TRAVEL 0.78f
#define VOXEL_BATTLE_CRANE 16.0f
#define VOXEL_BATTLE_CRANE_MAX 75.0f
#define VOXEL_BATTLE_VERTIGO 0.40f
#define VOXEL_BATTLE_PUNCH 0.16f
#define VOXEL_BATTLE_QUAKE 0.07f
#define VOXEL_BATTLE_BARS_IN 18.0f
#define VOXEL_BATTLE_INTRO_WAIT 150u
#define VOXEL_BATTLE_STAGE_CELLS 32u
#define VOXEL_BATTLE_STAGE_SPOTS 8u

static struct
{
    bool on, begun, chosen, sliding, glided;
    uint32_t frames, glide;
    VoxelCamera field;
    float targetX, targetZ, ground;
    float shakeX, shakeY;
    Voxel3DBattleIntro intro;
} sBattle;

void Voxel3D_SetBattleFrame(bool introSliding, float shakeX, float shakeY)
{
    sBattle.sliding = introSliding;
    sBattle.shakeX = shakeX;
    sBattle.shakeY = shakeY;
}

void Voxel3D_BattleIntro(Voxel3DBattleIntro *intro)
{
    static const Voxel3DBattleIntro none;
    *intro = sBattle.on ? sBattle.intro : none;
}

static float BClamp(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }
static float BSmooth(float t) { t = BClamp(t, 0.0f, 1.0f); return t * t * (3.0f - 2.0f * t); }
static float BSmoother(float t) { t = BClamp(t, 0.0f, 1.0f); return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); }

static void BattleCamera(void)
{
    const float degrees = 3.14159265f / 180.0f;
    const VoxelCamera *from = &sBattle.field;
    float u, p, q, t, arc, fov, punch, quake, k, x, z, ground, pitch, distance;
    Voxel3DBattleIntro *intro = &sBattle.intro;

    if (sBattle.sliding || sBattle.frames >= VOXEL_BATTLE_INTRO_WAIT)
        sBattle.glided = true;
    ++sBattle.frames;
    if (sBattle.chosen && sBattle.glided && sBattle.glide < VOXEL_BATTLE_GLIDE_FRAMES)
        ++sBattle.glide;
    u = (float)sBattle.glide / (float)VOXEL_BATTLE_GLIDE_FRAMES;
    p = BClamp(u / VOXEL_BATTLE_TRAVEL, 0.0f, 1.0f);
    q = BClamp((u - VOXEL_BATTLE_TRAVEL) / (1.0f - VOXEL_BATTLE_TRAVEL), 0.0f, 1.0f);
    t = BSmoother(p);
    arc = p > 0.0f && p < 1.0f ? sinf(p * 3.14159265f) : 0.0f;
    x = from->targetX + (sBattle.targetX - from->targetX) * t;
    z = from->targetZ + (sBattle.targetZ - from->targetZ) * t;
    ground = from->ground + (sBattle.ground - from->ground) * t;
    pitch = from->pitch + (VOXEL_BATTLE_PITCH - from->pitch) * t;
    distance = from->distance + (VOXEL_BATTLE_DISTANCE - from->distance) * t;
    pitch += VOXEL_BATTLE_CRANE * arc;
    if (pitch > VOXEL_BATTLE_CRANE_MAX && pitch > from->pitch)
        pitch = from->pitch > VOXEL_BATTLE_CRANE_MAX ? from->pitch : VOXEL_BATTLE_CRANE_MAX;
    fov = from->fov * (1.0f - VOXEL_BATTLE_VERTIGO * arc * arc);
    if (arc > 0.0f)
        distance *= tanf(from->fov * 0.5f * degrees) / tanf(fov * 0.5f * degrees);
    else
        fov = from->fov;
    punch = q > 0.0f && q < 1.0f ? expf(-6.0f * q) * sinf(3.0f * 3.14159265f * q) : 0.0f;
    quake = q > 0.0f && q < 1.0f ? VOXEL_BATTLE_QUAKE * expf(-7.0f * q) : 0.0f;
    distance *= 1.0f - VOXEL_BATTLE_PUNCH * punch;
    x += quake * sinf((float)sBattle.glide * 2.7f);
    z += quake * cosf((float)sBattle.glide * 3.9f);
    sCamera.fov = fov;

    intro->flight = arc;
    intro->speed = p > 0.0f && p < 1.0f ? 16.0f * p * p * (1.0f - p) * (1.0f - p) : 0.0f;
    intro->impact = q > 0.0f && q < 1.0f ? expf(-5.0f * q) * (1.0f - q) : 0.0f;
    intro->bars = BSmooth((float)sBattle.frames / VOXEL_BATTLE_BARS_IN) * (1.0f - BSmooth((q - 0.35f) / 0.65f));
    intro->time = (float)sBattle.frames;
    k = CTR_BATTLE_ZOOM * 2.0f * distance * tanf(sCamera.fov * 0.5f * degrees) / (float)CTR_GAME_HEIGHT;
    x += sBattle.shakeX * k;
    z += sBattle.shakeY * k / sinf(pitch * degrees);
    VoxelCamera_Frame(&sCamera, x, z, ground, pitch, distance);
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
    VoxelTree_KeepAlpha(texels);
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
/* DLX-23: indoors the room is unlit (full colour), so outdoor bloom makes every white wall glow. */
static bool sIndoorNow;
static float IndoorBloomScale(void)
{
    const char *e = getenv("WJPC_3D_INDOOR_BLOOM");
    return e != NULL ? (float)atof(e) : 0.0f; /* Emerald 3DS: no bloom indoors (ctr_voxel.c BaseLightFor) */
}
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
    "uniform vec4 room;\n"           // x colour gain, y vignette intensity, z height/width (Gamma's triggers)\n"
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
    // POL-2/3/8/28 (Brennan 2026-11-16: "the bases around trees or some edge lines still show either black or blue. try
    // using a green safety screen behind it so the colors match and dont show backgrounds"): a pixel nothing was drawn
    // on (its depth still the clear's) between two drawn ones, across or down, up to 2 px apart, is a crack between
    // faces - at 4K the seams too thin to see at console sizes - not sky or the room's dark: it takes its drawn
    // neighbours' colour, so the screen behind it is the ground's own. radius > 0.5: on (WJPC_NO_CRACK_FILL=1 off).
    "  if (radius > 0.5 && texture2D(depthTex, uv).r >= 0.9999999) {\n"
    "    vec2 tx = vec2(texel.x, 0.0), ty = vec2(0.0, texel.y); vec2 a = vec2(-9.0), b = vec2(-9.0);\n"
    "    if (texture2D(depthTex, uv - tx).r < 0.9999999) {\n"
    "      if (texture2D(depthTex, uv + tx).r < 0.9999999) { a = uv - tx; b = uv + tx; }\n"
    "      else if (texture2D(depthTex, uv + 2.0 * tx).r < 0.9999999) { a = uv - tx; b = uv + 2.0 * tx; } }\n"
    "    else if (texture2D(depthTex, uv - 2.0 * tx).r < 0.9999999 && texture2D(depthTex, uv + tx).r < 0.9999999) { a = uv - 2.0 * tx; b = uv + tx; }\n"
    "    if (a.x < -8.0 && texture2D(depthTex, uv - ty).r < 0.9999999) {\n"
    "      if (texture2D(depthTex, uv + ty).r < 0.9999999) { a = uv - ty; b = uv + ty; }\n"
    "      else if (texture2D(depthTex, uv + 2.0 * ty).r < 0.9999999) { a = uv - ty; b = uv + 2.0 * ty; } }\n"
    "    else if (a.x < -8.0 && texture2D(depthTex, uv - 2.0 * ty).r < 0.9999999 && texture2D(depthTex, uv + ty).r < 0.9999999) { a = uv - 2.0 * ty; b = uv + ty; }\n"
    "    if (a.x > -8.0) c = 0.5 * (texture2D(src, a).rgb + texture2D(src, b).rgb);\n"
    "  }\n"
    "  c += (texture2D(src2, uv).rgb * tint[0] + texture2D(b1, uv).rgb * tint[1] + texture2D(b2, uv).rgb * tint[2]\n"
    "       + texture2D(b3, uv).rgb * tint[3] + texture2D(b4, uv).rgb * tint[4] + texture2D(b5, uv).rgb * tint[5]) * misc.z;\n"
    "  c *= misc.x;\n"
    // UE's vignette as Gamma's compiled tonemap shaders do it (see gamma2d.c), and the colour gain
    // (uniform, so it can go before the saturation it follows in UE)
    "  { vec2 vp = (uv * 2.0 - 1.0) * vec2(1.0, room.z) * (1.41421354 / sqrt(1.0 + room.z * room.z)) * room.y;\n"
    "    float vt = dot(vp, vp) + 1.0; c *= room.x / (vt * vt); }\n"
    "  vec3 ap = sRGB_2_AP1 * c;\n"
    // UE CombineLUTs as Gamma's compiled LUT shader has it (its matrices are literals there): expand bright
    // saturated colours (ExpandGamut 1.0), saturation, blue correction 0.6, film curve, blue correction undone
    "  { float l = dot(ap, AP1_Y); vec3 ch = ap / max(l, 1e-10); float d2 = dot(ch - 1.0, ch - 1.0);\n"
    "    float ex = (1.0 - exp2(-4.0 * d2)) * (1.0 - exp2(-4.0 * 1.0 * l * l));\n"
    "    vec3 e = vec3(dot(vec3(1.37041271, -0.3292913, -0.06368294), ap), dot(vec3(-0.08343419, 1.09709098, -0.01086125), ap),\n"
    "                  dot(vec3(-0.02579326, -0.09862564, 1.20369427), ap));\n"
    "    ap = mix(ap, e, ex); }\n"
    "  ap = max(mix(vec3(dot(ap, AP1_Y)), ap, misc.y), 0.0);\n"
    "  ap = mix(ap, vec3(dot(vec3(0.93863938, 0.0, 0.06136062), ap), dot(vec3(0.0, 0.83079413, 0.16920587), ap), ap.b), 0.6);\n"
    "  vec3 tn = film(AP1_2_sRGB * ap);\n"
    "  tn = mix(tn, vec3(dot(vec3(1.06537488, 0.00000145, -0.06537101), tn), dot(vec3(-0.00000035, 1.20366352, -0.20366772), tn),\n"
    "                    dot(vec3(0.00000002, 0.00000002, 0.9999996), tn)), 0.6);\n"
    "  vec3 outc = toSrgb(max(AP1_2_sRGB * tn, 0.0));\n"
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

/*
 * Polish option 5 (Brennan 2026-11-16, "go"): anti-aliasing. Emerald 3DS draws 400x240 with no smoothing, where a
 * hair-wide seam or a stair-stepped edge is never seen; at 1080p-4K the same seams show as thin lines and every
 * edge crawls while the camera moves. The scene is drawn into a multisampled target (4 samples a pixel by
 * default) and resolved into the half-float target the screen pass reads, depth too (depth of field, the
 * lamps' glows). "antialias=" in WallysJourney.cfg (sdl2.c: 0 off, 2, 4, 8); WJPC_MSAA overrides it.
 * Gamma lighting's path only (the one with the half-float target). The functions are looked up on their own:
 * a driver without them draws as before.
 */
typedef void (APIENTRY *PFN_wj_glRenderbufferStorageMultisample)(GLenum, GLsizei, GLenum, GLsizei, GLsizei);
typedef void (APIENTRY *PFN_wj_glBlitFramebuffer)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint,
                                                  GLbitfield, GLenum);
static PFN_wj_glRenderbufferStorageMultisample wj_glRenderbufferStorageMultisample;
static PFN_wj_glBlitFramebuffer wj_glBlitFramebuffer;
static GLuint sMsFbo, sMsColor, sMsDepth;
static int sMsW, sMsH, sMsSamples, sMsWanted = -1;
static bool sMsActive, sMsLooked;

/* Brennan 2026-11-21 10:34 ("put toggles on F1-F4 if you want me to verify each one live on Route 110"): effects
 * switched off live by F1-F6 (sdl2.c), as the WJPC_ test switches do at start-up. VOXEL_FX_* in voxel_gl.h. */
unsigned gVoxelFxOff;

void Voxel3D_SetAntiAlias(int samples)
{
    sMsWanted = samples < 0 ? 0 : samples > 8 ? 8 : samples;
}

static int MsaaSamples(void)
{
    const char *e = getenv("WJPC_MSAA");
    int n = e != NULL ? atoi(e) : sMsWanted >= 0 ? sMsWanted : 4;
    GLint most = 0;

    if (gVoxelFxOff & VOXEL_FX_ANTIALIAS)
        return 0;

    if (n <= 1)
        return 0;
    if (!sMsLooked)
    {
        sMsLooked = true;
        wj_glRenderbufferStorageMultisample =
            (PFN_wj_glRenderbufferStorageMultisample)SDL_GL_GetProcAddress("glRenderbufferStorageMultisample");
        wj_glBlitFramebuffer = (PFN_wj_glBlitFramebuffer)SDL_GL_GetProcAddress("glBlitFramebuffer");
        if (wj_glRenderbufferStorageMultisample == NULL || wj_glBlitFramebuffer == NULL)
            Voxel3D_Log("[3D] anti-aliasing not available (no multisampled framebuffers)");
    }
    if (wj_glRenderbufferStorageMultisample == NULL || wj_glBlitFramebuffer == NULL)
        return 0;
    wj_glGetIntegerv(0x8D57 /* GL_MAX_SAMPLES */, &most);
    n = n > most ? (int)most : n;
    return n;
}

/* Draw into the multisampled target (after PostBeginScene bound the half-float one); false: draw as before. */
static bool MsaaBegin(int w, int h)
{
    int n = MsaaSamples();

    /* Brennan 2026-11-16 (build 16f at 4K: laggy, and a crash in the graphics driver): a multisampled
     * half-float target is 8 bytes a sample - 4 samples at 3840x2160 are 265 MB with their depth, every frame
     * resolved again. Above 1440p it is off (the pixels are small enough there), at 1440p 2 samples at most. */
    if ((long)w * h > 2560L * 1440L && getenv("WJPC_MSAA") == NULL)
        n = 0;
    else if ((long)w * h > 1920L * 1080L && n > 2 && getenv("WJPC_MSAA") == NULL)
        n = 2;

    sMsActive = false;
    if (n <= 1)
        return false;
    if (sMsFbo == 0 || sMsW != w || sMsH != h || sMsSamples != n)
    {
        if (sMsFbo == 0)
            wj_glGenFramebuffers(1, &sMsFbo);
        if (sMsColor == 0)
            wj_glGenRenderbuffers(1, &sMsColor);
        if (sMsDepth == 0)
            wj_glGenRenderbuffers(1, &sMsDepth);
        wj_glBindRenderbuffer(GL_RENDERBUFFER, sMsColor);
        wj_glRenderbufferStorageMultisample(GL_RENDERBUFFER, n, 0x881A /* RGBA16F */, w, h);
        wj_glBindRenderbuffer(GL_RENDERBUFFER, sMsDepth);
        wj_glRenderbufferStorageMultisample(GL_RENDERBUFFER, n, GL_DEPTH_COMPONENT24, w, h);
        wj_glBindFramebuffer(GL_FRAMEBUFFER, sMsFbo);
        wj_glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, sMsColor);
        wj_glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sMsDepth);
        sMsW = w;
        sMsH = h;
        sMsSamples = n;
    }
    wj_glBindFramebuffer(GL_FRAMEBUFFER, sMsFbo);
    if (wj_glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        static bool logged;
        if (!logged)
            Voxel3D_Log("[3D] anti-aliasing target incomplete (%d samples): drawn without", n);
        logged = true;
        wj_glBindFramebuffer(GL_FRAMEBUFFER, sHdrFbo);
        return false;
    }
    sMsActive = true;
    return true;
}

/* The multisampled picture and depth into the half-float target, for the screen pass. */
static void MsaaResolve(void)
{
    if (!sMsActive)
        return;
    sMsActive = false;
    wj_glBindFramebuffer(0x8CA8 /* GL_READ_FRAMEBUFFER */, sMsFbo);
    wj_glBindFramebuffer(0x8CA9 /* GL_DRAW_FRAMEBUFFER */, sHdrFbo);
    wj_glBlitFramebuffer(0, 0, sMsW, sMsH, 0, 0, sMsW, sMsH, 0x4000 /* COLOR */, GL_NEAREST);
    wj_glBlitFramebuffer(0, 0, sMsW, sMsH, 0, 0, sMsW, sMsH, 0x100 /* DEPTH */, GL_NEAREST);
    wj_glBindFramebuffer(GL_FRAMEBUFFER, sHdrFbo);
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
    if (wj_glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return false;
    MsaaBegin(w, h);
    return true;
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
        {
            extern int WideView_GammaRoom(void);
            /* POL-31: New Mauville's (room 4) white walls bloomed into a halo over the dark - a third of it there */
            float roomBloom = WideView_GammaRoom() == 4 ? 0.35f : 1.0f;

            wj_glUniform4f(sPU_misc, sExposure, 1.25f, getenv("WJPC_3D_NOBLOOM") || (gVoxelFxOff & VOXEL_FX_BLOOM) ? 0.0f
                           : 6.0f * roomBloom * (sIndoorNow ? IndoorBloomScale() : 1.0f), 0.4f);
        }
        wj_glUniform1fv(wj_glGetUniformLocation(sPostProg, "tint"), 6, tint);
        {
            /* Gamma: engine default vignette 0.4; Granite Cave gain 0.49 + vignette 3, Brawly's gym 0.49 + 2 */
            extern int WideView_GammaRoom(void);
            int room = WideView_GammaRoom();
            /* room 4 (POL-31, New Mauville): gain NEW_MAUVILLE_GAIN, the default vignette; WJPC_ROOM_GAIN=x tries another */
            float gain4 = getenv("WJPC_ROOM_GAIN") ? (float)atof(getenv("WJPC_ROOM_GAIN")) : 0.7f;
            wj_glUniform4f(wj_glGetUniformLocation(sPostProg, "room"), room == 1 || room == 2 ? 0.49f : room == 4 ? gain4 : 1.0f,
                           room == 1 ? 3.0f : room == 2 ? 2.0f : 0.4f, (float)h / (float)w, 0.0f);
        }
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
            wj_glUniform4f(sPU_coc, getenv("WJPC_3D_NODOF") || (gVoxelFxOff & VOXEL_FX_DOF) ? 0.0f : k, S, 0.5f, 400.0f);
        }
        {
            static int sNoFill = -1;   /* the crack fill in the final pass (POL-2/3/8/28) */

            if (sNoFill < 0)
                sNoFill = getenv("WJPC_NO_CRACK_FILL") != NULL;
            wj_glUniform1f(sPU_radius, sNoFill || (gVoxelFxOff & VOXEL_FX_CRACKFILL) ? 0.0f : 1.0f);
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
    LoadGammaClouds();
    if (getenv("WJPC_3D_GAMMALIGHT") != NULL)
        Voxel3D_SetGammaLight(atoi(getenv("WJPC_3D_GAMMALIGHT")) != 0);
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
    sGlowVerts = malloc(LAMP_VERTS * sizeof(VoxelVertex) * 3);
    sPoolVerts = sGlowVerts != NULL ? sGlowVerts + LAMP_VERTS : NULL;
    sGlassVerts = sGlowVerts != NULL ? sGlowVerts + 2 * LAMP_VERTS : NULL;
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


/*
 * DLX-24, ported from Emerald 3DS (ctr_voxel.c CtrVoxel_ProjectPictureTile + 3ds_video.c PlaceScreenObject):
 * the healing machine's balls and monitors are placed by hand on the 240x160 picture around the player (the
 * player's tile centre at GBA (120, 80)), so they go where the map's machine is: the tile (tileX, tileY) from
 * the player, a tile up in the air, seen by this frame's camera. Out: 0..1 across and down the 3D picture.
 */
static float sFieldMvp[16];
static bool sHaveFieldMvp;
bool Voxel3D_ProjectPictureTile(float tileX, float tileY, float *outX, float *outY)
{
    float px, pz, wx, wy, wz, cx, cy, cw;
    if (!sHaveFieldMvp)
        return false;
    VoxelEntities_GetPlayerWorldPos(&px, &pz);
    wx = px + 0.5f + tileX;
    wy = sCamera.ground + 1.0f;   /* "A tile up in the air: seen from above, what stands a tile high is a
                                   * tile further up the picture than the floor under it." */
    wz = pz + 0.5f + tileY + 1.0f;
    cx = sFieldMvp[0] * wx + sFieldMvp[4] * wy + sFieldMvp[8] * wz + sFieldMvp[12];
    cy = sFieldMvp[1] * wx + sFieldMvp[5] * wy + sFieldMvp[9] * wz + sFieldMvp[13];
    cw = sFieldMvp[3] * wx + sFieldMvp[7] * wy + sFieldMvp[11] * wz + sFieldMvp[15];
    if (cw <= 1e-4f)
        return false;
    *outX = (cx / cw) * 0.5f + 0.5f;
    /* The field is drawn upside down (SDL shows render targets as stored), so NDC y already points down. */
    *outY = 0.5f + (cy / cw) * 0.5f;
    if (getenv("WJPC_DEBUG_MACHINE"))
        fprintf(stderr, "machine tile %.2f %.2f player %.2f %.2f ground %.2f cam target %.2f %.2f %.2f -> %.3f %.3f\n",
                tileX, tileY, px, pz, sCamera.ground, sCamera.targetX, sCamera.targetY, sCamera.targetZ, *outX, *outY);
    return true;
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
    return m->atlas != NULL && m->atlas->valid && m->atlas->primary == m->primary && m->atlas->secondary == m->secondary
        && m->atlasGen >= m->atlas->layoutGen; // POL-15: laid on an earlier growth of it: the same slots
}

/* (#28 above) a line in WallysJourney_stalls.txt: the map, the frame, why it is not drawn (at most 300 lines) */
static void MapHiddenNote(const InstanceMesh *m, const VoxelMapInstance *inst)
{
    static unsigned sLines;
    FILE *f;
    const char *why = m->atlas == NULL ? "no atlas"
                    : !m->atlas->valid ? "its atlas is being made again"
                    : (m->atlas->primary != m->primary || m->atlas->secondary != m->secondary) ? "its atlas slot now holds other tile sets"
                    : m->atlasGen < m->atlas->layoutGen ? "its atlas was laid out anew"
                    : "other";
    if (sLines >= 300 || (f = fopen("WallysJourney_stalls.txt", "a")) == NULL)
        return;
    sLines++;
    fprintf(f, "MAP HIDDEN frame %u: map %d:%d (player on %d:%d) - %s\n", sFrame, m->mapGroup, m->mapNum,
            VoxelWorld_Instance(0)->mapGroup, VoxelWorld_Instance(0)->mapNum, why);
    fclose(f);
    (void)inst;
}

/* (#28) and the other way it can look: the map ahead not drawn yet while the player is near it ("the area running
 * into was taking too long to load"). A map the player comes within 16 tiles of while it has nothing drawn is
 * timed; when it shows up (or leaves the world), one line: how many frames it was missing and how close the player
 * got. */
static void MapLateNote(const VoxelMapInstance *inst, const InstanceMesh *m)
{
    static struct { int g, n; uint32_t first; float closest; const char *why; } sWait[16];
    bool shown = m != NULL && m->shown;
    static unsigned sLines;
    float px, pz, dx, dz, d;
    int k, freeK = -1;
    for (k = 0; k < 16; k++)
    {
        if (sWait[k].first != 0 && sWait[k].g == inst->mapGroup && sWait[k].n == inst->mapNum)
            break;
        if (sWait[k].first == 0 && freeK < 0)
            freeK = k;
    }
    if (k < 16 && sFrame - sWait[k].first > 1200)
        sWait[k].first = 0; // left behind long ago without showing: forgotten
    if (shown)
    {
        FILE *f;
        if (k < 16 && sWait[k].first != 0)
        {
            if (sLines < 300 && (f = fopen("WallysJourney_stalls.txt", "a")) != NULL)
            {
                sLines++;
                fprintf(f, "MAP LATE frame %u: map %d:%d drawn after %u frames missing (%s); the player came within %.0f tiles of it\n",
                        sFrame, inst->mapGroup, inst->mapNum, sFrame - sWait[k].first, sWait[k].why, sWait[k].closest);
                fclose(f);
            }
            sWait[k].first = 0;
        }
        return;
    }
    VoxelWorld_GetPlayerWorldCoords(&px, &pz);
    dx = px < inst->originX ? inst->originX - px : px > inst->originX + inst->width ? px - inst->originX - inst->width : 0;
    dz = pz < inst->originY ? inst->originY - pz : pz > inst->originY + inst->height ? pz - inst->originY - inst->height : 0;
    d = dx > dz ? dx : dz;
    if (d > 16.0f)
        return;
    if (k == 16)
    {
        if (freeK < 0)
            return;
        k = freeK;
        sWait[k].g = inst->mapGroup;
        sWait[k].n = inst->mapNum;
        sWait[k].first = sFrame ? sFrame : 1;
        sWait[k].closest = d;
        sWait[k].why = m == NULL ? "not built yet" : m->atlas == NULL ? "no atlas" : !m->atlas->valid ? "atlas being made"
                     : (m->atlas->primary != m->primary || m->atlas->secondary != m->secondary) ? "atlas slot holds other tile sets"
                     : m->atlasGen < m->atlas->layoutGen ? "atlas laid out anew" : "other";
    }
    else if (d < sWait[k].closest)
        sWait[k].closest = d;
}

static float sFogNear = 40.0f, sFogFar = 90.0f;

/* Gamma's cloud-shadow textures, loaded once (the lighting can be switched on at any time). */
static void LoadGammaClouds(void)
{

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

/*
 * The Gamma lighting switch (Options: "Gamma Lighting"; WJPC_3D_GAMMALIGHT overrides). Its sun is not the
 * plain 3D's, and the sun is baked into the ground, so every map is laid again.
 */
static bool sGammaWanted;
void Voxel3D_SetGammaLight(bool on)
{
    if (on == sGammaWanted && (gVoxelGammaLight != 0) == on)
        return;
    sGammaWanted = on;
    gVoxelGammaLight = on ? 1 : 0;
    if (on)
        VoxelLighting_SetSun(-0.2718f, -1.1604f, 1.0f, 0x6A33u); /* Gamma's sun: 40 degrees up, behind, a little right */
    else
        VoxelLighting_SetSun(0.85f, 0.55f, 1.0f, 0);
    VoxelLighting_Reset();
    for (int i = 0; i < MAX_INSTANCES; i++)
        if (sInstanceMeshes[i].used)
            InstanceFree(&sInstanceMeshes[i]);
    BeltCache_Flush();
    sBeltX = 0x7FFFFFFF;
    sBeltResetGen++;
}

static void BattleZoomBlur(int w, int h);
// For sdl2.c's stall log (WallysJourney_stalls.txt, POL-11/14/15): where the last 3D frame's time went, and how
// much is loaded.
double gVoxel3DMapMs, gVoxel3DBeltMs, gVoxel3DDrawMs;
unsigned gVoxel3DMeshes, gVoxel3DPages, gVoxel3DBeltLayers, gVoxel3DBeltFlushes;
static Uint64 sStallDrawStart;

/*
 * POL-23 (Brennan: "There's no sandstorm in the desert"): rain, sand and ash drawn in the 3D world, ported from
 * Emerald 3DS (ctr_voxel.c DrawWeather / MakeWeather, the weather kinds, and voxel_weather.v.pica). Particles sit
 * at fixed places in a box round the camera; each frame the box's contents move by one offset and the shader wraps
 * them back in, fading them at the box's sides, near the lens and into the haze. Two or three groups per weather,
 * each with its own speed, wind and size. How many are drawn follows the game's weather as it comes and goes
 * (VoxelWorld_WeatherAmount). The sand's dust sheets (3DS DrawBanks): DrawSandBanks (POL-38).
 * Our haze is distance-based (the world's own fog rule here), 3DS's is depth-based.
 */
#define VOXEL_STREAKS 768
#define VOXEL_FLAKES 512
#define VOXEL_STREAK_VERTICES 6
#define VOXEL_FLAKE_VERTICES 12
#define VOXEL_WEATHER_GROUPS 3
#define VOXEL_FLAKE_RIM 0.35f
#define VOXEL_WEATHER_SOUTH 1.0f
#define VOXEL_WEATHER_EDGE 8.0f
#define VOXEL_WEATHER_EDGE_ALONG 24.0f
#define VOXEL_WEATHER_TOP 6.0f
#define VOXEL_WEATHER_LENS 1.5f
#define VOXEL_WEATHER_LENS_RAMP 1.6f

typedef struct
{
    float velocity[3]; // tiles per second
    float width;       // a streak's width, or a flake's size
    float length;      // a streak's length; 0: a flake
    float alpha;       // x the weather's own
} WeatherGroup;

typedef struct
{
    float box[3];    // tiles: across, up, north to south
    float low;       // the box's bottom, tiles over the player's ground
    uint32_t colour; // 0xAABBGGRR
    float alpha;
    WeatherGroup groups[VOXEL_WEATHER_GROUPS];
} WeatherKind;

// Values exactly as Emerald 3DS's.
static const WeatherKind sRainKind = {
    {24.0f, 8.5f, 28.0f}, -3.0f, 0xFFF4E8DEu, 0.62f,
    {
        {{-1.6f, -13.0f, 0.5f}, 0.034f, 0.95f, 1.00f},
        {{-1.3f, -11.0f, 0.4f}, 0.028f, 0.75f, 0.85f},
        {{-0.9f, -8.5f, 0.3f}, 0.020f, 0.45f, 0.60f},
    },
};
static const WeatherKind sSandKind = {
    {24.0f, 5.0f, 28.0f}, -1.5f, 0xFF7EB8E6u, 0.55f,
    {
        {{-13.0f, -0.6f, 1.0f}, 0.030f, 1.10f, 1.00f},
        {{-17.0f, -0.3f, 0.6f}, 0.024f, 1.60f, 0.75f},
        {{-9.5f, 0.4f, 1.6f}, 0.040f, 0.60f, 0.85f},
    },
};
static const WeatherKind sAshKind = {
    {24.0f, 8.0f, 28.0f}, -2.0f, 0xFFE2E6EAu, 1.00f,
    {
        {{-0.50f, -0.75f, 0.15f}, 0.120f, 0.0f, 1.00f},
        {{-0.25f, -1.00f, -0.12f}, 0.100f, 0.0f, 0.95f},
        {{-0.80f, -0.55f, 0.30f}, 0.145f, 0.0f, 0.85f},
    },
};

static float MoteRandom(unsigned i, unsigned salt)
{
    uint32_t h = i * 2654435761u ^ salt * 2246822519u;

    h = (h ^ (h >> 15)) * 2246822519u;
    h ^= h >> 13;
    return (float)(h & 0xFFFFu) / 65535.0f;
}

typedef struct
{
    float place[4];  // x, y, z in the box as fractions, alpha
    float corner[2]; // across, along (3DS: in 1/64)
} WeatherVertex;

static GLuint sWeatherProg, sWeatherVbo[2];
static GLint sWUniProj, sWUniView, sWUniOffset, sWUniSize, sWUniBase, sWUniAcross, sWUniAlong, sWUniColour, sWUniStrength,
    sWUniFog, sWUniCam, sWUniFade;
static bool sWeatherTried;

static void WeatherCorner(WeatherVertex *v, int across, int along, const float place[3], float alpha)
{
    v->place[0] = place[0];
    v->place[1] = place[1];
    v->place[2] = place[2];
    v->place[3] = alpha;
    v->corner[0] = across / 64.0f;
    v->corner[1] = along / 64.0f;
}

static bool MakeWeather(void)
{
    static const char *vs =
        "#version 120\n"
        "uniform mat4 proj, view;\n"
        "uniform vec4 boxOffset, boxSize, boxBase, across, along;\n"
        "uniform vec2 fog;\n"
        "uniform vec3 cam;\n"
        "attribute vec4 place;\n"
        "attribute vec2 corner;\n"
        "varying float vAlpha;\n"
        "void main() {\n"
        "  vec3 p = fract(place.xyz + boxOffset.xyz);\n"
        "  vec3 side = 0.5 - abs(p - 0.5);\n"                               // 0 at a side, 0.5 in the middle
        "  float edge = min(min(boxSize.w * side.x, boxOffset.w * side.z), boxBase.w * side.y);\n"
        "  edge = min(1.0, edge);\n"
        "  vec3 w = boxBase.xyz + p * boxSize.xyz + across.xyz * corner.x + along.xyz * corner.y;\n"
        "  vec4 v = view * vec4(w, 1.0);\n"
        "  gl_Position = proj * v;\n"
        "  float lens = clamp((-v.z - across.w) * along.w, 0.0, 1.0);\n"
        "  float h = clamp((length(w - cam) - fog.x) / max(fog.y - fog.x, 0.001), 0.0, 1.0);\n"
        "  vAlpha = place.w * edge * lens * (1.0 - h * h);\n"
        "}\n";
    static const char *fs =
        "#version 120\n"
        "uniform vec3 colour;\n"
        "uniform float strength;\n"
        "uniform vec4 fade;\n"
        "varying float vAlpha;\n"
        "void main() { gl_FragColor = vec4(mix(colour, fade.rgb, fade.a), clamp(vAlpha * strength, 0.0, 1.0)); }\n";
    WeatherVertex *streaks, *flakes;
    GLuint v, f;
    GLint ok = 0;

    sWeatherTried = true;
    v = Compile(GL_VERTEX_SHADER, vs);
    f = Compile(GL_FRAGMENT_SHADER, fs);
    if (v == 0 || f == 0)
        return false;
    sWeatherProg = wj_glCreateProgram();
    wj_glAttachShader(sWeatherProg, v);
    wj_glAttachShader(sWeatherProg, f);
    wj_glBindAttribLocation(sWeatherProg, 0, "place");
    wj_glBindAttribLocation(sWeatherProg, 1, "corner");
    wj_glLinkProgram(sWeatherProg);
    wj_glGetProgramiv(sWeatherProg, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        Voxel3D_Log("[3D] weather program does not link");
        sWeatherProg = 0;
        return false;
    }
    sWUniProj = wj_glGetUniformLocation(sWeatherProg, "proj");
    sWUniView = wj_glGetUniformLocation(sWeatherProg, "view");
    sWUniOffset = wj_glGetUniformLocation(sWeatherProg, "boxOffset");
    sWUniSize = wj_glGetUniformLocation(sWeatherProg, "boxSize");
    sWUniBase = wj_glGetUniformLocation(sWeatherProg, "boxBase");
    sWUniAcross = wj_glGetUniformLocation(sWeatherProg, "across");
    sWUniAlong = wj_glGetUniformLocation(sWeatherProg, "along");
    sWUniColour = wj_glGetUniformLocation(sWeatherProg, "colour");
    sWUniStrength = wj_glGetUniformLocation(sWeatherProg, "strength");
    sWUniFog = wj_glGetUniformLocation(sWeatherProg, "fog");
    sWUniCam = wj_glGetUniformLocation(sWeatherProg, "cam");
    sWUniFade = wj_glGetUniformLocation(sWeatherProg, "fade");

    streaks = malloc(VOXEL_STREAKS * VOXEL_STREAK_VERTICES * sizeof(WeatherVertex));
    flakes = malloc(VOXEL_FLAKES * VOXEL_FLAKE_VERTICES * sizeof(WeatherVertex));
    if (streaks == NULL || flakes == NULL)
    {
        free(streaks);
        free(flakes);
        sWeatherProg = 0;
        return false;
    }
    // MakeWeather (3DS): a streak runs from its bright head back up its fall to a gone tail.
    for (unsigned i = 0; i < VOXEL_STREAKS; ++i)
    {
        WeatherVertex *q = streaks + i * VOXEL_STREAK_VERTICES;
        float place[3] = {MoteRandom(i, 21), MoteRandom(i, 22), MoteRandom(i, 23)};
        int half = 44 + (int)(MoteRandom(i, 24) * 20.0f);
        int tail = 38 + (int)(MoteRandom(i, 25) * 26.0f);
        float alpha = 0.55f + 0.45f * MoteRandom(i, 26);

        WeatherCorner(&q[0], -half, 0, place, alpha);
        WeatherCorner(&q[1], half, 0, place, alpha);
        WeatherCorner(&q[2], half, tail, place, 0.0f);
        q[3] = q[0];
        q[4] = q[2];
        WeatherCorner(&q[5], -half, tail, place, 0.0f);
    }
    // A flake is a fan of four triangles, bright in the middle, fainter at its corners.
    for (unsigned i = 0; i < VOXEL_FLAKES; ++i)
    {
        static const int fan[4][2][2] = {
            {{0, 1}, {1, 0}}, {{1, 0}, {0, -1}}, {{0, -1}, {-1, 0}}, {{-1, 0}, {0, 1}},
        };
        WeatherVertex *q = flakes + i * VOXEL_FLAKE_VERTICES;
        float place[3] = {MoteRandom(i, 31), MoteRandom(i, 32), MoteRandom(i, 33)};
        int size = 40 + (int)(MoteRandom(i, 34) * 24.0f);
        float alpha = 0.70f + 0.30f * MoteRandom(i, 35), rim = alpha * VOXEL_FLAKE_RIM;

        for (unsigned k = 0; k < 4; ++k)
        {
            WeatherCorner(&q[k * 3], 0, 0, place, alpha);
            WeatherCorner(&q[k * 3 + 1], fan[k][0][0] * size, fan[k][0][1] * size, place, rim);
            WeatherCorner(&q[k * 3 + 2], fan[k][1][0] * size, fan[k][1][1] * size, place, rim);
        }
    }
    wj_glGenBuffers(2, sWeatherVbo);
    wj_glBindBuffer(GL_ARRAY_BUFFER, sWeatherVbo[0]);
    wj_glBufferData(GL_ARRAY_BUFFER, VOXEL_STREAKS * VOXEL_STREAK_VERTICES * sizeof(WeatherVertex), streaks, GL_STATIC_DRAW);
    wj_glBindBuffer(GL_ARRAY_BUFFER, sWeatherVbo[1]);
    wj_glBufferData(GL_ARRAY_BUFFER, VOXEL_FLAKES * VOXEL_FLAKE_VERTICES * sizeof(WeatherVertex), flakes, GL_STATIC_DRAW);
    free(streaks);
    free(flakes);
    return true;
}

static float Normalise3(float v[3])
{
    float length = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

    if (length > 0.0001f)
        for (int i = 0; i < 3; ++i)
            v[i] /= length;
    return length;
}

static float WeatherOffset(double seconds, float velocity, float corner, float size)
{
    double f = (seconds * velocity - corner) / size;

    return (float)(f - floor(f));
}

static void DrawWeather(const float *proj, const float *view, const float fade[4])
{
    VoxelWeatherClass weather = VoxelWorld_Weather();
    float amount = VoxelWorld_WeatherAmount();
    const WeatherKind *kind;
    bool flakes;
    unsigned count, per, group;
    double seconds;
    float corner[3], forward[3], flash, alpha;
    uint32_t colour;

    if (weather != VOXEL_WEATHER_RAIN && weather != VOXEL_WEATHER_SAND && weather != VOXEL_WEATHER_ASH)
        return;
    if (amount <= 0.0f || getenv("WJPC_3D_NO_WEATHER") != NULL)
        return;
    if (!sWeatherTried)
        MakeWeather();
    if (sWeatherProg == 0)
        return;
    kind = weather == VOXEL_WEATHER_RAIN ? &sRainKind : weather == VOXEL_WEATHER_SAND ? &sSandKind : &sAshKind;
    flakes = weather == VOXEL_WEATHER_ASH;
    count = flakes ? VOXEL_FLAKES : VOXEL_STREAKS;
    per = flakes ? VOXEL_FLAKE_VERTICES : VOXEL_STREAK_VERTICES;
    group = count / VOXEL_WEATHER_GROUPS;
    seconds = fmod((double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency(), 86400.0);
    corner[0] = sCamera.targetX - kind->box[0] * 0.5f;
    corner[1] = sCamera.ground + kind->low;
    corner[2] = sCamera.z - VOXEL_WEATHER_SOUTH - kind->box[2];
    forward[0] = sCamera.targetX - sCamera.x;
    forward[1] = sCamera.targetY - sCamera.y;
    forward[2] = sCamera.targetZ - sCamera.z;
    Normalise3(forward);
    flash = weather == VOXEL_WEATHER_RAIN ? VoxelWorld_Lightning() : 0.0f;
    alpha = kind->alpha * (1.0f + 0.5f * flash);
    colour = flash > 0.0f ? 0xFFFFFFFFu : kind->colour;

    wj_glUseProgram(sWeatherProg);
    wj_glUniformMatrix4fv(sWUniProj, 1, GL_FALSE, proj);
    wj_glUniformMatrix4fv(sWUniView, 1, GL_FALSE, view);
    wj_glUniform4f(sWUniSize, kind->box[0], kind->box[1], kind->box[2], VOXEL_WEATHER_EDGE);
    wj_glUniform4f(sWUniBase, corner[0], corner[1], corner[2], VOXEL_WEATHER_TOP);
    wj_glUniform2f(sWUniFog, sFogNear, sFogFar);
    wj_glUniform3f(sWUniCam, sCamera.x, sCamera.y, sCamera.z);
    wj_glUniform4f(sWUniFade, fade[0], fade[1], fade[2], fade[3]);
    {
        /* POL-38: with Gamma lighting the scene is linear light the screen pass multiplies by the exposure;
         * the 3DS's colour put in as it is came out near white (the sand's streaks read as rain) */
        float c[3] = {(colour & 0xFF) / 255.0f, ((colour >> 8) & 0xFF) / 255.0f, ((colour >> 16) & 0xFF) / 255.0f};

        if (gVoxelGammaLight)
            for (int k = 0; k < 3; ++k)
                c[k] = (c[k] <= 0.04045f ? c[k] / 12.92f : powf((c[k] + 0.055f) / 1.055f, 2.4f)) / sExposure;
        wj_glUniform3f(sWUniColour, c[0], c[1], c[2]);
    }
    wj_glBindBuffer(GL_ARRAY_BUFFER, sWeatherVbo[flakes ? 1 : 0]);
    wj_glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(WeatherVertex), (void *)0);
    wj_glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(WeatherVertex), (void *)(4 * sizeof(float)));
    wj_glEnableVertexAttribArray(0);
    wj_glEnableVertexAttribArray(1);
    wj_glDisableVertexAttribArray(2);
    wj_glEnable(GL_BLEND);
    wj_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    wj_glEnable(GL_DEPTH_TEST);
    wj_glDepthMask(GL_FALSE); // tested against the finished world, never written
    wj_glDisable(GL_CULL_FACE);
    for (unsigned g = 0; g < VOXEL_WEATHER_GROUPS; ++g)
    {
        const WeatherGroup *wg = &kind->groups[g];
        unsigned shown = (unsigned)(group * amount + 0.5f);
        float strength = alpha * wg->alpha;
        float across[3], along[3];

        if (shown == 0 || strength <= 0.0f)
            continue;
        if (shown > group)
            shown = group;
        if (wg->length > 0.0f)
        {
            // back up the fall, and across it as the camera sees it
            float back[3] = {-wg->velocity[0], -wg->velocity[1], -wg->velocity[2]};

            Normalise3(back);
            across[0] = back[1] * forward[2] - back[2] * forward[1];
            across[1] = back[2] * forward[0] - back[0] * forward[2];
            across[2] = back[0] * forward[1] - back[1] * forward[0];
            Normalise3(across);
            for (int c = 0; c < 3; ++c)
            {
                across[c] *= wg->width;
                along[c] = back[c] * wg->length;
            }
        }
        else
        {
            // turned to the camera: the view's right and up rows (column-major: m[0], m[4], m[8] ...)
            across[0] = view[0] * wg->width;
            across[1] = view[4] * wg->width;
            across[2] = view[8] * wg->width;
            along[0] = view[1] * wg->width;
            along[1] = view[5] * wg->width;
            along[2] = view[9] * wg->width;
        }
        wj_glUniform4f(sWUniAcross, across[0], across[1], across[2], VOXEL_WEATHER_LENS);
        wj_glUniform4f(sWUniAlong, along[0], along[1], along[2], 1.0f / VOXEL_WEATHER_LENS_RAMP);
        wj_glUniform4f(sWUniOffset, WeatherOffset(seconds, wg->velocity[0], corner[0], kind->box[0]),
                       WeatherOffset(seconds, wg->velocity[1], corner[1], kind->box[1]),
                       WeatherOffset(seconds, wg->velocity[2], corner[2], kind->box[2]), VOXEL_WEATHER_EDGE_ALONG);
        wj_glUniform1f(sWUniStrength, strength > 1.0f ? 1.0f : strength);
        wj_glDrawArrays(GL_TRIANGLES, (GLint)(g * group * per), (GLsizei)(shown * per));
    }
    wj_glDepthMask(GL_TRUE);
    wj_glDisable(GL_BLEND);
    wj_glDisableVertexAttribArray(1);
    wj_glUseProgram(sProgram);
}

/*
 * POL-38 (Brennan 2026-11-16: "sandstorm north of mauville looks weird not like a sandstorm"): the rest of Emerald
 * 3DS's sandstorm - its dust. ctr_voxel.c DrawBanks with sSandBanks: two horizontal sheets round the camera,
 * 0.35 and 1.30 tiles over the player's ground, textured with soft tileable noise (MakeFog: four octaves of
 * DappleNoise, thresholded so clear gaps open between the wisps), swept along by the wind (5.2 and 7.8 tiles a
 * second west), fading out towards the sheet's rim; depth tested, never written - a wall or a rock rising
 * through the dust parts it. Colour 0xFF88BEE0 (ABGR), strength 0.50 / 0.30 x the weather's amount. Values
 * exactly as Emerald 3DS's.
 */
#define VOXEL_FOG_DIM 128
#define VOXEL_FOG_GRID 8
#define VOXEL_FOG_REACH 32.0f
#define VOXEL_FOG_FADE 0.55f
#define VOXEL_FOG_SHEET_VERTICES (VOXEL_FOG_GRID * VOXEL_FOG_GRID * 6)

typedef struct { float period, height, driftX, driftZ, strength; } FogBank;
static const FogBank sSandBanks[2] = {
    {12.0f, 0.35f, -5.20f, 0.55f, 0.50f},
    {7.0f, 1.30f, -7.80f, 0.25f, 0.30f},
};
static GLuint sBankProg, sBankVbo, sBankTex;
static GLint sBUProj, sBUView, sBUOffset, sBUStrength, sBUColour, sBUFog, sBUCam, sBUFade, sBUTex;
static bool sBankTried;

static float DappleLattice(int x, int y, int period, uint32_t seed)
{
    uint32_t h = (uint32_t)(x % period) * 374761393u + (uint32_t)(y % period) * 668265263u + seed * 2246822519u;

    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return (float)(h & 0xFFFFu) / 65535.0f;
}

static float Quintic(float t)
{
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static float DappleNoise(float u, float v, int period, uint32_t seed)
{
    int x = (int)u, y = (int)v;
    float fx = Quintic(u - (float)x), fy = Quintic(v - (float)y);
    float a = DappleLattice(x, y, period, seed), b = DappleLattice(x + 1, y, period, seed);
    float c = DappleLattice(x, y + 1, period, seed), d = DappleLattice(x + 1, y + 1, period, seed);
    float top = a + (b - a) * fx, bottom = c + (d - c) * fx;

    return top + (bottom - top) * fy;
}

static bool MakeBanks(void)
{
    static const char *vs =
        "#version 120\n"
        "uniform mat4 proj, view;\n"
        "uniform vec3 offset;\n"
        "uniform vec2 fog;\n"
        "uniform vec3 cam;\n"
        "attribute vec3 pos;\n"
        "attribute vec2 uv;\n"
        "attribute float shade;\n"
        "varying vec2 vUv;\n"
        "varying float vAlpha;\n"
        "void main() {\n"
        "  vec3 w = pos + offset;\n"
        "  gl_Position = proj * (view * vec4(w, 1.0));\n"
        "  float h = clamp((length(w - cam) - fog.x) / max(fog.y - fog.x, 0.001), 0.0, 1.0);\n"
        "  vUv = uv;\n"
        "  vAlpha = shade * (1.0 - h * h);\n"
        "}\n";
    static const char *fs =
        "#version 120\n"
        "uniform sampler2D tex;\n"
        "uniform vec3 colour;\n"
        "uniform float strength;\n"
        "uniform vec4 fade;\n"
        "varying vec2 vUv;\n"
        "varying float vAlpha;\n"
        "void main() { gl_FragColor = vec4(mix(colour, fade.rgb, fade.a),\n"
        "  clamp(texture2D(tex, vUv).a * vAlpha * strength, 0.0, 1.0)); }\n";
    static const struct { int period; float weight; } octaves[] = {{4, 1.0f}, {8, 0.55f}, {16, 0.30f}, {32, 0.12f}};
    GLuint v, f;
    GLint ok = 0;
    uint8_t *texels;
    VoxelVertex *quad;

    sBankTried = true;
    v = Compile(GL_VERTEX_SHADER, vs);
    f = Compile(GL_FRAGMENT_SHADER, fs);
    if (v == 0 || f == 0)
        return false;
    sBankProg = wj_glCreateProgram();
    wj_glAttachShader(sBankProg, v);
    wj_glAttachShader(sBankProg, f);
    wj_glBindAttribLocation(sBankProg, 0, "pos");
    wj_glBindAttribLocation(sBankProg, 1, "uv");
    wj_glBindAttribLocation(sBankProg, 2, "shade");
    wj_glLinkProgram(sBankProg);
    wj_glGetProgramiv(sBankProg, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        Voxel3D_Log("[3D] sand dust program does not link");
        sBankProg = 0;
        return false;
    }
    sBUProj = wj_glGetUniformLocation(sBankProg, "proj");
    sBUView = wj_glGetUniformLocation(sBankProg, "view");
    sBUOffset = wj_glGetUniformLocation(sBankProg, "offset");
    sBUStrength = wj_glGetUniformLocation(sBankProg, "strength");
    sBUColour = wj_glGetUniformLocation(sBankProg, "colour");
    sBUFog = wj_glGetUniformLocation(sBankProg, "fog");
    sBUCam = wj_glGetUniformLocation(sBankProg, "cam");
    sBUFade = wj_glGetUniformLocation(sBankProg, "fade");
    sBUTex = wj_glGetUniformLocation(sBankProg, "tex");

    // MakeFog (3DS): wisps rather than a wash
    texels = malloc(VOXEL_FOG_DIM * VOXEL_FOG_DIM * 4);
    quad = malloc(2 * VOXEL_FOG_SHEET_VERTICES * sizeof(VoxelVertex));
    if (texels == NULL || quad == NULL)
    {
        free(texels);
        free(quad);
        sBankProg = 0;
        return false;
    }
    for (unsigned y = 0; y < VOXEL_FOG_DIM; ++y)
        for (unsigned x = 0; x < VOXEL_FOG_DIM; ++x)
        {
            float n = 0.0f, total = 0.0f, t;

            for (unsigned o = 0; o < sizeof(octaves) / sizeof(octaves[0]); ++o)
            {
                float scale = (float)octaves[o].period / VOXEL_FOG_DIM;

                n += octaves[o].weight * DappleNoise((float)x * scale, (float)y * scale, octaves[o].period, o + 11);
                total += octaves[o].weight;
            }
            t = (n / total - 0.32f) / 0.40f;
            t = t < 0.0f ? 0.0f : t > 1.0f ? 1.0f : t;
            uint8_t *px = texels + 4 * (y * VOXEL_FOG_DIM + x);
            px[0] = px[1] = px[2] = 255;
            px[3] = (uint8_t)(t * t * (3.0f - 2.0f * t) * 255.0f + 0.5f);
        }
    wj_glGenTextures(1, &sBankTex);
    wj_glBindTexture(GL_TEXTURE_2D, sBankTex);
    /* Brennan's crash, 16f, entering Route 111's sandstorm: SDL leaves its own GL_UNPACK_ROW_LENGTH set (its
     * picture's pitch), so the driver read this 128-wide texture as rows that long, past the end of `texels` */
    wj_glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    wj_glPixelStorei(0x0CF2 /* GL_UNPACK_ROW_LENGTH */, 0);
    wj_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, VOXEL_FOG_DIM, VOXEL_FOG_DIM, 0, GL_RGBA, GL_UNSIGNED_BYTE, texels);
    wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    free(texels);
    // the sheets: a grid round their own centre, the pattern in world tiles, coverage falling to the rim
    {
        float cell = 2.0f * VOXEL_FOG_REACH / VOXEL_FOG_GRID;
        unsigned n = 0;

        for (unsigned bank = 0; bank < 2; ++bank)
            for (unsigned gz = 0; gz < VOXEL_FOG_GRID; ++gz)
                for (unsigned gx = 0; gx < VOXEL_FOG_GRID; ++gx)
                {
                    static const unsigned corners[6][2] = {{0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1}};

                    for (unsigned k = 0; k < 6; ++k)
                    {
                        float x = -VOXEL_FOG_REACH + (float)(gx + corners[k][0]) * cell;
                        float z = -VOXEL_FOG_REACH + (float)(gz + corners[k][1]) * cell;
                        float r = sqrtf(x * x + z * z) / VOXEL_FOG_REACH;
                        float edge = (1.0f - r) / (1.0f - VOXEL_FOG_FADE);

                        edge = edge < 0.0f ? 0.0f : edge > 1.0f ? 1.0f : edge;
                        quad[n].x = x;
                        quad[n].y = 0.0f;
                        quad[n].z = z;
                        quad[n].u = x / sSandBanks[bank].period;
                        quad[n].v = z / sSandBanks[bank].period;
                        quad[n].shade = edge * edge * (3.0f - 2.0f * edge);
                        ++n;
                    }
                }
        wj_glGenBuffers(1, &sBankVbo);
        wj_glBindBuffer(GL_ARRAY_BUFFER, sBankVbo);
        wj_glBufferData(GL_ARRAY_BUFFER, n * sizeof(VoxelVertex), quad, GL_STATIC_DRAW);
    }
    free(quad);
    return true;
}

static void DrawSandBanks(const float *proj, const float *view, const float fade[4])
{
    float amount = VoxelWorld_WeatherAmount();
    const uint32_t colour = 0xFF88BEE0u; /* 0xAABBGGRR */
    double seconds;

    if (VoxelWorld_Weather() != VOXEL_WEATHER_SAND || amount <= 0.0f || getenv("WJPC_3D_NO_WEATHER") != NULL
     || getenv("WJPC_NO_SAND_DUST") != NULL)
        return;
    if (!sBankTried)
        MakeBanks();
    if (sBankProg == 0)
        return;
    seconds = fmod((double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency(), 86400.0);
    wj_glUseProgram(sBankProg);
    wj_glUniformMatrix4fv(sBUProj, 1, GL_FALSE, proj);
    wj_glUniformMatrix4fv(sBUView, 1, GL_FALSE, view);
    wj_glUniform2f(sBUFog, sFogNear, sFogFar);
    wj_glUniform3f(sBUCam, sCamera.x, sCamera.y, sCamera.z);
    wj_glUniform4f(sBUFade, fade[0], fade[1], fade[2], fade[3]);
    {
        /* 3DS: coverage = texture x shade (its colour carries the shade halved and the stage doubles it back).
         * With Gamma lighting the scene is linear light that the screen pass multiplies by the exposure: the
         * dust's colour goes in as the light that comes out as that colour. */
        float c[3] = {(colour & 0xFF) / 255.0f, ((colour >> 8) & 0xFF) / 255.0f, ((colour >> 16) & 0xFF) / 255.0f};

        if (gVoxelGammaLight)
            for (int k = 0; k < 3; ++k)
                c[k] = (c[k] <= 0.04045f ? c[k] / 12.92f : powf((c[k] + 0.055f) / 1.055f, 2.4f)) / sExposure;
        wj_glUniform3f(sBUColour, c[0], c[1], c[2]);
    }
    wj_glUniform1i(sBUTex, 0);
    wj_glActiveTexture(GL_TEXTURE0);
    wj_glBindTexture(GL_TEXTURE_2D, sBankTex);
    wj_glBindBuffer(GL_ARRAY_BUFFER, sBankVbo);
    wj_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void *)offsetof(VoxelVertex, x));
    wj_glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void *)offsetof(VoxelVertex, u));
    wj_glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void *)offsetof(VoxelVertex, shade));
    wj_glEnableVertexAttribArray(0);
    wj_glEnableVertexAttribArray(1);
    wj_glEnableVertexAttribArray(2);
    wj_glEnable(GL_BLEND);
    wj_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    wj_glEnable(GL_DEPTH_TEST);
    wj_glDepthMask(GL_FALSE);
    wj_glDisable(GL_CULL_FACE);
    for (unsigned bank = 0; bank < 2; ++bank)
    {
        const FogBank *b = &sSandBanks[bank];
        float dx = (float)fmod(seconds * b->driftX, b->period);
        float dz = (float)fmod(seconds * b->driftZ, b->period);

        wj_glUniform3f(sBUOffset, floorf(sCamera.targetX / b->period) * b->period + dx, sCamera.ground + b->height,
                       floorf(sCamera.targetZ / b->period) * b->period + dz);
        wj_glUniform1f(sBUStrength, b->strength * amount);
        wj_glDrawArrays(GL_TRIANGLES, (GLint)(bank * VOXEL_FOG_SHEET_VERTICES), VOXEL_FOG_SHEET_VERTICES);
    }
    wj_glDepthMask(GL_TRUE);
    wj_glDisable(GL_BLEND);
    wj_glUseProgram(sProgram);
}

/*
 * Audit step 5 (Brennan 2026-11-21 02:12, 240 fps): the world drawn from what the last game frame's update left
 * (meshes, belt, people), so it can be drawn again on the refreshes between game frames (Voxel3D_Redraw) with the
 * camera and the people part of the way to where they are going. `in` holds what the update read from the game
 * (its backdrop colour, time-of-day tint, screen fade): read on a full frame, kept for the redraws - the game runs
 * on its own thread meanwhile, so a redraw reads nothing of it.
 */
typedef struct
{
    const VoxelMapInstance *inst0;
    float playerX, playerZ;
    bool windowsLit;
    uint16_t backdrop;
    bool tintOk, fadeOk;
    float fadeAmount, fadeRgb[3];
} WorldDrawInputs;

static bool DrawWorld(unsigned colorTexture, int w, int h, SavedGl *savedP, WorldDrawInputs *in, bool redraw)
{
    const VoxelMapInstance *inst0 = in->inst0;
    float playerX = in->playerX, playerZ = in->playerZ;
    bool windowsLit = in->windowsLit;
    float proj[16], view[16], mvp[16];
    float fadeAmount = 0.0f, fadeRgb[3] = {0, 0, 0};
    bool fieldFadeInShader = false;
    uint16_t backdrop;

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
        RestoreGl(savedP);
        return false;
    }

    if (gVoxelGammaLight && !PostBeginScene(w, h))
        Voxel3D_SetGammaLight(false);
    wj_glViewport(0, 0, w, h);
    wj_glDepthMask(GL_TRUE);
    if (getenv("WJPC_VOXEL_LOG") != NULL && (sFrame % 120) == 1)
        Voxel3D_Log("[3D] saved viewport %d %d %d %d fbo %d", savedP->viewport[0], savedP->viewport[1], savedP->viewport[2], savedP->viewport[3], savedP->fbo);
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
    if (!redraw)
        in->backdrop = *(const uint16_t *)PLTT;
    backdrop = in->backdrop;
    {
        float r = (backdrop & 31) / 31.0f, g = ((backdrop >> 5) & 31) / 31.0f, b = ((backdrop >> 10) & 31) / 31.0f;
        if (!inst0->indoor && !inst0->underground)
        {
            // Outdoors: a sky colour, not the GBA backdrop (a cave keeps its backdrop, as indoors and the 3DS).
            r = 0.55f; g = 0.75f; b = 0.95f;
        }
        // POL-26b (Treston: 3D night, "Match our 2D"): the 2D game's time-of-day colour for the world this frame
        if (!redraw)
            in->tintOk = VoxelWorld_TimeTint(sTintMul, sTintAdd);
        if (sBattleMode || !in->tintOk)
        {
            sTintMul[0] = sTintMul[1] = sTintMul[2] = 1.0f;
            sTintAdd[0] = sTintAdd[1] = sTintAdd[2] = 0.0f;
        }
        if (!inst0->indoor && !inst0->underground)
        {
            r = r * sTintMul[0] + sTintAdd[0];
            g = g * sTintMul[1] + sTintAdd[1];
            b = b * sTintMul[2] + sTintAdd[2];
        }
        if (gVoxelGammaLight)
        {   /* backdrop in linear light, so the screen pass returns it unchanged in look */
            r = r * r / sExposure; g = g * g / sExposure; b = b * b / sExposure;
        }
        if (gVoxelFxOff & VOXEL_FX_GAPS)
            r = 1.0f, g = 0.0f, b = 1.0f; /* F7: whatever nothing is drawn on shows magenta */
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

    if (sBattle.on)
    {
        // Emerald 3DS's battle view is its 400x240 top screen with the GBA picture on it at 1.4x, the
        // picture's (120, 112) on (200, 192); this draws the part of it the GBA picture covers.
        float l = (200.0f + (0.0f - 120.0f) * CTR_BATTLE_ZOOM - 200.0f) / 200.0f;
        float r = (200.0f + (240.0f - 120.0f) * CTR_BATTLE_ZOOM - 200.0f) / 200.0f;
        float tp = (120.0f - (192.0f + (0.0f - 112.0f) * CTR_BATTLE_ZOOM)) / 120.0f;
        float bt = (120.0f - (192.0f + (160.0f - 112.0f) * CTR_BATTLE_ZOOM)) / 120.0f;
        Perspective(proj, sCamera.fov, (float)CTR_GAME_WIDTH / (float)CTR_GAME_HEIGHT, 0.5f, 400.0f);
        proj[0] *= 2.0f / (r - l);
        proj[8] += (r + l) / (r - l);
        proj[5] *= 2.0f / (tp - bt);
        proj[9] += (tp + bt) / (tp - bt);
    }
    else
    Perspective(proj, sCamera.fov, (float)w / (float)h, 0.5f, 400.0f);
    // SDL shows render targets as they are stored: draw upside down so it comes out right.
    proj[1] = -proj[1]; proj[5] = -proj[5]; proj[9] = -proj[9]; proj[13] = -proj[13];
    LookAt(view, sCamera.x, sCamera.y, sCamera.z, sCamera.targetX, sCamera.targetY, sCamera.targetZ);
    Multiply(mvp, proj, view);
    memcpy(sFieldMvp, mvp, sizeof(sFieldMvp));
    if (getenv("WJPC_DUMP_MVP"))
    {
        FILE *f = fopen(getenv("WJPC_DUMP_MVP"), "wb");
        if (f != NULL)
        {
            float extra[4] = {(float)sBeltShiftX, (float)sBeltShiftZ, (float)w, (float)h};
            fwrite(mvp, sizeof(float), 16, f);
            fwrite(extra, sizeof(float), 4, f);
            fclose(f);
        }
        /* (with it, where each shown map is drawn this frame: "<mvp file>.origins", lines "group num x z",
         * for tools/pc/gapscan/pick.py - a map's origin moves when the world is recentred) */
        {
            char name[256];
            snprintf(name, sizeof(name), "%s.origins", getenv("WJPC_DUMP_MVP"));
            f = fopen(name, "w");
            if (f != NULL)
            {
                for (int i = 0; i < MAX_INSTANCES; i++)
                    if (sInstanceMeshes[i].used && sInstanceMeshes[i].shown)
                        fprintf(f, "%d %d %d %d\n", sInstanceMeshes[i].mapGroup, sInstanceMeshes[i].mapNum,
                                sInstanceMeshes[i].originX, sInstanceMeshes[i].originY);
                fclose(f);
            }
        }
    }
    sHaveFieldMvp = !sBattleMode;
    wj_glUniformMatrix4fv(sUniMvp, 1, GL_FALSE, mvp);
    wj_glUniform3f(sUniCam, sCamera.x, sCamera.y, sCamera.z);
    /*
     * POL-16 (Brennan: "When walking into doors all npcs turn black and disappear then reappear"): the screen
     * fades of the field (doors, warps) now fade the 3D world too, as Emerald 3DS does (VoxelWorld_ScreenFade,
     * ctr_voxel.c PrepareFades: sWorldFade). Before, only the people faded (their colours come from the faded
     * palettes) and the world stayed lit, so they turned to black figures. As on the 3DS the people get no fade
     * of their own on top (sSpriteFade = none): their palettes already carry it (see the sprite pass below).
     * With Gamma lighting the fade is done in the screen pass (PostFinish), after the tone curve, as for battles:
     * done in the world shader the curve squashed it (a lit room stayed bright half the fade) and the people's
     * haze stayed blue at black. The people's pass then undoes their palette fade first (`unfade`), so the
     * screen pass fades them once, like the world.
     */
    if (!redraw)
        in->fadeOk = VoxelWorld_ScreenFade(&in->fadeAmount, in->fadeRgb);
    fadeAmount = in->fadeAmount;
    fadeRgb[0] = in->fadeRgb[0];
    fadeRgb[1] = in->fadeRgb[1];
    fadeRgb[2] = in->fadeRgb[2];
    fieldFadeInShader = in->fadeOk && !gVoxelGammaLight;
    wj_glUniform3f(sUniTintMul, sTintMul[0], sTintMul[1], sTintMul[2]);
    wj_glUniform3f(sUniTintAdd, sTintAdd[0], sTintAdd[1], sTintAdd[2]);
    wj_glUniform3f(sUniHazeMul, sTintMul[0] + sTintAdd[0], sTintMul[1] + sTintAdd[1], sTintMul[2] + sTintAdd[2]);
    // the lit window colour in the Gamma path's linear light: c * c / exposure comes out of the screen pass as
    // c (as the backdrop above)
    wj_glUniform3f(sUniGlowLin, 1.0f / sExposure, 0.847f * 0.847f / sExposure, 0.627f * 0.627f / sExposure);
    if (fieldFadeInShader)
        wj_glUniform4f(sUniFade, fadeRgb[0], fadeRgb[1], fadeRgb[2], fadeAmount);
    else
        wj_glUniform4f(sUniFade, 0, 0, 0, 0);
    sIndoorNow = inst0->indoor;
    // Haze, outdoors only: starts well past the player, the sky's colour.
    if (inst0->indoor || inst0->underground)
        wj_glUniform2f(sUniFog, 1.0e6f, 2.0e6f);
    else
        wj_glUniform2f(sUniFog, sFogNear, sFogFar);
    wj_glUniform3f(sUniFogColor, 0.55f * sTintMul[0] + sTintAdd[0], 0.75f * sTintMul[1] + sTintAdd[1],
                   0.95f * sTintMul[2] + sTintAdd[2]); // POL-26b: the sky's colour at this time of day
    GammaLightUniforms();
    // Gamma's cloud shadows are its sun's light function: none where the sun does not reach (indoors, caves)
    wj_glUniform1f(wj_glGetUniformLocation(sProgram, "cloudsOn"), inst0->indoor || inst0->underground || getenv("WJPC_3D_NOCLOUDS") != NULL || (gVoxelFxOff & VOXEL_FX_CLOUDS) ? 0.0f : 1.0f);   /* WJPC_3D_NOCLOUDS: test aid */
    {
        int sx, sy;
        VoxelWorld_SteadyOrigin(&sx, &sy);
        // (wrapped well inside a float's whole numbers; the pattern repeats every 5000/64*... tiles anyway)
        wj_glUniform2f(wj_glGetUniformLocation(sProgram, "steady"), (float)(sx % 100000), (float)(sy % 100000));
    }

    // Ground, trees and models of every map.
    for (int i = 0; i < MAX_INSTANCES; i++)
    {
        InstanceMesh *m = &sInstanceMeshes[i];
        if (!m->used || !m->shown)
            continue;
        static int sSkipEnv = -1;
        if (sSkipEnv < 0)
            sSkipEnv = getenv("WJPC_3D_SKIP") ? atoi(getenv("WJPC_3D_SKIP")) : 0; /* debug: 1 terrain 2 trees 4 models 8 belt */
        int skip = sSkipEnv | ((gVoxelFxOff & VOXEL_FX_GROUND) ? 1 : 0) | ((gVoxelFxOff & VOXEL_FX_TREES) ? 2 : 0)
                 | ((gVoxelFxOff & VOXEL_FX_MODELS) ? 4 : 0); /* F8, F9, F10 */
        for (unsigned p = 0; p < VOXEL_ATLAS_PAGES && !(skip & 1); p++)
            MeshDraw(&m->terrain[p], m->atlas->tex[p], (float)m->originX, 0, (float)m->originY);
        if (!(skip & 2))
            MeshDraw(&m->trees, sTreeTex, (float)m->originX, 0, (float)m->originY);
        if (m->buildingPage >= 0 && !(skip & 4))
        {
            if (inst0->underground && m->buildingPage < 2048 && !sPageCave[m->buildingPage] && !CavePagesOneGrade())
            {
                sPageCave[m->buildingPage] = true;
                if (sPageTex[m->buildingPage] != 0)
                    LoadBuildingPage(m->buildingPage, true);
            }
            GLuint page = BuildingPage(m->buildingPage);
            if (getenv("WJPC_VOXEL_LOG") != NULL && (sFrame % 120) == 1)
                Voxel3D_Log("[3D]   map %d:%d buildings page %d tex %u, %u chunk verts, %u statics, first model %p", m->mapGroup, m->mapNum, m->buildingPage, page, m->buildings.count, m->staticCount, m->staticCount ? (const void *)StaticModel(m->statics[0].pageModel, m->statics[0].lit) : NULL);
            GLuint glow = windowsLit ? GlowPage(m->buildingPage) : 0; // POL-26b
            if (glow != 0)
            {
                wj_glActiveTexture(GL_TEXTURE0 + GLOW_UNIT);
                wj_glBindTexture(GL_TEXTURE_2D, glow);
                wj_glActiveTexture(GL_TEXTURE0);
                wj_glUniform1f(sUniGlowStrength, 1.0f);
            }
            GLuint kindTex[FACE_KINDS] = { page, page, page };
            /* (#29, only under the sky: the split is for winter snow on roofs. Brennan 2026-11-24 07:36: "Caves
             * shouldn't be turning white in winter. Only stuff outside should be as caves are inside" - a cave's
             * rocks and crystals, and a house's table, chairs, sink and TV, came out with white tops in winter;
             * the 2D game gives inside maps no season. 07:42: "Any place inside should not have white tops when
             * it's winter" - so only maps under the sky (MapHasNaturalLight's types): not indoors, caves, secret
             * bases, underwater. WJPC_INSIDE_MODELS_SPLIT: split there too, as before) */
            static int sInsideSplit = -1;
            if (sInsideSplit < 0)
                sInsideSplit = getenv("WJPC_INSIDE_MODELS_SPLIT") != NULL;
            bool split = !WinterWallsOld() && sPageTexRaw[m->buildingPage] != 0 && sPageTexRoof[m->buildingPage] != 0
                      && (inst0->sky || sInsideSplit);
            if (split)
            {
                kindTex[FACE_TOP] = sPageTexRoof[m->buildingPage];
                kindTex[FACE_WALL] = sPageTexRaw[m->buildingPage];
            }
            if (split)
                for (int kind = 0; kind < FACE_KINDS; kind++)
                    MeshDraw(&m->buildingKind[kind], kindTex[kind], (float)m->originX, 0, (float)m->originY);
            else
                MeshDraw(&m->buildings, page, (float)m->originX, 0, (float)m->originY);
            for (unsigned s = 0; s < m->staticCount; s++)
            {
                const VoxelStaticDraw *d = &m->statics[s];
                const StaticMesh *sm = StaticModelParts(d->pageModel, d->lit);
                if (sm == NULL)
                    continue;
                {
                    /* #29: a cave's models lit as its floor (WJPC_CAVE_MODELS_NORMAL_LIT: by their facing) */
                    static int sNormalLit = -1;
                    if (sNormalLit < 0)
                        sNormalLit = getenv("WJPC_CAVE_MODELS_NORMAL_LIT") != NULL;
                    wj_glUniform1f(sUniFlatLight, inst0->flatModels && !sNormalLit ? 1.0f : 0.0f);
                }
                if (split)
                    for (int kind = 0; kind < FACE_KINDS; kind++)
                        MeshDraw(&sm->kind[kind], kindTex[kind], d->x + (float)m->originX, d->y, d->z + (float)m->originY);
                else
                    MeshDraw(&sm->mesh, page, d->x + (float)m->originX, d->y, d->z + (float)m->originY);
            }
            wj_glUniform1f(sUniFlatLight, 0.0f);
            if (glow != 0)
                wj_glUniform1f(sUniGlowStrength, 0.0f);
        }
    }
    if (sBeltAtlas != NULL && !inst0->indoor && !inst0->underground && !(getenv("WJPC_3D_SKIP") && (atoi(getenv("WJPC_3D_SKIP")) & 8))
     && !(gVoxelFxOff & VOXEL_FX_RING)) /* F12 */
    {
        for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
            MeshDraw(&sBeltTerrain[0][p], sBeltAtlas->tex[p], (float)sBeltShiftX, 0, (float)sBeltShiftZ);
        // (POL-30: the other layers, each with the atlas it was built with - not one made again since)
        for (unsigned l = 1; l < BELT_LAYERS; l++)
            if (sBeltDrawn[l] != NULL && sBeltDrawn[l]->valid && sBeltDrawn[l]->layoutGen <= sBeltDrawnGen[l])
                for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
                    MeshDraw(&sBeltTerrain[l][p], sBeltDrawn[l]->tex[p], (float)sBeltShiftX, 0, (float)sBeltShiftZ);
        if (getenv("WJPC_DUMP_MVP")) /* (test aid: each belt layer's state this frame, "<mvp>.belt": layer, its mesh's
                                      * vertices per page, whether drawn and why not) */
        {
            char name[256];
            FILE *bf;
            snprintf(name, sizeof(name), "%s.belt", getenv("WJPC_DUMP_MVP"));
            if ((bf = fopen(name, "w")) != NULL)
            {
                for (unsigned l = 0; l < BELT_LAYERS; l++)
                {
                    unsigned n = 0;
                    for (unsigned p = 0; p < VOXEL_ATLAS_PAGES; p++)
                        n += sBeltTerrain[l][p].count;
                    fprintf(bf, "layer %u verts %u atlas %s valid %d layoutGen %u drawnGen %u\n", l, n,
                            l == 0 ? (sBeltAtlas ? "yes" : "no") : (sBeltDrawn[l] ? "yes" : "no"),
                            l == 0 ? 1 : (sBeltDrawn[l] ? sBeltDrawn[l]->valid : -1),
                            l == 0 ? 0u : (sBeltDrawn[l] ? (unsigned)sBeltDrawn[l]->layoutGen : 0u),
                            (unsigned)sBeltDrawnGen[l]);
                }
                fclose(bf);
            }
        }
        MeshDraw(&sBeltTrees, sTreeTex, (float)sBeltShiftX, 0, (float)sBeltShiftZ);
    }
    // POL-48: Ninja Boys hidden as a tree, as the 3D small tree (voxel_entities.c noted their cells this frame)
    {
        static Mesh sDisguiseMesh;
        static VoxelVertex sDisguiseVerts[16 * 12];
        const int (*cells)[2];
        unsigned n = VoxelEntities_DisguiseTrees(&cells);
        VoxelBuilder b;

        VoxelBuilder_Init(&b, sDisguiseVerts, (unsigned)(sizeof sDisguiseVerts / sizeof sDisguiseVerts[0]));
        VoxelBuilder_SetOrigin(&b, 0, 0);
        for (unsigned i = 0; i < n; i++)
            VoxelTree_EmitDisguise(&b, cells[i][0], cells[i][1]);
        if (b.count > 0)
        {
            MeshSetQuantised(&sDisguiseMesh, sDisguiseVerts, b.count, GL_DYNAMIC_DRAW);
            MeshDraw(&sDisguiseMesh, sTreeTex, 0, 0, 0);
        }
    }

    // Shadows and reflections under the cards, then the cards.
    // As on the 3DS: blended, without writing depth.
    wj_glEnable(GL_BLEND);
    wj_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    wj_glDepthMask(GL_FALSE);
    if (!sBattleMode)
    {
        // POL-16: the people's palettes are faded already (3DS sSpriteFade = none)
        wj_glUniform4f(sUniFade, 0, 0, 0, 0);
        // POL-26b: and carry the time of day already (the game tints sprite palettes too)
        wj_glUniform3f(sUniTintMul, 1, 1, 1);
        wj_glUniform3f(sUniTintAdd, 0, 0, 0);
        if (gVoxelGammaLight && fadeAmount > 0.0f)
            wj_glUniform4f(sUniUnfade, fadeRgb[0], fadeRgb[1], fadeRgb[2], fadeAmount); // the screen pass fades them
    }
    wj_glUniform1f(sUniMode, 2.0f);
    if (getenv("WJPC_3D_NOREFLECT") == NULL)
        MeshDraw(&sReflectMesh, sSpriteTex, 0, 0, 0);
    wj_glUniform1f(sUniMode, 1.0f);
    if (getenv("WJPC_3D_NOSHADOW") == NULL)
        MeshDraw(&sShadowMesh, sSpriteTex, 0, 0, 0);
    wj_glDepthMask(GL_TRUE);
    wj_glDisable(GL_BLEND);
    wj_glUniform1f(sUniMode, 0.0f);
    {
        int chunky = getenv("WJPC_3D_CHUNKY") ? atoi(getenv("WJPC_3D_CHUNKY")) : 0;
        if (chunky > 0)
        {
            /* Slices from the back to the front, half a sprite pixel apart, along the camera's horizontal
             * direction (cards face the camera). Sprite pixel = 1/16 tile. Back slices a little darker so
             * the sides read. */
            float dx = sCamera.x - sCamera.targetX, dz = sCamera.z - sCamera.targetZ;
            float len = sqrtf(dx * dx + dz * dz);
            dx /= len > 0.0f ? len : 1.0f;
            dz /= len > 0.0f ? len : 1.0f;
            wj_glUniform2f(sUniAtlasTexel, 1.0f / VOXEL_SPRITE_ATLAS_W, 1.0f / VOXEL_SPRITE_ATLAS_H);
            for (int k = -8; k <= 8; k++)
            {
                float d = k * 0.5f; /* sprite pixels */
                wj_glUniform3f(sUniChunky, (float)chunky, d, 0.0f);
                MeshDraw(&sSpriteMesh, sSpriteTex, dx * d / 16.0f, 0, dz * d / 16.0f);
            }
            wj_glUniform3f(sUniChunky, 0.0f, 0.0f, 0.0f);
        }
        else
            MeshDraw(&sSpriteMesh, sSpriteTex, 0, 0, 0);
    }
    if (!sBattleMode && (sPoolMesh.count != 0 || sGlowMesh.count != 0 || sGlassMesh.count != 0))
    {
        // POL-26b: the lamps - pools on the ground first (3DS: 0.5 x glow x 255,150,60; glow 1 while the game
        // shows its lights), then the night-light cards over them, both blended, depth tested, not written
        wj_glEnable(GL_BLEND);
        wj_glDepthMask(GL_FALSE);
        if (sPoolMesh.count != 0 && sPoolTex != 0)
        {
            wj_glBlendFunc(GL_DST_COLOR, GL_ONE);
            wj_glUniform1f(sUniMode, 4.0f);
            wj_glUniform4f(sUniLamp, 0.5f, 0.5f * 150.0f / 255.0f, 0.5f * 60.0f / 255.0f, 1.0f / sExposure);
            MeshDraw(&sPoolMesh, sPoolTex, 0, 0.02f, 0); // just over the ground it lies on
        }
        if (sGlassMesh.count != 0)
        {   // the 3DS's lit glass (LampTexEnv): 255,191,107 at alpha 220/255 over the lantern
            wj_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            wj_glUniform1f(sUniMode, 5.0f);
            wj_glUniform4f(sUniLamp, 1.0f, 191.0f / 255.0f, 107.0f / 255.0f, 1.0f / sExposure);
            MeshDraw(&sGlassMesh, sPoolTex, 0, 0, 0);
        }
        if (sGlowMesh.count != 0 && !gVoxelGammaLight) // Gamma lighting: over the finished picture, below
        {
            wj_glBlendFunc(GL_ONE, GL_SRC_ALPHA);
            wj_glUniform1f(sUniMode, 3.0f);
            wj_glUniform4f(sUniLamp, 0, 0, 0, 1.0f / sExposure);
            MeshDraw(&sGlowMesh, sSpriteTex, 0, 0, 0);
        }
        wj_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        wj_glUniform1f(sUniMode, 0.0f);
        wj_glDepthMask(GL_TRUE);
        wj_glDisable(GL_BLEND);
    }
    wj_glUniform4f(sUniUnfade, 0, 0, 0, 0);

    if (!sBattleMode && !inst0->indoor)
    {
        // POL-23: rain, sand and ash over the finished world (3DS: DrawWeather after the world)
        float fd[4] = {fadeRgb[0], fadeRgb[1], fadeRgb[2], fieldFadeInShader ? fadeAmount : 0.0f};
        DrawSandBanks(proj, view, fd); // POL-38: the dust under the streaks, as 3DS draws its banks first
        DrawWeather(proj, view, fd);
    }
    if (gVoxelGammaLight)
    {
        float fd[4] = {fadeRgb[0], fadeRgb[1], fadeRgb[2], fieldFadeInShader ? 0.0f : fadeAmount};
        float dx = sCamera.x - sCamera.targetX, dy = sCamera.y - sCamera.targetY, dz = sCamera.z - sCamera.targetZ;
        MsaaResolve();
        PostFinish(sFbo, w, h, fd, sqrtf(dx * dx + dy * dy + dz * dz));
        if (!sBattleMode && sGlowMesh.count != 0 && sHdrDepth != 0)
        {
            /*
             * POL-26b (Treston, 2026-11-15: "look at how the 2D looks"): with Gamma lighting the night-light
             * glows go over the finished picture, as the GBA blends them over its finished picture (scene x
             * 12/16 + glow x 7/16). Added to the scene's linear light before the tone curve, the glow swamped
             * the night's dark colours and washed the lamp's head out. The scene's own depth (sHdrDepth) is put
             * on the output for it, so what stands in front of a lamp still hides its glow.
             */
            wj_glBindFramebuffer(GL_FRAMEBUFFER, sFbo);
            wj_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sHdrDepth, 0);
            wj_glViewport(0, 0, w, h);
            wj_glUseProgram(sProgram);
            wj_glEnableVertexAttribArray(0);
            wj_glEnableVertexAttribArray(1);
            wj_glEnableVertexAttribArray(2);
            wj_glActiveTexture(GL_TEXTURE0);
            wj_glEnable(GL_DEPTH_TEST);
            wj_glDepthFunc(GL_LEQUAL);
            wj_glDepthMask(GL_FALSE);
            wj_glEnable(GL_BLEND);
            wj_glBlendFunc(GL_ONE, GL_SRC_ALPHA);
            wj_glUniform1f(sUniGamma, 0.0f);
            wj_glUniform1f(sUniMode, 3.0f);
            wj_glUniform4f(sUniUnfade, 0, 0, 0, 0);
            wj_glUniform3f(sUniTintMul, 1, 1, 1);
            wj_glUniform3f(sUniTintAdd, 0, 0, 0);
            MeshDraw(&sGlowMesh, sSpriteTex, 0, 0, 0);
            wj_glUniform1f(sUniMode, 0.0f);
            wj_glUniform1f(sUniGamma, 1.0f);
            wj_glDisable(GL_BLEND);
            wj_glDepthMask(GL_TRUE);
            wj_glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sDepth);
        }
    }
    if (sBattle.on)
        BattleZoomBlur(w, h);

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
    if (sStallDrawStart != 0)
        gVoxel3DDrawMs = (SDL_GetPerformanceCounter() - sStallDrawStart) * 1000.0 / SDL_GetPerformanceFrequency();
    RestoreGl(savedP);
    return true;
}

static WorldDrawInputs sLastDraw;
static bool sLastDrawValid;
/* step 5: the camera before and after the last update (a redraw eases from one to the other); the update alone,
 * without drawing, while the refreshes draw (Voxel3D_SetUpdateOnly) */
static VoxelCamera sCamPrev, sCamCur;
static bool sUpdateOnly;
static VoxelVertex *sInterpVerts; // copies of the people's vertices, moved for a redraw

void Voxel3D_SetUpdateOnly(bool on)
{
    sUpdateOnly = on;
}

/* (Brennan 2026-11-23 21:46, 4x fast forward jitter: sdl2.c sDrawSpan) the camera and the people are eased over that
 * many game frames: what counts as a jump not eased (a warp, a crossing's shift) grows with it */
static unsigned sDrawSpan = 1;

void Voxel3D_SetDrawSpan(unsigned frames)
{
    sDrawSpan = frames < 1 ? 1 : frames > 24 ? 24 : frames;
    VoxelEntities_SetDrawSpan(sDrawSpan);
}

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
    bool fieldFadeInShader = false;
    // POL-26b: windows lit at the 2D game's times (as the 3DS: the full mask, glow strength 1 at night)
    bool windowsLit = !sBattleMode && VoxelWorld_WindowsLit();

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
    VoxelCamera camStart = sCamera;   // (step 5: where the camera was before this game frame)
    bool camStartValid = sCameraValid;

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
    sAtlasLiveDigest = digest; // POL-27: AtlasCovers checks the map underfoot again when its grid changes
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
            sBeltResetGen++;
    sBeltResetGen++;
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
        /* Brennan 2026-11-20 20:15 (stall log: "belt" up to 1680 ms in one frame walking in and out of towns): maps
         * coming into reach or going out of it rebuilt the whole belt in that frame. Now its pieces again a few a
         * frame, the old belt up meanwhile (as at a crossing). WJPC_BELT_FULL_ON_WORLD=1: as before. */
        if (sBeltX != 0x7FFFFFFF && getenv("WJPC_BELT_FULL_ON_WORLD") == NULL)
        {
            BeltCache_Flush();
            sBeltRedo = true;
        }
        else
            sBeltX = 0x7FFFFFFF;
            sBeltResetGen++;
    sBeltResetGen++;
    }
    sLastDigest = digest;

    SaveGl(&saved);
    Uint64 workStart = SDL_GetPerformanceCounter();
    sStallDrawStart = 0;
    // POL-30: the belt also draws the borders of the maps next door (VoxelWorld_BorderMetatile). An atlas
    // made before such a map was loaded may lack its border's metatiles: made again then, as AtlasCovers
    // does for a map's own cells. Looked at once per atlas build and set of maps.
    {
        Atlas *atlas0 = (inst0->indoor || inst0->underground) ? NULL : FindAtlas(inst0);

        static const Atlas *seenAtlas;
        static uint32_t seenGen, seenKey;
        uint32_t key = 0;
        int ids[4];

        for (unsigned i = 0; i < count; i++)
            key = key * 31u + (uint32_t)(VoxelWorld_Instance(i)->mapGroup << 8 | VoxelWorld_Instance(i)->mapNum) + 1u;
        if (atlas0 != NULL && (seenAtlas != atlas0 || seenGen != atlas0->gen || seenKey != key))
        {
            bool lacks = false;

            for (unsigned i = 1; i < count && !lacks; i++)
                if (VoxelWorld_BeltBorderOf(i, ids))
                    for (int t = 0; t < 4; t++)
                        if (ids[t] < VOXEL_METATILE_REAL && atlas0->map.slotOf[ids[t]] == 0)
                        {
                            Voxel3D_Log("[3D] atlas lacks border metatile %x of %d:%d: made again", ids[t],
                                        VoxelWorld_Instance(i)->mapGroup, VoxelWorld_Instance(i)->mapNum);
                            lacks = true;
                            break;
                        }
            for (unsigned i = 1; i < count && !lacks; i++)
                for (unsigned k = 0; VoxelWorld_BeltFillOf(i, k, &ids[0]); k++)
                    if (ids[0] >= 0 && ids[0] < VOXEL_METATILE_REAL && atlas0->map.slotOf[ids[0]] == 0)
                    {
                        Voxel3D_Log("[3D] atlas lacks edge fill metatile %x of %d:%d: made again", ids[0],
                                    VoxelWorld_Instance(i)->mapGroup, VoxelWorld_Instance(i)->mapNum);
                        lacks = true;
                        break;
                    }
            seenAtlas = atlas0;
            seenGen = atlas0->gen;
            seenKey = key;
            if (lacks && sBorderRemadeKey != key)
            {
                sBorderRemadeKey = key; // once per set of maps: never a loop if a build leaves it out
                atlas0->grow = true;    // (POL-15: grown, not thrown away)
            }
        }
    }

    // Maps (DLX-18). The player's own is built at once if it has to be; the others in the background, a few
    // ms a frame, nearest first, old meshes shown meanwhile. While the screen is dark (a warp's fade) the
    // budget is a frame's worth. Meshes of maps that left the view are kept for when they come back.
    {
        Uint64 freq = SDL_GetPerformanceFrequency();
        float dark = 0.0f, rgb[3];
        bool fadedOut = VoxelWorld_ScreenFade(&dark, rgb) && dark >= 0.5f;
        double budget = fadedOut ? 40.0 : 3.0;
        /* Brennan 2026-11-21 02:12 (audit step 3, docs/polish_reports/PERF_AUDIT_2026-11-21.md; his 72 ms "maps" on
         * Route 121 was its atlas growing, then the map underfoot laid again, both at once): the map underfoot that
         * needs laying again while it still has a mesh that draws keeps that mesh on screen and is laid again in
         * the background, before any other map (its atlas grown over frames too). Laid at once only with nothing
         * of it to draw, or behind a warp's fade. WJPC_UNDERFOOT_NOW=1: always at once, as before. */
        static int sUnderfootNow = -1;
        if (sUnderfootNow < 0)
            sUnderfootNow = getenv("WJPC_UNDERFOOT_NOW") != NULL;
        Uint64 deadline = SDL_GetPerformanceCounter() + (Uint64)(budget * freq / 1000.0);
        const VoxelMapInstance *backInst = NULL;
        bool asked = false;
        bool threaded = WorkerOn();
        bool workerBusy = threaded && WorkerBusy();
        int backIndex = -1;

        if (getenv("WJPC_3D_BUILD_MS") != NULL)
            deadline = SDL_GetPerformanceCounter() + (Uint64)(atof(getenv("WJPC_3D_BUILD_MS")) * freq / 1000.0);
        /* Brennan 2026-11-23 21:03 (#28: "going from route to city the right or left side of the screen will be blue
         * sky for a frame or two then the rest of the map loads"): not reproduced on the test machine (11 crossings
         * pictured every frame, none). So the stall log says it when it happens: a map drawn on one frame that is not
         * drawn on the next while it is still one of the world's maps, and why (MapHiddenNote below). */
        bool wasShown[MAX_INSTANCES];
        for (int i = 0; i < MAX_INSTANCES; i++)
        {
            wasShown[i] = sInstanceMeshes[i].used && sInstanceMeshes[i].shown;
            sInstanceMeshes[i].shown = false;
        }
        /* #28 (Brennan 2026-11-23 21:03: "the area running into was taking too long to load instead of
         * instantly"): the maps not built yet are built one at a time, and the order was the order of the current
         * map's connections in its header: after a warp to Route 102's west end, Oldale (20+ tiles away) was built
         * before Petalburg (7 tiles away), which then showed after the stall log's "MAP LATE" frames. Now the map
         * underfoot first, then the others nearest the player first. WJPC_BUILD_ORDER_OLD=1: the header's order. */
        unsigned order[MAX_VOXEL_MAP_INSTANCES];
        {
            static int sOrderOld = -1;
            float gap[MAX_VOXEL_MAP_INSTANCES], px, pz;
            if (sOrderOld < 0)
                sOrderOld = getenv("WJPC_BUILD_ORDER_OLD") != NULL;
            VoxelWorld_GetPlayerWorldCoords(&px, &pz);
            for (unsigned i = 0; i < count && i < MAX_VOXEL_MAP_INSTANCES; i++)
            {
                const VoxelMapInstance *in = VoxelWorld_Instance(i);
                float dx = px < in->originX ? in->originX - px : px > in->originX + in->width ? px - in->originX - in->width : 0;
                float dz = pz < in->originY ? in->originY - pz : pz > in->originY + in->height ? pz - in->originY - in->height : 0;
                unsigned j = i;
                gap[i] = i == 0 ? -1.0f : dx > dz ? dx : dz;
                order[i] = i;
                while (!sOrderOld && j > 1 && gap[order[j - 1]] > gap[i])
                {
                    order[j] = order[j - 1];
                    j--;
                }
                order[j] = i;
            }
        }
        for (unsigned oi = 0; oi < count; oi++)
        {
            unsigned i = order[oi];
            const VoxelMapInstance *inst = VoxelWorld_Instance(i);
            InstanceMesh *m = MeshOf(inst->mapGroup, inst->mapNum);
            uint32_t sig = VoxelBuildings_LiveSignature(inst);
            bool keepOld = i == 0 && !sUnderfootNow && !fadedOut && m != NULL && MeshAtlasOk(m);
            Atlas *atlas = i == 0 && !keepOld ? GetAtlas(inst) : FindAtlas(inst);
            bool stale = m == NULL || m->signature != sig || m->lift != inst->lift || atlas == NULL
                      || m->atlas != atlas || !MeshAtlasOk(m)
                      || m->atlasGen != atlas->gen || atlas->grow; // (POL-15: laid again once it has grown)

            if (BuildIsFor(&sBackBuild, inst))
            {
                if (sBackBuild.signature != sig || (atlas != NULL && sBackBuild.atlas != atlas) || !sBackBuild.atlas->valid)
                    sBackBuild.active = false; // what it was building for has changed
                else
                    backInst = inst;
            }
            if (i == 0 && stale && keepOld && backInst != inst)
                sBackBuild.active = false; // (the map underfoot comes first: another map's build waits)
            /* Audit step 4d (Brennan 2026-11-21 02:12): behind a warp's fade the map underfoot, with nothing of it to
             * draw yet, is built on the worker while the screen is dark (nothing drawn of it meanwhile); once the
             * fade lifts, a build of it still on the worker is waited for (not begun again on this thread).
             * WJPC_UNDERFOOT_ON_WORKER_OFF=1: built here at once, as before. */
            static int sUnderfootWorkerOff = -1;
            if (sUnderfootWorkerOff < 0)
                sUnderfootWorkerOff = getenv("WJPC_UNDERFOOT_ON_WORKER_OFF") != NULL;
            if (i == 0 && stale && atlas != NULL && !keepOld && threaded && !sUnderfootWorkerOff
             && (backInst == inst || (fadedOut && !workerBusy)))
            {
                if (backInst != inst)
                {
                    BuildBegin(&sBackBuild, inst, atlas, sig);
                    backInst = inst;
                    WorkerStart(i);
                    workerBusy = true;
                    asked = true;
                }
                if (!fadedOut && workerBusy)
                {
                    Uint64 w0 = SDL_GetPerformanceCounter();
                    while (WorkerBusy())
                        SDL_Delay(0);
                    workerBusy = false;
                    Voxel3D_Log("[3D] waited %.1f ms for the worker's build of %d:%d (the fade lifted)",
                                (SDL_GetPerformanceCounter() - w0) * 1000.0 / SDL_GetPerformanceFrequency(),
                                inst->mapGroup, inst->mapNum);
                }
            }
            else if (i == 0 && stale && atlas != NULL && !keepOld)
            {
                // Underfoot: now. A background build of it is finished rather than begun again (on the main
                // thread; one on the worker is left to it and thrown away).
                InstanceBuild *pb = backInst == inst && !threaded ? &sBackBuild : &sNowBuild;

                if (threaded && backInst == inst)
                {
                    sBackBuild.active = false;
                    backInst = NULL;
                }

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
            else if (stale && !sBackBuild.active && !asked && !workerBusy)
            {
                // The first one in need: its tile sheet (over frames), then its mesh.
                asked = true;
                atlas = PrepareAtlas(inst, deadline);
                if (atlas != NULL)
                {
                    BuildBegin(&sBackBuild, inst, atlas, sig);
                    backInst = inst;
                    if (threaded)
                    {
                        WorkerStart(i);
                        workerBusy = true;
                    }
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
        for (unsigned i = 0; i < count; i++)
        {
            const VoxelMapInstance *inst = VoxelWorld_Instance(i);
            InstanceMesh *m = MeshOf(inst->mapGroup, inst->mapNum);
            int k = m != NULL ? (int)(m - sInstanceMeshes) : -1;
            if (k >= 0 && wasShown[k] && !m->shown)
                MapHiddenNote(m, inst);
            if (i > 0 && !inst->indoor)
                MapLateNote(inst, m);
        }
        if (backInst == NULL)
            sBackBuild.active = false; // its map left the view or moved: dropped (the worker's, when it is done)
        else if (threaded ? !workerBusy : BuildSteps(&sBackBuild, backInst, deadline))
        {
            InstanceMesh *m = MeshSlotFor(backInst->mapGroup, backInst->mapNum);
            if (m != NULL)
                BuildFinish(&sBackBuild, backInst, m);
            sBackBuild.active = false;
        }
    }

    if (!sBattleMode)
    {
        Uint64 anim0 = SDL_GetPerformanceCounter();
        if (getenv("WJPC_PROFILE_ANIM"))
            gVoxelProfileOn = 1;
        AnimateAtlasTiles(inst0); // POL-12
        gVoxelProfileOn = 0;
        double animMs = (SDL_GetPerformanceCounter() - anim0) * 1000.0 / SDL_GetPerformanceFrequency();
        if (animMs > 4.0)
            Voxel3D_Log("[3D] animated tiles took %.1f ms (frame %u): composing %.1f, uploading %.1f", animMs, sFrame,
                        sAnimComposeTicks * 1000.0 / SDL_GetPerformanceFrequency(),
                        sAnimUploadTicks * 1000.0 / SDL_GetPerformanceFrequency());
        sAnimComposeTicks = sAnimUploadTicks = 0;
    }
    Uint64 workMaps = SDL_GetPerformanceCounter();
    // Camera follows the player.
    VoxelWorld_GetPlayerWorldCoords(&playerX, &playerZ);
    VoxelEntities_GetPlayerWorldPos(&smoothX, &smoothZ);
    if (sBattleMode)
    {
        // Battle (Emerald 3DS): the camera kept from the field, the stage searched, then the glide.
        if (!sBattle.on)
        {
            memset(&sBattle, 0, sizeof(sBattle));
            sBattle.on = true;
            if (!sCameraValid)
            {
                // a battle on a map the field never drew: the field camera placed on the player
                VoxelCamera_SetGround(&sCamera, VoxelEntities_PlayerGround(), 1);
                VoxelCamera_Snap(&sCamera, playerX, playerZ);
                sCameraValid = true;
            }
            sBattle.field = sCamera;
        }
        if (!sBattle.chosen)
        {
            if (!sBattle.begun)
                VoxelBattle_BeginStage();
            sBattle.begun = true;
            sBattle.chosen = VoxelBattle_StepStage(VOXEL_BATTLE_STAGE_CELLS, VOXEL_BATTLE_STAGE_SPOTS,
                                                   &sBattle.targetX, &sBattle.targetZ, &sBattle.ground);
        }
        BattleCamera();
    }
    else if (sBattle.on)
    {
        sBattle.on = false;
        sCamera = sBattle.field;
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
    // POL-30 test aid: WJPC_BELT_TABLE=file - the belt round the player as cells (map-relative x y, metatile,
    // height), appended once the player has stood a second on a new spot, to compare one map's view of the
    // belt with the next map's (tools: docs/polish_reports/POL-30_belt_sweep.py).
    if (getenv("WJPC_BELT_TABLE") != NULL && !inst0->indoor && !inst0->underground)
    {
        static int lastKey = -1, still, dumped = -1;
        int px = (int)floorf(playerX), py = (int)floorf(playerZ), mt, h;
        int key = ((inst0->mapGroup << 8 | inst0->mapNum) << 16) ^ ((px - inst0->originX) << 8) ^ (py - inst0->originY);

        still = key == lastKey ? still + 1 : 0;
        lastKey = key;
        if (still == 60 && dumped != key)
        {
            FILE *f = fopen(getenv("WJPC_BELT_TABLE"), "a");
            dumped = key;
            if (f != NULL)
            {
                fprintf(f, "MAP %d %d %d %d\n", inst0->mapGroup, inst0->mapNum, px - inst0->originX, py - inst0->originY);
                for (int y = py - 36; y <= py + 36; y++)
                    for (int x = px - 36; x <= px + 36; x++)
                        if (VoxelWorld_GetInstanceAt(x, y) == NULL && VoxelMesh_BeltCell(x, y, &mt, &h))
                            fprintf(f, "%d %d %d %d\n", x - inst0->originX, y - inst0->originY, mt, h);
                fclose(f);
            }
        }
    }
    // The belt round the maps, rebuilt when the player has moved a chunk.
    {
        int cx = ((int)floorf(playerX) / CHUNK) * CHUNK, cy = ((int)floorf(playerZ) / CHUNK) * CHUNK;
        Atlas *atlas0 = GetAtlas(inst0);
        // (not in a cave either: the belt was a plateau of the cave's border tiles round it, walls and all)
        if (atlas0 != NULL && !inst0->indoor && !inst0->underground)
        {
            Uint64 t0 = SDL_GetPerformanceCounter();
            bool full = sBeltX == 0x7FFFFFFF || sBeltAtlas == NULL;

            // POL-30: the belt's other layers - the ready atlases of the loaded maps on other tile sets. A new
            // set of them: the pieces again, a few a frame (as into a map with other tile sets).
            if (!VoxelWorld_BeltOld())
            {
                Atlas *want[BELT_LAYERS] = {NULL};
                const void *pairs[BELT_LAYERS][2];
                unsigned n = 1;
                bool same;

                for (unsigned i = 1; i < count && n < BELT_LAYERS; i++)
                {
                    const VoxelMapInstance *m = VoxelWorld_Instance(i);
                    Atlas *a;
                    bool have = false;

                    if (m == NULL || m->layout == NULL || m->indoor || m->underground
                     || (m->primaryTileset == inst0->primaryTileset && m->secondaryTileset == inst0->secondaryTileset))
                        continue;
                    for (unsigned k = 1; k < n; k++)
                        have |= want[k]->primary == m->primaryTileset && want[k]->secondary == m->secondaryTileset;
                    if (have || (a = FindAtlas(m)) == NULL || a->tex[0] == 0)
                        continue;
                    pairs[n - 1][0] = a->primary;
                    pairs[n - 1][1] = a->secondary;
                    want[n++] = a;
                }
                same = n == sBeltLayers;
                for (unsigned k = 1; same && k < n; k++)
                    same = want[k] == sBeltLayer[k] && want[k]->gen == sBeltLayerGen[k];
                if (!same)
                {
                    for (unsigned k = 1; k < n; k++)
                    {
                        sBeltLayer[k] = want[k];
                        sBeltLayerGen[k] = want[k]->gen;
                    }
                    sBeltLayers = n;
                    VoxelWorld_SetBeltPairs((const void *const (*)[2])pairs, n - 1);
                    Voxel3D_Log("[3D] belt layers: %u", n);
                    if (!full)
                    {
                        BeltCache_Flush();
                        sBeltRedo = true;
                    }
                }
            }
            if (!full && (atlas0 != sBeltCacheAtlas || atlas0->gen != sBeltCacheGen))
            {
                // Into a map with other tile sets: its pieces again, a few a frame, the old belt up meanwhile.
                BeltCache_Flush();
                sBeltRedo = true;
            }
            sBeltCacheAtlas = atlas0;
            sBeltCacheGen = atlas0->gen;
            if (sBeltRedo && !full)
            {
                sBeltWantX = cx;
                sBeltWantY = cy;
                sBeltRedo = false;
            }
            static int sWarpAll = -1;
            if (sWarpAll < 0)
                sWarpAll = getenv("WJPC_BELT_FULL_ON_WARP") != NULL;
            if (full && !sWarpAll)
            {
                // New map, new world or new atlas: start again, a few ms a frame; no belt till it is whole
                if (!sBeltWarpPending || sBeltWarpGen != sBeltResetGen || (sBeltAtlas != NULL && !sBeltWarpShown))
                {
                    sBeltRadius = getenv("WJPC_BELT_RADIUS") ? atoi(getenv("WJPC_BELT_RADIUS"))
                                : (inst0->layoutId == 302 || inst0->layoutId == 303 || inst0->layoutId == 9) ? 64 : 56;
                    BeltCache_Flush();
                    for (unsigned l = 0; l < BELT_LAYERS; l++)
                        sBeltDrawn[l] = NULL;
                    sBeltAtlas = NULL;
                    sBeltWarpShown = false;
                    sBeltWarpPending = true;
                    sBeltWarpGen = sBeltResetGen;
                    sBeltWarpX = cx;
                    sBeltWarpY = cy;
                }
                {
                    static double sWarpMs = -1.0;
                    if (sWarpMs < 0.0)
                        sWarpMs = getenv("WJPC_BELT_WARP_MS") ? atof(getenv("WJPC_BELT_WARP_MS")) : 8.0;
                    sBeltBudgetOverrideMs = sWarpMs;
                }
                static unsigned sWarpFrames;
                bool ready = (++sWarpFrames % 3) != 0 && BeltBuildStep(inst0, atlas0, sBeltWarpX, sBeltWarpY, 1 << 20);
                sBeltBudgetOverrideMs = 0.0;
                if (!ready && sWarpFrames % 3 == 0)
                {
                    // every third frame, what is built so far is shown (nearest first): no sky round the map
                    BeltAssemble(atlas0, sBeltWarpX, sBeltWarpY);
                    sBeltShiftX = sBeltShiftZ = 0;
                    sBeltWarpShown = true;
                }
                if (ready)
                {
                    BeltAssemble(atlas0, sBeltWarpX, sBeltWarpY);
                    sBeltX = sBeltWarpX;
                    sBeltY = sBeltWarpY;
                    sBeltShiftX = sBeltShiftZ = 0;
                    sBeltRedo = false;
                    sBeltWantX = 0x7FFFFFFF;
                    sBeltWarpPending = false;
                }
            }
            else if (full)
            {
                // New map, new world or new atlas: start again and build it all now (as before).
                sBeltRadius = getenv("WJPC_BELT_RADIUS") ? atoi(getenv("WJPC_BELT_RADIUS"))
                            : (inst0->layoutId == 302 || inst0->layoutId == 303 || inst0->layoutId == 9) ? 64 : 56;
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
                Voxel3D_Log("[3D] belt %s %.1f ms, %u pieces (map layout %d)", full ? "full" : "step", ms, sBeltBuilt,
                            inst0->layoutId);
            sBeltBuilt = 0;
        }
    }

    {
        // Test aid: frames whose map and belt work took long (the hitches DLX-18 is about).
        double ms = (SDL_GetPerformanceCounter() - workStart) * 1000.0 / SDL_GetPerformanceFrequency();
        gVoxel3DMapMs = (workMaps - workStart) * 1000.0 / SDL_GetPerformanceFrequency();
        gVoxel3DBeltMs = ms - (workCam - workStart) * 1000.0 / SDL_GetPerformanceFrequency();
        gVoxel3DMeshes = 0;
        for (int i = 0; i < MAX_INSTANCES; i++)
            gVoxel3DMeshes += sInstanceMeshes[i].used;
        gVoxel3DBeltLayers = sBeltLayers;
        sStallDrawStart = SDL_GetPerformanceCounter();
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
        VoxelBuilder glows, pools, glass; // POL-26b
        bool lamps = sGlowVerts != NULL && getenv("WJPC_3D_FLAT_LAMPS") == NULL;
        if (lamps)
        {
            VoxelBuilder_Init(&glows, sGlowVerts, LAMP_VERTS);
            VoxelBuilder_SetOrigin(&glows, 0, 0);
            VoxelBuilder_Init(&pools, sPoolVerts, LAMP_VERTS);
            VoxelBuilder_SetOrigin(&pools, 0, 0);
            VoxelBuilder_Init(&glass, sGlassVerts, LAMP_VERTS);
            VoxelBuilder_SetOrigin(&glass, 0, 0);
            if (sPoolTex == 0)
            {   // Emerald 3DS MakeLampPool: 64 x 64, (1 - r^2)^2, linear filter, black at the rim
                static uint8_t px[64 * 64];
                for (int y = 0; y < 64; y++)
                    for (int x = 0; x < 64; x++)
                    {
                        float dx = ((float)x + 0.5f) / 32.0f - 1.0f, dy = ((float)y + 0.5f) / 32.0f - 1.0f;
                        float k = 1.0f - (dx * dx + dy * dy);
                        px[y * 64 + x] = k > 0.0f ? (uint8_t)(k * k * 255.0f + 0.5f) : 0;
                    }
                wj_glGenTextures(1, &sPoolTex);
                wj_glBindTexture(GL_TEXTURE_2D, sPoolTex);
                wj_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                wj_glPixelStorei(0x0CF2 /* GL_UNPACK_ROW_LENGTH */, 0);
                wj_glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 64, 64, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, px);
                wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
        }
        VoxelEntities_SetLampBuilders(lamps ? &glows : NULL, lamps ? &pools : NULL, lamps ? &glass : NULL);
        unsigned changed = 0;
        if (!sBattleMode) // in a battle the people are not there (the battle draws its own)
            changed = VoxelEntities_Emit(&sprites, sSpriteAtlas, &sCamera, &shadows, &reflections);
        VoxelEntities_SetLampBuilders(NULL, NULL, NULL);
        MeshSet(&sGlowMesh, sGlowVerts, lamps && !sBattleMode ? glows.count : 0, GL_STREAM_DRAW);
        MeshSet(&sPoolMesh, sPoolVerts, lamps && !sBattleMode ? pools.count : 0, GL_STREAM_DRAW);
        MeshSet(&sGlassMesh, sGlassVerts, lamps && !sBattleMode ? glass.count : 0, GL_STREAM_DRAW);
        if (lamps && getenv("WJPC_VOXEL_LOG") != NULL && (sFrame % 120) == 1)
            Voxel3D_Log("[3D] lamps: %u glow verts, %u pool verts", glows.count, pools.count);
        if (changed != 0 || sSpriteTex == 0) // 1024x512 now: not every frame when nothing in it changed
            sSpriteTex = UploadTiled(sSpriteTex, sSpriteAtlas, VOXEL_SPRITE_ATLAS_W, VOXEL_SPRITE_ATLAS_H);
        MeshSet(&sSpriteMesh, sSpriteVerts, sprites.count, GL_STREAM_DRAW);
        MeshSet(&sShadowMesh, sShadowVerts, shadows.count, GL_STREAM_DRAW);
        MeshSet(&sReflectMesh, sReflectVerts, reflections.count, GL_STREAM_DRAW);
    }

    sLastDraw.inst0 = inst0;
    sLastDraw.playerX = playerX;
    sLastDraw.playerZ = playerZ;
    sLastDraw.windowsLit = windowsLit;
    sLastDrawValid = true;
    sCamCur = sCamera;
    sCamPrev = camStartValid ? camStart : sCamera;
    float jump = sDrawSpan > 3 ? 0.5f * (float)sDrawSpan : 1.5f; // (a bike's game frame is at most a quarter tile)
    if (fabsf(sCamCur.x - sCamPrev.x) > jump || fabsf(sCamCur.z - sCamPrev.z) > jump
     || fabsf(sCamCur.y - sCamPrev.y) > jump || sCamCur.fov != sCamPrev.fov)
        sCamPrev = sCamCur; // (a crossing's shift, a snap, a warp: not eased)
    if (sUpdateOnly)
    {
        /* the refreshes draw it (Voxel3D_Redraw); read now what a redraw may not read from the game */
        sLastDraw.backdrop = *(const uint16_t *)PLTT;
        sLastDraw.tintOk = !sBattleMode && VoxelWorld_TimeTint(sTintMul, sTintAdd);
        sLastDraw.fadeOk = VoxelWorld_ScreenFade(&sLastDraw.fadeAmount, sLastDraw.fadeRgb);
        RestoreGl(&saved);
        return true;
    }
    return DrawWorld(colorTexture, w, h, &saved, &sLastDraw, false);
}

/*
 * Audit step 5 (Brennan 2026-11-21 02:12: a locked 240 fps; the 3D world was drawn once a game frame, 60 times a
 * second, the refreshes between showing the same picture): the world drawn again on a refresh between game frames,
 * `t` (0..1) of the way from the last game frame's predecessor to it - the camera eased between where it was and
 * where it is, every person and effect likewise (VoxelEntities_Interpolate). Nothing of the game is read (it runs
 * on its own thread meanwhile): only what the last update left. False if there is nothing to draw yet.
 */
bool Voxel3D_Redraw(unsigned colorTexture, int w, int h, float t)
{
    SavedGl saved;
    VoxelCamera keep = sCamera;
    float back;
    bool ok;

    if (!sInited || !sLastDrawValid)
        return false;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    back = 1.0f - t;
    SaveGl(&saved);
    {
        VoxelCamera c = sCamCur;
        c.x = sCamPrev.x + (sCamCur.x - sCamPrev.x) * t;
        c.y = sCamPrev.y + (sCamCur.y - sCamPrev.y) * t;
        c.z = sCamPrev.z + (sCamCur.z - sCamPrev.z) * t;
        c.targetX = sCamPrev.targetX + (sCamCur.targetX - sCamPrev.targetX) * t;
        c.targetY = sCamPrev.targetY + (sCamCur.targetY - sCamPrev.targetY) * t;
        c.targetZ = sCamPrev.targetZ + (sCamCur.targetZ - sCamPrev.targetZ) * t;
        sCamera = c;
    }
    if (sInterpVerts == NULL)
        sInterpVerts = malloc(3u * SPRITE_VERTS * sizeof(VoxelVertex));
    if (sInterpVerts != NULL && back > 0.0f)
    {
        VoxelVertex *sp = sInterpVerts, *sh = sInterpVerts + SPRITE_VERTS, *rf = sInterpVerts + 2u * SPRITE_VERTS;
        unsigned ns = sSpriteMesh.count, nh = sShadowMesh.count, nr = sReflectMesh.count;

        memcpy(sp, sSpriteVerts, ns * sizeof(VoxelVertex));
        memcpy(sh, sShadowVerts, nh * sizeof(VoxelVertex));
        memcpy(rf, sReflectVerts, nr * sizeof(VoxelVertex));
        VoxelEntities_Interpolate(sp, sh, rf, back);
        MeshSet(&sSpriteMesh, sp, ns, GL_STREAM_DRAW);
        MeshSet(&sShadowMesh, sh, nh, GL_STREAM_DRAW);
        MeshSet(&sReflectMesh, rf, nr, GL_STREAM_DRAW);
    }
    else
    {
        MeshSet(&sSpriteMesh, sSpriteVerts, sSpriteMesh.count, GL_STREAM_DRAW);
        MeshSet(&sShadowMesh, sShadowVerts, sShadowMesh.count, GL_STREAM_DRAW);
        MeshSet(&sReflectMesh, sReflectVerts, sReflectMesh.count, GL_STREAM_DRAW);
    }
    ok = DrawWorld(colorTexture, w, h, &saved, &sLastDraw, true);
    sCamera = keep;
    return ok;
}

/*
 * Emerald 3DS's BattleIntroBlur: while the camera travels, two copies of the world magnified about the
 * 3DS screen's middle (where the camera looks), see-through - 1 + 0.035 s at 0.50 s, 1 + 0.080 s at 0.32 s
 * (s = the intro's speed), linear filtered - over the world, under everything else.
 */
static GLuint sZoomProg, sZoomTex, sZoomVbo;
static int sZoomW, sZoomH;
static void BattleZoomBlur(int w, int h)
{
    Voxel3DBattleIntro intro;
    float s;
    Voxel3D_BattleIntro(&intro);
    s = intro.speed;
    if (s < 0.04f)
        return;
    if (sZoomProg == 0)
    {
        static const char *vs = "#version 120\nattribute vec2 p;\nvarying vec2 uv;\n"
                                "void main() { uv = p * 0.5 + 0.5; gl_Position = vec4(p, 0.0, 1.0); }\n";
        static const char *fs = "#version 120\nuniform sampler2D src;\nuniform vec2 centre;\nuniform float scale, alpha;\n"
                                "varying vec2 uv;\n"
                                "void main() { vec2 t = centre + (uv - centre) / scale;\n"
                                "  gl_FragColor = vec4(texture2D(src, t).rgb, alpha); }\n";
        static const float quad[8] = {-1, -1, 1, -1, -1, 1, 1, 1};
        GLuint v = Compile(GL_VERTEX_SHADER, vs), f = Compile(GL_FRAGMENT_SHADER, fs);
        GLint ok = 0;
        if (v == 0 || f == 0)
            return;
        sZoomProg = wj_glCreateProgram();
        wj_glAttachShader(sZoomProg, v);
        wj_glAttachShader(sZoomProg, f);
        wj_glBindAttribLocation(sZoomProg, 0, "p");
        wj_glLinkProgram(sZoomProg);
        wj_glGetProgramiv(sZoomProg, GL_LINK_STATUS, &ok);
        if (!ok)
        {
            sZoomProg = 0;
            return;
        }
        wj_glGenBuffers(1, &sZoomVbo);
        wj_glBindBuffer(GL_ARRAY_BUFFER, sZoomVbo);
        wj_glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    }
    if (sZoomTex == 0 || sZoomW != w || sZoomH != h)
    {
        if (sZoomTex == 0)
            wj_glGenTextures(1, &sZoomTex);
        wj_glBindTexture(GL_TEXTURE_2D, sZoomTex);
        wj_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        wj_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        sZoomW = w;
        sZoomH = h;
    }
    wj_glBindFramebuffer(GL_FRAMEBUFFER, sFbo);
    wj_glActiveTexture(GL_TEXTURE0);
    wj_glBindTexture(GL_TEXTURE_2D, sZoomTex);
    wj_glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, w, h);
    wj_glViewport(0, 0, w, h);
    wj_glDisable(GL_DEPTH_TEST);
    wj_glDisable(GL_CULL_FACE);
    wj_glEnable(GL_BLEND);
    wj_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    wj_glUseProgram(sZoomProg);
    wj_glUniform1i(wj_glGetUniformLocation(sZoomProg, "src"), 0);
    /* the 3DS screen's middle (200, 120) is GBA (120, 60.57); the picture is stored top row first here */
    wj_glUniform2f(wj_glGetUniformLocation(sZoomProg, "centre"), 0.5f, (112.0f + (120.0f - 192.0f) / 1.4f) / 160.0f);
    wj_glBindBuffer(GL_ARRAY_BUFFER, sZoomVbo);
    wj_glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    wj_glEnableVertexAttribArray(0);
    wj_glDisableVertexAttribArray(1);
    wj_glDisableVertexAttribArray(2);
    wj_glUniform1f(wj_glGetUniformLocation(sZoomProg, "scale"), 1.0f + 0.035f * s);
    wj_glUniform1f(wj_glGetUniformLocation(sZoomProg, "alpha"), 0.50f * s);
    wj_glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    wj_glUniform1f(wj_glGetUniformLocation(sZoomProg, "scale"), 1.0f + 0.080f * s);
    wj_glUniform1f(wj_glGetUniformLocation(sZoomProg, "alpha"), 0.32f * s);
    wj_glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
