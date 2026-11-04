// Minimal headless libretro frontend for the Azahar core (software renderer).
// usage: fe core.so game.3dsx script.txt sysdir savedir
// script lines: "N" run N frames | "N KEYS" hold KEYS for N frames (A B X Y L R S=start s=select U D L R as u d l r)
//               "touch X Y N" touch bottom-screen pixel (X,Y) for N frames | "shot NAME" save NAME.ppm
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
static char g_defk[64][64], g_defv[64][64]; static int g_ndef;
#include "libretro.h"

static const char *g_sys, *g_save;
static unsigned g_fmt = RETRO_PIXEL_FORMAT_0RGB1555;
static const void *g_frame; static unsigned g_w, g_h; static size_t g_pitch;
static uint8_t *g_copy; static size_t g_copysz;
static unsigned g_keys; static int g_touch, g_tx, g_ty;
static unsigned g_fw, g_fh; // full output size for pointer mapping

// ---- OpenGL via EGL surfaceless (Mesa llvmpipe), loaded at run time
typedef void *EGLDisplay, *EGLContext, *EGLConfig, *EGLSurface; typedef int EGLint; typedef unsigned EGLenum; typedef unsigned EGLBoolean;
static EGLDisplay (*p_eglGetPlatformDisplay)(EGLenum, void *, const intptr_t *);
static EGLBoolean (*p_eglInitialize)(EGLDisplay, EGLint *, EGLint *);
static EGLBoolean (*p_eglChooseConfig)(EGLDisplay, const EGLint *, EGLConfig *, EGLint, EGLint *);
static EGLBoolean (*p_eglBindAPI)(EGLenum);
static EGLContext (*p_eglCreateContext)(EGLDisplay, EGLConfig, EGLContext, const EGLint *);
static EGLBoolean (*p_eglMakeCurrent)(EGLDisplay, EGLSurface, EGLSurface, EGLContext);
static void *(*p_eglGetProcAddress)(const char *);
static void (*gl_GenFramebuffers)(int, unsigned *), (*gl_BindFramebuffer)(unsigned, unsigned), (*gl_GenRenderbuffers)(int, unsigned *),
    (*gl_BindRenderbuffer)(unsigned, unsigned), (*gl_RenderbufferStorage)(unsigned, unsigned, int, int),
    (*gl_FramebufferRenderbuffer)(unsigned, unsigned, unsigned, unsigned), (*gl_ReadPixels)(int, int, int, int, unsigned, unsigned, void *),
    (*gl_Finish)(void), (*gl_ClearColor)(float,float,float,float), (*gl_Clear)(unsigned);
