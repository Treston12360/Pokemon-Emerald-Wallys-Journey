// DLX-9: Gamma Emerald's daylight on the 2D overworld (the 3D version is in voxel3d/voxel_gl.c).
//
// The same light as 3D, seen from straight above: every map pixel is lit by the sun (its colour and
// intensity, times the up component of its direction 0.6428), shaded by Gamma's cloud shadows (its
// two noise textures, M_SunCloudShadows_LF1's numbers, fixed to the map and drifting with time),
// plus the sky light from straight up. Then the screen pass of 3D: UE's bloom (BloomSetup threshold
// 0.4, six stages, default tints, intensity 6), exposure 2.06 (see voxel_gl.c), saturation 1.25 in
// AP1 and UE's FilmToneMap. All values: docs/GAMMA_LIGHTING_VALUES.md.
//
// Done on the CPU on the overworld picture before it is uploaded, so it works with any SDL renderer.
// The colour pipeline after the light (exposure, saturation, film curve, sRGB) is one 3D table.

#if defined(__GNUC__) && !defined(__clang__) && defined(__i386__)
#pragma GCC target("sse2", "fpmath=sse")
#endif
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("O3", "fast-math")
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "platform/gamma2d.h"
#ifndef GAMMA2D_NO_THREADS
#if defined(__has_include)
#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#else
#include <SDL2/SDL.h>
#endif
#endif

#define CLOUD_SIZE 1024
#define WIDE_X 2048
#define LUT_N 48           // table points per channel, log2 spaced
#define LUT_LO (-14.0f)    // log2 of the smallest value in the table
#define LUT_HI (5.0f)

static uint8_t *sPuddles, *sRough;
static bool sTried;
static float sLinear[32];            // GBA 5-bit channel -> linear
static uint8_t (*sLut)[3];            // LUT_N^3 -> sRGB 8-bit
static float *sBright[3], *sTmp[3];   // bloom work buffers (per channel)
static float *sStage[3];
static int sBufW, sBufH;

static const float kExposure = 2.06f, kSaturation = 1.25f, kBloom = 6.0f, kThreshold = 0.4f;
static const float kTints[6] = {0.3465f, 0.138f, 0.1176f, 0.066f, 0.066f, 0.061f};

static float Srgb2Lin(float c) { return c <= 0.04045f ? c / 12.92f : powf((c + 0.055f) / 1.055f, 2.4f); }
static float Lin2Srgb(float c)
{
    if (c < 0) c = 0;
    if (c > 1) c = 1;
    return c <= 0.0031308f ? c * 12.92f : 1.055f * powf(c, 1.0f / 2.4f) - 0.055f;
}

static void MulM(const float m[9], const float v[3], float o[3])
{
    float a = m[0] * v[0] + m[1] * v[1] + m[2] * v[2];
    float b = m[3] * v[0] + m[4] * v[1] + m[5] * v[2];
    float c = m[6] * v[0] + m[7] * v[1] + m[8] * v[2];
    o[0] = a; o[1] = b; o[2] = c;
}

// rows (the GLSL in voxel_gl.c writes the same matrices by column)
static const float sRGB_2_AP1[9] = {0.61319f, 0.33951f, 0.04737f, 0.07021f, 0.91634f, 0.01345f, 0.02062f, 0.10957f, 0.86961f};
static const float AP1_2_sRGB[9] = {1.70505f, -0.62179f, -0.08326f, -0.13026f, 1.14080f, -0.01055f, -0.02400f, -0.12897f, 1.15297f};
static const float sRGB_2_AP0[9] = {0.4397010f, 0.3829780f, 0.1773350f, 0.0897923f, 0.8134230f, 0.0967616f, 0.0175440f, 0.1115440f, 0.8707040f};
static const float AP0_2_AP1[9] = {1.4514393161f, -0.2365107469f, -0.2149285693f, -0.0765537734f, 1.1762296998f, -0.0996759264f, 0.0083161484f, -0.0060324498f, 0.9977163014f};
static const float AP1_Y[3] = {0.2722287168f, 0.6740817658f, 0.0536895174f};