static unsigned (*gl_CheckFramebufferStatus)(unsigned);
static struct retro_hw_render_callback g_hw; static int g_usehw; static unsigned g_fbo; static int g_hwframe;
static uintptr_t hw_fb(void) { return g_fbo; }
static void (*real_BindFramebuffer)(unsigned, unsigned); static void (*real_BlitFramebuffer)(int,int,int,int,int,int,int,int,unsigned,unsigned);
static int g_log;
static void wrap_BindFramebuffer(unsigned t, unsigned f) { if (g_log && g_log < 400) { g_log++; fprintf(stderr, "fe: bindfb %#x -> %u\n", t, f); } real_BindFramebuffer(t, f); }
static void wrap_BlitFramebuffer(int a,int b,int c,int d,int e,int f,int g,int h,unsigned m,unsigned fl) { if (g_log && g_log < 400) { g_log++; fprintf(stderr, "fe: blit %d,%d-%d,%d -> %d,%d-%d,%d\n",a,b,c,d,e,f,g,h); } real_BlitFramebuffer(a,b,c,d,e,f,g,h,m,fl); }
static void (*real_DrawArrays)(unsigned,int,int); static unsigned (*real_GetError)(void); static long g_draws, g_errs; static unsigned g_lasterr;
static void wrap_DrawArrays(unsigned m,int f,int c) { real_DrawArrays(m,f,c); g_draws++; if (getenv("FE_ERR")) { unsigned e = real_GetError(); if (e) { g_errs++; g_lasterr = e; } } }
static void (*real_Viewport)(int,int,int,int); static int g_vplog;
static void wrap_Viewport(int x,int y,int w,int h) { if (getenv("FE_VP") && g_vplog < 60) { g_vplog++; fprintf(stderr, "fe: viewport %d,%d %dx%d\n", x,y,w,h); } real_Viewport(x,y,w,h); }
static retro_proc_address_t hw_proc(const char *n) {
    if (!strcmp(n, "glViewport")) { real_Viewport = p_eglGetProcAddress(n); return (retro_proc_address_t)wrap_Viewport; }
    if (!strcmp(n, "glDrawArrays")) { real_DrawArrays = p_eglGetProcAddress(n); real_GetError = p_eglGetProcAddress("glGetError"); return (retro_proc_address_t)wrap_DrawArrays; }
    if (!strcmp(n, "glBindFramebuffer")) { real_BindFramebuffer = p_eglGetProcAddress(n); return (retro_proc_address_t)wrap_BindFramebuffer; }
    if (!strcmp(n, "glBlitFramebuffer")) { real_BlitFramebuffer = p_eglGetProcAddress(n); return (retro_proc_address_t)wrap_BlitFramebuffer; }
    return (retro_proc_address_t)p_eglGetProcAddress(n);
}
static int gl_setup(unsigned w, unsigned h) {
    void *e = dlopen("libEGL.so.1", RTLD_NOW | RTLD_GLOBAL); if (!e) { fprintf(stderr, "fe: no libEGL\n"); return 0; }
#define E(n) *(void **)&p_##n = dlsym(e, #n)
    E(eglGetPlatformDisplay); E(eglInitialize); E(eglChooseConfig); E(eglBindAPI); E(eglCreateContext); E(eglMakeCurrent); E(eglGetProcAddress);
    EGLDisplay d = p_eglGetPlatformDisplay(0x31DD /*SURFACELESS_MESA*/, NULL, NULL); EGLint ma, mi;
    if (!d || !p_eglInitialize(d, &ma, &mi)) { fprintf(stderr, "fe: eglInitialize failed\n"); return 0; }
    EGLint ca[] = {0x3033, 0x0001, 0x3040, 0x0008, 0x3024, 8, 0x3023, 8, 0x3022, 8, 0x3021, 8, 0x3025, 24, 0x3026, 8, 0x3038};
    EGLConfig cfg; EGLint n = 0;
    if (!p_eglChooseConfig(d, ca, &cfg, 1, &n) || n < 1) { EGLint cb[] = {0x3040, 0x0008, 0x3038}; p_eglChooseConfig(d, cb, &cfg, 1, &n); }
    p_eglBindAPI(0x30A2);
    EGLint xa[] = {0x3098, g_hw.version_major ? (EGLint)g_hw.version_major : 4, 0x30FB, g_hw.version_major ? (EGLint)g_hw.version_minor : 3, 0x30FD, 1, 0x3038};
    EGLContext c = p_eglCreateContext(d, n ? cfg : NULL, NULL, xa);
    if (!c || !p_eglMakeCurrent(d, NULL, NULL, c)) { fprintf(stderr, "fe: EGL context failed\n"); return 0; }
#define G(n) *(void **)&gl_##n = p_eglGetProcAddress("gl" #n)
    G(GenFramebuffers); G(BindFramebuffer); G(GenRenderbuffers); G(BindRenderbuffer); G(RenderbufferStorage);
    G(FramebufferRenderbuffer); G(ReadPixels); G(Finish); G(CheckFramebufferStatus); G(ClearColor); G(Clear);
    unsigned rb[2]; gl_GenFramebuffers(1, &g_fbo); gl_BindFramebuffer(0x8D40, g_fbo); gl_GenRenderbuffers(2, rb);
    gl_BindRenderbuffer(0x8D41, rb[0]); gl_RenderbufferStorage(0x8D41, 0x8058, w, h); gl_FramebufferRenderbuffer(0x8D40, 0x8CE0, 0x8D41, rb[0]);
    gl_BindRenderbuffer(0x8D41, rb[1]); gl_RenderbufferStorage(0x8D41, 0x88F0, w, h); gl_FramebufferRenderbuffer(0x8D40, 0x821A, 0x8D41, rb[1]);
    fprintf(stderr, "fe: GL %d.%d context, FBO %u (%ux%u) status %#x\n", xa[1], xa[3], g_fbo, w, h, gl_CheckFramebufferStatus(0x8D40));
    return 1;
}

static void logcb(enum retro_log_level lvl, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt); fprintf(stderr, "[core %d] ", lvl); vfprintf(stderr, fmt, ap); va_end(ap);
}

static const char *opt(const char *key) {
    if (!strcmp(key, "citra_graphics_api")) return getenv("FE_GFX") ? getenv("FE_GFX") : "Software";
    if (!strcmp(key, "citra_use_virtual_sd")) return "enabled";
    if (!strcmp(key, "citra_use_libretro_save_path")) return "LibRetro Default";
    if (!strcmp(key, "citra_layout_option")) return "default";
    if (!strcmp(key, "citra_is_new_3ds")) return "New 3DS";
    if (!strcmp(key, "citra_use_cpu_jit")) return "enabled";
    if (!strcmp(key, "citra_use_hw_shader")) return getenv("FE_HWSH") ? getenv("FE_HWSH") : "disabled";
    if (!strcmp(key, "citra_enable_touch_touchscreen")) return "enabled";
    if (!strcmp(key, "citra_enable_mouse_touchscreen")) return "enabled";
    if (!strcmp(key, "citra_resolution_factor")) return "1";
    if (!strcmp(key, "citra_use_disk_shader_cache")) return "disabled";
    if (!strcmp(key, "citra_use_shader_jit")) return getenv("FE_SJIT") ? getenv("FE_SJIT") : "disabled";
    for (int i = 0; i < g_ndef; i++) if (!strcmp(g_defk[i], key)) return g_defv[i];
    return NULL;
}

static bool env(unsigned cmd, void *data) {
    switch (cmd & ~RETRO_ENVIRONMENT_EXPERIMENTAL) {
    case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: ((struct retro_log_callback *)data)->log = logcb; return true;
    case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: *(const char **)data = g_sys; return true;
    case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: *(const char **)data = g_save; return true;
    case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: g_fmt = *(enum retro_pixel_format *)data; fprintf(stderr, "fe: pixel format %u\n", g_fmt); return true;
    case RETRO_ENVIRONMENT_GET_CAN_DUPE: *(bool *)data = true; return true;
    case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION: *(unsigned *)data = 2; return true;
    case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2: {
        const struct retro_core_options_v2 *o = data;
        for (const struct retro_core_option_v2_definition *d = o->definitions; d && d->key; d++) {
            fprintf(stderr, "opt %s:", d->key);
            for (int i = 0; i < RETRO_NUM_CORE_OPTION_VALUES_MAX && d->values[i].value; i++) fprintf(stderr, " [%s]", d->values[i].value);
            fprintf(stderr, " default=%s\n", d->default_value ? d->default_value : "");
            if (g_ndef < 64 && d->default_value) { snprintf(g_defk[g_ndef], 64, "%s", d->key); snprintf(g_defv[g_ndef], 64, "%s", d->default_value); g_ndef++; }
        }
        return true;
    }
    case RETRO_ENVIRONMENT_GET_VARIABLE: {
        struct retro_variable *v = data; v->value = opt(v->key);
        return v->value != NULL;
    }
    case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: *(bool *)data = false; return true;
    case RETRO_ENVIRONMENT_SET_HW_RENDER: {
        struct retro_hw_render_callback *cb = data;
        fprintf(stderr, "fe: core asked for HW render ctx %d v%u.%u\n", cb->context_type, cb->version_major, cb->version_minor);
        if (!getenv("FE_GL") || (cb->context_type != RETRO_HW_CONTEXT_OPENGL_CORE && cb->context_type != RETRO_HW_CONTEXT_OPENGL)) return false;
        cb->get_current_framebuffer = hw_fb; cb->get_proc_address = hw_proc; g_hw = *cb; g_usehw = 1; return true;
    }
    case RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER: *(unsigned *)data = getenv("FE_GL") ? RETRO_HW_CONTEXT_OPENGL_CORE : RETRO_HW_CONTEXT_NONE; return true;
    case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS: return true;
    case RETRO_ENVIRONMENT_SET_GEOMETRY: case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO: {
        return true;
    }
    default: return false;
    }
}