static float Sigmoid(float x) { float t = 1.0f - fabsf(0.5f * x); if (t < 0) t = 0; return (1.0f + (x > 0 ? 1.0f : x < 0 ? -1.0f : 0.0f) * (1.0f - t * t)) * 0.5f; }

// UE FilmToneMap (slope 1, toe 0.3, shoulder 1, black 0, white 0): linear sRGB in, AP1 out
static void Film(const float lin[3], float out[3])
{
    const float slope = 1.0f, toe = 0.3f, shoulder = 1.0f, black = 0.0f, white = 0.0f;
    float a[3], w[3];
    MulM(sRGB_2_AP0, lin, a);
    float mi = fminf(a[0], fminf(a[1], a[2])), ma = fmaxf(a[0], fmaxf(a[1], a[2]));
    float s = (fmaxf(ma, 1e-10f) - fmaxf(mi, 1e-10f)) / fmaxf(ma, 1e-2f);
    float ch = sqrtf(fmaxf(a[2] * (a[2] - a[1]) + a[1] * (a[1] - a[0]) + a[0] * (a[0] - a[2]), 0.0f));
    float yc = (a[2] + a[1] + a[0] + 1.75f * ch) / 3.0f;
    float g = 0.05f * Sigmoid((s - 0.4f) / 0.2f), m = 0.08f, gl;
    gl = yc <= 2.0f / 3.0f * m ? g : yc >= 2.0f * m ? 0.0f : g * (m / yc - 0.5f);
    for (int i = 0; i < 3; i++) a[i] *= 1.0f + gl;
    float h = 0.0f;
    if (!(a[0] == a[1] && a[1] == a[2]))
    {
        h = atan2f(sqrtf(3.0f) * (a[1] - a[2]), 2.0f * a[0] - a[1] - a[2]) * 57.29578f;
        if (h < 0) h += 360.0f;
    }
    if (h > 180.0f) h -= 360.0f;
    float hw = 1.0f - fabsf(2.0f * h / 135.0f);
    hw = hw < 0 ? 0 : hw > 1 ? 1 : hw;
    hw = hw * hw * (3.0f - 2.0f * hw);
    hw *= hw;
    a[0] += hw * s * (0.03f - a[0]) * (1.0f - 0.82f);
    MulM(AP0_2_AP1, a, w);
    float wy = 0;
    for (int i = 0; i < 3; i++) { w[i] = w[i] < 0 ? 0 : w[i] > 65504.0f ? 65504.0f : w[i]; }
    wy = w[0] * AP1_Y[0] + w[1] * AP1_Y[1] + w[2] * AP1_Y[2];
    for (int i = 0; i < 3; i++) w[i] = wy + (w[i] - wy) * 0.96f;
    float ts = 1.0f + black - toe, ss = 1.0f + white - shoulder;
    float bt = (0.18f + black) / ts - 1.0f;
    float tm = log10f(0.18f) - 0.5f * logf((1.0f + bt) / (1.0f - bt)) * (ts / slope);
    float sm = (1.0f - toe) / slope - tm, shm = shoulder / slope - sm;
    float tone[3];
    for (int i = 0; i < 3; i++)
    {
        float L = log10f(fmaxf(w[i], 1e-10f)), st = slope * (L + sm);
        float tc = -black + 2.0f * ts / (1.0f + expf((-2.0f * slope / ts) * (L - tm)));
        float sc = (1.0f + white) - 2.0f * ss / (1.0f + expf((2.0f * slope / fmaxf(ss, 1e-6f)) * (L - shm)));
        if (!(L < tm)) tc = st;
        if (!(L > shm)) sc = st;
        float t = (L - tm) / (shm - tm);
        t = t < 0 ? 0 : t > 1 ? 1 : t;
        if (shm < tm) t = 1.0f - t;
        t = (3.0f - 2.0f * t) * t * t;
        tone[i] = tc + (sc - tc) * t;
    }
    float ty = tone[0] * AP1_Y[0] + tone[1] * AP1_Y[1] + tone[2] * AP1_Y[2];
    for (int i = 0; i < 3; i++) { out[i] = ty + (tone[i] - ty) * 0.93f; if (out[i] < 0) out[i] = 0; }
}

// light (linear sRGB, before exposure) -> displayed sRGB
static void Grade(const float hdr[3], float out[3])
{
    float c[3] = {hdr[0] * kExposure, hdr[1] * kExposure, hdr[2] * kExposure}, ap[3], s[3], t[3];
    MulM(sRGB_2_AP1, c, ap);
    float y = ap[0] * AP1_Y[0] + ap[1] * AP1_Y[1] + ap[2] * AP1_Y[2];
    for (int i = 0; i < 3; i++) { ap[i] = y + (ap[i] - y) * kSaturation; if (ap[i] < 0) ap[i] = 0; }
    MulM(AP1_2_sRGB, ap, s);
    Film(s, t);
    MulM(AP1_2_sRGB, t, s);
    for (int i = 0; i < 3; i++) out[i] = Lin2Srgb(s[i]);
}

// log2 read straight from a float's bits (exponent + mantissa as a straight line); the table is laid out
// with its exact inverse, so its points sit exactly where the lookups put them
static inline float FastLog2(float v)
{
    union { float f; uint32_t i; } u = {v};
    return (float)u.i * (1.0f / 8388608.0f) - 127.0f;
}
static inline float FastExp2(float l)
{
    union { float f; uint32_t i; } u;
    u.i = (uint32_t)((l + 127.0f) * 8388608.0f);
    return u.f;
}

static bool Load(const char *dataDir)
{
    char path[700];
    FILE *f;
    if (sTried)
        return sLut != NULL;
    sTried = true;
    for (int i = 0; i < 32; i++)
        sLinear[i] = Srgb2Lin(i / 31.0f);
    snprintf(path, sizeof(path), "%s/gamma_clouds.bin", dataDir != NULL ? dataDir : "voxel");
    f = fopen(path, "rb");
    if (f != NULL)
    {
        sPuddles = malloc(CLOUD_SIZE * CLOUD_SIZE);
        sRough = malloc(CLOUD_SIZE * CLOUD_SIZE);
        if (sPuddles == NULL || sRough == NULL
         || fread(sPuddles, 1, CLOUD_SIZE * CLOUD_SIZE, f) != CLOUD_SIZE * CLOUD_SIZE
         || fread(sRough, 1, CLOUD_SIZE * CLOUD_SIZE, f) != CLOUD_SIZE * CLOUD_SIZE)
        {
            free(sPuddles); free(sRough);
            sPuddles = sRough = NULL;
        }
        fclose(f);
    }
    sLut = malloc((size_t)LUT_N * LUT_N * LUT_N * 3);
    if (sLut == NULL)
        return false;
    for (int b = 0; b < LUT_N; b++)
        for (int g = 0; g < LUT_N; g++)
            for (int r = 0; r < LUT_N; r++)
            {
                float hdr[3], o[3];
                int idx[3] = {r, g, b};
                for (int i = 0; i < 3; i++)
                    hdr[i] = idx[i] == 0 ? 0.0f : FastExp2(LUT_LO + (LUT_HI - LUT_LO) * idx[i] / (LUT_N - 1));
                Grade(hdr, o);
                uint8_t *p = sLut[(b * LUT_N + g) * LUT_N + r];
                for (int i = 0; i < 3; i++) p[i] = (uint8_t)(o[i] * 255.0f + 0.5f);
            }
    return true;
}

static float Bilinear(const uint8_t *t, float u, float v)
{
    float x = u * CLOUD_SIZE, y = v * CLOUD_SIZE;
    float fx = floorf(x), fy = floorf(y);
    int x0 = (int)fx & (CLOUD_SIZE - 1), y0 = (int)fy & (CLOUD_SIZE - 1);
    int x1 = (x0 + 1) & (CLOUD_SIZE - 1), y1 = (y0 + 1) & (CLOUD_SIZE - 1);
    float ax = x - fx, ay = y - fy;
    float a = t[y0 * CLOUD_SIZE + x0] * (1 - ax) + t[y0 * CLOUD_SIZE + x1] * ax;
    float b = t[y1 * CLOUD_SIZE + x0] * (1 - ax) + t[y1 * CLOUD_SIZE + x1] * ax;
    return (a * (1 - ay) + b * ay) / 255.0f;
}