static void video(const void *data, unsigned w, unsigned h, size_t pitch) {
    if (data == RETRO_HW_FRAME_BUFFER_VALID) {
        size_t need = (size_t)w * h * 4; if (need > g_copysz) { g_copy = realloc(g_copy, need); g_copysz = need; }
        uint8_t *tmp = malloc(need); gl_BindFramebuffer(0x8D40, g_fbo); gl_Finish(); gl_ReadPixels(0, 0, w, h, 0x1908, 0x1401, tmp);
        for (unsigned y = 0; y < h; y++) {
            const uint8_t *src = tmp + (size_t)(g_hw.bottom_left_origin ? h - 1 - y : y) * w * 4; uint32_t *dst = (uint32_t *)(g_copy + (size_t)y * w * 4);
            for (unsigned x = 0; x < w; x++) dst[x] = (src[4*x] << 16) | (src[4*x+1] << 8) | src[4*x+2];
        }
        if (getenv("FE_LOGFB") && !g_log) g_log = 1;
        if (getenv("FE_PROBE")) { static int cnt; if (++cnt % 100 == 0) { unsigned W = 800, H = 960; uint8_t *b = malloc((size_t)W*H*4); gl_ReadPixels(0, 0, W, H, 0x1908, 0x1401, b);
            unsigned x0=W,y0=H,x1=0,y1=0; for (unsigned yy=0; yy<H; yy++) for (unsigned xx=0; xx<W; xx++) { uint8_t *q=b+((size_t)yy*W+xx)*4; if (q[0]|q[1]|q[2]) { if(xx<x0)x0=xx; if(yy<y0)y0=yy; if(xx>x1)x1=xx; if(yy>y1)y1=yy; } }
            fprintf(stderr, "fe: draws %ld errs %ld last %#x | probe frame %d w=%u h=%u nonzero box x%u-%u y%u-%u\n", g_draws, g_errs, g_lasterr, cnt, w, h, x0, x1, y0, y1); free(b); } }
        free(tmp); g_frame = g_copy; g_w = w; g_h = h; g_pitch = (size_t)w * 4; g_hwframe = 1; return;
    }
    if (!data) return;
    g_hwframe = 0;
    size_t need = (size_t)h * pitch;
    if (need > g_copysz) { g_copy = realloc(g_copy, need); g_copysz = need; }
    memcpy(g_copy, data, need);
    g_frame = g_copy; g_w = w; g_h = h; g_pitch = pitch;
}
static void audio(int16_t l, int16_t r) { (void)l; (void)r; }
static size_t audiob(const int16_t *d, size_t f) { (void)d; return f; }
static void poll(void) { if (getenv("FE_MAGENTA") && g_usehw) { gl_BindFramebuffer(0x8D40, g_fbo); gl_ClearColor(getenv("FE_MAGENTA")[0]==(char)0x31 ? 1 : 0, 0, getenv("FE_MAGENTA")[0]==(char)0x31 ? 1 : 0, 1); gl_Clear(0x4000 | 0x100 | 0x400); } }
static int16_t input(unsigned port, unsigned dev, unsigned idx, unsigned id) {
    if (port) return 0;
    if (dev == RETRO_DEVICE_JOYPAD) {
        if (id == RETRO_DEVICE_ID_JOYPAD_MASK) return (int16_t)g_keys;
        return (g_keys >> id) & 1;
    }
    if (dev == RETRO_DEVICE_POINTER || dev == RETRO_DEVICE_MOUSE) {
        if (dev == RETRO_DEVICE_MOUSE) return 0;
        // bottom screen sits under the 400x240 top screen, centred (40px offset), 320x240
        unsigned fw = g_w ? g_w : 400, fh = g_h ? g_h : 480;
        int px = 40 + g_tx, py = 240 + g_ty;
        if (id == RETRO_DEVICE_ID_POINTER_PRESSED) return g_touch;
        if (id == RETRO_DEVICE_ID_POINTER_X) return (int16_t)((px * 65535) / (int)fw - 32767);
        if (id == RETRO_DEVICE_ID_POINTER_Y) return (int16_t)((py * 65535) / (int)fh - 32767);
        if (id == RETRO_DEVICE_ID_POINTER_COUNT) return g_touch ? 1 : 0;
    }
    return 0;
}

static void shot(const char *name) {
    if (!g_frame) { fprintf(stderr, "fe: no frame yet for %s\n", name); return; }
    char path[512]; snprintf(path, sizeof path, "%s.ppm", name);
    FILE *f = fopen(path, "wb"); fprintf(f, "P6\n%u %u\n255\n", g_w, g_h);
    for (unsigned y = 0; y < g_h; y++) {
        const uint8_t *row = (const uint8_t *)g_frame + y * g_pitch;
        for (unsigned x = 0; x < g_w; x++) {
            uint8_t rgb[3];
            if (g_hwframe || g_fmt == RETRO_PIXEL_FORMAT_XRGB8888) { uint32_t p = ((const uint32_t *)row)[x]; rgb[0] = p >> 16; rgb[1] = p >> 8; rgb[2] = p; }
            else { uint16_t p = ((const uint16_t *)row)[x];
                if (g_fmt == RETRO_PIXEL_FORMAT_RGB565) { rgb[0] = (p >> 11) << 3; rgb[1] = ((p >> 5) & 63) << 2; rgb[2] = (p & 31) << 3; }
                else { rgb[0] = ((p >> 10) & 31) << 3; rgb[1] = ((p >> 5) & 31) << 3; rgb[2] = (p & 31) << 3; } }
            fwrite(rgb, 1, 3, f);
        }
    }
    fclose(f); fprintf(stderr, "fe: shot %s %ux%u\n", name, g_w, g_h);
}