// M_SunCloudShadows_LF1 at a map pixel (16 to a tile, 64 cm a tile: 4 cm a pixel)
static float Cloud(float wx, float wy, float time)
{
    if (sPuddles == NULL)
        return 1.0f;
    float px = -wy * 4.0f, py = wx * 4.0f; // Gamma X = north (up the map), Y = east
    float n = Bilinear(sRough, px / 888.25f - 0.0010f * time, py / 888.25f - 0.0007f * time);
    float c = Bilinear(sPuddles, (px + n * 40.0f) / 5000.0f - 0.0021f * time, (py + n * 40.0f) / 5000.0f - 0.0030f * time);
    float k = (c - 0.164f) * 3.8417f;
    k = k < 0 ? 0 : k > 1 ? 1 : k;
    return 1.0f - k * 0.75f;
}

static void Lut(const float hdr[3], uint8_t out[3])
{
    float f[3];
    int i0[3];
    for (int i = 0; i < 3; i++)
    {
        float v = hdr[i] <= 1e-30f ? 0.0f : (FastLog2(hdr[i]) - LUT_LO) * ((LUT_N - 1) / (LUT_HI - LUT_LO));
        if (v < 0) v = 0;
        if (v > LUT_N - 1.001f) v = LUT_N - 1.001f;
        i0[i] = (int)v;
        f[i] = v - i0[i];
    }
    // tetrahedral: the cell's corner, the far corner, and the two between them on the path the three
    // fractions take (largest first) - four lookups instead of eight
    const int sr = 3, sg = 3 * LUT_N, sb = 3 * LUT_N * LUT_N;
    const uint8_t *base = sLut[(i0[2] * LUT_N + i0[1]) * LUT_N + i0[0]];
    float fr = f[0], fg = f[1], fb = f[2];
    int s1, s2;
    float w0, w1, w2, w3;
    if (fr >= fg)
    {
        if (fg >= fb) { s1 = sr; s2 = sr + sg; w0 = 1 - fr; w1 = fr - fg; w2 = fg - fb; w3 = fb; }
        else if (fr >= fb) { s1 = sr; s2 = sr + sb; w0 = 1 - fr; w1 = fr - fb; w2 = fb - fg; w3 = fg; }
        else { s1 = sb; s2 = sr + sb; w0 = 1 - fb; w1 = fb - fr; w2 = fr - fg; w3 = fg; }
    }
    else
    {
        if (fb >= fg) { s1 = sb; s2 = sg + sb; w0 = 1 - fb; w1 = fb - fg; w2 = fg - fr; w3 = fr; }
        else if (fb >= fr) { s1 = sg; s2 = sg + sb; w0 = 1 - fg; w1 = fg - fb; w2 = fb - fr; w3 = fr; }
        else { s1 = sg; s2 = sr + sg; w0 = 1 - fg; w1 = fg - fr; w2 = fr - fb; w3 = fb; }
    }
    const uint8_t *p1 = base + s1, *p2 = base + s2, *p3 = base + sr + sg + sb;
    float acc[3];
    for (int i = 0; i < 3; i++)
        acc[i] = w0 * base[i] + w1 * p1[i] + w2 * p2[i] + w3 * p3[i];
    for (int i = 0; i < 3; i++) out[i] = (uint8_t)(acc[i] + 0.5f);
}

// One box pass of radius r along a line of n values with stride s (running sum, edges clamped).
static void Box(const float *in, float *out, int n, int s, int r)
{
    float sum = 0.0f, inv = 1.0f / (2 * r + 1);
    for (int k = -r; k <= r; k++)
        sum += in[(k < 0 ? 0 : k >= n ? n - 1 : k) * s];
    for (int i = 0; i < n; i++)
    {
        out[i * s] = sum * inv;
        int add = i + r + 1, sub = i - r;
        sum += in[(add >= n ? n - 1 : add) * s] - in[(sub < 0 ? 0 : sub) * s];
    }
}

// Gaussian of sigma s (pixels) by three box passes each way.
static void Blur(float *a, float *tmp, int w, int h, float sigma)
{
    int r = (int)floorf(sqrtf(4.0f * sigma * sigma + 1.0f) * 0.5f); // three boxes of width 2r+1
    if (r < 1)
        return;
    for (int pass = 0; pass < 3; pass++)
    {
        for (int y = 0; y < h; y++) Box(a + y * w, tmp + y * w, w, 1, r);
        for (int x = 0; x < w; x++) Box(tmp + x, a + x, h, w, r);
    }
}


// The per-pixel passes are split by rows over a few threads (the bloom stays on one: it is small).
typedef void (*RowFn)(void *ctx, int y0, int y1);
#ifndef GAMMA2D_NO_THREADS
#define MAX_WORKERS 3
static SDL_sem *sGo[MAX_WORKERS], *sDone;
static int sWorkers = -1;
static volatile RowFn sJobFn;
static void *volatile sJobCtx;
static volatile int sJobH;
static volatile bool sQuit;

static int Worker(void *arg)
{
    int me = (int)(intptr_t)arg;
    for (;;)
    {
        SDL_SemWait(sGo[me]);
        if (sQuit)
            return 0;
        int parts = sWorkers + 1, h = sJobH;
        sJobFn(sJobCtx, h * (me + 1) / parts, h * (me + 2) / parts);
        SDL_SemPost(sDone);
    }
}
#endif

static void Rows(RowFn fn, void *ctx, int h)
{
#ifndef GAMMA2D_NO_THREADS
    if (sWorkers < 0)
    {
        int cpus = SDL_GetCPUCount();
        sWorkers = cpus - 1 < MAX_WORKERS ? cpus - 1 : MAX_WORKERS;
        if (sWorkers < 0)
            sWorkers = 0;
        sDone = SDL_CreateSemaphore(0);
        for (int i = 0; i < sWorkers; i++)
        {
            sGo[i] = SDL_CreateSemaphore(0);
            if (sDone == NULL || sGo[i] == NULL
             || SDL_CreateThread(Worker, "gamma2d", (void *)(intptr_t)i) == NULL)
            {
                sWorkers = i;
                break;
            }
        }
    }
    if (sWorkers > 0)
    {
        int parts = sWorkers + 1;
        sJobFn = fn;
        sJobCtx = ctx;
        sJobH = h;
        for (int i = 0; i < sWorkers; i++)
            SDL_SemPost(sGo[i]);
        fn(ctx, 0, h / parts);
        for (int i = 0; i < sWorkers; i++)
            SDL_SemWait(sDone);
        return;
    }
#endif
    fn(ctx, 0, h);
}

typedef struct
{
    const uint16_t *src;
    uint32_t *argb;
    const float *grid;
    int w, gw;
    float sun[3], sky[3];
} Job;

static void LightRows(void *p, int y0, int y1)
{
    const Job *j = p;
    int w = j->w, gw = j->gw;
    for (int y = y0; y < y1; y++)
    {
        int gy = y >> 2;
        float fy = (y & 3) / 4.0f;
        for (int x = 0; x < w; x++)
        {
            int gx = x >> 2;
            float fx = (x & 3) / 4.0f;
            const float *g = j->grid + gy * gw + gx;
            float cl = (g[0] * (1 - fx) + g[1] * fx) * (1 - fy) + (g[gw] * (1 - fx) + g[gw + 1] * fx) * fy;
            uint16_t c = j->src[y * w + x];
            int i = y * w + x;
            sStage[0][i] = sLinear[c & 31] * (j->sun[0] * cl + j->sky[0]);
            sStage[1][i] = sLinear[(c >> 5) & 31] * (j->sun[1] * cl + j->sky[1]);
            sStage[2][i] = sLinear[(c >> 10) & 31] * (j->sun[2] * cl + j->sky[2]);
        }
    }
}