static unsigned keymask(const char *s) {
    unsigned m = 0;
    for (; *s; s++) switch (*s) {
        case 'A': m |= 1 << RETRO_DEVICE_ID_JOYPAD_A; break; case 'B': m |= 1 << RETRO_DEVICE_ID_JOYPAD_B; break;
        case 'X': m |= 1 << RETRO_DEVICE_ID_JOYPAD_X; break; case 'Y': m |= 1 << RETRO_DEVICE_ID_JOYPAD_Y; break;
        case 'L': m |= 1 << RETRO_DEVICE_ID_JOYPAD_L; break; case 'R': m |= 1 << RETRO_DEVICE_ID_JOYPAD_R; break;
        case 'S': m |= 1 << RETRO_DEVICE_ID_JOYPAD_START; break; case 's': m |= 1 << RETRO_DEVICE_ID_JOYPAD_SELECT; break;
        case 'u': m |= 1 << RETRO_DEVICE_ID_JOYPAD_UP; break; case 'd': m |= 1 << RETRO_DEVICE_ID_JOYPAD_DOWN; break;
        case 'l': m |= 1 << RETRO_DEVICE_ID_JOYPAD_LEFT; break; case 'r': m |= 1 << RETRO_DEVICE_ID_JOYPAD_RIGHT; break;
    }
    return m;
}

int main(int argc, char **argv) {
    if (argc < 6) { fprintf(stderr, "usage\n"); return 1; }
    g_sys = argv[4]; g_save = argv[5];
    void *h = dlopen(argv[1], RTLD_NOW); if (!h) { fprintf(stderr, "%s\n", dlerror()); return 1; }
#define SYM(n) typeof(&n) p_##n = dlsym(h, #n)
    SYM(retro_set_environment); SYM(retro_set_video_refresh); SYM(retro_set_audio_sample); SYM(retro_set_audio_sample_batch);
    SYM(retro_set_input_poll); SYM(retro_set_input_state); SYM(retro_init); SYM(retro_load_game); SYM(retro_run);
    SYM(retro_get_system_av_info);
    { Dl_info di; if (dladdr(dlsym(h, "retro_run"), &di)) fprintf(stderr, "fe: core base %p\n", di.dli_fbase); }
    p_retro_set_environment(env); p_retro_set_video_refresh(video); p_retro_set_audio_sample(audio);
    p_retro_set_audio_sample_batch(audiob); p_retro_set_input_poll(poll); p_retro_set_input_state(input);
    p_retro_init();
    struct retro_game_info gi = {argv[2], NULL, 0, NULL};
    if (!p_retro_load_game(&gi)) { fprintf(stderr, "fe: load failed\n"); return 1; }
    struct retro_system_av_info av; p_retro_get_system_av_info(&av);
    fprintf(stderr, "fe: av %ux%u (max %ux%u) fps %.2f\n", av.geometry.base_width, av.geometry.base_height, av.geometry.max_width, av.geometry.max_height, av.timing.fps);
    if (g_usehw) { if (!gl_setup(av.geometry.max_width, av.geometry.max_height)) return 1; if (g_hw.context_reset) g_hw.context_reset(); }
    FILE *sc = fopen(argv[3], "r"); char line[256]; long frame = 0;
    while (fgets(line, sizeof line, sc)) {
        char a[64] = "", b[64] = ""; int n = 0, x = 0, y = 0;
        if (sscanf(line, "shot %63s", a) == 1) { shot(a); continue; }
        if (sscanf(line, "touch %d %d %d", &x, &y, &n) == 3) {
            g_tx = x; g_ty = y; g_touch = 1;
            for (int i = 0; i < n; i++, frame++) p_retro_run();
            g_touch = 0; continue;
        }
        int k = sscanf(line, "%d %63s", &n, b);
        if (k < 1) continue;
        g_keys = k == 2 ? keymask(b) : 0;
        for (int i = 0; i < n; i++, frame++) p_retro_run();
        g_keys = 0;
    }
    fprintf(stderr, "fe: done after %ld frames\n", frame);
    _exit(0);
}