static void OutRows(void *p, int y0, int y1)
{
    const Job *j = p;
    for (int i = y0 * j->w; i < y1 * j->w; i++)
    {
        float v[3] = {sStage[0][i], sStage[1][i], sStage[2][i]};
        uint8_t o[3];
        Lut(v, o);
        j->argb[i] = 0xFF000000u | ((uint32_t)o[0] << 16) | ((uint32_t)o[1] << 8) | o[2];
    }
}

// dst (dw x dh) += src (sw x sh) scaled up bilinearly
static void UpAdd(const float *src, int sw, int sh, float *dst, int dw, int dh)
{
    static int xa[WIDE_X], xb[WIDE_X];
    static float fx[WIDE_X];
    float rx = (float)sw / dw, ry = (float)sh / dh;
    for (int x = 0; x < dw && x < WIDE_X; x++)
    {
        float sx = (x + 0.5f) * rx - 0.5f;
        int x0 = (int)floorf(sx);
        fx[x] = sx - x0;
        xa[x] = x0 < 0 ? 0 : x0 >= sw ? sw - 1 : x0;
        xb[x] = x0 + 1 < 0 ? 0 : x0 + 1 >= sw ? sw - 1 : x0 + 1;
    }
    for (int y = 0; y < dh; y++)
    {
        float sy = (y + 0.5f) * ry - 0.5f;
        int y0 = (int)floorf(sy);
        float fy = sy - y0;
        const float *ra = src + (y0 < 0 ? 0 : y0 >= sh ? sh - 1 : y0) * sw;
        const float *rb = src + (y0 + 1 < 0 ? 0 : y0 + 1 >= sh ? sh - 1 : y0 + 1) * sw;
        float *d = dst + y * dw;
        for (int x = 0; x < dw; x++)
        {
            float a = ra[xa[x]] + (ra[xb[x]] - ra[xa[x]]) * fx[x];
            float b = rb[xa[x]] + (rb[xb[x]] - rb[xa[x]]) * fx[x];
            d[x] += a + (b - a) * fy;
        }
    }
}

void Gamma2D_Apply(uint32_t *argb, const uint16_t *src, int w, int h, int worldX0, int worldY0,
                   unsigned frame, const char *dataDir)
{
    const float pi = 3.14159265f;
    float sun[3] = {0.788f * 3.0f / pi * 0.6428f, 0.738f * 3.0f / pi * 0.6428f, 0.700f * 3.0f / pi * 0.6428f};
    float sky[3] = {1.0f * 0.32f * 0.1305f, 0.701f * 0.32f * 0.3063f, 0.271f * 0.32f * 0.5345f};
    float time = frame / 60.0f;

    if (!Load(dataDir) || w <= 0 || h <= 0)
        return;
    if (w != sBufW || h != sBufH)
    {
        for (int c = 0; c < 3; c++)
        {
            free(sBright[c]); free(sTmp[c]); free(sStage[c]);
            sBright[c] = malloc(sizeof(float) * w * h);
            sTmp[c] = malloc(sizeof(float) * w * h);
            sStage[c] = malloc(sizeof(float) * w * h);
            if (!sBright[c] || !sTmp[c] || !sStage[c])
            {
                sBufW = sBufH = 0;
                return;
            }
        }
        sBufW = w;
        sBufH = h;
    }
    // Cloud shadow varies slowly: worked out on a 4-pixel grid and interpolated.
    int gw = (w + 3) / 4 + 2, gh = (h + 3) / 4 + 2;
    float *grid = sTmp[0];
    for (int gy = 0; gy < gh; gy++)
        for (int gx = 0; gx < gw; gx++)
            grid[gy * gw + gx] = Cloud((float)(worldX0 + gx * 4), (float)(worldY0 + gy * 4), time);
    Job job = {src, argb, grid, w, gw, {sun[0], sun[1], sun[2]}, {sky[0], sky[1], sky[2]}};
    Rows(LightRows, &job, h);
    // Bloom as UE does it: BloomSetup (Rec709 luminance x exposure - threshold, half) at half size, then
    // six stages, each half the size of the last, blurred 31 texels of its own size (r.Filter.LoopMode 0
    // caps the kernel at 32 samples: sigma 31/3), summed with UE's default tints at half size.
    {
        static float *lv[7][3], *sum[3], *tmp;
        static int lw[7], lh[7], allocW, allocH;
        if (allocW != w || allocH != h)
        {
            for (int k = 0; k < 7; k++)
                for (int c = 0; c < 3; c++) { free(lv[k][c]); lv[k][c] = NULL; }
            for (int c = 0; c < 3; c++) { free(sum[c]); sum[c] = NULL; }
            free(tmp);
            for (int k = 0; k < 7; k++)
            {
                lw[k] = (w >> (k + 1)) > 0 ? (w >> (k + 1)) : 1;
                lh[k] = (h >> (k + 1)) > 0 ? (h >> (k + 1)) : 1;
                for (int c = 0; c < 3; c++)
                    lv[k][c] = malloc(sizeof(float) * lw[k] * lh[k]);
            }
            for (int c = 0; c < 3; c++)
                sum[c] = malloc(sizeof(float) * lw[0] * lh[0]);
            tmp = malloc(sizeof(float) * lw[0] * lh[0]);
            allocW = w;
            allocH = h;
        }
        // level 0: half size, 2x2 average of the bright pass
        for (int y = 0; y < lh[0]; y++)
            for (int x = 0; x < lw[0]; x++)
            {
                float acc[3] = {0, 0, 0};
                for (int dy = 0; dy < 2; dy++)
                    for (int dx = 0; dx < 2; dx++)
                    {
                        int i = (2 * y + dy) * w + 2 * x + dx;
                        float l = (0.212639f * sStage[0][i] + 0.715169f * sStage[1][i] + 0.072192f * sStage[2][i]) * kExposure;
                        float k = (l - kThreshold) * 0.5f;
                        k = k < 0 ? 0 : k > 1 ? 1 : k;
                        for (int c = 0; c < 3; c++) acc[c] += sStage[c][i] * k;
                    }
                for (int c = 0; c < 3; c++) lv[0][c][y * lw[0] + x] = acc[c] * 0.25f;
            }
        for (int k = 1; k < 6; k++)
            for (int c = 0; c < 3; c++)
                for (int y = 0; y < lh[k]; y++)
                    for (int x = 0; x < lw[k]; x++)
                    {
                        const float *s = lv[k - 1][c];
                        int pw = lw[k - 1], x2 = 2 * x < pw - 1 ? 2 * x : pw - 2, y2 = 2 * y < lh[k - 1] - 1 ? 2 * y : lh[k - 1] - 2;
                        if (x2 < 0) x2 = 0;
                        if (y2 < 0) y2 = 0;
                        int x3 = x2 + 1 < pw ? x2 + 1 : x2, y3 = y2 + 1 < lh[k - 1] ? y2 + 1 : y2;
                        lv[k][c][y * lw[k] + x] = (s[y2 * pw + x2] + s[y2 * pw + x3] + s[y3 * pw + x2] + s[y3 * pw + x3]) * 0.25f;
                    }
        // the levels are blurred in place, then folded together from the smallest up (each added, bilinear,
        // to the next bigger one): the same sum as adding each to the half-size picture, at a sixth the work
        for (int k = 5; k >= 0; k--)
            for (int c = 0; c < 3; c++)
            {
                float *b = lv[k][c];
                Blur(b, sTmp[1], lw[k], lh[k], 31.0f / 3.0f);
                for (int i = 0; i < lw[k] * lh[k]; i++)
                    b[i] *= kTints[k];
                if (k < 5)
                    UpAdd(lv[k + 1][c], lw[k + 1], lh[k + 1], b, lw[k], lh[k]);
            }
        for (int c = 0; c < 3; c++)
            memcpy(sum[c], lv[0][c], sizeof(float) * lw[0] * lh[0]);
        // add to the picture (bilinear up from half size)
        for (int c = 0; c < 3; c++)
        {
            for (int i = 0; i < lw[0] * lh[0]; i++)
                sum[c][i] *= kBloom;
            UpAdd(sum[c], lw[0], lh[0], sStage[c], w, h);
        }
    }
    Rows(OutRows, &job, h);
}
