#include <stdlib.h>
#ifdef PLATFORM_SDL2
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <xinput.h>
#endif

#ifdef __ANDROID__
#include <jni.h>
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#ifdef NATIVE_LINUX
#include <SDL2/SDL_image.h>
#endif

#include <string.h>
#include "tandem_net.h"   /* Project Tandem T1 (docs/TANDEM_TASKS.md) */
#include "global.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "constants/characters.h"
#include "berry.h"
#include "constants/event_objects.h"
#include "sprite.h"
#include "platform/null_guard.h"
#include "platform/wide_view.h"
#include "platform/gamma2d.h"
#include "platform.h"
#include "random.h"
#include "rtc.h"
#include "gba/defines.h"
#include "gba/m4a_internal.h"
#include "cgb_audio.h"
#include "gba/flash_internal.h"
#include "platform/dma.h"
#include "platform/framedraw.h"
#include "overworld.h"
#include "field_screen_effect.h"
#include "field_player_avatar.h"
#include "field_effect.h"
#include "constants/field_effects.h"
#include "constants/event_objects.h"
#include "script.h"
#include "pokemon_storage_system.h"
#include "tandem_link.h"   /* (Project Tandem T5: no fast forward while linked with a friend) */
#include "link.h"

extern void (*const gIntrTable[])(void);

SDL_Thread *mainLoopThread;
SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_Texture *sdlTexture;
SDL_Texture *sdlOverlayTexture; // smooth motion: text layer drawn over the sliding map
SDL_Texture *sdlUiTexture;      // UI layer (text boxes, menus, map name pop-up), drawn at its own scale
// The 3D overworld (src/platform/voxel3d, "world3d=1" in WallysJourney.cfg or WJPC_3D=1): drawn
// into this texture during the frame, under the UI layer, in place of the flat map.
#include "../platform/voxel3d/voxel_gl.h"
#include "../platform/voxel3d/voxel_battle.h"
static bool s3DWanted, s3DReady, s3DFrame;
/* Audit step 5 (Brennan 2026-11-21 02:12, 240 fps): the 3D field drawn on every refresh (Voxel3D_Redraw, eased
 * between game frames) rather than once a game frame - with a frame rate over 60 (Settings), outside battles and
 * battle transitions. WJPC_3D_EVERY_REFRESH_OFF=1: once a game frame, as before; WJPC_3D_EVERY_REFRESH=1: on at any
 * frame rate (tests). */
static bool s3DEveryRefresh;
static void ToggleFx(unsigned k); // F1-F6 (below)
// DLX-6: 3D can be turned on and off in Settings. The renderer is OpenGL whenever the 3D data is there
// (s3DAvailable), and the 3D world loads the first time it is wanted (s3DInitTried), so no restart.
static bool s3DAvailable, s3DInitTried;
// DLX-5: 3D battle scenery (Settings "3D Battles"); s3DBattleFrame: this frame is a 3D battle.
static bool s3DBattleWanted = true, s3DBattleFrame;
static unsigned sAntiAlias = 4; // Polish option 5: "antialias" in the cfg, samples a pixel (0 off)
static bool sGammaWanted; // DLX-9: Gamma Emerald's lighting ("gammaLight" in the cfg; Options > Gamma Lighting)
static SDL_Texture *sdl3DBattleTexture;
static char s3DDir[600];
static void Try3DInit(void)
{
    if (!(s3DWanted || s3DBattleWanted) || s3DReady || s3DInitTried || !s3DAvailable)
        return;
    s3DInitTried = true;
    s3DReady = Voxel3D_Init(s3DDir);
    if (!s3DReady)
        fprintf(stderr, "3D world unavailable (OpenGL or %s missing): drawing the map flat\n", s3DDir);
}
static SDL_Texture *sdl3DTexture, *sdl3DTransTexture;
static int sUiPlaceX, sUiPlaceY, sUiOutW, sUiOutH;
static float sUiPlaceS; // DLX-24: fractional only if the output is smaller than a GBA screen
static bool s3DTransition;
// DLX-4: a battle transition over the wide 2D field (no 3D): the last field picture stays in the map
// texture and the transition's own picture goes over it, as in 3D.
static bool s2DTransition;
static struct WideViewFrame sLastField;
static bool sHaveLastField;
// DLX-3: in-between pictures of the GBA screen (battles, menus) and the frame they lead up to
#define INTERP_MAX 3
static uint32_t sInterpImage[INTERP_MAX][DISPLAY_WIDTH * DISPLAY_HEIGHT];
static uint32_t sInterpCurrent[DISPLAY_WIDTH * DISPLAY_HEIGHT], sLastUploaded[DISPLAY_WIDTH * DISPLAY_HEIGHT];
static int sInterpCount, sInterpSteps, sInterpShown = -1;
static bool sInterpBattle; // the in-between pictures are 3D-battle overlays (DLX-5), not the GBA screen
static uint32_t sBattleImage[DISPLAY_WIDTH * DISPLAY_HEIGHT];
static int s3DW, s3DH;
#define SMOOTH_M WIDE_VIEW_SMOOTH_MARGIN
#define TEX_W (WIDE_VIEW_MAX_W + 2 * SMOOTH_M)
#define TEX_H (WIDE_VIEW_MAX_H + 2 * SMOOTH_M)
// Camera (world pixels) of the last two game frames, for smooth motion.
static int sPrevCamX, sPrevCamY, sCurCamX, sCurCamY;
static bool sCamMoveValid;
#if defined(NATIVE_LINUX) || defined(_WIN32)
#define MAX_BORDER_BACKGROUNDS 15
SDL_Texture *sdlBackgroundTextures[MAX_BORDER_BACKGROUNDS];
SDL_Texture *sdlBorderTexture;
#endif
static u8 sBorderBackgroundCount = 1;
SDL_AudioDeviceID sdlAudioDevice;
SDL_sem *vBlankSemaphore;
SDL_atomic_t isFrameAvailable;
bool speedUp = false;
unsigned int videoScale = 1;
bool isRunning = true;
bool paused = false;
double simTime = 0;
double lastGameTime = 0;
double curGameTime = 0;
double fixedTimestep = 1.0 / 60.0; // 16.666667ms
double timeScale = 1.0;
struct SiiRtcInfo internalClock;

static FILE *sSaveFile = NULL;
static char sSavePath[1024] = "WallysJourney.sav";
static char sConfigPath[1024] = "WallysJourney.cfg";
static u8 sBorderBackground;
static bool sHasBorderBackgroundConfig;
static u8 sBackgroundOrderVersion;
static u8 sPlatformSettings[PLATFORM_SETTING_COUNT] = {
    [PLATFORM_SETTING_FULLSCREEN] = 0,
    [PLATFORM_SETTING_WINDOW_SCALE] = 4,
    [PLATFORM_SETTING_INTEGER_SCALE] = 1,
    [PLATFORM_SETTING_VSYNC] = 1,
    [PLATFORM_SETTING_BORDER] = 1,
    [PLATFORM_SETTING_VOLUME] = 10,
    [PLATFORM_SETTING_MUSIC_VOLUME] = 10,
    [PLATFORM_SETTING_SOUND_VOLUME] = 10,
    [PLATFORM_SETTING_FF_SPEED] = 2,
    [PLATFORM_SETTING_FF_TOGGLE] = 0,
    [PLATFORM_SETTING_PAD_A] = PAD_BUTTON_A,
    [PLATFORM_SETTING_PAD_B] = PAD_BUTTON_B,
    [PLATFORM_SETTING_PAD_START] = PAD_BUTTON_START,
    [PLATFORM_SETTING_PAD_SELECT] = PAD_BUTTON_BACK,
    [PLATFORM_SETTING_PAD_L] = PAD_BUTTON_LB,
    [PLATFORM_SETTING_PAD_R] = PAD_BUTTON_RB,
    [PLATFORM_SETTING_PAD_FF] = PAD_BUTTON_RT,
    [PLATFORM_SETTING_ZOOM] = 4,
    [PLATFORM_SETTING_SMOOTHING] = 0,
    [PLATFORM_SETTING_RESOLUTION] = 0,
    [PLATFORM_SETTING_LOCK_ASPECT] = 1,
    [PLATFORM_SETTING_INTERFRAME] = 0,
    [PLATFORM_SETTING_LOCK_FRAME] = 0,
};

// Fast forward: the keyboard key (Space) and the controller button are both read here.
static SDL_atomic_t sFfKeyHeld, sFfPadHeld, sFfToggled;
static void ApplyFastForward(void);
const u16 gPlatformFrameRates[FRAME_RATE_COUNT] = {60, 120, 144, 165, 180, 240};
static const char *const sPadSettingNames[] = {"padA", "padB", "padStart", "padSelect", "padL", "padR", "padFastForward"};
static SDL_Keycode sKeyMap[KEYBIND_COUNT] = {
    [KEYBIND_UP] = SDLK_UP, [KEYBIND_DOWN] = SDLK_DOWN, [KEYBIND_LEFT] = SDLK_LEFT, [KEYBIND_RIGHT] = SDLK_RIGHT,
    [KEYBIND_A] = SDLK_z, [KEYBIND_B] = SDLK_x, [KEYBIND_START] = SDLK_RETURN, [KEYBIND_SELECT] = SDLK_BACKSLASH,
    [KEYBIND_L] = SDLK_a, [KEYBIND_R] = SDLK_s, [KEYBIND_FAST_FORWARD] = SDLK_SPACE,
};
static const u16 sKeyBindButtons[KEYBIND_COUNT] = {
    DPAD_UP, DPAD_DOWN, DPAD_LEFT, DPAD_RIGHT, A_BUTTON, B_BUTTON, START_BUTTON, SELECT_BUTTON, L_BUTTON, R_BUTTON, 0,
};
static const char *const sKeyBindNames[KEYBIND_COUNT] = {
    "keyUp", "keyDown", "keyLeft", "keyRight", "keyA", "keyB", "keyStart", "keySelect", "keyL", "keyR", "keyFastForward",
};
static SDL_atomic_t sKeyCapture = {-1};
static void CaptureKey(SDL_Keycode key);
#ifdef __ANDROID__
static SDL_GameController *androidController;
#endif

extern void AgbMain(void);
extern void DoSoftReset(void);

int DoMain(void *param);
void ProcessEvents(void);
void VDraw(SDL_Texture *texture);

static void ReadSaveFile(const char *path);
static void ReadConfigFile(void);
static void StoreConfigFile(void);
static void ApplyPlatformSettings(void);
static void StoreSaveFile(void);
static void FlushSaveIfDirty(void);
static void ApplyChangedWindowSettings(void);
static void CloseSaveFile(void);

static void UpdateInternalClock(void);
static void ResetInternalClock(void); // (the clock set up before the game thread starts)

#ifdef __ANDROID__
static void HandleTouchEvent(const SDL_TouchFingerEvent *event);
static void DrawTouchControls(void);
#endif


static u16 keyboardKeys;
static struct WideViewFrame sFrameInfo = {DISPLAY_WIDTH, DISPLAY_HEIGHT, WIDE_VIEW_GBA};

// ---- Wally's Journey PC test mode -----------------------------------------
// WJPC_SCRIPT=file runs the game as fast as possible, frame by frame, driven
// by a script:  "wait N" | "press KEYS N" | "shot PATH" | "quit"
// KEYS: A B S(start) s(select) U D L R l r
static FILE *sTestScript;
// Test aid WJPC_SCRIPT_REALTIME=1: the script only presses the buttons; the game runs through its normal
// real-time loop (frame rate setting, in-between frames, vsync, sound timing) instead of the test loop that
// runs frame by frame as fast as it can. For bugs that only show in real play (POL-11/14). No shots.
static bool sScriptRealtime;

// POL-11/14 (Brennan: the game freezes for a second or two in 3D, then crashes after about 10 minutes):
// WallysJourney_stalls.txt, next to the game, gets a line whenever one trip round the main loop takes longer
// than 150 ms, saying which part took the time, and a line a minute with the memory still free. Always on:
// it costs nothing while the game runs well, and the file is what to send when it does not.
static unsigned sStallDraws; // pictures drawn in this trip
static Uint64 sStallVDraw, sStall3D, sStall2D; // (2D: the GBA/wide picture drawn on the CPU, POL-15)
// POL-15: time spent in the SDL/GL calls of a frame that can wait for the graphics card (texture uploads, flushes,
// render-target switches) - where a full GPU queue shows up.
static Uint64 sStallGl;
/* Brennan 2026-11-21 15:33 (21i: every second LAGGY, frame 28 ms though 2D 1 and 3D 6): the picture's drawing in
 * parts, so the stall log names where the rest goes - before the 2D picture, after it (pictures converted and sent),
 * the 3D world's update, and after it (menus layer, smooth-motion layer, in-betweens). People nearby: the 3D
 * stand-ins for the neighbouring maps' people (inside the 3D update). */
static Uint64 sVdSetup, sVdAfter2D, sVd3DBlock, sVdEnd;
static unsigned g2DFieldShown; // refreshes that showed the 2D field picture with 3D on (DrawFieldScene)
extern Uint64 gVoxelStandInTicks; // voxel_entities.c
// POL-15: the main thread's own CPU time during a trip (Windows), against the clock: much less means the thread
// was waiting for a CPU (another program, power saving) rather than working.
static double StallThreadCpuMs(void)
{
#ifdef _WIN32
    FILETIME c, e, k, u;
    if (GetThreadTimes(GetCurrentThread(), &c, &e, &k, &u))
        return ((((Uint64)k.dwHighDateTime << 32) | k.dwLowDateTime) + (((Uint64)u.dwHighDateTime << 32) | u.dwLowDateTime)) / 10000.0;
#endif
    return 0.0;
}
static double sStallCpuStart;
#define STALL_GL(call) do { Uint64 stallGl0_ = SDL_GetPerformanceCounter(); call; sStallGl += SDL_GetPerformanceCounter() - stallGl0_; } while (0)
static int sStallFieldW, sStallFieldH;
extern double gVoxel3DMapMs, gVoxel3DBeltMs, gVoxel3DDrawMs;
extern unsigned gVoxel3DMeshes, gVoxel3DPages, gVoxel3DBeltLayers, gVoxel3DBeltFlushes, gVoxel3DMapBuilds;

static unsigned StallFreeAddressMb(void)
{
#ifdef _WIN32
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(ms);
    if (GlobalMemoryStatusEx(&ms))
        return (unsigned)(ms.ullAvailVirtual >> 20); // address space this (32-bit) program can still use
#endif
    return 0;
}

static void StallLog(Uint64 start, Uint64 batchEnd, Uint64 drawn, Uint64 presented)
{
    static Uint64 sFirst, sLastMinute;
    static unsigned sLines, sLoops;
    static bool sStarted;
    Uint64 freq = SDL_GetPerformanceFrequency();
    double total = (presented - start) * 1000.0 / freq;
    bool minute;
    FILE *f;

    sLoops++;
    if (!sStarted)
    {
        sStarted = true;
        sFirst = sLastMinute = start;
        f = fopen("WallysJourney_stalls.txt", "w");
        if (f != NULL)
        {
            fprintf(f, "Wally's Journey stall log. Build " __DATE__ " " __TIME__ ". Lines: a trip round the main loop over 150 ms,\n"
                       "and one line a minute. Times in ms. Send this file with a freeze or crash report.\n");
            /* Brennan 2026-11-21 10:34: "Print GL_VENDOR, GL_RENDERER, and GL_VERSION at startup so any logs from my PC
             * immediately show you my exact GPU and OpenGL driver state." */
            {
                typedef const unsigned char *(APIENTRY *GetStringFn)(unsigned int);
                GetStringFn getString = (GetStringFn)SDL_GL_GetProcAddress("glGetString");
                SDL_RendererInfo info;
                int ow = 0, oh = 0;

                if (getString != NULL)
                {
                    const unsigned char *vendor = getString(0x1F00), *renderer = getString(0x1F01), *version = getString(0x1F02);
                    fprintf(f, "GPU: %s | %s | OpenGL %s\n", vendor ? (const char *)vendor : "?",
                            renderer ? (const char *)renderer : "?", version ? (const char *)version : "?");
                }
                if (sdlRenderer != NULL && SDL_GetRendererInfo(sdlRenderer, &info) == 0)
                {
                    SDL_GetRendererOutputSize(sdlRenderer, &ow, &oh);
                    fprintf(f, "SDL renderer: %s, drawing %dx%d, anti-aliasing asked %u\n", info.name, ow, oh, sAntiAlias);
                }
                /* (Brennan 2026-11-21 16:26: "The internal battery has run dry" - the clock as the game found it at
                 * start: 0 = fine; otherwise rtc.h RTC_ERR_* bits) */
                fprintf(f, "Clock: 20%02X-%02X-%02X %02X:%02X, the game's clock check at start: %s (flags 0x%04X)\n",
                        internalClock.year, internalClock.month, internalClock.day, internalClock.hour, internalClock.minute,
                        (RtcGetErrorStatus() & RTC_ERR_FLAG_MASK) ? "FAILED" : "ok", (unsigned)RtcGetErrorStatus());
            }
            fclose(f);
        }
        return; // the first trip loads everything
    }
    minute = (presented - sLastMinute) / freq >= 60;
    /* POL-15 (Brennan 2026-11-15: after Route 110 -> Mauville "the whole game starts becoming laggy and not
     * smooth"): lag is many slow trips, not one long one. A line for every second in which more than 10 trips
     * took over 20 ms: how many, the worst, and the 3D work of the last one. */
    {
        static Uint64 sSecond;
        static unsigned sSlow, sTrips, sFlushesAt, sBuildsAt;
        static unsigned sWorstDraws;
        static double sWorstParts[5];
        static double sWorst, sWorstMaps, sWorstBelt, sWorstDraw, sWorstGame, sWorstFrame, sWorst3D, sWorstScreen, sWorstPresent, sWorst2D, sWorstGl, sWorstCpu;

        sTrips++;
        if (total > 20.0)
        {
            sSlow++;
            if (total > sWorst)
            {
                sWorst = total;
                sWorstMaps = gVoxel3DMapMs;
                sWorstBelt = gVoxel3DBeltMs;
                sWorstDraw = gVoxel3DDrawMs;
                sWorstGame = (batchEnd - start - sStallVDraw) * 1000.0 / freq;
                sWorstFrame = sStallVDraw * 1000.0 / freq;
                sWorst3D = sStall3D * 1000.0 / freq;
                sWorst2D = sStall2D * 1000.0 / freq;
                sWorstGl = sStallGl * 1000.0 / freq;
                sWorstCpu = StallThreadCpuMs() - sStallCpuStart;
                sWorstDraws = sStallDraws;
                sWorstParts[0] = sVdSetup * 1000.0 / freq;
                sWorstParts[1] = sVdAfter2D * 1000.0 / freq;
                sWorstParts[2] = sVd3DBlock * 1000.0 / freq;
                sWorstParts[3] = sVdEnd * 1000.0 / freq;
                sWorstParts[4] = gVoxelStandInTicks * 1000.0 / freq;
                sWorstScreen = (drawn - batchEnd) * 1000.0 / freq;
                sWorstPresent = (presented - drawn) * 1000.0 / freq;
            }
        }
        if (sSecond == 0)
            sSecond = presented;
        if ((presented - sSecond) >= freq)
        {
            if (sSlow > 10 && sLines <= 5000 && (f = fopen("WallysJourney_stalls.txt", "a")) != NULL)
            {
                sLines++;
                fprintf(f, "%7.1fs LAGGY  %u of %u trips over 20 ms, worst %.0f (thread CPU %.0f): game %.0f, frame %.0f (%u drawn, 2D picture %.0f at %dx%d, GL waits %.0f, 3D %.0f:"
                           " maps %.0f, belt %.0f, drawing %.0f), screen %.0f, present %.0f | map %d:%d, belt layers %u, belt rebuilds %u, maps laid %u"
                           " | frame parts: before 2D %.1f, after 2D %.1f, 3D update %.1f, after 3D %.1f, people nearby %.1f\n",
                        (presented - sFirst) / (double)freq, sSlow, sTrips, sWorst, sWorstCpu, sWorstGame, sWorstFrame, sWorstDraws, sWorst2D,
                        sStallFieldW, sStallFieldH, sWorstGl, sWorst3D,
                        sWorstMaps, sWorstBelt, sWorstDraw, sWorstScreen, sWorstPresent,
                        gSaveBlock1Ptr != NULL ? gSaveBlock1Ptr->location.mapGroup : -1,
                        gSaveBlock1Ptr != NULL ? gSaveBlock1Ptr->location.mapNum : -1,
                        gVoxel3DBeltLayers, gVoxel3DBeltFlushes - sFlushesAt, gVoxel3DMapBuilds - sBuildsAt,
                        sWorstParts[0], sWorstParts[1], sWorstParts[2], sWorstParts[3], sWorstParts[4]);
                fclose(f);
            }
            sSecond = presented;
            sSlow = sTrips = 0;
            sWorst = 0.0;
            sFlushesAt = gVoxel3DBeltFlushes;
            sBuildsAt = gVoxel3DMapBuilds;
        }
    }
    if (total < 150.0 && !minute)
        return;
    if (sLines > 5000)
        return;
    f = fopen("WallysJourney_stalls.txt", "a");
    if (f == NULL)
        return;
    sLines++;
    static unsigned sLineBuildsAt;
    fprintf(f, "%7.1fs %s %6.0f: game %.0f, frame %.0f (3D %.0f: maps %.0f, belt %.0f, drawing %.0f), screen %.0f, present %.0f"
               " | map %d:%d, 3D %d, meshes %u, pages %u, free memory %u MB, loops %u, maps laid since the last line %u, 2D field shown %u"
               " | frame parts: before 2D %.1f, after 2D %.1f, 3D update %.1f, after 3D %.1f, people nearby %.1f\n",
            (presented - sFirst) / (double)freq, total >= 150.0 ? "STALL" : "minute", total,
            (batchEnd - start - sStallVDraw) * 1000.0 / freq, sStallVDraw * 1000.0 / freq, sStall3D * 1000.0 / freq,
            gVoxel3DMapMs, gVoxel3DBeltMs, gVoxel3DDrawMs,
            (drawn - batchEnd) * 1000.0 / freq, (presented - drawn) * 1000.0 / freq,
            gSaveBlock1Ptr != NULL ? gSaveBlock1Ptr->location.mapGroup : -1,
            gSaveBlock1Ptr != NULL ? gSaveBlock1Ptr->location.mapNum : -1,
            (int)(s3DReady && s3DWanted), gVoxel3DMeshes, gVoxel3DPages, StallFreeAddressMb(), sLoops, gVoxel3DMapBuilds - sLineBuildsAt, g2DFieldShown,
            sVdSetup * 1000.0 / freq, sVdAfter2D * 1000.0 / freq, sVd3DBlock * 1000.0 / freq, sVdEnd * 1000.0 / freq,
            gVoxelStandInTicks * 1000.0 / freq);
    sLineBuildsAt = gVoxel3DMapBuilds;
    fclose(f);
    if (minute)
        sLastMinute = presented;
}
static unsigned int sTestFfSpeed; // test script "ff N": fast forward without a clock (0/1 = off)
static unsigned int sTestFfRealtime; // the same with WJPC_SCRIPT_REALTIME: the real loop's fast forward at N
static u16 sTestKeys;
static int sTestHold;
// "waitfor WHAT MAX" (test aid, Project Tandem T5 - two games linked keep in step): frames left, and what for
static int sTestWaitFor, sTestWaitForMax;
enum { WAITFOR_NONE, WAITFOR_LOCKED, WAITFOR_FREE, WAITFOR_LINK, WAITFOR_NOLINK };
static u32 sTestFrame;
static u16 *sTestLastGbaImage;
static bool sTestLockRng;
static u32 sTestLockValue[4] = {0x12345678, 0x9ABCDEF0, 0x0F1E2D3C, 1};
static u16 sTestGbaImage[(WIDE_VIEW_MAX_W + 2 * WIDE_VIEW_SMOOTH_MARGIN) * (WIDE_VIEW_MAX_H + 2 * WIDE_VIEW_SMOOTH_MARGIN)];
static u16 sTestOverlay[(WIDE_VIEW_MAX_W + 2 * WIDE_VIEW_SMOOTH_MARGIN) * (WIDE_VIEW_MAX_H + 2 * WIDE_VIEW_SMOOTH_MARGIN)];
static struct WideViewFrame sTestFrameInfo = {DISPLAY_WIDTH, DISPLAY_HEIGHT, WIDE_VIEW_GBA};

static u16 TestKeyMask(const char *keys)
{
    u16 mask = 0;
    for (; *keys; keys++)
    {
        switch (*keys)
        {
        case 'A': mask |= A_BUTTON; break;
        case 'B': mask |= B_BUTTON; break;
        case 'S': mask |= START_BUTTON; break;
        case 's': mask |= SELECT_BUTTON; break;
        case 'U': mask |= DPAD_UP; break;
        case 'D': mask |= DPAD_DOWN; break;
        case 'L': mask |= DPAD_LEFT; break;
        case 'R': mask |= DPAD_RIGHT; break;
        case 'l': mask |= L_BUTTON; break;
        case 'r': mask |= R_BUTTON; break;
        }
    }
    return mask;
}

static char sTestWinShot[512];
static double sTestWinShotT = -1.0; // "winshot FILE T": drawn t of the way between game frames (step 5 tests)
static void TestWriteShot(const char *path)
{
    FILE *f = fopen(path, "wb");
    if (f == NULL)
        return;
    int m = sTestFrameInfo.margin, stride = sTestFrameInfo.width + 2 * m;
    fprintf(f, "P6\n%d %d\n255\n", sTestFrameInfo.width, sTestFrameInfo.height);
    for (int i = 0; i < sTestFrameInfo.width * sTestFrameInfo.height; i++)
    {
        int idx = (i / sTestFrameInfo.width + m) * stride + i % sTestFrameInfo.width + m;
        u16 c = (m > 0 && (sTestOverlay[idx] & 0x8000)) ? sTestOverlay[idx] : sTestGbaImage[idx];
        u8 rgb[3] = {(c & 0x1F) << 3, ((c >> 5) & 0x1F) << 3, ((c >> 10) & 0x1F) << 3};
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
    printf("shot %s at frame %u\n", path, sTestFrame);
    fflush(stdout);
}

// Returns FALSE when the script is finished.
static bool TestStep(void)
{
    char line[512];
    if (sTestHold > 0)
    {
        sTestHold--;
        return true;
    }
    if (sTestWaitFor != WAITFOR_NONE)
    {
        bool met = false;
        switch (sTestWaitFor)
        {
        case WAITFOR_LOCKED: met = ArePlayerFieldControlsLocked(); break;
        case WAITFOR_FREE: met = TandemLink_PlayerFree(); break;
        case WAITFOR_LINK: met = gReceivedRemoteLinkPlayers != 0; break;
        case WAITFOR_NOLINK: met = gReceivedRemoteLinkPlayers == 0; break;
        }
        if (!met && --sTestWaitForMax > 0)
        {
            sTestKeys = 0;
            return true;
        }
        printf("waitfor %s at frame %u\n", met ? "met" : "timed out", sTestFrame);
        fflush(stdout);
        sTestWaitFor = WAITFOR_NONE;
    }
    sTestKeys = 0;
    while (fgets(line, sizeof(line), sTestScript))
    {
        char cmd[32] = "", arg1[256] = "";
        int n = 0;
        if (sscanf(line, "%31s %255s %d", cmd, arg1, &n) < 1 || cmd[0] == '#')
            continue;
        if (!strcmp(cmd, "wait"))
        {
            sTestHold = atoi(arg1) - 1;
            return true;
        }
        if (!strcmp(cmd, "waitfor"))
        {
            // waitfor locked|free|link|nolink MAX (test aid): until a script holds the player (a question came up) /
            // the player is free in the overworld / the game's link has the friend (the trade screen is coming) /
            // it hasn't - at most MAX frames
            sTestWaitFor = !strcmp(arg1, "locked") ? WAITFOR_LOCKED : !strcmp(arg1, "free") ? WAITFOR_FREE
                         : !strcmp(arg1, "link") ? WAITFOR_LINK : !strcmp(arg1, "nolink") ? WAITFOR_NOLINK : WAITFOR_NONE;
            sTestWaitForMax = n > 0 ? n : 3600;
            return true;
        }
        if (!strcmp(cmd, "press"))
        {
            sTestKeys = TestKeyMask(arg1);
            sTestHold = (n > 0 ? n : 1) - 1;
            return true;
        }
        if (!strcmp(cmd, "objcount"))
        {
            // objcount LABEL (test aid): people and sprites in use, and the highest slots used.
            int objs = 0, hiObj = -1, sprs = 0, hiSpr = -1, saved = 0, pl = -1;
            for (int i = 0; i < OBJECT_EVENTS_COUNT; i++)
                if (gObjectEvents[i].active)
                {
                    objs++; hiObj = i;
                    if (i < SAVED_OBJECT_EVENTS_COUNT) saved++;
                    if (gObjectEvents[i].localId == OBJ_EVENT_ID_PLAYER || gObjectEvents[i].localId == OBJ_EVENT_ID_FOLLOWER) pl = pl < i ? i : pl;
                }
            for (int i = 0; i < MAX_SPRITES; i++)
                if (gSprites[i].inUse) { sprs++; hiSpr = i; }
            FILE *f = fopen("objcount.log", "a");
            if (f != NULL)
            {
                fprintf(f, "%s frame %u objects %d (in saved 16: %d, highest slot %d, player/follower max slot %d) sprites %d (highest %d)\n",
                        arg1, sTestFrame, objs, saved, hiObj, pl, sprs, hiSpr);
                for (int i = 0; i < OBJECT_EVENTS_COUNT; i++)
                    if (gObjectEvents[i].active)
                        fprintf(f, "  slot %d: map %d:%d local %d gfx %d at %d,%d%s\n", i, gObjectEvents[i].mapGroup,
                                gObjectEvents[i].mapNum, gObjectEvents[i].localId, gObjectEvents[i].graphicsId,
                                gObjectEvents[i].currentCoords.x - 7, gObjectEvents[i].currentCoords.y - 7,
                                gObjectEvents[i].invisible ? " (invisible)" : "");
                fclose(f);
            }
            continue;
        }
        if (!strcmp(cmd, "ff"))
        {
            // ff N (test aid): fast forward N times, as the real loop does it: only every Nth
            // frame is drawn and the sound engine runs once every N frames. "ff 0" = off.
            sTestFfSpeed = atoi(arg1);
            if (sScriptRealtime) // (WJPC_SCRIPT_REALTIME: the real loop's fast forward, as the key turns it on, N times)
                sTestFfRealtime = sTestFfSpeed;
            continue;
        }
        if (!strcmp(cmd, "winshot"))
        {
            SDL_snprintf(sTestWinShot, sizeof(sTestWinShot), "%s", arg1);
            {
                char extra[32];
                sTestWinShotT = sscanf(line, "%*s %*s %31s", extra) == 1 ? atof(extra) : -1.0;
            }
            continue;
        }
        if (!strcmp(cmd, "set"))
        {
            // set SETTING VALUE (test aid), e.g. "set zoom 2"
            static const char *const names[] = {"zoom", "lockAspect", "integer", "bilinear", "resolution", "interframe", "framerate"};
            static const u8 ids[] = {PLATFORM_SETTING_ZOOM, PLATFORM_SETTING_LOCK_ASPECT, PLATFORM_SETTING_INTEGER_SCALE,
                                     PLATFORM_SETTING_SMOOTHING, PLATFORM_SETTING_RESOLUTION, PLATFORM_SETTING_INTERFRAME,
                                     PLATFORM_SETTING_SMOOTH};
            for (size_t k = 0; k < ARRAY_COUNT(names); k++)
                if (!strcmp(arg1, names[k]))
                    Platform_SetSetting(ids[k], n); // applied as the Settings menu does
            continue;
        }
        if (!strcmp(cmd, "plant"))
        {
            // plant TREE_ID STAGE (test aid, POL-49/Q11: see a planted or sprouted berry in 3D): plants berry 1 in
            // the berry tree TREE_ID (include/constants/berry.h) at that stage; warp afterwards to see it.
            int tree, stage;
            if (sscanf(line, "%*s %d %d", &tree, &stage) == 2)
                PlantBerryTree((u8)tree, 1, (u8)stage, FALSE);
            continue;
        }
        if (!strcmp(cmd, "warp"))
        {
            // warp GROUP NUM X Y (test aid)
            int g, m, x, y;
            if (sscanf(line, "%*s %d %d %d %d", &g, &m, &x, &y) == 4)
            {
                SetWarpDestination(g, m, WARP_ID_NONE, x, y);
                DoWarp();
            }
            continue;
        }
        if (!strcmp(cmd, "enemyhp"))
        {
            // enemyhp SLOT HP (test aid): set an enemy party member's HP (0 = fainted).
            int slot = atoi(arg1), hp = 0;
            sscanf(line, "%*s %*s %d", &hp);
            if (slot >= 0 && slot < PARTY_SIZE)
                SetMonData(&gEnemyParty[slot], MON_DATA_HP, &hp);
            continue;
        }
        if (!strcmp(cmd, "enemymoves"))
        {
            // enemymoves SLOT M1 M2 M3 M4 (test aid): set an enemy party member's moves (with full PP).
            int slot = atoi(arg1), m[4] = {0};
            sscanf(line, "%*s %*s %d %d %d %d", &m[0], &m[1], &m[2], &m[3]);
            if (slot >= 0 && slot < PARTY_SIZE)
                for (int k = 0; k < 4; k++)
                    SetMonMoveSlot(&gEnemyParty[slot], m[k], k);
            // and the opposing battler's live copy, if one is out (battler 1)
            {
                extern void WJPC_TestSetBattlerMoves(int battler, const int *moves);
                WJPC_TestSetBattlerMoves(1, m);
            }
            continue;
        }
        if (!strcmp(cmd, "state"))
        {
            // state WHAT ID [VALUE] (test aid, Project ARIA K4): read or write game state; state.log gets what it
            // is after. WHAT: var, flag, trainer (a trainer's "defeated" flag). No VALUE: only read.
            // "state save 0": save the game as the Save menu does.
            extern int WJPC_TestState(const char *what, int id, int set, int value);
            char what[32] = "";
            int id = 0, value = 0;
            int got = sscanf(line, "%*s %31s %i %i", what, &id, &value);
            FILE *f = fopen("state.log", "a"); // (a Windows program has no console to print to)
            int now = WJPC_TestState(what, id, got >= 3, value);
            if (f != NULL)
            {
                fprintf(f, "state %s 0x%X = %d\n", what, id, now);
                fclose(f);
            }
            continue;
        }
        if (!strcmp(cmd, "battlerhp"))
        {
            // battlerhp BATTLER HP (test aid): the HP of a Pokemon already in battle (1 = the foe).
            extern void WJPC_TestSetBattlerHp(int battler, int hp);
            int hp = 0;
            sscanf(line, "%*s %*s %d", &hp);
            WJPC_TestSetBattlerHp(atoi(arg1), hp);
            continue;
        }
        if (!strcmp(cmd, "partyswap") || !strcmp(cmd, "enemyswap"))
        {
            // partyswap A B / enemyswap A B (test aid): swap two party slots.
            struct Pokemon *party = !strcmp(cmd, "partyswap") ? gPlayerParty : gEnemyParty;
            int a = atoi(arg1), b2 = 0;
            struct Pokemon tmp;
            sscanf(line, "%*s %*s %d", &b2);
            if (a >= 0 && a < PARTY_SIZE && b2 >= 0 && b2 < PARTY_SIZE)
            {
                tmp = party[a];
                party[a] = party[b2];
                party[b2] = tmp;
            }
            continue;
        }
        if (!strcmp(cmd, "wildbattle"))
        {
            // wildbattle SPECIES LEVEL (test aid, Brennan 2026-11-21 quick run): a wild battle from the field.
            extern void ZeroEnemyPartyMons(void);
            extern void BattleSetup_StartWildBattle(void);
            int species = 0, level = 5;
            sscanf(line, "%*s %d %d", &species, &level);
            ZeroEnemyPartyMons();
            CreateMon(&gEnemyParty[0], (enum Species)species, (u8)level, 0x12345678u, OTID_STRUCT_RANDOM_NO_SHINY);
            {   /* (2026-11-22: it came in with 0 HP - fainted before the first turn) */
                u32 hp;
                CalculateMonStats(&gEnemyParty[0]);
                hp = GetMonData(&gEnemyParty[0], MON_DATA_MAX_HP);
                SetMonData(&gEnemyParty[0], MON_DATA_HP, &hp);
            }
            BattleSetup_StartWildBattle();
            continue;
        }
        if (!strcmp(cmd, "partyspecies"))
        {
            // partyspecies SLOT SPECIES (test aid, Brennan's C6 shiny Toxtricity Low Key): that party Pokémon becomes
            // SPECIES (stats worked out again).
            int slot = 0, sp = 0;
            if (sscanf(line, "%*s %d %d", &slot, &sp) == 2)
            {
                u16 s16 = (u16)sp;
                SetMonData(&gPlayerParty[slot], MON_DATA_SPECIES, &s16);
                CalculateMonStats(&gPlayerParty[slot]);
            }
            continue;
        }
        if (!strcmp(cmd, "partynature"))
        {
            // partynature SLOT NATURE (test aid, C6): the nature the summary shows (a mint's)
            int slot = 0, nat = 0;
            if (sscanf(line, "%*s %d %d", &slot, &nat) == 2)
            {
                u32 n32 = (u32)nat;
                SetMonData(&gPlayerParty[slot], MON_DATA_HIDDEN_NATURE, &n32);
            }
            continue;
        }
        if (!strcmp(cmd, "evocheck"))
        {
            // evocheck SLOT FILE (test aid, C6): writes "species personality-nature shown-nature level -> target"
            int slot = 0;
            char out[256] = "evocheck.txt";
            sscanf(line, "%*s %d %255s", &slot, out);
            FILE *ef = fopen(out, "a");
            if (ef != NULL)
            {
                struct Pokemon *m = &gPlayerParty[slot];
                fprintf(ef, "%u %u %u %u -> %u\n", (unsigned)GetMonData(m, MON_DATA_SPECIES),
                        (unsigned)(GetMonData(m, MON_DATA_PERSONALITY) % NUM_NATURES),
                        (unsigned)GetMonData(m, MON_DATA_HIDDEN_NATURE), (unsigned)GetMonData(m, MON_DATA_LEVEL),
                        (unsigned)GetEvolutionTargetSpecies(m, EVO_MODE_NORMAL, ITEM_NONE, NULL, NULL, CHECK_EVO));
                fclose(ef);
            }
            continue;
        }
        if (!strcmp(cmd, "dumptox"))
        {
            // dumptox FILE (test aid, C6): every Toxel/Toxtricity in the party and boxes: where, species, natures
            FILE *ef = fopen(arg1, "a");
            if (ef != NULL)
            {
                for (int b = -1; b < TOTAL_BOXES_COUNT; b++)
                    for (int k = 0; k < (b < 0 ? PARTY_SIZE : IN_BOX_COUNT); k++)
                    {
                        struct BoxPokemon *bm = b < 0 ? &gPlayerParty[k].box : GetBoxedMonPtr(b, k);
                        u32 sp = GetBoxMonData(bm, MON_DATA_SPECIES);
                        if (sp == SPECIES_TOXEL || sp == SPECIES_TOXTRICITY_AMPED || sp == SPECIES_TOXTRICITY_LOW_KEY)
                            fprintf(ef, "box %d slot %d species %u personality %08x nature %u shown %u shiny %u\n", b, k, sp,
                                    (unsigned)GetBoxMonData(bm, MON_DATA_PERSONALITY),
                                    (unsigned)(GetBoxMonData(bm, MON_DATA_PERSONALITY) % NUM_NATURES),
                                    (unsigned)GetBoxMonData(bm, MON_DATA_HIDDEN_NATURE), (unsigned)GetBoxMonData(bm, MON_DATA_IS_SHINY));
                    }
                fclose(ef);
            }
            continue;
        }
        if (!strcmp(cmd, "partylevel"))
        {
            // partylevel SLOT LEVEL (test aid, C6): that level, one experience point short of the next
            int slot = 0, lv = 5;
            if (sscanf(line, "%*s %d %d", &slot, &lv) == 2)
            {
                struct Pokemon *m = &gPlayerParty[slot];
                u32 exp = gExperienceTables[gSpeciesInfo[GetMonData(m, MON_DATA_SPECIES)].growthRate][lv + 1] - 1;
                SetMonData(m, MON_DATA_EXP, &exp);
                CalculateMonStats(m);
            }
            continue;
        }
        if (!strcmp(cmd, "leaflog"))
        {
            // leaflog FILE (test aid, falling leaves): the frame and every leaf's screen position
            extern u32 WallyJourney_FallingLeafLog(s16 *xy, u32 max);
            s16 xy[64];
            u32 n = WallyJourney_FallingLeafLog(xy, 32);
            FILE *lf = fopen(arg1, "a");
            if (lf != NULL)
            {
                fprintf(lf, "%u %u", sTestFrame, n);
                for (u32 k = 0; k < n; k++)
                    fprintf(lf, " %d,%d", xy[2 * k], xy[2 * k + 1]);
                fprintf(lf, "\n");
                fclose(lf);
            }
            continue;
        }
        if (!strcmp(cmd, "fixtox"))
        {
            WJ_FixToxtricityForms(); // (test aid, C6: as loading a save does)
            continue;
        }
        if (!strcmp(cmd, "partyshiny"))
        {
            // partyshiny SLOT [0|1] (test aid, Brennan 2026-11-22 quick run with a shiny): that party Pokémon shiny (or not).
            bool32 shiny = TRUE;
            int slot = 0, on = 1;
            if (sscanf(line, "%*s %d %d", &slot, &on) == 2)
                shiny = on != 0;
            SetMonData(&gPlayerParty[slot], MON_DATA_IS_SHINY, &shiny);
            continue;
        }
        if (!strcmp(cmd, "trainerbattle"))
        {
            // trainerbattle ID (test aid): start that trainer's battle from the field, as a script would.
            memset(&gTrainerBattleParameter, 0, sizeof(gTrainerBattleParameter));
            gTrainerBattleParameter.params.opponentA = (u16)atoi(arg1);
            gTrainerBattleParameter.params.opponentB = 0;
            gNoOfApproachingTrainers = 0;
            BattleSetup_StartTrainerBattle();
            continue;
        }
        if (!strcmp(cmd, "surfon"))
        {
            // Surf from the shore, as the HM does (test aid; face the water first).
            gFieldEffectArguments[0] = 0;
            LockPlayerFieldControls();
            FieldEffectStart(FLDEFF_USE_SURF);
            continue;
        }
        if (!strcmp(cmd, "flymap"))
        {
            // The Fly map, straight from the field (test aid, Project ARIA K6).
            extern void WJPC_TestOpenFlyMap(void);
            WJPC_TestOpenFlyMap();
            continue;
        }
        if (!strcmp(cmd, "fieldmove"))
        {
            // The HM "Pokémon shows itself" animation for party slot 0 (test aid).
            gFieldEffectArguments[0] = 0;
            FieldEffectStart(FLDEFF_FIELD_MOVE_SHOW_MON_INIT);
            continue;
        }
        if (!strcmp(cmd, "shot"))
        {
            TestWriteShot(arg1);
            continue;
        }
        if (!strcmp(cmd, "smoothshot"))
        {
            // Test of smooth motion (WJPC_SMOOTH_TEST=1): raw map + text layers and the camera.
            FILE *f = fopen(arg1, "wb");
            if (f != NULL)
            {
                int m = sTestFrameInfo.margin;
                int hdr[5] = {sTestFrameInfo.width, sTestFrameInfo.height, m, sTestFrameInfo.camX, sTestFrameInfo.camY};
                int count = (sTestFrameInfo.width + 2 * m) * (sTestFrameInfo.height + 2 * m);
                fwrite(hdr, sizeof(hdr), 1, f);
                fwrite(sTestGbaImage, 2, count, f);
                fwrite(sTestOverlay, 2, count, f);
                fclose(f);
            }
            continue;
        }
        if (!strcmp(cmd, "setrng") || !strcmp(cmd, "setrng2"))
        {
            // Test (GBA vs PC comparison): "setrng a b c ctr" in hex, same state as the GBA run.
            unsigned int v[4] = {0};
            if (sscanf(line, "%*s %x %x %x %x", &v[0], &v[1], &v[2], &v[3]) == 4)
            {
                rng_value_t *r = !strcmp(cmd, "setrng") ? &gRngValue : &gRng2Value;
                r->a = v[0]; r->b = v[1]; r->c = v[2]; r->ctr = v[3];
            }
            continue;
        }
        if (!strcmp(cmd, "lockrng"))
        {
            // Test (GBA vs PC comparison): "lockrng 1" = same random state at the start of every frame.
            sTestLockRng = atoi(arg1) != 0;
            {
                unsigned int v[4];
                if (sscanf(line, "%*s %*s %x %x %x %x", &v[0], &v[1], &v[2], &v[3]) == 4)
                    for (int j = 0; j < 4; j++)
                        sTestLockValue[j] = v[j];
            }
            continue;
        }
        if (!strcmp(cmd, "peekrng"))
        {
            // Test (GBA vs PC comparison): log the random-number state with a label.
            FILE *f = fopen("rng.log", "a");
            if (f != NULL)
            {
                fprintf(f, "%s %08X %08X %08X %08X frame %u\n", arg1, (unsigned)gRngValue.a, (unsigned)gRngValue.b,
                        (unsigned)gRngValue.c, (unsigned)gRngValue.ctr, sTestFrame);
                fclose(f);
            }
            continue;
        }
        if (!strcmp(cmd, "pcttest"))
        {
            // Test: RandomPercentage(t) must be true about t% of the time (10000 rolls each).
            FILE *f = fopen(arg1, "w");
            int c0 = 0, c10 = 0, c30 = 0, c50 = 0, c90 = 0, c100 = 0;
            for (int i = 0; i < 10000; i++)
            {
                c0 += RandomPercentage(RNG_NONE, 0);
                c10 += RandomPercentage(RNG_NONE, 10);
                c30 += RandomPercentage(RNG_NONE, 30);
                c50 += RandomPercentage(RNG_NONE, 50);
                c90 += RandomPercentage(RNG_NONE, 90);
                c100 += RandomPercentage(RNG_NONE, 100);
            }
            if (f != NULL)
            {
                fprintf(f, "0%%:%d 10%%:%d 30%%:%d 50%%:%d 90%%:%d 100%%:%d (of 10000)\n", c0, c10, c30, c50, c90, c100);
                fclose(f);
            }
            continue;
        }
        if (!strcmp(cmd, "divzero"))
        {
            // Test: divide by zero must give the GBA's answers (see null_guard.c) and carry on.
            volatile int a = 5, b = 0, c = -7;
            volatile unsigned int u = 9, z = 0;
            int q1 = a / b, r1 = a % b, q2 = c / b;
            unsigned int q3 = u / z, r3 = u % z;
            FILE *f = fopen(arg1, "w");
            if (f != NULL)
            {
                fprintf(f, "5/0=%d 5%%0=%d -7/0=%d 9u/0=%u 9u%%0=%u\n", q1, r1, q2, q3, r3);
                fclose(f);
            }
            continue;
        }
        if (!strcmp(cmd, "fxkey"))
        {
            // "fxkey N": as pressing F<N> (ToggleFx)
            if (atoi(arg1) >= 1 && atoi(arg1) <= 10)
                ToggleFx((unsigned)atoi(arg1) - 1);
            else if (atoi(arg1) == 12)
                ToggleFx(10);
            continue;
        }
        if (!strcmp(cmd, "typekey"))
        {
            // "typekey NAME": a keyboard key press for the Settings "Press a key" rows (SDL key name).
            SDL_Keycode key = SDL_GetKeyFromName(arg1);
            if (SDL_AtomicGet(&sKeyCapture) >= 0)
                CaptureKey(key);
            continue;
        }
        if (!strcmp(cmd, "quit"))
            return false;
    }
    return false;
}

static bool sDrewFrame;
static int sGameFramesCounted; // test aid (WJPC_FPS_LOG)
/* Brennan 2026-11-23 21:46 (4x fast forward: "the player character skips/jitters rapidly back and forth ... like the
 * player model is chasing its own shadow"): with fast forward only the last game frame of a batch is drawn, so the
 * two pictures the refreshes ease between are a whole batch apart (4 game frames at 4x on a 60 Hz screen), but the
 * easing went by the time into ONE game frame (accumulator / dt): each refresh showed anywhere from the batch's start
 * to its end - back and forth. sDrawSpan: game frames from the drawn frame before to the last one; the refreshes
 * now show a steady one game frame behind the game at any speed: (span - 1 + t) / span of the way between the two
 * pictures (span 1, normal speed: t, as before). WJPC_FF_EASE_OLD=1: as before. */
static unsigned sFramesSinceDraw, sDrawSpan = 1;
/* Brennan 2026-11-21 17:25 / 2026-11-23 06:28 (falling leaves under fast-forward): how many game frames run per
 * real 60th of a second, x256 (256 = normal speed), measured over about a quarter second - fast-forward's chosen
 * speed is only what the PC aims for (a slow PC stops a batch early). Platform_GameSpeedQ8. */
static int sSpeedFrames;
static Uint64 sSpeedStamp;
static u32 sGameSpeedQ8 = 256;

// ---- Clean scaling / 4K / screen filters ------------------------------------------
static SDL_Texture *sGameTarget;   // the game at a whole-number size
static SDL_Texture *sFilterTexture; // scanlines / LCD grid for that size
static int sGameTargetW, sGameTargetH, sFilterKind = -1, sFilterScale;
static bool sGameTargetOk;

static bool PrepareGameTarget(int w, int h, int k)
{
    int filter = sPlatformSettings[PLATFORM_SETTING_FILTER];
    if (sGameTarget == NULL || w != sGameTargetW || h != sGameTargetH)
    {
        if (sGameTarget != NULL)
            SDL_DestroyTexture(sGameTarget);
        sGameTarget = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, w, h);
        if (sGameTarget == NULL)
        {
            sGameTargetOk = false; // no render targets here: draw straight to the window
            return false;
        }
        sGameTargetW = w;
        sGameTargetH = h;
        sFilterKind = -1;
    }
    if (filter && (filter != sFilterKind || k != sFilterScale || sFilterTexture == NULL))
    {
        // One game pixel is k x k screen pixels: darken its last row (scanlines), or its last
        // row and column (LCD grid). Only drawn when a pixel is at least 3 screen pixels.
        uint32_t *pixels = malloc((size_t)w * h * 4);
        if (sFilterTexture != NULL)
            SDL_DestroyTexture(sFilterTexture);
        sFilterTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
        if (pixels != NULL && sFilterTexture != NULL)
        {
            int line = k >= 6 ? 2 : 1;
            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                {
                    bool row = k >= 3 && (y % k) >= k - line;
                    bool col = k >= 3 && filter == 2 && (x % k) >= k - line;
                    uint32_t a = row ? (filter == 1 ? 110 : 70) : col ? 70 : 0;
                    pixels[y * w + x] = a << 24;
                }
            SDL_UpdateTexture(sFilterTexture, NULL, pixels, w * 4);
            SDL_SetTextureBlendMode(sFilterTexture, SDL_BLENDMODE_BLEND);
        }
        free(pixels);
        sFilterKind = filter;
        sFilterScale = k;
    }
    return true;
}

// ---- Zoom (overworld, like gen1recomp) ----------------------------------------------
// Each zoom level specifies a target view height, with nearest-neighbour sampling to
// preserve hard pixel edges even at fractional output scale. The view fills the screen,
// within the PC wide-view buffer capacity. Keys - and =, or the mouse wheel.
static const u16 sZoomHeights[] = {120, 144, 162, 180, 216, 240, 270, 324};
#define ZOOM_COUNT ARRAY_COUNT(sZoomHeights)
static float sFieldZ = 1.0f;
static int sFieldViewW = DISPLAY_WIDTH, sFieldViewH = DISPLAY_HEIGHT;

static void ComputeFieldView(int outW, int outH)
{
    int level = sPlatformSettings[PLATFORM_SETTING_ZOOM] % ZOOM_COUNT;
    int target;
    float z, minZ;
    if (WideView_IsIndoorMap() && level > 0)
        level--; // rooms are small: one step closer
    target = sZoomHeights[level];

    // The old integer-only zoom rounded several adjacent settings to the exact
    // same view (e.g. heights 216 and 240 at 1080p both used 5x). On PC a
    // fractional nearest-neighbour scale is better than a setting that does
    // nothing. The game is still drawn from unfiltered, sharp source pixels.
    z = (float)outH / target;
    minZ = (float)outW / WIDE_VIEW_MAX_W;
    if ((float)outH / WIDE_VIEW_MAX_H > minZ)
        minZ = (float)outH / WIDE_VIEW_MAX_H;
    if (z < minZ)
        z = minZ; // never ask WideView to render past its allocated buffers
    if (z <= 0.0f)
        z = 1.0f;
    sFieldZ = z;
    sFieldViewW = (int)ceilf((float)outW / z);
    sFieldViewH = (int)ceilf((float)outH / z);
    if (sFieldViewW > WIDE_VIEW_MAX_W)
        sFieldViewW = WIDE_VIEW_MAX_W;
    if (sFieldViewH > WIDE_VIEW_MAX_H)
        sFieldViewH = WIDE_VIEW_MAX_H;
}

static void ZoomStep(int delta)
{
    int z = (int)sPlatformSettings[PLATFORM_SETTING_ZOOM] + delta;
    if (z < 0)
        z = 0;
    if (z >= (int)ZOOM_COUNT)
        z = ZOOM_COUNT - 1;
    Platform_SetSetting(PLATFORM_SETTING_ZOOM, z);
}

static const int sResolutions[][2] = {{0, 0}, {1280, 720}, {1920, 1080}, {2560, 1440}, {3840, 2160}};
#define RESOLUTION_COUNT ARRAY_COUNT(sResolutions)
static SDL_Texture *sResTarget;
static int sResTargetW, sResTargetH;
static SDL_Texture *sOuterTarget; // where the scene ends up (NULL = the window)

// Size the scene is drawn at: the window, or the chosen resolution.
static void GetSceneSize(int *w, int *h)
{
    int r = sPlatformSettings[PLATFORM_SETTING_RESOLUTION] % RESOLUTION_COUNT;
    if (r != 0)
    {
        *w = sResolutions[r][0];
        *h = sResolutions[r][1];
        return;
    }
    SDL_GetRendererOutputSize(sdlRenderer, w, h);
}

extern u8 GetMapNamePopUpWindowId(void);
extern bool32 Vol_StartMenu_IsOpen(void);
extern bool32 Vol_StartMenu_IsSwshOpen(void);

// Overworld: the map at the zoom scale, edge to edge; the UI layer (text boxes, menus) at the
// biggest whole size the GBA screen fits, bottom-centred like on the GBA; the map name pop-up
// always in the top-left corner of the screen.
/* Brennan 2026-11-21 17:42 (21k: the small trees' bases "flickering/strobing when moving" on every route; they share
 * no height with the ground - checked on Routes 110, 112, 117, Verdanturf): his "resolution" setting draws the
 * picture at 3840x2160 and the window is 1280x720. The plain linear shrink reads 4 of every 9 pixels, so a thin dark
 * edge (a tree base's outline) is read in one frame and missed in the next as the camera slides - strobing. Shrunk
 * more than 1.5 times, the picture is now averaged 2x2 first (its half-size mipmap), then shrunk linearly, so every
 * pixel counts.
 * Brennan 2026-11-21 17:54: "i do not like the softer look, use old sharpness" - off again; WJPC_RES_AVERAGE=1 turns
 * it on (tests). */
static void ResTargetAverage(SDL_Texture *texture, int texW, int outW)
{
    typedef void (APIENTRY *GenMipFn)(unsigned int);
    typedef void (APIENTRY *TexParamFn)(unsigned int, unsigned int, int);
    static GenMipFn sGenMip;
    static TexParamFn sTexParam;
    static int sState = -1; // -1 not looked, 0 off, 1 on
    float tw, th;

    if (sState < 0)
    {
        SDL_RendererInfo info;
        sState = 0;
        if (getenv("WJPC_RES_AVERAGE") != NULL && SDL_GetRendererInfo(sdlRenderer, &info) == 0
         && !strcmp(info.name, "opengl"))
        {
            sGenMip = (GenMipFn)SDL_GL_GetProcAddress("glGenerateMipmap");
            sTexParam = (TexParamFn)SDL_GL_GetProcAddress("glTexParameteri");
            sState = sGenMip != NULL && sTexParam != NULL;
        }
    }
    if (sState != 1 || outW <= 0 || texW * 2 < outW * 3)
        return;
    SDL_RenderFlush(sdlRenderer); // (the picture drawn into it is finished first)
    if (SDL_GL_BindTexture(texture, &tw, &th) != 0)
        return;
    /* only the half-size level (every 2x2 averaged), read linearly: every pixel counts, without the trilinear blur
     * of a smaller level (21l test: trilinear at 3x was visibly soft) */
    sTexParam(0x0DE1 /* GL_TEXTURE_2D */, 0x813D /* GL_TEXTURE_MAX_LEVEL */, 1);
    sGenMip(0x0DE1);
    sTexParam(0x0DE1, 0x2801 /* GL_TEXTURE_MIN_FILTER */, 0x2701 /* GL_LINEAR_MIPMAP_NEAREST */);
    SDL_GL_UnbindTexture(texture);
}

static void DrawFieldScene(double t, int outW, int outH)
{
    float z = sFieldZ;
    int m = sFrameInfo.margin;
    int cw = sFrameInfo.width, ch = sFrameInfo.height;
    float shiftX = 0, shiftY = 0;
    if (m > 0 && sCamMoveValid && sPlatformSettings[PLATFORM_SETTING_SMOOTH])
    {
        shiftX = (float)((1.0 - t) * (sCurCamX - sPrevCamX));
        shiftY = (float)((1.0 - t) * (sCurCamY - sPrevCamY));
    }
    float baseX = outW / 2.0f - (cw / 2.0f + m) * z, baseY = outH / 2.0f - (ch / 2.0f + m) * z;
    SDL_Rect full = {0, 0, cw + 2 * m, ch + 2 * m};
    SDL_FRect mapDest = {baseX + shiftX * z, baseY + shiftY * z, (float)full.w * z, (float)full.h * z};
    SDL_FRect stillDest = {baseX, baseY, (float)full.w * z, (float)full.h * z};
    if (s3DFrame && s3DEveryRefresh && sdl3DTexture != NULL)
    {
        // step 5: the world drawn for this refresh, t of the way from the game frame before to the last one
        SDL_RenderFlush(sdlRenderer);
        unsigned id = Voxel3D_TextureId(sdl3DTexture);
        if (id != 0)
        {
            SDL_Texture *was = SDL_GetRenderTarget(sdlRenderer);
            Uint64 r0 = SDL_GetPerformanceCounter();
            Voxel3D_Redraw(id, s3DW, s3DH, (float)t);
            sStall3D += SDL_GetPerformanceCounter() - r0;
            // SDL caches its GL state: switching targets makes it send viewport and target again.
            SDL_SetRenderTarget(sdlRenderer, sdl3DTexture);
            SDL_SetRenderTarget(sdlRenderer, was);
        }
    }
    if (s3DFrame)
    {
        // The 3D world fills the screen; the UI layer goes over it below.
        SDL_Rect whole = {0, 0, outW, outH};
        SDL_RenderCopy(sdlRenderer, sdl3DTexture, NULL, &whole);
    }
    else
    {
    /* (Brennan 2026-11-21 17:54, Route 111: "the flat 2D sprite version of those trees is popping on and off": the
     * stall log counts the refreshes that show the 2D field picture while 3D is on) */
    if (s3DReady && s3DWanted)
        g2DFieldShown++;
    SDL_SetTextureScaleMode(sdlTexture, SDL_ScaleModeNearest);
    SDL_RenderCopyF(sdlRenderer, sdlTexture, &full, &mapDest);
    if (m > 0)
    {
        SDL_SetTextureScaleMode(sdlOverlayTexture, SDL_ScaleModeNearest);
        SDL_RenderCopyF(sdlRenderer, sdlOverlayTexture, &full, &stillDest);
    }
    }
    if (sdlUiTexture != NULL && !s2DTransition)
    {
        // Keep the usual largest whole-pixel UI scale for normal window sizes.
        // If the effective output target is smaller than 240x160, downscale
        // instead of forcing 1x (which draws part of the start menu offscreen).
        float scaleLimitW = (float)outW / DISPLAY_WIDTH;
        float scaleLimitH = (float)outH / DISPLAY_HEIGHT;
        float scaleLimit = scaleLimitW < scaleLimitH ? scaleLimitW : scaleLimitH;
        float s = floorf(scaleLimit);
        if (s < 1.0f)
            s = scaleLimit;
        if (s <= 0.0f)
            s = 1.0f;
        // Start menu open: the UI slides right so the menu is flush with the screen's right edge.
        int ux = (int)(Vol_StartMenu_IsOpen() ? outW - DISPLAY_WIDTH * s : (outW - DISPLAY_WIDTH * s) / 2);
        int uy = (int)(outH - DISPLAY_HEIGHT * s);
        int band = 0;
        if (GetMapNamePopUpWindowId() != 0xFF)
        {
            // Map name pop-up: the top 5 tile rows of the UI go to the screen's top-left corner.
            band = 40;
            SDL_Rect src = {sFrameInfo.gbaX, sFrameInfo.gbaY, DISPLAY_WIDTH, band};
            SDL_Rect dst = {0, 0, DISPLAY_WIDTH * s, band * s};
            SDL_RenderCopy(sdlRenderer, sdlUiTexture, &src, &dst);
        }
        if (FieldEffectActiveListContains(FLDEFF_FIELD_MOVE_SHOW_MON))
        {
            // HM "Pokémon shows itself" banner: its rows run the whole width of the screen.
            int top = (REG_WIN0V >> 8) & 0xFF, bottom = REG_WIN0V & 0xFF;
            if (bottom > DISPLAY_HEIGHT || bottom < top)
                bottom = DISPLAY_HEIGHT;
            int extra = (outW - DISPLAY_WIDTH * s + s - 1) / s / 2 + 1;
            if (extra > sFrameInfo.gbaX)
                extra = sFrameInfo.gbaX;
            SDL_Rect bsrc = {sFrameInfo.gbaX - extra, sFrameInfo.gbaY + top, DISPLAY_WIDTH + 2 * extra, bottom - top};
            SDL_Rect bdst = {ux - extra * s, uy + top * s, bsrc.w * s, bsrc.h * s};
            SDL_RenderCopy(sdlRenderer, sdlUiTexture, &bsrc, &bdst);
            if (bdst.x > 0)
            {
                // Zoomed far out: past the drawn picture, carry on its edge column.
                SDL_Rect es = {bsrc.x, bsrc.y, 1, bsrc.h}, ed = {0, bdst.y, bdst.x, bdst.h};
                SDL_RenderCopy(sdlRenderer, sdlUiTexture, &es, &ed);
                es.x = bsrc.x + bsrc.w - 1;
                ed.x = bdst.x + bdst.w;
                ed.w = outW - ed.x;
                SDL_RenderCopy(sdlRenderer, sdlUiTexture, &es, &ed);
            }
        }
        SDL_Rect src = {sFrameInfo.gbaX, sFrameInfo.gbaY + band, DISPLAY_WIDTH, DISPLAY_HEIGHT - band};
        SDL_Rect dst = {ux, uy + band * s, DISPLAY_WIDTH * s, (DISPLAY_HEIGHT - band) * s};
        SDL_RenderCopy(sdlRenderer, sdlUiTexture, &src, &dst);
        if (ux > 0 && Vol_StartMenu_IsSwshOpen())
        {
            // POL-6 (Brennan 2026-11-15): the Sword/Shield start menu's bottom bar runs from the screen's
            // left edge, not from where the GBA screen starts: its first column (GBA rows 124-159, the bar's
            // edge lines and green, no text) carries on to the left.
            enum { BAR_TOP = 124 };
            SDL_Rect bs = {sFrameInfo.gbaX, sFrameInfo.gbaY + BAR_TOP, 1, DISPLAY_HEIGHT - BAR_TOP};
            SDL_Rect bd = {0, uy + (int)(BAR_TOP * s), ux, (int)((DISPLAY_HEIGHT - BAR_TOP) * s)};
            SDL_RenderCopy(sdlRenderer, sdlUiTexture, &bs, &bd);
        }
        if (uy > 0 && Vol_StartMenu_IsSwshOpen())
        {
            // ...and its right-hand column runs from the screen's top: the GBA screen's first row (the
            // column's border and green, no icons; see-through elsewhere) carries on upwards.
            SDL_Rect cs = {sFrameInfo.gbaX, sFrameInfo.gbaY, DISPLAY_WIDTH, 1};
            SDL_Rect cd = {ux, 0, (int)(DISPLAY_WIDTH * s), uy};
            SDL_RenderCopy(sdlRenderer, sdlUiTexture, &cs, &cd);
        }
        sUiPlaceX = ux;
        sUiPlaceY = uy;
        sUiPlaceS = s3DFrame ? s : 0;
        sUiOutW = outW;
        sUiOutH = outH;
    }
}

/*
 * DLX-24 (Emerald 3DS 3ds_video.c PlaceScreenObject): a healing-machine sprite at GBA (x, y), w x h, moved to
 * where the camera sees the machine's tile. Uses last frame's UI placement; false outside the 3D field.
 */
bool Pc_MachineSpriteSpot(int x, int y, int w, int h, int *outX, int *outY)
{
    float fx, fy;
    if (!s3DFrame || sUiPlaceS <= 0)
        return false;
    if (!Voxel3D_ProjectPictureTile((x + w / 2.0f - 120.0f) / 16.0f, (y + h / 2.0f - 80.0f) / 16.0f, &fx, &fy))
        return false;
    *outX = (int)((fx * sUiOutW - sUiPlaceX) / sUiPlaceS - w / 2.0f);
    *outY = (int)((fy * sUiOutH - sUiPlaceY) / sUiPlaceS - h / 2.0f);
    return true;
}

static void DrawScene(double t, int outputWidth, int outputHeight)
{
#if defined(NATIVE_LINUX) || defined(_WIN32)
    if (s3DTransition && s3DFrame)
    {
        // A battle transition over the 3D world: the GBA screen's picture covers the screen
        // (cropped top and bottom, as the screen is wider than the GBA's).
        SDL_Rect whole = {0, 0, outputWidth, outputHeight};
        int h = outputWidth * DISPLAY_HEIGHT / DISPLAY_WIDTH, w = outputWidth;
        if (h < outputHeight)
        {
            h = outputHeight;
            w = outputHeight * DISPLAY_WIDTH / DISPLAY_HEIGHT;
        }
        SDL_Rect cover = {(outputWidth - w) / 2, (outputHeight - h) / 2, w, h};
        {
            // Emerald 3DS (RenderTransition / TransitionBandAt): the world moved line by line as the
            // transition moves the field's layers (BG1's scroll against line 0's), in bands of lines that
            // move alike; its brightness is the overlay's. Where a band moves off, the clear (black) shows.
            extern s16 gPcLineScrollX[], gPcLineScrollY[];
            int tw = 0, th = 0;
            float k = (float)w / DISPLAY_WIDTH;
            SDL_QueryTexture(sdl3DTexture, NULL, NULL, &tw, &th);
            SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
            SDL_RenderFillRect(sdlRenderer, &whole);
            for (int g = 0; g < DISPLAY_HEIGHT;)
            {
                int dx = (s16)(gPcLineScrollX[g] - gPcLineScrollX[0]), dy = (s16)(gPcLineScrollY[g] - gPcLineScrollY[0]);
                int e = g + 1;
                while (e < DISPLAY_HEIGHT && (s16)(gPcLineScrollX[e] - gPcLineScrollX[0]) == dx
                       && (s16)(gPcLineScrollY[e] - gPcLineScrollY[0]) == dy)
                    e++;
                {
                    int y0 = cover.y + (int)floorf(g * k), y1 = cover.y + (int)floorf(e * k);
                    if (g == 0) y0 = 0;
                    if (e == DISPLAY_HEIGHT) y1 = outputHeight;
                    // the line shows the field from (x + dx, y + dy): the picture moves the other way
                    SDL_Rect src = {(int)(dx * k), y0 + (int)(dy * k), outputWidth, y1 - y0};
                    SDL_Rect dst = {0, y0, outputWidth, y1 - y0};
                    if (src.x < 0) { dst.x -= src.x; dst.w += src.x; src.w += src.x; src.x = 0; }
                    if (src.x + src.w > tw) { src.w = tw - src.x; dst.w = src.w; }
                    if (src.y < 0) { dst.y -= src.y; dst.h += src.y; src.h += src.y; src.y = 0; }
                    if (src.y + src.h > th) { src.h = th - src.y; dst.h = src.h; }
                    if (src.w > 0 && src.h > 0)
                        SDL_RenderCopy(sdlRenderer, sdl3DTexture, &src, &dst);
                }
                g = e;
            }
        }
        if (sdl3DTransTexture != NULL)
        {
            SDL_SetTextureScaleMode(sdl3DTransTexture, SDL_ScaleModeNearest);
            SDL_RenderCopy(sdlRenderer, sdl3DTransTexture, NULL, &cover);
        }
        return;
    }
    if (s2DTransition)
    {
        // DLX-4: the field as it was when the transition began, then the transition over it, covering
        // the screen as in 3D (the map's own pixels in it are see-through, carrying only its fade).
        struct WideViewFrame now = sFrameInfo;
        int h = outputWidth * DISPLAY_HEIGHT / DISPLAY_WIDTH, w = outputWidth;
        sFrameInfo = sLastField;
        DrawFieldScene(1.0, outputWidth, outputHeight);
        sFrameInfo = now;
        if (h < outputHeight)
        {
            h = outputHeight;
            w = outputHeight * DISPLAY_WIDTH / DISPLAY_HEIGHT;
        }
        SDL_Rect cover = {(outputWidth - w) / 2, (outputHeight - h) / 2, w, h};
        if (sdl3DTransTexture != NULL)
        {
            SDL_SetTextureScaleMode(sdl3DTransTexture, SDL_ScaleModeNearest);
            SDL_RenderCopy(sdlRenderer, sdl3DTransTexture, NULL, &cover);
        }
        return;
    }
    if (sFrameInfo.mode == WIDE_VIEW_FIELD)
    {
        DrawFieldScene(t, outputWidth, outputHeight);
        return;
    }
    u8 backgroundOption = Platform_GetBorderBackground();
    if (backgroundOption < sBorderBackgroundCount
     && sdlBackgroundTextures[backgroundOption] != NULL)
        SDL_RenderCopy(sdlRenderer, sdlBackgroundTextures[backgroundOption], NULL, NULL);
    int gameHeight;
    int gameWidth;
    int srcX = 0, srcY = 0;
    int srcW = sFrameInfo.width, srcH = sFrameInfo.height;
    bool integer = sPlatformSettings[PLATFORM_SETTING_INTEGER_SCALE];
    bool lockAspect = sPlatformSettings[PLATFORM_SETTING_LOCK_ASPECT];
    if (sFrameInfo.mode == WIDE_VIEW_CENTER)
    {
        // Transitions: just the GBA picture in the middle of the canvas.
        srcX = (srcW - DISPLAY_WIDTH) / 2;
        srcY = (srcH - DISPLAY_HEIGHT) / 2;
        srcW = DISPLAY_WIDTH;
        srcH = DISPLAY_HEIGHT;
    }
    // Same rules as mGBA: "Lock aspect ratio" keeps the shape (off = stretch each way to the
    // screen); "Force integer scaling" rounds the size down to whole steps.
    if (lockAspect)
    {
        gameWidth = outputWidth;
        if (gameWidth > outputHeight * srcW / srcH)
            gameWidth = outputHeight * srcW / srcH;
        gameHeight = gameWidth * srcH / srcW;
        if (integer)
        {
            int scale = gameWidth / srcW;
            // There is no whole-pixel scale that fits a target smaller than
            // the native picture. Preserve the fitted fractional viewport
            // rather than clipping the image by forcing a 1x scale.
            if (scale >= 1)
            {
                gameWidth = srcW * scale;
                gameHeight = srcH * scale;
            }
        }
    }
    else
    {
        gameWidth = outputWidth;
        gameHeight = outputHeight;
        if (integer)
        {
            int sx = outputWidth / srcW;
            int sy = outputHeight / srcH;
            if (sx >= 1 && sy >= 1)
            {
                gameWidth = srcW * sx;
                gameHeight = srcH * sy;
            }
        }
    }
    SDL_Rect gameViewport = {(outputWidth - gameWidth) / 2,
                             (outputHeight - gameHeight) / 2,
                             gameWidth, gameHeight};
    // Draw the game at a whole-number size first (sharp pixels), add the screen filter, then
    // stretch only that last bit to the screen (smoothed only with Bilinear filtering on).
    SDL_Rect finalViewport = gameViewport;
    bool useTarget = sGameTargetOk;
    if (useTarget)
    {
        int k = (gameWidth + srcW - 1) / srcW;
        if ((gameHeight + srcH - 1) / srcH > k)
            k = (gameHeight + srcH - 1) / srcH;
        if (k < 1)
            k = 1;
        if (!PrepareGameTarget(srcW * k, srcH * k, k))
            useTarget = false;
        else
        {
            SDL_SetRenderTarget(sdlRenderer, sGameTarget);
            SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
            SDL_RenderClear(sdlRenderer);
            gameViewport = (SDL_Rect){0, 0, srcW * k, srcH * k};
        }
    }
    {
        SDL_Rect gameSource = {srcX, srcY, srcW, srcH};
        if (s3DBattleFrame && s3DFrame)
        {
            // DLX-5: the 3D world, then the battle without its scenery on top.
            SDL_RenderCopy(sdlRenderer, sdl3DTexture, NULL, &gameViewport);
            SDL_SetTextureScaleMode(sdl3DBattleTexture, SDL_ScaleModeNearest);
            SDL_RenderCopy(sdlRenderer, sdl3DBattleTexture, &gameSource, &gameViewport);
        }
        else
        {
        SDL_SetTextureScaleMode(sdlTexture, SDL_ScaleModeNearest);
        SDL_RenderCopy(sdlRenderer, sdlTexture, &gameSource, &gameViewport);
        }
    }
    if (useTarget)
    {
        if (sPlatformSettings[PLATFORM_SETTING_FILTER] && sFilterTexture != NULL)
            SDL_RenderCopy(sdlRenderer, sFilterTexture, NULL, &gameViewport);
        SDL_SetRenderTarget(sdlRenderer, sOuterTarget);
        gameViewport = finalViewport;
        bool whole = gameViewport.w % srcW == 0 && gameViewport.h % srcH == 0;
        SDL_SetTextureScaleMode(sGameTarget, (whole || !sPlatformSettings[PLATFORM_SETTING_SMOOTHING])
                                             ? SDL_ScaleModeNearest : SDL_ScaleModeLinear);
        SDL_RenderCopy(sdlRenderer, sGameTarget, NULL, &gameViewport);
    }
    if (sFrameInfo.mode == WIDE_VIEW_GBA && lockAspect && sPlatformSettings[PLATFORM_SETTING_BORDER] && sdlBorderTexture != NULL)
    {
        SDL_Rect borderSource = {141, 18, 1000, 683};
        int innerWidth = gameViewport.w - 2;
        int innerHeight = gameViewport.h - 2;
        SDL_Rect borderViewport = {
            gameViewport.x + 1 - innerWidth * 19 / 961,
            gameViewport.y + 1 - innerHeight * 20 / 643,
            innerWidth * 1000 / 961,
            innerHeight * 683 / 643
        };
        SDL_RenderCopy(sdlRenderer, sdlBorderTexture, &borderSource, &borderViewport);
    }
#else
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
#endif
}

static void DrawScreen(double t)
{
    int winW, winH, w, h;
    if (sInterpCount > 0)
    {
        // DLX-3: between game frames show the in-between pictures, then the frame itself
        int idx = (int)(t * sInterpSteps);
        int want = idx < sInterpCount ? idx : sInterpCount;
        if (want != sInterpShown)
        {
            SDL_Rect a = {0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT};
            SDL_Texture *dst = sInterpBattle ? sdl3DBattleTexture : sdlTexture;
            if (dst != NULL)
                SDL_UpdateTexture(dst, &a, want < sInterpCount ? sInterpImage[want] : sInterpCurrent,
                                  DISPLAY_WIDTH * sizeof(Uint32));
            sInterpShown = want;
        }
    }
    SDL_GetRendererOutputSize(sdlRenderer, &winW, &winH);
    GetSceneSize(&w, &h);
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    sOuterTarget = NULL;
    if (w != winW || h != winH)
    {
        // Resolution setting: draw the scene at that size, then fit it to the window.
        if (sResTarget == NULL || sResTargetW != w || sResTargetH != h)
        {
            if (sResTarget != NULL)
                SDL_DestroyTexture(sResTarget);
            sResTarget = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, w, h);
            sResTargetW = w;
            sResTargetH = h;
        }
        if (sResTarget != NULL)
        {
            sOuterTarget = sResTarget;
            SDL_SetRenderTarget(sdlRenderer, sResTarget);
            SDL_RenderClear(sdlRenderer);
            DrawScene(t, w, h);
            SDL_SetRenderTarget(sdlRenderer, NULL);
            sOuterTarget = NULL;
            SDL_RenderClear(sdlRenderer);
            int fw = winW, fh = winW * h / w;
            if (fh > winH)
            {
                fh = winH;
                fw = winH * w / h;
            }
            SDL_Rect dst = {(winW - fw) / 2, (winH - fh) / 2, fw, fh};
            SDL_SetTextureScaleMode(sResTarget, SDL_ScaleModeLinear);
            ResTargetAverage(sResTarget, w, fw);
            SDL_RenderCopy(sdlRenderer, sResTarget, NULL, &dst);
            return;
        }
    }
    SDL_RenderClear(sdlRenderer);
    DrawScene(t, winW, winH);
#ifdef __ANDROID__
    SDL_RenderPresent(sdlRenderer);
#endif
}

#ifdef _WIN32
/* Test aid (session 014PwRDj, Brennan 2026-11-20 20:15 belt stalls): WJPC_PROFILE=1 - a sampling profiler. A thread
 * looks at where the main thread is every millisecond while gVoxelProfileOn is set (the belt's building) and writes
 * the addresses seen to WallysJourney_profile.txt at the end (count, address; llvm-addr2line names them). */
volatile int gVoxelProfileOn;
static HANDLE sProfMain;
static unsigned sProfAddr[1 << 16], sProfCount[1 << 16];
static DWORD WINAPI ProfThread(LPVOID arg)
{
    (void)arg;
    for (;;)
    {
        Sleep(1);
        if (!gVoxelProfileOn)
            continue;
        if (SuspendThread(sProfMain) == (DWORD)-1)
            continue;
        CONTEXT c;
        c.ContextFlags = CONTEXT_CONTROL;
        if (GetThreadContext(sProfMain, &c))
        {
            unsigned a = (unsigned)c.Eip, h = (a * 2654435761u) >> 16;
            while (sProfAddr[h] != 0 && sProfAddr[h] != a)
                h = (h + 1) & 0xFFFF;
            sProfAddr[h] = a;
            sProfCount[h]++;
        }
        ResumeThread(sProfMain);
    }
    return 0;
}
static void ProfDump(void)
{
    FILE *f = fopen("WallysJourney_profile.txt", "w");
    if (f == NULL)
        return;
    for (unsigned i = 0; i < 0x10000; i++)
        if (sProfCount[i])
            fprintf(f, "%u %08x\n", sProfCount[i], sProfAddr[i]);
    fclose(f);
}
static void ProfStart(void)
{
    if (getenv("WJPC_PROFILE") == NULL)
        return;
    DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &sProfMain, 0, FALSE, DUPLICATE_SAME_ACCESS);
    CreateThread(NULL, 0, ProfThread, NULL, 0, NULL);
    atexit(ProfDump);
}
#else
volatile int gVoxelProfileOn;
static void ProfStart(void) {}
#endif

int main(int argc, char **argv)
{
    ProfStart();
    // Open an output console on Windows
#ifdef _WIN32
    if (getenv("WJPC_CONSOLE") || getenv("WJPC_SCRIPT"))
    {
        AllocConsole() ;
        AttachConsole( GetCurrentProcessId() ) ;
        freopen( "CON", "w", stdout ) ;
    }
#endif

#ifdef __ANDROID__
    SDL_setenv("SDL_AUDIODRIVER", "openslES", 1);
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
#endif
    // 4K screens: tell Windows the game scales itself, so it isn't stretched (blurry) at 150%/200%.
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO
#ifdef __ANDROID__
                | SDL_INIT_GAMECONTROLLER
#endif
                ) < 0)
    {
        DBGPRINTF("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

#ifdef __ANDROID__
    for (int i = 0; i < SDL_NumJoysticks() && androidController == NULL; i++)
    {
        if (SDL_IsGameController(i))
            androidController = SDL_GameControllerOpen(i);
    }
#endif

#ifdef __ANDROID__
    char *prefPath = SDL_GetPrefPath("pokeemerald", "pokeemerald");
    if (prefPath != NULL)
    {
        SDL_snprintf(sSavePath, sizeof(sSavePath), "%spokeemerald.sav", prefPath);
        SDL_snprintf(sConfigPath, sizeof(sConfigPath), "%spokeemerald.cfg", prefPath);
        SDL_free(prefPath);
    }
#endif
    PcGuard_Init();
    if (getenv("WJPC_SAVE"))
        SDL_snprintf(sSavePath, sizeof(sSavePath), "%s", getenv("WJPC_SAVE"));
    if (getenv("WJPC_SCRIPT"))
        sTestScript = fopen(getenv("WJPC_SCRIPT"), "r");
        sScriptRealtime = sTestScript != NULL && getenv("WJPC_SCRIPT_REALTIME") != NULL;
    ReadSaveFile(sSavePath);
    ReadConfigFile();

#ifdef __ANDROID__
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif
#if defined(NATIVE_LINUX) || defined(_WIN32)
    {
        /* Test aid: WJPC_WINDOW=WxH opens the window at that size (a later SDL_SetWindowSize left the test machine's
         * X server - no window manager - drawing into the old 1280x720 corner; Brennan 2026-11-21 10:34 asked for
         * 1080p test pictures) */
        int ww = 1280, wh = 720;
        if (getenv("WJPC_WINDOW") && sscanf(getenv("WJPC_WINDOW"), "%dx%d", &ww, &wh) != 2)
            ww = 1280, wh = 720;
        sdlWindow = SDL_CreateWindow("Pokemon Wally's Journey", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ww, wh, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    }
#else
    sdlWindow = SDL_CreateWindow("pokeemerald", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DISPLAY_WIDTH * videoScale, DISPLAY_HEIGHT * videoScale, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
#endif
    if (sdlWindow == NULL)
    {
        DBGPRINTF("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

#ifdef __ANDROID__
    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
#else
    if (getenv("WJPC_3D") != NULL)
        s3DWanted = atoi(getenv("WJPC_3D")) != 0;
    if (getenv("WJPC_3D_GAMMALIGHT") != NULL)
        sGammaWanted = atoi(getenv("WJPC_3D_GAMMALIGHT")) != 0;
    {
        // The 3D models (voxel/*.bin, *.rgba5551) live in a "voxel" folder next to the game.
        const char *env = getenv("WJPC_VOXEL_DIR");
        char probe[640];
        FILE *f;
        if (env != NULL)
            snprintf(s3DDir, sizeof(s3DDir), "%s", env);
        else
        {
            char *base = SDL_GetBasePath();
            snprintf(s3DDir, sizeof(s3DDir), "%svoxel", base != NULL ? base : "");
            SDL_free(base);
        }
        snprintf(probe, sizeof(probe), "%s/regions.bin", s3DDir);
        f = fopen(probe, "rb");
        s3DAvailable = f != NULL;
        if (f != NULL)
            fclose(f);
    }
    if (s3DAvailable || s3DWanted)
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl"); // the 3D world draws with OpenGL in the same context
    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_PRESENTVSYNC);
#endif
    if (sdlRenderer == NULL)
    {
        DBGPRINTF("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    if (s3DAvailable)
    {
        SDL_RendererInfo info;
        if (SDL_GetRendererInfo(sdlRenderer, &info) != 0 || strncmp(info.name, "opengl", 6) != 0)
            s3DAvailable = false; // no OpenGL here: 3D can't be drawn
    }
    Try3DInit();

    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    sGameTargetOk = SDL_RenderTargetSupported(sdlRenderer);

    for (int i = 1; i < 15; i++)
    {
        char filename[16];
#ifdef _WIN32
        snprintf(filename, sizeof(filename), "BG%d.bmp", i);
#else
        snprintf(filename, sizeof(filename), "BG%d.png", i);
#endif
        SDL_RWops *backgroundFile = SDL_RWFromFile(filename, "rb");
        if (backgroundFile == NULL)
            break;
        SDL_RWclose(backgroundFile);
        sBorderBackgroundCount++;
    }
    if (sBackgroundOrderVersion < 2)
    {
        if (sHasBorderBackgroundConfig)
        {
            if (sBorderBackground == 1)
                sBorderBackground = sBorderBackgroundCount;
            else if (sBorderBackground >= 2)
                sBorderBackground--;
        }
        sBackgroundOrderVersion = 2;
        StoreConfigFile();
    }
#ifdef NATIVE_LINUX
    SDL_RenderSetLogicalSize(sdlRenderer, 0, 0);
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0)
    {
        SDL_Log("SDL_image could not initialize: %s", IMG_GetError());
    }
    else
    {
        for (int i = 0; i < sBorderBackgroundCount; i++)
        {
            char filename[16];
            snprintf(filename, sizeof(filename), i == 0 ? "BG.png" : "BG%d.png", i);
            sdlBackgroundTextures[i] = IMG_LoadTexture(sdlRenderer, filename);
        }
        sdlBorderTexture = IMG_LoadTexture(sdlRenderer, "Border.png");
        if (sdlBackgroundTextures[0] == NULL)
            SDL_Log("Background image could not be loaded: %s", IMG_GetError());
        if (sdlBorderTexture == NULL)
            SDL_Log("Border image could not be loaded: %s", IMG_GetError());
    }
#elif defined(_WIN32)
    SDL_RenderSetLogicalSize(sdlRenderer, 0, 0);
    SDL_Surface *borderSurface = SDL_LoadBMP("Border.bmp");
    for (int i = 0; i < sBorderBackgroundCount; i++)
    {
        char filename[16];
        snprintf(filename, sizeof(filename), i == 0 ? "BG.bmp" : "BG%d.bmp", i);
        SDL_Surface *backgroundSurface = SDL_LoadBMP(filename);
        if (backgroundSurface == NULL)
            continue;
        sdlBackgroundTextures[i] = SDL_CreateTextureFromSurface(sdlRenderer, backgroundSurface);
        SDL_FreeSurface(backgroundSurface);
    }
    if (sdlBackgroundTextures[0] == NULL)
        SDL_Log("Background image could not be loaded: %s", SDL_GetError());
    if (borderSurface == NULL)
    {
        SDL_Log("Border image could not be loaded: %s", SDL_GetError());
    }
    else
    {
        sdlBorderTexture = SDL_CreateTextureFromSurface(sdlRenderer, borderSurface);
        SDL_FreeSurface(borderSurface);
    }
#else
    SDL_RenderSetLogicalSize(sdlRenderer, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    SDL_RenderSetIntegerScale(sdlRenderer, SDL_TRUE);
#endif
    ApplyPlatformSettings();
    if (getenv("WJPC_WINDOW"))
    {
        // Test aid: WJPC_WINDOW=1920x1080 stands in for a fullscreen monitor.
        int ww = 0, wh = 0;
        if (sscanf(getenv("WJPC_WINDOW"), "%dx%d", &ww, &wh) == 2)
            SDL_SetWindowSize(sdlWindow, ww, wh);
    }

    sdlTexture = SDL_CreateTexture(sdlRenderer,
                                   SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   TEX_W, TEX_H);
    if (sdlTexture == NULL)
    {
        DBGPRINTF("Texture could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetTextureBlendMode(sdlTexture, SDL_BLENDMODE_NONE);
    sdlOverlayTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, TEX_W, TEX_H);
    if (sdlOverlayTexture != NULL)
        SDL_SetTextureBlendMode(sdlOverlayTexture, SDL_BLENDMODE_BLEND);
    sdlUiTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, TEX_W, TEX_H);
    if (sdlUiTexture != NULL)
    {
        SDL_SetTextureBlendMode(sdlUiTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(sdlUiTexture, SDL_ScaleModeNearest);
    }

    simTime = curGameTime = lastGameTime = SDL_GetPerformanceCounter();

    isFrameAvailable.value = 0;
    vBlankSemaphore = SDL_CreateSemaphore(0);

    SDL_AudioSpec want;

    SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
    want.freq = 42060;
    want.format = AUDIO_F32;
    want.channels = 2;
    want.samples = 1024;
    cgb_audio_init(want.freq);


    sdlAudioDevice = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
    if (sdlAudioDevice == 0)
        SDL_Log("Failed to open audio: %s", SDL_GetError());
    else
    {
        if (want.format != AUDIO_F32) /* we let this one thing change. */
            SDL_Log("We didn't get Float32 audio format.");
        SDL_PauseAudioDevice(sdlAudioDevice, 0);
    }
#ifndef __ANDROID__
    VDraw(sdlTexture);
#endif
    /* Brennan 2026-11-21 16:26 ("The internal battery has run dry" on boot): the clock was set up here AFTER the game
     * thread started, so the game's RtcInit could read it first - status 0 (a 12-hour clock) or month 0 - and take it
     * for a dead battery (rtc.c RtcCheckInfo -> main_menu.c's message). 21h's 100 ms clock reading made it likelier:
     * the reading taken before this memset was kept, so the zeroed date stayed for up to 100 ms. Set up first now,
     * with a fresh reading. */
    ResetInternalClock();
    /* Project Tandem T1 (Brennan's multiplayer, docs/TANDEM_TASKS.md): host or join if the settings ask - its own
     * thread, never the game's; nothing starts otherwise. Its log beside the settings file. */
    {
        char tandemLog[1100];
        const char *dot = strrchr(sConfigPath, '.');
        int stem = dot != NULL ? (int)(dot - sConfigPath) : (int)strlen(sConfigPath);
        SDL_snprintf(tandemLog, sizeof(tandemLog), "%.*s_tandem.log", stem, sConfigPath);
        TandemNet_Start(tandemLog);
    }
    mainLoopThread = SDL_CreateThread(DoMain, "AgbMain", NULL);

    double accumulator = 0.0;

    while (isRunning)
    {
        ApplyChangedWindowSettings();
        /* (Project Tandem: who is connected, in the window's title) */
        if (TandemNet_Active())
        {
            static unsigned sTandemGen;
            unsigned gen;
            const char *status = TandemNet_StatusText(&gen);
            if (gen != sTandemGen)
            {
                char title[200];
                sTandemGen = gen;
                SDL_snprintf(title, sizeof(title), "Pokemon Wally's Journey - %s", status);
                SDL_SetWindowTitle(sdlWindow, title);
            }
            /* (the server gave this PC its key: kept in the settings, the invite code dropped) */
            if (TandemNet_TakeConfigChanged())
                StoreConfigFile();
        }
        if (sTestScript != NULL && !sScriptRealtime)
        {
            while (!SDL_AtomicGet(&isFrameAvailable))
                SDL_Delay(0);
            if (!TestStep())
                break;
            if (sTestLockRng)
            {
                gRngValue.a = sTestLockValue[0]; gRngValue.b = sTestLockValue[1];
                gRngValue.c = sTestLockValue[2]; gRngValue.ctr = sTestLockValue[3];
            }
            keyboardKeys = sTestKeys;
            /* (test aid WJPC_TEST_PACE=1: at most 60 game frames a second, as in real play - for two games linked
             * together, Project Tandem T5; the test loop otherwise runs as fast as the PC can) */
            {
                static int sPace = -1;
                static Uint64 sPaceLast;
                if (sPace < 0)
                    sPace = getenv("WJPC_TEST_PACE") != NULL;
                if (sPace)
                {
                    Uint64 period = SDL_GetPerformanceFrequency() / 60;
                    while (sPaceLast != 0 && SDL_GetPerformanceCounter() - sPaceLast < period)
                        SDL_Delay(1);
                    sPaceLast = SDL_GetPerformanceCounter();
                }
            }
            if (sTestWinShot[0] != '\0' || getenv("WJPC_TEST_WINDOW"))
            {
                // Test aid: draw exactly like a normal frame (zoom, UI layer, screen options),
                // and with "winshot FILE" save the whole window.
                Uint64 vd0 = SDL_GetPerformanceCounter();
                VDraw(sdlTexture);
                if (getenv("WJPC_VDRAW_TIME") != NULL) // (test aid: the whole game frame's drawing, every 60)
                {
                    static double vacc, v3d;
                    static unsigned vn;
                    vacc += (SDL_GetPerformanceCounter() - vd0) * 1000.0 / SDL_GetPerformanceFrequency();
                    v3d += sStall3D * 1000.0 / SDL_GetPerformanceFrequency();
                    sStall3D = 0;
                    if (++vn % 60 == 0)
                    {
                        FILE *tf = fopen(getenv("WJPC_VDRAW_TIME"), "a");
                        if (tf != NULL)
                        {
                            fprintf(tf, "[vdraw] whole frame %.2f ms (3D %.2f)\n", vacc / 60.0, v3d / 60.0);
                            fclose(tf);
                        }
                        vacc = v3d = 0.0;
                    }
                }
                if (sTestWinShot[0] != '\0')
                {
                    int ww, wh;
                    DrawScreen(sTestWinShotT >= 0.0 ? sTestWinShotT : getenv("WJPC_WINSHOT_T") ? atof(getenv("WJPC_WINSHOT_T")) : 1.0);
                    SDL_GetRendererOutputSize(sdlRenderer, &ww, &wh);
                    uint8_t *px = malloc((size_t)ww * wh * 3);
                    if (px != NULL && SDL_RenderReadPixels(sdlRenderer, NULL, SDL_PIXELFORMAT_RGB24, px, ww * 3) == 0)
                    {
                        FILE *f = fopen(sTestWinShot, "wb");
                        if (f != NULL)
                        {
                            fprintf(f, "P6\n%d %d\n255\n", ww, wh);
                            fwrite(px, 1, (size_t)ww * wh * 3, f);
                            fclose(f);
                        }
                    }
                    free(px);
                    sTestWinShot[0] = '\0';
                }
            }
            else if (sTestFfSpeed > 1 && sTestFrame % sTestFfSpeed != 0)
            {
                WideView_SkipFrame();
            }
            else
            {
            WideView_SetSmooth(getenv("WJPC_SMOOTH_TEST") ? sTestOverlay : NULL, WIDE_VIEW_SMOOTH_MARGIN);
            WideView_DrawFrame(sTestGbaImage, &sTestFrameInfo);
            WideView_SetSmooth(NULL, 0);
            }
            REG_VCOUNT = 161;
            SDL_AtomicSet(&isFrameAvailable, 0);
            REG_DISPSTAT |= INTR_FLAG_VBLANK;
            RunDMAs(DMA_HBLANK);
            if (REG_DISPSTAT & DISPSTAT_VBLANK_INTR)
                PcGuard_Call0(gIntrTable[4]);
            REG_DISPSTAT &= ~INTR_FLAG_VBLANK;
            sTestFrame++;
            if (sTestFrame % 1000 == 0)
            {
                printf("frame %u\n", sTestFrame);
                fflush(stdout);
            }
            FlushSaveIfDirty();
            SDL_SemPost(vBlankSemaphore);
            continue;
        }
        ProcessEvents();
        ApplyFastForward();
        Uint64 stallLoopStart = SDL_GetPerformanceCounter();
        sStallCpuStart = StallThreadCpuMs();
        sStallVDraw = sStall3D = sStall2D = sStallGl = 0;
        sVdSetup = sVdAfter2D = sVd3DBlock = sVdEnd = 0;
        gVoxelStandInTicks = 0;
        sStallDraws = 0;

        if (!paused)
        {
            double dt = fixedTimestep / timeScale; // TODO: Fix speedup

            curGameTime = SDL_GetPerformanceCounter();
            double deltaTime = (double)((curGameTime - lastGameTime) / (double)SDL_GetPerformanceFrequency());
            // Catch up at most a few frames after a stall (window drag, etc.).
            // The old clamp compared against the fast-forward step, which made
            // fast-forward stutter between 1x and 5x.
            if (deltaTime > 0.1)
                deltaTime = 0.1;
            lastGameTime = curGameTime;

            accumulator += deltaTime;

            Uint64 batchStart = SDL_GetPerformanceCounter();
            while (accumulator >= dt)
            {
                if (SDL_AtomicGet(&isFrameAvailable))
                {
                    // Fast forward runs several game frames per picture. Only the last one of the
                    // batch is drawn (drawing is most of the work); the others just keep the
                    // per-line timing going. If this PC can't reach the chosen speed, stop after
                    // about one screen refresh, show that frame and forget the backlog, so the
                    // picture stays smooth and the game runs as fast as the PC manages.
                    if (sTestScript != NULL)
                    {
                        // (WJPC_SCRIPT_REALTIME: one script step a game frame)
                        if (!TestStep())
                            isRunning = false;
                        keyboardKeys = sTestKeys;
                    }
                    bool lastOfBatch = accumulator - dt < dt;
                    bool outOfTime = speedUp && (SDL_GetPerformanceCounter() - batchStart) * 1000 / SDL_GetPerformanceFrequency() >= 14;
                    /* Brennan 2026-11-16 (16g, 4K, 240 fps: "the whole game lags behind and visually lags"):
                     * every game frame of a catch-up batch was drawn - the CPU picture and the whole 3D world -
                     * although only the last is ever shown. One slow trip (a map's meshes being built) made the
                     * next trip run up to 6 game frames, each drawn, so that one was slow too, and so on: the
                     * game never caught up. Now only the last frame of a batch is drawn, as fast-forward
                     * already does (WJPC_DRAW_EVERY_FRAME=1: as before). */
                    static int sDrawEvery = -1;
                    if (sDrawEvery < 0)
                        sDrawEvery = getenv("WJPC_DRAW_EVERY_FRAME") != NULL;
                    if ((!speedUp && sDrawEvery) || lastOfBatch || outOfTime)
                    {
                        Uint64 v0 = SDL_GetPerformanceCounter();
                        sDrawSpan = sFramesSinceDraw + 1;
                        sFramesSinceDraw = 0;
                        Voxel3D_SetDrawSpan(sDrawSpan);
                        static int sProfVDraw = -1;
                        if (sProfVDraw < 0)
                            sProfVDraw = getenv("WJPC_PROFILE_VDRAW") != NULL; // (test: the profiler samples the picture's drawing)
                        if (sProfVDraw)
                            gVoxelProfileOn = 2;
                        VDraw(sdlTexture);
                        if (sProfVDraw)
                            gVoxelProfileOn = 0;
                        sStallDraws++;
                        sStallVDraw += SDL_GetPerformanceCounter() - v0;
                        sDrewFrame = true;
                    }
                    else
                    {
                        WideView_SkipFrame();
                        REG_VCOUNT = 161;
                        sFramesSinceDraw++;
                    }
                    sGameFramesCounted++;
                    sSpeedFrames++;
                    SDL_AtomicSet(&isFrameAvailable, 0);

                    REG_DISPSTAT |= INTR_FLAG_VBLANK;

                    RunDMAs(DMA_HBLANK);

#ifdef __ANDROID__
                    if (REG_IE & INTR_FLAG_VBLANK)
#else
                    if (REG_DISPSTAT & DISPSTAT_VBLANK_INTR)
#endif
                        PcGuard_Call0(gIntrTable[4]);
                    REG_DISPSTAT &= ~INTR_FLAG_VBLANK;

                    FlushSaveIfDirty();
            SDL_SemPost(vBlankSemaphore);

                    accumulator -= dt;
                    if (outOfTime)
                    {
                        accumulator = 0;
                        break;
                    }
                }
            }
        }

        Uint64 stallBatchEnd = SDL_GetPerformanceCounter();
        {
            Uint64 freq = SDL_GetPerformanceFrequency();
            if (sSpeedStamp == 0)
                sSpeedStamp = stallBatchEnd;
            if (!speedUp || paused)
            {
                sGameSpeedQ8 = 256;
                sSpeedFrames = 0;
                sSpeedStamp = stallBatchEnd;
            }
            else if (stallBatchEnd - sSpeedStamp >= freq / 4)
            {
                double ticks = (double)(stallBatchEnd - sSpeedStamp) / (double)freq * 60.0;
                double q = sSpeedFrames / ticks * 256.0;
                sGameSpeedQ8 = q < 256.0 ? 256u : q > 256.0 * 24 ? 256u * 24 : (u32)q;
                sSpeedFrames = 0;
                sSpeedStamp = stallBatchEnd;
            }
        }
#ifndef __ANDROID__
        // Draw every loop (every monitor refresh with vsync). With smooth motion the map is slid
        // part of the way between the last two game frames.
        if (sDrewFrame)
        {
            double t = paused ? 1.0 : accumulator / (fixedTimestep / timeScale);
            static int sEaseOld = -1;
            if (sEaseOld < 0)
                sEaseOld = getenv("WJPC_FF_EASE_OLD") != NULL;
            t = t < 0 ? 0 : t > 1 ? 1 : t;
            if (!sEaseOld && sDrawSpan > 1)
                t = (sDrawSpan - 1 + t) / sDrawSpan; // (the span's last game frame eased in, as at normal speed)
            if (getenv("WJPC_EASE_LOG")) // (test aid: each refresh's game frame, span and how far it is eased)
            {
                FILE *ef = fopen(getenv("WJPC_EASE_LOG"), "a");
                if (ef != NULL)
                {
                    fprintf(ef, "%u %u %.3f %.6f\n", sGameFramesCounted, sDrawSpan, t,
                            SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency());
                    fclose(ef);
                }
            }
            DrawScreen(t);
        }
        Uint64 stallDrawn = SDL_GetPerformanceCounter();
        SDL_RenderPresent(sdlRenderer);
        StallLog(stallLoopStart, stallBatchEnd, stallDrawn, SDL_GetPerformanceCounter());
        // Frame rate 60/120/180/240: don't draw faster than chosen (with vsync the monitor may cap it lower).
        {
            static Uint64 lastPresent;
            Uint64 freq = SDL_GetPerformanceFrequency();
            Uint64 period = freq / gPlatformFrameRates[sPlatformSettings[PLATFORM_SETTING_SMOOTH] % FRAME_RATE_COUNT];
            Uint64 now = SDL_GetPerformanceCounter();
            if (lastPresent != 0 && now - lastPresent < period)
            {
                Uint64 waitMs = (period - (now - lastPresent)) * 1000 / freq;
                if (waitMs > 1)
                    SDL_Delay((Uint32)(waitMs - 1));
                while (SDL_GetPerformanceCounter() - lastPresent < period)
                    ;
            }
            lastPresent = SDL_GetPerformanceCounter();
        }
        if (getenv("WJPC_FPS_LOG"))
        {
            // Test aid: WJPC_FPS_LOG=file gets the frames drawn each second.
            static Uint64 fpsStart;
            static int fpsCount;
            Uint64 now = SDL_GetPerformanceCounter();
            fpsCount++;
            if (fpsStart == 0)
                fpsStart = now;
            else if (now - fpsStart >= SDL_GetPerformanceFrequency())
            {
                FILE *f = fopen(getenv("WJPC_FPS_LOG"), "a");
                if (f != NULL)
                {
                    fprintf(f, "fps %d game %d\n", fpsCount, sGameFramesCounted);
                    fclose(f);
                }
                fpsCount = 0;
                sGameFramesCounted = 0;
                fpsStart = now;
            }
        }
#endif
    }

    //StoreSaveFile();
    CloseSaveFile();
    TandemNet_Stop();   /* (Project Tandem: tells friends it is leaving) */

#if defined(NATIVE_LINUX) || defined(_WIN32)
    for (int i = 0; i < sBorderBackgroundCount; i++)
        SDL_DestroyTexture(sdlBackgroundTextures[i]);
    SDL_DestroyTexture(sdlBorderTexture);
#endif
#ifdef NATIVE_LINUX
    IMG_Quit();
#endif
    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();
    return 0;
}

static void ReadSaveFile(const char *path)
{
    // Check whether the saveFile exists, and create it if not
    sSaveFile = fopen(path, "r+b");
    if (sSaveFile == NULL)
    {
        sSaveFile = fopen(path, "w+b");
    }

    if (sSaveFile == NULL)
    {
        memset(FLASH_BASE, 0xFF, sizeof(FLASH_BASE));
        SDL_Log("Unable to open save file: %s", path);
        return;
    }

    fseek(sSaveFile, 0, SEEK_END);
    int fileSize = ftell(sSaveFile);
    fseek(sSaveFile, 0, SEEK_SET);

    // Only read as many bytes as fit inside the buffer
    // or as many bytes as are in the file
    int bytesToRead = (fileSize < sizeof(FLASH_BASE)) ? fileSize : sizeof(FLASH_BASE);

    int bytesRead = fread(FLASH_BASE, 1, bytesToRead, sSaveFile);

    // Fill the buffer if the savefile was just created or smaller than the buffer itself
    for (int i = bytesRead; i < sizeof(FLASH_BASE); i++)
    {
        FLASH_BASE[i] = 0xFF;
    }
}

static void ReadConfigFile(void)
{
    FILE *configFile = fopen(sConfigPath, "r");
    char line[64];
    unsigned int value;

    if (configFile == NULL)
        return;
    while (fgets(line, sizeof(line), configFile) != NULL)
    {
        if (sscanf(line, "borderBackground=%u", &value) == 1 && value < 16)
        {
            sBorderBackground = value;
            sHasBorderBackgroundConfig = true;
        }
        else if (sscanf(line, "backgroundOrder=%u", &value) == 1)
            sBackgroundOrderVersion = value;
        else if (sscanf(line, "fullscreen=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_FULLSCREEN] = value != 0;
        else if (sscanf(line, "windowScale=%u", &value) == 1 && value >= 1 && value <= 12)
            sPlatformSettings[PLATFORM_SETTING_WINDOW_SCALE] = value;
        else if (sscanf(line, "integerScale=%u", &value) == 1) /* (Brennan 2026-11-22: setting removed - always off) */
            sPlatformSettings[PLATFORM_SETTING_INTEGER_SCALE] = 0;
        else if (sscanf(line, "vsync=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_VSYNC] = value != 0;
        else if (sscanf(line, "border=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_BORDER] = value != 0;
        else if (sscanf(line, "volume=%u", &value) == 1 && value <= 10)
            sPlatformSettings[PLATFORM_SETTING_VOLUME] = value;
        else if (sscanf(line, "cameraAngle=%u", &value) == 1 && value <= 2)
            sPlatformSettings[PLATFORM_SETTING_CAMERA_ANGLE] = value;
        else if (sscanf(line, "musicVolume=%u", &value) == 1 && value <= 10)
            sPlatformSettings[PLATFORM_SETTING_MUSIC_VOLUME] = value;
        else if (sscanf(line, "soundVolume=%u", &value) == 1 && value <= 10)
            sPlatformSettings[PLATFORM_SETTING_SOUND_VOLUME] = value;
        else if (sscanf(line, "fastForward=%u", &value) == 1 && value < FF_SPEED_COUNT)
            sPlatformSettings[PLATFORM_SETTING_FF_SPEED] = value;
        else if (sscanf(line, "world3d=%u", &value) == 1)
            s3DWanted = value != 0;
        else if (sscanf(line, "battle3d=%u", &value) == 1)
            s3DBattleWanted = value != 0;
        else if (sscanf(line, "gammaLight=%u", &value) == 1)
            sGammaWanted = value != 0;
        else if (sscanf(line, "antialias=%u", &value) == 1)
            sAntiAlias = value > 8 ? 8 : value;
        else if (TandemNet_ConfigLine(line))
            ;   /* (Project Tandem: tandemHost / tandemJoin / tandemName) */
        else if (sscanf(line, "widescreen=%u", &value) == 1)
            WideView_SetEnabled(value != 0);
        else if (strncmp(line, "key", 3) == 0)
        {
            int i;
            for (i = 0; i < KEYBIND_COUNT; i++)
            {
                char name[32];
                int key;
                SDL_snprintf(name, sizeof(name), "%s=%%d", sKeyBindNames[i]);
                if (sscanf(line, name, &key) == 1)
                    sKeyMap[i] = key;
            }
        }
        else if (sscanf(line, "zoom=%u", &value) == 1 && value < ZOOM_COUNT)
            sPlatformSettings[PLATFORM_SETTING_ZOOM] = value;
        else if (sscanf(line, "bilinear=%u", &value) == 1) /* (Brennan 2026-11-22: setting removed - always off) */
            sPlatformSettings[PLATFORM_SETTING_SMOOTHING] = 0;
        else if (sscanf(line, "resolution=%u", &value) == 1 && value < RESOLUTION_COUNT)
            sPlatformSettings[PLATFORM_SETTING_RESOLUTION] = value;
        else if (sscanf(line, "lockAspect=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_LOCK_ASPECT] = value != 0;
        else if (sscanf(line, "interframe=%u", &value) == 1) /* (Brennan 2026-11-22: setting removed - always off) */
            sPlatformSettings[PLATFORM_SETTING_INTERFRAME] = 0;
        else if (sscanf(line, "lockFrameSize=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_LOCK_FRAME] = value != 0;
        else if (sscanf(line, "filter=%u", &value) == 1 && value <= 2)
            sPlatformSettings[PLATFORM_SETTING_FILTER] = value;
        else if (sscanf(line, "frameRate=%u", &value) == 1)
        {
            // Stored as frames per second, so adding choices later doesn't change what was picked.
            sPlatformSettings[PLATFORM_SETTING_SMOOTH] = 0;
            for (int i = 0; i < FRAME_RATE_COUNT; i++)
                if (gPlatformFrameRates[i] == value)
                    sPlatformSettings[PLATFORM_SETTING_SMOOTH] = i;
        }
        else if (sscanf(line, "smooth=%u", &value) == 1)
        {
            // Older settings file: 0..3 = 60/120/180/240.
            static const u16 sOld[] = {60, 120, 180, 240};
            for (int i = 0; i < FRAME_RATE_COUNT; i++)
                if (value <= 3 && gPlatformFrameRates[i] == sOld[value])
                    sPlatformSettings[PLATFORM_SETTING_SMOOTH] = i;
        }
        else if (sscanf(line, "fastForwardToggle=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_FF_TOGGLE] = value != 0;
        else
        {
            int i;
            for (i = 0; i < 7; i++)
            {
                char name[32];
                SDL_snprintf(name, sizeof(name), "%s=%%u", sPadSettingNames[i]);
                if (sscanf(line, name, &value) == 1 && value < PAD_BUTTON_COUNT)
                    sPlatformSettings[PLATFORM_SETTING_PAD_A + i] = value;
            }
        }
    }
    fclose(configFile);
}

static void StoreConfigFile(void)
{
    FILE *configFile = fopen(sConfigPath, "w");

    if (configFile == NULL)
        return;
    fprintf(configFile, "borderBackground=%u\n", sBorderBackground);
    fprintf(configFile, "backgroundOrder=2\n");
    fprintf(configFile, "fullscreen=%u\n", sPlatformSettings[PLATFORM_SETTING_FULLSCREEN]);
    fprintf(configFile, "windowScale=%u\n", sPlatformSettings[PLATFORM_SETTING_WINDOW_SCALE]);
    fprintf(configFile, "integerScale=%u\n", sPlatformSettings[PLATFORM_SETTING_INTEGER_SCALE]);
    fprintf(configFile, "vsync=%u\n", sPlatformSettings[PLATFORM_SETTING_VSYNC]);
    fprintf(configFile, "border=%u\n", sPlatformSettings[PLATFORM_SETTING_BORDER]);
    fprintf(configFile, "volume=%u\n", sPlatformSettings[PLATFORM_SETTING_VOLUME]);
    fprintf(configFile, "musicVolume=%u\n", sPlatformSettings[PLATFORM_SETTING_MUSIC_VOLUME]);
    fprintf(configFile, "cameraAngle=%u\n", sPlatformSettings[PLATFORM_SETTING_CAMERA_ANGLE]);
    fprintf(configFile, "soundVolume=%u\n", sPlatformSettings[PLATFORM_SETTING_SOUND_VOLUME]);
    fprintf(configFile, "fastForward=%u\n", sPlatformSettings[PLATFORM_SETTING_FF_SPEED]);
    fprintf(configFile, "widescreen=%u\n", WideView_IsEnabled() ? 1 : 0);
    fprintf(configFile, "world3d=%u\n", s3DWanted ? 1 : 0);
    fprintf(configFile, "battle3d=%u\n", s3DBattleWanted ? 1 : 0);
    fprintf(configFile, "gammaLight=%u\n", sGammaWanted ? 1 : 0);
    fprintf(configFile, "antialias=%u\n", sAntiAlias);
    TandemNet_WriteConfig(configFile);
    fprintf(configFile, "fastForwardToggle=%u\n", sPlatformSettings[PLATFORM_SETTING_FF_TOGGLE]);
    fprintf(configFile, "frameRate=%u\n", gPlatformFrameRates[sPlatformSettings[PLATFORM_SETTING_SMOOTH] % FRAME_RATE_COUNT]);
    fprintf(configFile, "filter=%u\n", sPlatformSettings[PLATFORM_SETTING_FILTER]);
    fprintf(configFile, "zoom=%u\n", sPlatformSettings[PLATFORM_SETTING_ZOOM]);
    fprintf(configFile, "bilinear=%u\n", sPlatformSettings[PLATFORM_SETTING_SMOOTHING]);
    fprintf(configFile, "resolution=%u\n", sPlatformSettings[PLATFORM_SETTING_RESOLUTION]);
    fprintf(configFile, "lockAspect=%u\n", sPlatformSettings[PLATFORM_SETTING_LOCK_ASPECT]);
    fprintf(configFile, "interframe=%u\n", sPlatformSettings[PLATFORM_SETTING_INTERFRAME]);
    fprintf(configFile, "lockFrameSize=%u\n", sPlatformSettings[PLATFORM_SETTING_LOCK_FRAME]);
    for (int i = 0; i < KEYBIND_COUNT; i++)
        fprintf(configFile, "%s=%d\n", sKeyBindNames[i], (int)sKeyMap[i]);
    for (int i = 0; i < 7; i++)
        fprintf(configFile, "%s=%u\n", sPadSettingNames[i], sPlatformSettings[PLATFORM_SETTING_PAD_A + i]);
    fclose(configFile);
}

static void ApplyPlatformSettings(void)
{
    SDL_RenderSetVSync(sdlRenderer, sPlatformSettings[PLATFORM_SETTING_VSYNC]);
#if defined(NATIVE_LINUX) || defined(_WIN32)
    // mGBA "Lock frame size": the window can't be resized by dragging its edges.
    SDL_SetWindowResizable(sdlWindow, sPlatformSettings[PLATFORM_SETTING_LOCK_FRAME] ? SDL_FALSE : SDL_TRUE);
    SDL_SetWindowFullscreen(sdlWindow, sPlatformSettings[PLATFORM_SETTING_FULLSCREEN]
                                      ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    if (!sPlatformSettings[PLATFORM_SETTING_FULLSCREEN])
    {
        int scale = sPlatformSettings[PLATFORM_SETTING_WINDOW_SCALE];
        int baseW = WideView_IsEnabled() ? 320 : DISPLAY_WIDTH, baseH = WideView_IsEnabled() ? 180 : DISPLAY_HEIGHT;
        SDL_Rect usable;
        // Wide view: the window is 16:9 (1x = 320x180 ... 6x = 1920x1080 ... 12x = 3840x2160, 4K).
        // A size bigger than the screen is shrunk to the biggest that fits.
        if (SDL_GetDisplayUsableBounds(SDL_GetWindowDisplayIndex(sdlWindow), &usable) == 0)
            while (scale > 1 && (baseW * scale > usable.w || baseH * scale > usable.h))
                scale--;
        SDL_SetWindowSize(sdlWindow, baseW * scale, baseH * scale);
        SDL_SetWindowPosition(sdlWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }
#endif
}

static void StoreSaveFile()
{
    if (sSaveFile != NULL)
    {
        fseek(sSaveFile, 0, SEEK_SET);
        fwrite(FLASH_BASE, 1, sizeof(FLASH_BASE), sSaveFile);
        fflush(sSaveFile);
    }
}

// Called between frames, while the game thread is waiting for VBlank.
extern bool8 gWjFlashDirty;
static void FlushSaveIfDirty(void)
{
    if (gWjFlashDirty)
    {
        gWjFlashDirty = FALSE;
        StoreSaveFile();
    }
}

// Windows game: the PC-only save block (struct SaveBlockPC, no size limit) sits after the GBA flash image.
// Layout at offset sizeof(FLASH_BASE): u32 length, then the block. A GBA .sav ends before it.
bool8 Platform_ReadExtraSave(void *dest, u32 size)
{
    u32 len = 0;
    if (sSaveFile == NULL || fseek(sSaveFile, sizeof(FLASH_BASE), SEEK_SET) != 0
     || fread(&len, 1, 4, sSaveFile) != 4 || len == 0)
        return FALSE;
    if (len > size)
        len = size;
    return fread(dest, 1, len, sSaveFile) == len;
}

void Platform_WriteExtraSave(const void *src, u32 size)
{
    if (sSaveFile == NULL || fseek(sSaveFile, sizeof(FLASH_BASE), SEEK_SET) != 0)
        return;
    fwrite(&size, 1, 4, sSaveFile);
    fwrite(src, 1, size, sSaveFile);
    fflush(sSaveFile);
}

void Platform_StoreSaveFile(void)
{
    StoreSaveFile();
}

void Platform_ReadFlash(u16 sectorNum, u32 offset, u8 *dest, u32 size)
{
    DBGPRINTF("ReadFlash(sectorNum=0x%04X,offset=0x%08X,size=0x%02X)\n",sectorNum,offset,size);
    FILE * savefile = fopen(sSavePath, "r+b");
    if (savefile == NULL)
    {
        puts("Error opening save file.");
        return;
    }
    if (fseek(savefile, (sectorNum << gFlash->sector.shift) + offset, SEEK_SET))
    {
        fclose(savefile);
        return;
    }
    if (fread(dest, 1, size, savefile) != size)
    {
        fclose(savefile);
        return;
    }
    fclose(savefile);
}

void Platform_QueueAudio(float *audioBuffer, s32 samplesPerFrame)
{
    // Test mode: WJPC_AUDIO_DUMP=file writes the raw mixer output (f32 stereo).
    static FILE *sAudioDump;
    static bool sAudioDumpChecked;
    if (!sAudioDumpChecked)
    {
        sAudioDumpChecked = true;
        if (getenv("WJPC_AUDIO_DUMP"))
            sAudioDump = fopen(getenv("WJPC_AUDIO_DUMP"), "wb");
    }
    if (sAudioDump)
        fwrite(audioBuffer, 1, samplesPerFrame, sAudioDump);
    if (sdlAudioDevice != 0)
    {
        int floatCount = samplesPerFrame / sizeof(float);
        float adjustedAudio[floatCount];
        float volume = sPlatformSettings[PLATFORM_SETTING_VOLUME] / 10.0f;
        for (int i = 0; i < floatCount; i++)
            adjustedAudio[i] = audioBuffer[i] * volume;
        // Never let more than ~1/4 s queue up, so sound stays in sync. (Fast forward no longer
        // drops sound: the sound engine itself only runs at normal speed, see Platform_SoundFrameDue.)
        if (SDL_GetQueuedAudioSize(sdlAudioDevice) > 42060 * 2 * sizeof(float) / 4)
            return;
        if (SDL_QueueAudio(sdlAudioDevice, adjustedAudio, samplesPerFrame) < 0)
            SDL_Log("Failed to queue audio: %s", SDL_GetError());
    }
}

u8 Platform_GetBorderBackgroundCount(void)
{
    return sBorderBackgroundCount + 1;
}

u8 Platform_GetBorderBackground(void)
{
    if (sHasBorderBackgroundConfig)
        return sBorderBackground;
    return 0;
}

void Platform_SetBorderBackground(u8 selection)
{
    sBorderBackground = selection;
    sHasBorderBackgroundConfig = true;
    StoreConfigFile();
}

bool8 Platform_Is3DAvailable(void)
{
    return s3DAvailable && !(s3DInitTried && !s3DReady);
}

u8 Platform_GetSetting(enum PlatformSetting setting)
{
    if (setting == PLATFORM_SETTING_WIDESCREEN)
        return WideView_IsEnabled();
    if (setting == PLATFORM_SETTING_WORLD3D)
        return s3DWanted && Platform_Is3DAvailable();
    if (setting == PLATFORM_SETTING_BATTLE3D)
        return s3DBattleWanted && Platform_Is3DAvailable();
    if (setting == PLATFORM_SETTING_GAMMALIGHT)
        return sGammaWanted;
    return sPlatformSettings[setting];
}

// Called from the game thread (Options menu). Window changes must happen on
// the main thread, so they are only flagged here and applied between frames.
static SDL_atomic_t sWindowSettingsChanged;

void Platform_SetSetting(enum PlatformSetting setting, u8 value)
{
    if (setting == PLATFORM_SETTING_GAMMALIGHT)
    {
        sGammaWanted = value != 0; // 2D from the next frame; 3D lays its maps again (main thread, next frame)
        StoreConfigFile();
        return;
    }
    if (setting == PLATFORM_SETTING_BATTLE3D)
    {
        if (Platform_Is3DAvailable())
            s3DBattleWanted = value != 0;
        StoreConfigFile();
        return;
    }
    if (setting == PLATFORM_SETTING_WORLD3D)
    {
        if (Platform_Is3DAvailable())
            s3DWanted = value != 0; // drawn (and loaded, the first time) from the next frame
        StoreConfigFile();
        return;
    }
    sPlatformSettings[setting] = value;
    if (setting == PLATFORM_SETTING_WIDESCREEN)
    {
        WideView_SetEnabled(value != 0);
        SDL_AtomicSet(&sWindowSettingsChanged, 1); // window shape follows (16:9 or 3:2)
    }
    if (setting == PLATFORM_SETTING_FF_TOGGLE)
        SDL_AtomicSet(&sFfToggled, 0);
    if (setting == PLATFORM_SETTING_VSYNC || setting == PLATFORM_SETTING_FULLSCREEN
     || setting == PLATFORM_SETTING_WINDOW_SCALE || setting == PLATFORM_SETTING_LOCK_FRAME)
        SDL_AtomicSet(&sWindowSettingsChanged, 1);
    StoreConfigFile();
}

static void ApplyChangedWindowSettings(void)
{
    if (SDL_AtomicGet(&sWindowSettingsChanged))
    {
        SDL_AtomicSet(&sWindowSettingsChanged, 0);
        ApplyPlatformSettings();
    }
}

#ifdef __ANDROID__
JNIEXPORT jint JNICALL Java_com_pokeemerald_experimental_GbaControlsView_getBorderBackground(JNIEnv *env, jclass clazz)
{
    return Platform_GetBorderBackground();
}

JNIEXPORT jint JNICALL Java_com_pokeemerald_experimental_GbaControlsView_getPlatformSetting(JNIEnv *env, jclass clazz, jint setting)
{
    if (setting < 0 || setting >= PLATFORM_SETTING_COUNT)
        return 0;
    return Platform_GetSetting(setting);
}
#endif


static void CloseSaveFile()
{
    if (sSaveFile != NULL)
    {
        fclose(sSaveFile);
    }
}

// Key mappings
#define KEY_A_BUTTON      SDLK_z
#define KEY_B_BUTTON      SDLK_x
#define KEY_START_BUTTON  SDLK_RETURN
#define KEY_SELECT_BUTTON SDLK_BACKSLASH
#define KEY_L_BUTTON      SDLK_a
#define KEY_R_BUTTON      SDLK_s
#define KEY_DPAD_UP       SDLK_UP
#define KEY_DPAD_DOWN     SDLK_DOWN
#define KEY_DPAD_LEFT     SDLK_LEFT
#define KEY_DPAD_RIGHT    SDLK_RIGHT

#define HANDLE_KEYUP(key) \
case KEY_##key:  keyboardKeys &= ~key; break;

#define HANDLE_KEYDOWN(key) \
case KEY_##key:  keyboardKeys |= key; break;


#ifdef __ANDROID__
#define MAX_TOUCH_FINGERS 10

struct TouchFinger
{
    SDL_FingerID id;
    float x;
    float y;
    bool active;
};

static struct TouchFinger touchFingers[MAX_TOUCH_FINGERS];
static u16 touchKeys;
static u16 controllerKeys;
static u16 controllerAxisKeys;
static Sint16 controllerAxisX;
static Sint16 controllerAxisY;

static bool IsInsideRect(int x, int y, SDL_Rect rect)
{
    SDL_Point point = {x, y};
    return SDL_PointInRect(&point, &rect);
}

static int MinInt(int a, int b)
{
    return a < b ? a : b;
}

static int GetControlSideWidth(int windowWidth, int windowHeight)
{
    int sideWidth = (windowWidth - windowHeight * 3 / 2) / 2;
    int minimumWidth = windowWidth * 14 / 100;
    return sideWidth > minimumWidth ? sideWidth : minimumWidth;
}

static void UpdateTouchKeys(void)
{
    int windowWidth;
    int windowHeight;
    SDL_GetWindowSize(sdlWindow, &windowWidth, &windowHeight);
    int sideWidth = GetControlSideWidth(windowWidth, windowHeight);
    int buttonSize = MinInt(sideWidth * 2 / 5, windowHeight / 6);
    int dpadUnit = MinInt(sideWidth / 3, windowHeight / 8);
    int dpadX = sideWidth * 2 / 3;
    int dpadY = windowHeight * 7 / 10;
    SDL_Rect dpadUp = {dpadX - dpadUnit / 2, dpadY - dpadUnit * 3 / 2,
                       dpadUnit, dpadUnit};
    SDL_Rect dpadDown = {dpadX - dpadUnit / 2, dpadY + dpadUnit / 2,
                         dpadUnit, dpadUnit};
    SDL_Rect dpadLeft = {dpadX - dpadUnit * 3 / 2, dpadY - dpadUnit / 2,
                         dpadUnit, dpadUnit};
    SDL_Rect dpadRight = {dpadX + dpadUnit / 2, dpadY - dpadUnit / 2,
                          dpadUnit, dpadUnit};
    SDL_Rect aButton = {windowWidth - sideWidth / 4 - buttonSize,
                        windowHeight * 58 / 100, buttonSize, buttonSize};
    SDL_Rect bButton = {windowWidth - sideWidth + sideWidth / 4,
                        windowHeight * 76 / 100, buttonSize, buttonSize};
    SDL_Rect selectButton = {sideWidth / 4, windowHeight / 4,
                             sideWidth / 2, windowHeight / 10};
    SDL_Rect startButton = {windowWidth - sideWidth * 3 / 4, windowHeight / 4,
                            sideWidth / 2, windowHeight / 10};
    SDL_Rect lButton = {sideWidth / 4, windowHeight / 20,
                        sideWidth / 2, windowHeight / 10};
    SDL_Rect rButton = {windowWidth - sideWidth * 3 / 4, windowHeight / 20,
                        sideWidth / 2, windowHeight / 10};

    touchKeys = 0;

    for (int i = 0; i < MAX_TOUCH_FINGERS; i++)
    {
        if (!touchFingers[i].active)
            continue;

        int x = touchFingers[i].x * windowWidth;
        int y = touchFingers[i].y * windowHeight;

        if (IsInsideRect(x, y, dpadUp)) touchKeys |= DPAD_UP;
        if (IsInsideRect(x, y, dpadDown)) touchKeys |= DPAD_DOWN;
        if (IsInsideRect(x, y, dpadLeft)) touchKeys |= DPAD_LEFT;
        if (IsInsideRect(x, y, dpadRight)) touchKeys |= DPAD_RIGHT;

        if (IsInsideRect(x, y, aButton)) touchKeys |= A_BUTTON;
        if (IsInsideRect(x, y, bButton)) touchKeys |= B_BUTTON;
        if (IsInsideRect(x, y, startButton)) touchKeys |= START_BUTTON;
        if (IsInsideRect(x, y, selectButton)) touchKeys |= SELECT_BUTTON;
        if (IsInsideRect(x, y, lButton)) touchKeys |= L_BUTTON;
        if (IsInsideRect(x, y, rButton)) touchKeys |= R_BUTTON;
    }
}

static void HandleTouchEvent(const SDL_TouchFingerEvent *event)
{
    int slot = -1;
    for (int i = 0; i < MAX_TOUCH_FINGERS; i++)
    {
        if (touchFingers[i].active && touchFingers[i].id == event->fingerId)
        {
            slot = i;
            break;
        }
        if (slot < 0 && !touchFingers[i].active)
            slot = i;
    }

    if (slot < 0)
        return;

    if (event->type == SDL_FINGERUP)
    {
        touchFingers[slot].active = false;
    }
    else
    {
        touchFingers[slot].id = event->fingerId;
        touchFingers[slot].x = event->x;
        touchFingers[slot].y = event->y;
        touchFingers[slot].active = true;
    }

    UpdateTouchKeys();
}

static const Uint8 *GetGlyph(char character)
{
    static const Uint8 glyphA[7] = {14, 17, 17, 31, 17, 17, 17};
    static const Uint8 glyphB[7] = {30, 17, 17, 30, 17, 17, 30};
    static const Uint8 glyphC[7] = {15, 16, 16, 16, 16, 16, 15};
    static const Uint8 glyphE[7] = {31, 16, 16, 30, 16, 16, 31};
    static const Uint8 glyphL[7] = {16, 16, 16, 16, 16, 16, 31};
    static const Uint8 glyphR[7] = {30, 17, 17, 30, 20, 18, 17};
    static const Uint8 glyphS[7] = {15, 16, 16, 14, 1, 1, 30};
    static const Uint8 glyphT[7] = {31, 4, 4, 4, 4, 4, 4};

    switch (character)
    {
    case 'A': return glyphA;
    case 'B': return glyphB;
    case 'C': return glyphC;
    case 'E': return glyphE;
    case 'L': return glyphL;
    case 'R': return glyphR;
    case 'S': return glyphS;
    case 'T': return glyphT;
    default:  return NULL;
    }
}

static void DrawControlLabel(SDL_Rect rect, const char *label)
{
    int length = SDL_strlen(label);
    int scale = MinInt(rect.h / 9, rect.w / (length * 6));
    if (scale < 1)
        scale = 1;
    int startX = rect.x + (rect.w - (length * 6 - 1) * scale) / 2;
    int startY = rect.y + (rect.h - 7 * scale) / 2;

    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 230);
    for (int character = 0; character < length; character++)
    {
        const Uint8 *glyph = GetGlyph(label[character]);
        if (glyph == NULL)
            continue;
        for (int row = 0; row < 7; row++)
        {
            for (int column = 0; column < 5; column++)
            {
                if (glyph[row] & (1 << (4 - column)))
                {
                    SDL_Rect pixel = {startX + (character * 6 + column) * scale,
                                      startY + row * scale, scale, scale};
                    SDL_RenderFillRect(sdlRenderer, &pixel);
                }
            }
        }
    }
}

static void DrawControlRect(SDL_Rect rect, bool pressed, const char *label)
{
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, pressed ? 150 : 65);
    SDL_RenderFillRect(sdlRenderer, &rect);
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, pressed ? 230 : 130);
    SDL_RenderDrawRect(sdlRenderer, &rect);
    if (label != NULL)
        DrawControlLabel(rect, label);
}

static void DrawTouchControls(void)
{
    int windowWidth;
    int windowHeight;
    SDL_GetWindowSize(sdlWindow, &windowWidth, &windowHeight);
    int sideWidth = GetControlSideWidth(windowWidth, windowHeight);
    int buttonSize = MinInt(sideWidth * 2 / 5, windowHeight / 6);
    int dpadUnit = MinInt(sideWidth / 3, windowHeight / 8);
    int dpadX = sideWidth * 2 / 3;
    int dpadY = windowHeight * 7 / 10;

    SDL_RenderSetLogicalSize(sdlRenderer, 0, 0);
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);

    DrawControlRect((SDL_Rect){dpadX - dpadUnit / 2, dpadY - dpadUnit * 3 / 2,
                               dpadUnit, dpadUnit}, touchKeys & DPAD_UP, NULL);
    DrawControlRect((SDL_Rect){dpadX - dpadUnit / 2, dpadY + dpadUnit / 2,
                               dpadUnit, dpadUnit}, touchKeys & DPAD_DOWN, NULL);
    DrawControlRect((SDL_Rect){dpadX - dpadUnit * 3 / 2, dpadY - dpadUnit / 2,
                               dpadUnit, dpadUnit}, touchKeys & DPAD_LEFT, NULL);
    DrawControlRect((SDL_Rect){dpadX + dpadUnit / 2, dpadY - dpadUnit / 2,
                               dpadUnit, dpadUnit}, touchKeys & DPAD_RIGHT, NULL);
    DrawControlRect((SDL_Rect){windowWidth - sideWidth / 4 - buttonSize,
                               windowHeight * 58 / 100, buttonSize, buttonSize}, touchKeys & A_BUTTON, "A");
    DrawControlRect((SDL_Rect){windowWidth - sideWidth + sideWidth / 4,
                               windowHeight * 76 / 100, buttonSize, buttonSize}, touchKeys & B_BUTTON, "B");
    DrawControlRect((SDL_Rect){windowWidth - sideWidth * 3 / 4, windowHeight / 4,
                               sideWidth / 2, windowHeight / 10}, touchKeys & START_BUTTON, "START");
    DrawControlRect((SDL_Rect){sideWidth / 4, windowHeight / 4,
                               sideWidth / 2, windowHeight / 10}, touchKeys & SELECT_BUTTON, "SELECT");
    DrawControlRect((SDL_Rect){sideWidth / 4, windowHeight / 20,
                               sideWidth / 2, windowHeight / 10}, touchKeys & L_BUTTON, "L");
    DrawControlRect((SDL_Rect){windowWidth - sideWidth * 3 / 4, windowHeight / 20,
                               sideWidth / 2, windowHeight / 10}, touchKeys & R_BUTTON, "R");

    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_NONE);
    SDL_RenderSetLogicalSize(sdlRenderer, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    SDL_RenderSetIntegerScale(sdlRenderer, SDL_TRUE);
}

static u16 ControllerButtonMask(Uint8 button)
{
    switch (button)
    {
    case SDL_CONTROLLER_BUTTON_A:             return A_BUTTON;
    case SDL_CONTROLLER_BUTTON_B:             return B_BUTTON;
    case SDL_CONTROLLER_BUTTON_BACK:          return SELECT_BUTTON;
    case SDL_CONTROLLER_BUTTON_START:         return START_BUTTON;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:  return L_BUTTON;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return R_BUTTON;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:       return DPAD_UP;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:     return DPAD_DOWN;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:     return DPAD_LEFT;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:    return DPAD_RIGHT;
    default:                                  return 0;
    }
}
#endif

// ---- Keyboard keys (Settings > Keyboard) ------------------------------------------

static void HandleBoundKey(SDL_Keycode key, bool8 down, bool8 repeat)
{
    int i;
    for (i = 0; i < KEYBIND_COUNT; i++)
    {
        if (sKeyMap[i] != key)
            continue;
        if (i == KEYBIND_FAST_FORWARD)
        {
            // Toggle mode: each press (not key repeat) turns fast forward on or off.
            if (down && !repeat && sPlatformSettings[PLATFORM_SETTING_FF_TOGGLE])
                SDL_AtomicSet(&sFfToggled, !SDL_AtomicGet(&sFfToggled));
            SDL_AtomicSet(&sFfKeyHeld, down);
        }
        else if (down)
            keyboardKeys |= sKeyBindButtons[i];
        else
            keyboardKeys &= ~sKeyBindButtons[i];
    }
}

/* Brennan 2026-11-21 10:34 ("put toggles on F1-F4 if you want me to verify each one live on Route 110"): F1-F6 each
 * switch one 3D effect off / on again (voxel_gl.h VOXEL_FX_*); the change goes to the stall log and the window title.
 * Not saved. (test script: "fxkey N") */
static void ToggleFx(unsigned k)
{
    /* (k: the bit in gVoxelFxOff, VOXEL_FX_*; F8, F9, F10, F12 are bits 7-10 - F11 is full screen) */
    static const char *const kNames[11] = { "cloud shadows", "glow (bloom)", "anti-aliasing",
                                            "3D every refresh", "depth blur", "crack fill", "magenta background (gap test)",
                                            "ground", "trees", "models (fences, buildings)", "far ground ring (belt)" };
    static const unsigned kKey[11] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12 };
    char title[256];
    int at;
    FILE *lf;

    gVoxelFxOff ^= 1u << k;
    at = snprintf(title, sizeof(title), "Pokemon Wally's Journey");
    for (unsigned b = 0; b < 11 && at < (int)sizeof(title) - 1; b++)
        if (gVoxelFxOff & (1u << b))
            at += snprintf(title + at, sizeof(title) - (size_t)at, b == 6 ? "%s F%u %s on" : "%s F%u %s off",
                           at == 23 ? " -" : ",", kKey[b], kNames[b]);
    SDL_SetWindowTitle(sdlWindow, title);
    lf = fopen("WallysJourney_stalls.txt", "a");
    if (lf != NULL)
    {
        fprintf(lf, "%8.1fs F%u: %s %s\n", SDL_GetTicks() / 1000.0, kKey[k], kNames[k],
                (gVoxelFxOff & (1u << k)) ? (k == 6 ? "ON" : "OFF") : (k == 6 ? "off again" : "on again"));
        fclose(lf);
    }
}

static void CaptureKey(SDL_Keycode key)
{
    int bind = SDL_AtomicGet(&sKeyCapture);
    int i;
    if (key != SDLK_ESCAPE && key != SDLK_F11)
    {
        for (i = 0; i < KEYBIND_COUNT; i++)
            if (i != bind && sKeyMap[i] == key)
                sKeyMap[i] = sKeyMap[bind]; // the other button takes this button's old key
        sKeyMap[bind] = key;
        StoreConfigFile();
    }
    keyboardKeys = 0;
    SDL_AtomicSet(&sFfKeyHeld, 0);
    SDL_AtomicSet(&sKeyCapture, -1);
}

void Platform_StartKeyCapture(u8 bind)
{
    if (bind < KEYBIND_COUNT)
        SDL_AtomicSet(&sKeyCapture, bind);
}

bool8 Platform_IsCapturingKey(void)
{
    return SDL_AtomicGet(&sKeyCapture) >= 0;
}

// Key names come from SDL in ASCII; turn them into the game's text encoding.
void Platform_GetKeyBindName(u8 bind, u8 *dst, u32 size)
{
    static const struct { char c; const char *name; } sSymbols[] = {
        {'\\', "Backslash"}, {'[', "L bracket"}, {']', "R bracket"}, {';', "Semicolon"}, {'\'', "Quote"},
        {'`', "Backquote"}, {'=', "Equals"}, {'+', "Plus"}, {'*', "Star"}, {'#', "Hash"}, {'<', "Less"}, {'>', "Greater"},
        {'_', "Underscore"}, {'^', "Caret"}, {'$', "Dollar"}, {'@', "At"}, {'|', "Bar"}, {'~', "Tilde"},
    };
    const char *name = (bind < KEYBIND_COUNT) ? SDL_GetKeyName(sKeyMap[bind]) : "";
    u32 n = 0;
    if (name[0] != '\0' && name[1] == '\0')
    {
        u32 i;
        for (i = 0; i < ARRAY_COUNT(sSymbols); i++)
            if (sSymbols[i].c == name[0])
                name = sSymbols[i].name;
    }
    if (name[0] == '\0')
        name = "None";
    for (; *name != '\0' && n + 1 < size; name++)
    {
        char c = *name;
        u8 out;
        if (c >= 'A' && c <= 'Z')      out = CHAR_A + (c - 'A');
        else if (c >= 'a' && c <= 'z') out = CHAR_a + (c - 'a');
        else if (c >= '0' && c <= '9') out = CHAR_0 + (c - '0');
        else if (c == ' ')             out = CHAR_SPACE;
        else if (c == '-')             out = CHAR_HYPHEN;
        else if (c == '.')             out = CHAR_PERIOD;
        else if (c == ',')             out = CHAR_COMMA;
        else if (c == '/')             out = CHAR_SLASH;
        else if (c == '!')             out = CHAR_EXCL_MARK;
        else if (c == '?')             out = CHAR_QUESTION_MARK;
        else if (c == '&')             out = CHAR_AMPERSAND;
        else if (c == '(')             out = CHAR_LEFT_PAREN;
        else if (c == ')')             out = CHAR_RIGHT_PAREN;
        else if (c == '%')             out = CHAR_PERCENT;
        else                           out = CHAR_QUESTION_MARK;
        dst[n++] = out;
    }
    dst[n] = EOS;
}

void ProcessEvents(void)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            isRunning = false;
            break;
#ifdef __ANDROID__
        case SDL_CONTROLLERDEVICEADDED:
            if (androidController == NULL && SDL_IsGameController(event.cdevice.which))
                androidController = SDL_GameControllerOpen(event.cdevice.which);
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            if (androidController != NULL
             && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(androidController)) == event.cdevice.which)
            {
                SDL_GameControllerClose(androidController);
                androidController = NULL;
                controllerKeys = 0;
                controllerAxisKeys = 0;
                controllerAxisX = 0;
                controllerAxisY = 0;
            }
            break;
        case SDL_CONTROLLERBUTTONDOWN:
            controllerKeys |= ControllerButtonMask(event.cbutton.button);
            break;
        case SDL_CONTROLLERBUTTONUP:
            controllerKeys &= ~ControllerButtonMask(event.cbutton.button);
            break;
        case SDL_CONTROLLERAXISMOTION:
            if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
                controllerAxisX = event.caxis.value;
            else if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
                controllerAxisY = event.caxis.value;

            controllerAxisKeys = 0;
            if (controllerAxisX < -16000) controllerAxisKeys |= DPAD_LEFT;
            if (controllerAxisX >  16000) controllerAxisKeys |= DPAD_RIGHT;
            if (controllerAxisY < -16000) controllerAxisKeys |= DPAD_UP;
            if (controllerAxisY >  16000) controllerAxisKeys |= DPAD_DOWN;
            break;
#endif
        case SDL_KEYUP:
            if (SDL_AtomicGet(&sKeyCapture) < 0)
                HandleBoundKey(event.key.keysym.sym, FALSE, event.key.repeat);
            break;
        case SDL_MOUSEWHEEL:
            if (event.wheel.y != 0)
                ZoomStep(event.wheel.y > 0 ? -1 : +1);
            break;
        case SDL_KEYDOWN:
            if (SDL_AtomicGet(&sKeyCapture) >= 0)
            {
                CaptureKey(event.key.keysym.sym);
                break;
            }
            HandleBoundKey(event.key.keysym.sym, TRUE, event.key.repeat);
            switch (event.key.keysym.sym)
            {
            case SDLK_MINUS:
            case SDLK_KP_MINUS:
                ZoomStep(+1); // zoom out: see more
                break;
            case SDLK_EQUALS:
            case SDLK_KP_PLUS:
                ZoomStep(-1); // zoom in
                break;
            case SDLK_F1: case SDLK_F2: case SDLK_F3: case SDLK_F4: case SDLK_F5: case SDLK_F6: case SDLK_F7:
            case SDLK_F8: case SDLK_F9: case SDLK_F10:
                ToggleFx((unsigned)(event.key.keysym.sym - SDLK_F1));
                break;
            case SDLK_F12:
                ToggleFx(10); // the far ground ring (F11 is full screen)
                break;
            case SDLK_F11:
            {
                // F11: toggle fullscreen
                Platform_SetSetting(PLATFORM_SETTING_FULLSCREEN, !sPlatformSettings[PLATFORM_SETTING_FULLSCREEN]);
                break;
            }
            case SDLK_r:
                if (event.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL))
                {
                    DoSoftReset();
                }
                break;
            case SDLK_p:
                if (event.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL))
                {
                    paused = !paused;
                }
                break;
            }
            break;
        }
    }
}

#ifdef _WIN32
#define STICK_THRESHOLD 0.5f
u16 GetXInputKeys()
{
    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(XINPUT_STATE));

    DWORD dwResult = XInputGetState(0, &state);
    u16 xinputKeys = 0;

    if (dwResult == ERROR_SUCCESS)
    {
        static const WORD sPadMasks[PAD_BUTTON_COUNT] = {
            [PAD_BUTTON_A] = XINPUT_GAMEPAD_A,
            [PAD_BUTTON_B] = XINPUT_GAMEPAD_B,
            [PAD_BUTTON_X] = XINPUT_GAMEPAD_X,
            [PAD_BUTTON_Y] = XINPUT_GAMEPAD_Y,
            [PAD_BUTTON_LB] = XINPUT_GAMEPAD_LEFT_SHOULDER,
            [PAD_BUTTON_RB] = XINPUT_GAMEPAD_RIGHT_SHOULDER,
            [PAD_BUTTON_BACK] = XINPUT_GAMEPAD_BACK,
            [PAD_BUTTON_START] = XINPUT_GAMEPAD_START,
            [PAD_BUTTON_LSTICK] = XINPUT_GAMEPAD_LEFT_THUMB,
            [PAD_BUTTON_RSTICK] = XINPUT_GAMEPAD_RIGHT_THUMB,
        };
        static const u16 sGameButtons[] = {A_BUTTON, B_BUTTON, START_BUTTON, SELECT_BUTTON, L_BUTTON, R_BUTTON};
        bool8 pressed[PAD_BUTTON_COUNT];
        int i;
        for (i = 0; i < PAD_BUTTON_COUNT; i++)
            pressed[i] = (sPadMasks[i] != 0 && (state.Gamepad.wButtons & sPadMasks[i]) != 0);
        pressed[PAD_BUTTON_LT] = state.Gamepad.bLeftTrigger > 0x80;
        pressed[PAD_BUTTON_RT] = state.Gamepad.bRightTrigger > 0x80;
        pressed[PAD_BUTTON_NONE] = FALSE;
        for (i = 0; i < 6; i++)
            if (pressed[sPlatformSettings[PLATFORM_SETTING_PAD_A + i] % PAD_BUTTON_COUNT])
                xinputKeys |= sGameButtons[i];
        /* D-pad */
        if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)    xinputKeys |= DPAD_UP;
        if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)  xinputKeys |= DPAD_DOWN;
        if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)  xinputKeys |= DPAD_LEFT;
        if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) xinputKeys |= DPAD_RIGHT;

        /* Fast forward button */
        {
            static bool8 sWasHeld;
            bool8 held = pressed[sPlatformSettings[PLATFORM_SETTING_PAD_FF] % PAD_BUTTON_COUNT];
            if (held && !sWasHeld && sPlatformSettings[PLATFORM_SETTING_FF_TOGGLE])
                SDL_AtomicSet(&sFfToggled, !SDL_AtomicGet(&sFfToggled));
            sWasHeld = held;
            SDL_AtomicSet(&sFfPadHeld, held);
        }

        /* Control Stick */
        float xAxis = (float)state.Gamepad.sThumbLX / (float)SHRT_MAX;
        float yAxis = (float)state.Gamepad.sThumbLY / (float)SHRT_MAX;

        if (xAxis < -STICK_THRESHOLD) xinputKeys |= DPAD_LEFT;
        if (xAxis >  STICK_THRESHOLD) xinputKeys |= DPAD_RIGHT;
        if (yAxis < -STICK_THRESHOLD) xinputKeys |= DPAD_DOWN;
        if (yAxis >  STICK_THRESHOLD) xinputKeys |= DPAD_UP;


    }
    else
    {
        SDL_AtomicSet(&sFfPadHeld, 0);
    }

    return xinputKeys;
}
#endif // _WIN32

// Main thread, every loop: turn fast forward on/off from the key, the controller and the mode.
static void ApplyFastForward(void)
{
    bool on = sPlatformSettings[PLATFORM_SETTING_FF_TOGGLE]
            ? SDL_AtomicGet(&sFfToggled) != 0
            : (SDL_AtomicGet(&sFfKeyHeld) || SDL_AtomicGet(&sFfPadHeld));
    // Fast forward speed: 2x, 4x, 6x ... 20x.
    double scale = on ? 2.0 * (sPlatformSettings[PLATFORM_SETTING_FF_SPEED] % FF_SPEED_COUNT + 1) : 1.0;
    if (sTestFfRealtime > 1)
    {
        on = true;
        scale = (double)sTestFfRealtime;
    }
    /* Project Tandem T5: linked with a friend (a trade), both games run at the same speed - the game's link code
     * counts frames, and a game running 4x would leave the friend's far behind */
    if (TandemLink_InSession())
    {
        on = false;
        scale = 1.0;
    }

    if (on != speedUp || scale != timeScale)
    {
        // The sound keeps playing: the game runs faster, the sound engine stays at normal speed.
        speedUp = on;
        timeScale = scale;
    }
}

// Fast forward keeps the music at its normal speed and pitch: the game runs N frames in the time
// of one, but the sound engine (sequencer + mixer) only runs often enough to keep the sound card
// fed in real time. Called once per game frame by m4aSoundMain; m4aSoundVSync asks
// Platform_SoundFrameSkipped whether that frame made any sound.
static bool sSoundFrameSkipped;
static unsigned int sTestFfFrame;

bool8 Platform_SoundFrameDue(void)
{
    bool due = true;

    if (sTestScript != NULL && !sScriptRealtime)
    {
        // Scripts run without a clock: exactly one sound frame every N game frames.
        if (sTestFfSpeed > 1 && !getenv("WJPC_FF_NOSOUNDGATE"))
            due = (sTestFfFrame++ % sTestFfSpeed) == 0;
    }
    else if (speedUp && sdlAudioDevice != 0)
    {
        // Real time: make a sound frame only while less than ~50 ms is waiting to be played.
        due = SDL_GetQueuedAudioSize(sdlAudioDevice) < 2048 * 2 * sizeof(float);
    }
    else if (speedUp)
    {
        // No sound card: one sound frame every N game frames.
        unsigned int n = (unsigned int)(timeScale + 0.5);
        due = n < 2 || (sTestFfFrame++ % n) == 0;
    }
    sSoundFrameSkipped = !due;
    return due;
}

u32 Platform_GameSpeedQ8(void)
{
    static int sOld = -1;
    if (sOld < 0)
        sOld = getenv("WJPC_LEAVES_GAME_SPEED") != NULL; // (the leaves by game frames, as before)
    if (sOld)
        return 256u;
    if (sTestScript != NULL && !sScriptRealtime)
        return sTestFfSpeed > 1 ? 256u * sTestFfSpeed : 256u; // (scripts: "ff N")
    return sGameSpeedQ8;
}

bool8 Platform_SoundFrameSkipped(void)
{
    return sSoundFrameSkipped;
}

bool8 WJPC_LockRandom(void)
{
    static int sLock = -1;
    if (sLock < 0)
        sLock = getenv("WJPC_RNG_LOCK") != NULL;
    return sLock;
}

bool8 WJPC_NoVBlankRandom(void)
{
    static int sNo = -1;
    if (sNo < 0)
        sNo = getenv("WJPC_NO_VBLANK_RNG") != NULL;
    return sNo;
}

u16 Platform_GetKeyInput(void)
{
#ifdef _WIN32
    u16 gamepadKeys = GetXInputKeys();
    return gamepadKeys | keyboardKeys;
#elif defined(__ANDROID__)
    return keyboardKeys | controllerKeys | controllerAxisKeys;
#endif

    return keyboardKeys;
}


static inline uint32_t ToArgb(uint16_t color);
// ---- DLX-5: the 3D battle's picture over the world (Emerald 3DS, 3ds_video.c) ----
static bool sBattleShow;          // the world is the scenery this frame
static unsigned sBattleLayers;    // pixel sources the world stands in for (gPcPixelSource numbering)

// The battle's own picture with the world's layers see-through (carrying their brightness fade: BG3's
// brightness is the world's).
static void BuildBattleOverlay(const uint16_t *pic, uint32_t *out)
{
    extern u8 gPcPixelSource[];
    extern s8 gPcPixelFade[];
    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
    {
        u8 src = gPcPixelSource[i];
        if (src < 8 && (sBattleLayers & (1u << src)))
        {
            int f = gPcPixelFade[i];
            out[i] = f > 0 ? ((uint32_t)(f * 255 / 16) << 24) | 0xFFFFFF : f < 0 ? ((uint32_t)(-f * 255 / 16) << 24) : 0;
        }
        else
            out[i] = ToArgb(pic[i]);
    }
}

// A colour with alpha laid over the see-through part of the overlay only (what is over the world, under the
// battle's own picture): the world's colour there is under it, so the two blend as A over B.
static inline void UnderBattle(uint32_t *px, float r, float g, float b, float a)
{
    uint32_t d = *px;
    float da = (d >> 24) / 255.0f;
    if (da >= 0.999f || a <= 0.0f)
        return; // the battle's own pixel
    {
        float dr = ((d >> 16) & 255) / 255.0f, dg = ((d >> 8) & 255) / 255.0f, db = (d & 255) / 255.0f;
        float oa = a + da * (1.0f - a);
        float orr = (r * a + dr * da * (1.0f - a)) / oa, og = (g * a + dg * da * (1.0f - a)) / oa, ob = (b * a + db * da * (1.0f - a)) / oa;
        *px = ((uint32_t)(oa * 255.0f + 0.5f) << 24) | ((uint32_t)(orr * 255.0f + 0.5f) << 16)
            | ((uint32_t)(og * 255.0f + 0.5f) << 8) | (uint32_t)(ob * 255.0f + 0.5f);
    }
}

// 3DS top-screen point -> GBA pixel (the GBA picture lies on it at 1.4x, its (120, 112) on (200, 192)).
static float BattleGbaX(float sx) { return 120.0f + (sx - 200.0f) / 1.4f; }
static float BattleGbaY(float sy) { return 112.0f + (sy - 192.0f) / 1.4f; }

// A rectangle of the 3DS screen with a colour at each corner (C2D_DrawRectangle), laid under the battle.
static void BattleRect(uint32_t *img, float x, float y, float w, float h, const float c[4][4])
{
    float gx0 = BattleGbaX(x), gy0 = BattleGbaY(y), gx1 = BattleGbaX(x + w), gy1 = BattleGbaY(y + h);
    for (int py = (int)floorf(gy0); py < (int)ceilf(gy1); py++)
        for (int px = (int)floorf(gx0); px < (int)ceilf(gx1); px++)
        {
            if (px < 0 || py < 0 || px >= DISPLAY_WIDTH || py >= DISPLAY_HEIGHT)
                continue;
            float u = ((px + 0.5f) - gx0) / (gx1 - gx0), v = ((py + 0.5f) - gy0) / (gy1 - gy0);
            if (u < 0 || u > 1 || v < 0 || v > 1)
                continue;
            float k[4];
            for (int ch = 0; ch < 4; ch++)
                k[ch] = (c[0][ch] * (1 - u) + c[1][ch] * u) * (1 - v) + (c[2][ch] * (1 - u) + c[3][ch] * u) * v;
            UnderBattle(&img[py * DISPLAY_WIDTH + px], k[0], k[1], k[2], k[3]);
        }
}

static void BattleEffects(uint32_t *img)
{
    Voxel3DBattleIntro intro;
    bool shown = !(REG_DISPCNT & 0x80);

    Voxel3D_BattleIntro(&intro);
    if (shown)
    {
        // BattleWorldShadows: a soft shadow on the ground under each battler, the world's dark blue
        // (8, 20, 40), 0.5 x d^1.5 from the middle (d = 1 - r^2), stretched to rx, ry GBA pixels.
        VoxelBattleShadow shadows[4];
        unsigned count = VoxelBattle_Shadows(shadows, 4);
        for (unsigned n = 0; n < count; n++)
            for (int py = (int)(shadows[n].y - shadows[n].ry); py <= (int)(shadows[n].y + shadows[n].ry); py++)
                for (int px = (int)(shadows[n].x - shadows[n].rx); px <= (int)(shadows[n].x + shadows[n].rx); px++)
                {
                    if (px < 0 || py < 0 || px >= DISPLAY_WIDTH || py >= DISPLAY_HEIGHT)
                        continue;
                    float u = (px + 0.5f - shadows[n].x) / shadows[n].rx, v = (py + 0.5f - shadows[n].y) / shadows[n].ry;
                    float d = 1.0f - (u * u + v * v);
                    if (d > 0.0f)
                        UnderBattle(&img[py * DISPLAY_WIDTH + px], 8 / 255.0f, 20 / 255.0f, 40 / 255.0f, 0.5f * d * sqrtf(d));
                }
        // BattleIntroAtmosphere: speed lines while it travels, a dark indigo vignette as high as it climbs.
        {
            const float w = 400.0f, bottom = 80.0f + 112.0f; // BATTLE_INTRO_FLOOR: CTR_BATTLE_Y + BATTLE_BAND_TOP
            float shade = fmaxf(0.60f * intro.flight, 0.30f * intro.bars);
            if (intro.speed > 0.04f)
                for (unsigned i = 0; i < 12u; ++i)
                {
                    uint32_t hh = (i + 1) * 2654435761u;
                    float y = 6.0f + (float)((hh >> 8) % (unsigned)(bottom - 12.0f));
                    float length = 70.0f + (float)((hh >> 16) % 110u);
                    float pace = 26.0f + (float)((hh >> 20) % 18u);
                    float x = w - fmodf(intro.time * pace + (float)(hh % 400u), w + length);
                    float alpha = intro.speed * (0.35f + 0.35f * (float)((hh >> 4) & 3) / 3.0f);
                    float c[4][4] = {{225 / 255.0f, 236 / 255.0f, 1, alpha}, {225 / 255.0f, 236 / 255.0f, 1, 0},
                                     {225 / 255.0f, 236 / 255.0f, 1, alpha}, {225 / 255.0f, 236 / 255.0f, 1, 0}};
                    if (fabsf(y - bottom * 0.55f) < 24.0f)
                        y = y < bottom * 0.55f ? y - 24.0f : y + 24.0f;
                    BattleRect(img, x, y, length, (hh & 16) ? 2.0f : 1.0f, c);
                }
            if (shade > 0.01f)
            {
                float a = fminf(shade, 1.0f) * 230.0f / 255.0f, r = 8 / 255.0f, g = 6 / 255.0f, b = 34 / 255.0f;
                float band = 64.0f, side = 96.0f, low = bottom;
                float top[4][4] = {{r, g, b, a}, {r, g, b, a}, {r, g, b, 0}, {r, g, b, 0}};
                float bot[4][4] = {{r, g, b, 0}, {r, g, b, 0}, {r, g, b, a}, {r, g, b, a}};
                float lef[4][4] = {{r, g, b, a}, {r, g, b, 0}, {r, g, b, a}, {r, g, b, 0}};
                float rig[4][4] = {{r, g, b, 0}, {r, g, b, a}, {r, g, b, 0}, {r, g, b, a}};
                BattleRect(img, 0, 0, w, band, top);
                BattleRect(img, 0, low - band, w, band, bot);
                BattleRect(img, 0, 0, side, low, lef);
                BattleRect(img, w - side, 0, side, low, rig);
            }
        }
        // BattleIntroFlash: the landing's flash over the world.
        if (intro.impact > 0.01f)
        {
            float a = fminf(intro.impact, 1.0f) * 170.0f / 255.0f;
            for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
                UnderBattle(&img[i], 1.0f, 250 / 255.0f, 235 / 255.0f, a);
        }
        // BattleIntroBars: over everything. On the 3DS they sit on its screen's top and bottom edges; here on
        // this picture's (each 22 3DS pixels closed, a 10-pixel soft edge: /1.4 in GBA pixels).
        if (intro.bars > 0.01f)
        {
            float bar = 22.0f / 1.4f * intro.bars, edge = 10.0f / 1.4f * intro.bars;
            for (int py = 0; py < DISPLAY_HEIGHT; py++)
            {
                float yy = py + 0.5f, d = fminf(yy, DISPLAY_HEIGHT - yy), a;
                a = d < bar ? 1.0f : d < bar + edge ? 1.0f - (d - bar) / edge : 0.0f;
                if (a <= 0.0f)
                    continue;
                for (int px = 0; px < DISPLAY_WIDTH; px++)
                {
                    uint32_t c = img[px + py * DISPLAY_WIDTH];
                    float ca = (c >> 24) / 255.0f, oa = a + ca * (1 - a);
                    float rr = (((c >> 16) & 255) * ca * (1 - a)) / oa, gg = (((c >> 8) & 255) * ca * (1 - a)) / oa, bb = ((c & 255) * ca * (1 - a)) / oa;
                    img[px + py * DISPLAY_WIDTH] = ((uint32_t)(oa * 255 + 0.5f) << 24) | ((uint32_t)rr << 16) | ((uint32_t)gg << 8) | (uint32_t)bb;
                }
            }
        }
    }
}

static uint16_t *sVDrawImage;
static void DrawFrameGuarded(void)
{
    WideView_DrawFrame(sVDrawImage, &sFrameInfo);
}

static inline uint32_t ToArgb(uint16_t color)
{
    uint32_t r = (color & 0x1F) * 255 / 31;
    uint32_t g = ((color >> 5) & 0x1F) * 255 / 31;
    uint32_t b = ((color >> 10) & 0x1F) * 255 / 31;
    return 0xFF000000 | (r << 16) | (g << 8) | b;
}

void VDraw(SDL_Texture *texture)
{
    static uint16_t gbaImage[TEX_W * TEX_H];
    static uint32_t image[TEX_W * TEX_H];
    static uint16_t overlay[TEX_W * TEX_H];
    static uint32_t overlayImage[TEX_W * TEX_H];
    static uint16_t uiLayer[TEX_W * TEX_H];
    static uint32_t uiImage[TEX_W * TEX_H];
    static uint32_t lastImage[TEX_W * TEX_H];
    static int lastW, lastH;
    Uint64 vdStart = SDL_GetPerformanceCounter(), vdMark;
    bool smooth = sPlatformSettings[PLATFORM_SETTING_SMOOTH] && (sTestScript == NULL || sScriptRealtime) && sdlOverlayTexture != NULL;
    int prevW = sFrameInfo.width, prevH = sFrameInfo.height, prevMargin = sFrameInfo.margin;

    {
        extern bool8 gWjPc3DField;
        gWjPc3DField = s3DReady && s3DWanted; // DLX-18: people load as far out as the 3D view draws them
    }
    {
        // Zoom: ask for a view that fills the screen at a whole number of screen pixels per
        // game pixel (worked out on the main thread, which owns the window).
        int sw, sh;
        GetSceneSize(&sw, &sh);
        ComputeFieldView(sw, sh);
        WideView_SetViewSize(sFieldViewW, sFieldViewH);
    }
    /* Brennan 2026-11-21 02:12 (240 fps; audit docs/polish_reports/PERF_AUDIT_2026-11-21.md): the 3D world was the
     * field's picture last frame, so this frame's CPU picture is only its UI layer (wide_view.c gWjPcUiOnly): no map
     * layers, no lines outside the GBA screen, no smooth-motion overlay, and the unused world picture is not
     * converted, copied or uploaded. WJPC_FULL_2D_UNDER_3D=1: everything drawn, as before. */
    static int sFull2D = -1;
    if (sFull2D < 0)
        sFull2D = getenv("WJPC_FULL_2D_UNDER_3D") != NULL;
    {
        extern bool8 gWjPcUiOnly;
        bool uiOnly = !sFull2D && s3DReady && s3DWanted && s3DFrame && !s3DTransition && sFrameInfo.mode == WIDE_VIEW_FIELD;
        gWjPcUiOnly = uiOnly;
        if (uiOnly)
            smooth = false;
    }
    WideView_SetSmooth(smooth ? overlay : NULL, SMOOTH_M);
    WideView_SetUiOverlay(sdlUiTexture != NULL ? uiLayer : NULL);
    sVDrawImage = gbaImage;
    Uint64 stall2D = SDL_GetPerformanceCounter();
    sVdSetup += stall2D - vdStart;
    if (!PcGuard_Call0(DrawFrameGuarded))
    {
        // The wide view hit a bad memory access: fall back to the normal picture for good.
        WideView_SetEnabled(FALSE);
        sVDrawImage = gbaImage;
        PcGuard_Call0(DrawFrameGuarded);
    }
    vdMark = SDL_GetPerformanceCounter();
    sStall2D += vdMark - stall2D;
    sStallFieldW = sFrameInfo.width;
    sStallFieldH = sFrameInfo.height;
    if (getenv("WJPC_VDRAW_TIME") != NULL) // test aid: the CPU picture's cost, every 60 frames
    {
        static double acc;
        static unsigned n;
        acc += (SDL_GetPerformanceCounter() - stall2D) * 1000.0 / SDL_GetPerformanceFrequency();
        if (++n % 60 == 0)
        {
            FILE *tf = fopen(getenv("WJPC_VDRAW_TIME"), "a");
            if (tf != NULL)
            {
                fprintf(tf, "[vdraw] field %dx%d: 2D picture %.2f ms/frame\n", sFrameInfo.width, sFrameInfo.height, acc / 60.0);
                fclose(tf);
            }
            acc = 0.0;
        }
    }
    WideView_SetSmooth(NULL, 0);
    WideView_SetUiOverlay(NULL);

    int fullW = sFrameInfo.width + 2 * sFrameInfo.margin, fullH = sFrameInfo.height + 2 * sFrameInfo.margin;
    extern bool8 gWjPcUiOnly;
    bool uiOnlyField = gWjPcUiOnly && sFrameInfo.mode == WIDE_VIEW_FIELD; // (the world picture: not made)
    if (uiOnlyField)
        ;
    else if (sGammaWanted && sFrameInfo.mode == WIDE_VIEW_FIELD && (!WideView_IsIndoorMap() || WideView_GammaRoom() != 0) && !(s3DReady && s3DWanted))
    {
        // DLX-9: Gamma Emerald's daylight on the 2D overworld (same switch as 3D's)
        static unsigned frames;
        Uint64 t0 = SDL_GetPerformanceCounter();
        Gamma2D_Apply(image, gbaImage, fullW, fullH, sFrameInfo.camX - sFrameInfo.margin,
                      sFrameInfo.camY - sFrameInfo.margin, frames++, s3DDir, WideView_GammaRoom());
        if (getenv("WJPC_GAMMA2D_TIME") != NULL)
        {
            static double total;
            total += (double)(SDL_GetPerformanceCounter() - t0) * 1000.0 / SDL_GetPerformanceFrequency();
            if (frames % 120 == 0)
            {
                FILE *tf = fopen(getenv("WJPC_GAMMA2D_TIME"), "a");
                if (tf != NULL)
                {
                    fprintf(tf, "[gamma2d] %dx%d %.2f ms/frame\n", fullW, fullH, total / 120.0);
                    fclose(tf);
                }
                total = 0;
            }
        }
    }
    else
    for (int i = 0; i < fullW * fullH; i++)
        image[i] = ToArgb(gbaImage[i]);
    if (uiOnlyField)
        lastW = 0; // (the next whole picture is not blended with an old one)
    else if (sPlatformSettings[PLATFORM_SETTING_INTERFRAME] && lastW == fullW && lastH == fullH)
    {
        // Interframe blending (as in mGBA): mix with the previous frame, for effects that flicker.
        for (int i = 0; i < fullW * fullH; i++)
        {
            uint32_t a = image[i], b = lastImage[i];
            lastImage[i] = a;
            image[i] = 0xFF000000 | ((((a & 0xFEFEFE) >> 1) + ((b & 0xFEFEFE) >> 1)) & 0xFFFFFF);
        }
    }
    else
    {
        memcpy(lastImage, image, fullW * fullH * sizeof(uint32_t));
    }
    if (!uiOnlyField)
    {
        lastW = fullW;
        lastH = fullH;
    }
    SDL_Rect area = {0, 0, fullW, fullH};
    s2DTransition = !(s3DReady && s3DWanted) && sHaveLastField && sFrameInfo.mode != WIDE_VIEW_FIELD
                 && WideView_BattleTransitionOnField() && getenv("WJPC_OLD_2D_TRANSITION") == NULL;
    static uint32_t sFieldImage[TEX_W * TEX_H]; // the field picture the transition starts from
    if (sFrameInfo.mode == WIDE_VIEW_FIELD && uiOnlyField)
        sHaveLastField = false; // (no field picture this frame; the 3D world is the transition's start)
    else if (sFrameInfo.mode == WIDE_VIEW_FIELD)
    {
        sLastField = sFrameInfo;
        sHaveLastField = true;
        memcpy(sFieldImage, image, (size_t)fullW * fullH * sizeof(uint32_t));
    }
    else if (!WideView_BattleTransitionOnField())
        sHaveLastField = false; // a battle, a menu: the next transition starts from a new field picture
    if (!s2DTransition && !uiOnlyField)
        STALL_GL(SDL_UpdateTexture(texture, &area, image, fullW * sizeof(Uint32)));
    if (fullW == DISPLAY_WIDTH && fullH == DISPLAY_HEIGHT)
        memcpy(sLastUploaded, image, sizeof(sLastUploaded));
    else if (uiOnlyField && getenv("WJPC_FADE_FIT_UNDER_3D") == NULL)
        ; /* Brennan 2026-11-21 15:33 (21i lag): under the 3D world the 2D field picture this fitted and sent every
           * frame is not shown (nor made: its copy here is an old one). WJPC_FADE_FIT_UNDER_3D=1: as before */
    else
    {
        // The transitions also fade by palette (flashes, fades to black or white): the GBA picture's map
        // pixels show it. A line per colour channel, fitted from the field picture to those pixels, is
        // put on the whole field picture (the camera does not move during a transition).
        extern u8 gPcPixelSource[];
        int fw = sLastField.width + 2 * sLastField.margin, fh = sLastField.height + 2 * sLastField.margin;
        int ox = sFrameInfo.mode == WIDE_VIEW_GBA ? 0 : (sFrameInfo.width - DISPLAY_WIDTH) / 2;
        int oy = sFrameInfo.mode == WIDE_VIEW_GBA ? 0 : (sFrameInfo.height - DISPLAY_HEIGHT) / 2;
        double n = 0, sx[3] = {0}, sy[3] = {0}, sxx[3] = {0}, sxy[3] = {0};
        static float ka[3] = {1, 1, 1}, kb[3] = {0, 0, 0};
        static uint32_t faded[TEX_W * TEX_H];
        for (int y = 0; y < DISPLAY_HEIGHT; y += 2)
            for (int x = 0; x < DISPLAY_WIDTH; x += 2)
            {
                u8 src = gPcPixelSource[y * DISPLAY_WIDTH + x];
                int fx = sLastField.gbaX + x, fy = sLastField.gbaY + y;
                if (src < 2 || src > 5 || fx < 0 || fy < 0 || fx >= fw || fy >= fh)
                    continue;
                uint32_t a = sFieldImage[fy * fw + fx], b = ToArgb(gbaImage[(y + oy) * sFrameInfo.width + x + ox]);
                for (int c = 0; c < 3; c++)
                {
                    double u = (a >> (16 - 8 * c)) & 0xFF, v = (b >> (16 - 8 * c)) & 0xFF;
                    sx[c] += u; sy[c] += v; sxx[c] += u * u; sxy[c] += u * v;
                }
                n++;
            }
        if (n >= 64)
            for (int c = 0; c < 3; c++)
            {
                double var = sxx[c] - sx[c] * sx[c] / n;
                ka[c] = var > 1e-3 * n ? (float)((sxy[c] - sx[c] * sy[c] / n) / var) : 1.0f;
                kb[c] = (float)((sy[c] - ka[c] * sx[c]) / n);
            }
        for (int i = 0; i < fw * fh; i++)
        {
            uint32_t a = sFieldImage[i], o = 0xFF000000u;
            for (int c = 0; c < 3; c++)
            {
                float v = ka[c] * (float)((a >> (16 - 8 * c)) & 0xFF) + kb[c];
                o |= (uint32_t)(v < 0 ? 0 : v > 255 ? 255 : (int)(v + 0.5f)) << (16 - 8 * c);
            }
            faded[i] = o;
        }
        SDL_Rect fieldArea = {0, 0, fw, fh};
        STALL_GL(SDL_UpdateTexture(texture, &fieldArea, faded, fw * sizeof(Uint32)));
    }

    s3DFrame = false;
    s3DTransition = s3DReady && s3DWanted && sFrameInfo.mode != WIDE_VIEW_FIELD && WideView_BattleTransitionOnField();
    if (s3DTransition || s2DTransition)
    {
        // A battle transition over the 3D world: what the GBA picture draws from the map (its
        // BGs and sprites) is left to the 3D view, with the fade it has; the rest (the wipes'
        // black, the transition's own graphics on BG0) goes over it.
        extern u8 gPcPixelSource[];
        extern s8 gPcPixelFade[];
        static uint32_t sTransImage[DISPLAY_WIDTH * DISPLAY_HEIGHT];
        int ox = sFrameInfo.mode == WIDE_VIEW_GBA ? 0 : (sFrameInfo.width - DISPLAY_WIDTH) / 2;
        int oy = sFrameInfo.mode == WIDE_VIEW_GBA ? 0 : (sFrameInfo.height - DISPLAY_HEIGHT) / 2;
        int stride = sFrameInfo.width;
        if (sdl3DTransTexture == NULL)
        {
            sdl3DTransTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                                  DISPLAY_WIDTH, DISPLAY_HEIGHT);
            if (sdl3DTransTexture != NULL)
                SDL_SetTextureBlendMode(sdl3DTransTexture, SDL_BLENDMODE_BLEND);
        }
        for (int y = 0; y < DISPLAY_HEIGHT; y++)
            for (int x = 0; x < DISPLAY_WIDTH; x++)
            {
                int i = y * DISPLAY_WIDTH + x;
                u8 src = gPcPixelSource[i];
                if (src == 2 || src == 3 || src == 4 || src == 5)
                {
                    int f = gPcPixelFade[i];
                    sTransImage[i] = f > 0 ? ((uint32_t)(f * 255 / 16) << 24) | 0xFFFFFF
                                   : f < 0 ? ((uint32_t)(-f * 255 / 16) << 24) : 0;
                }
                else
                    sTransImage[i] = ToArgb(gbaImage[(y + oy) * stride + x + ox]);
            }
        if (sdl3DTransTexture != NULL)
            STALL_GL(SDL_UpdateTexture(sdl3DTransTexture, NULL, sTransImage, DISPLAY_WIDTH * sizeof(uint32_t)));
    }
    if ((s3DWanted || s3DBattleWanted) && !s3DReady)
        Try3DInit(); // turned on in Settings
    // DLX-5: a battle over the 3D world, as Emerald 3DS does it (3ds_video.c): the world is the battle's
    // scenery (BG3) while BG3 is on and holds the scenery (not a move's background); while the intro slides
    // the scenery in, BG1/BG2 (its entry picture) are the world's too (not in a link battle). Where the
    // windows hide BG3 (the intro's curtain) the backdrop shows, as on the GBA. The battle's own picture
    // goes on top; the world's shadows, the entrance effects and the cinema bars go in with it.
    s3DBattleFrame = false;
    sBattleShow = false;
    {
        extern bool8 PcBattleIntro_Sliding(void);
        extern bool8 PcBattleBg_MoveBgShown(void);
        bool battle = s3DReady && s3DBattleWanted && !s3DTransition && VoxelBattle_GameInBattle()
                   && sFrameInfo.mode != WIDE_VIEW_FIELD
                   && sFrameInfo.width == DISPLAY_WIDTH && sFrameInfo.height == DISPLAY_HEIGHT;
        Voxel3D_SetBattle(battle);
        if (battle)
        {
            bool sliding = PcBattleIntro_Sliding() != 0, moveBg = PcBattleBg_MoveBgShown() != 0;
            Voxel3D_SetBattleFrame(sliding, sliding || moveBg ? 0.0f : (float)((int)((REG_BG3HOFS + 128) & 255) - 128),
                                   sliding || moveBg ? 0.0f : (float)((int)((REG_BG3VOFS + 128) & 255) - 128));
            sBattleShow = !moveBg && (REG_DISPCNT & 0x800) && !(REG_DISPCNT & 0x80);
            sBattleLayers = (1u << 4) | (sliding && !VoxelBattle_IsLink() ? (1u << 2) | (1u << 3) : 0u);
            if (sdl3DBattleTexture == NULL)
            {
                sdl3DBattleTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                                       DISPLAY_WIDTH, DISPLAY_HEIGHT);
                if (sdl3DBattleTexture != NULL)
                    SDL_SetTextureBlendMode(sdl3DBattleTexture, SDL_BLENDMODE_BLEND);
            }
            BuildBattleOverlay(gbaImage, sBattleImage);
            // the world is drawn (and its camera moves) every battle frame, shown only when it is the scenery
            s3DBattleFrame = sdl3DBattleTexture != NULL;
        }
    }
    {
        Uint64 now = SDL_GetPerformanceCounter();
        sVdAfter2D += now - vdMark;
        vdMark = now;
    }
    if (s3DReady && ((s3DWanted && (sFrameInfo.mode == WIDE_VIEW_FIELD || s3DTransition)) || s3DBattleFrame))
    {
        // The 3D world, while the game thread still waits on this frame.
        int w, h;
        GetSceneSize(&w, &h);
        if (s3DBattleFrame)
            w = h * DISPLAY_WIDTH / DISPLAY_HEIGHT; // the battle picture's shape
        if (sdl3DTexture == NULL || s3DW != w || s3DH != h)
        {
            if (sdl3DTexture != NULL)
                SDL_DestroyTexture(sdl3DTexture);
            sdl3DTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, w, h);
            s3DW = w;
            s3DH = h;
        }
        if (sdl3DTexture != NULL)
        {
            unsigned id;
            STALL_GL(SDL_RenderFlush(sdlRenderer));
            STALL_GL(id = Voxel3D_TextureId(sdl3DTexture));
            if (id != 0)
            {
                SDL_Texture *was = SDL_GetRenderTarget(sdlRenderer);
                {
                    // The zoom setting moves the 3D camera: the 2D view's height in GBA pixels
                    // against the 3DS's ~160 at its 100%.
                    int level = sPlatformSettings[PLATFORM_SETTING_ZOOM] % ZOOM_COUNT;
                    if (WideView_IsIndoorMap() && level > 0)
                        level--;
                    // (test aid, Brennan 2026-11-18 "Fix zoom first" for the 3D audit pictures: WJPC_TEST_3D_ZOOM=percent
                    // takes the camera further out than the widest setting, e.g. 30; WJPC_TEST_3D_PITCH=degrees)
                    const char *tz = getenv("WJPC_TEST_3D_ZOOM"), *tp = getenv("WJPC_TEST_3D_PITCH");
                    // Brennan 2026-11-24 18:24 ("make the way you see these areas be more at an angle"; Options > Looks >
                    // Camera angle): Top 40 degrees (as it was, the default), Angled 32, Low 26
                    static const int sCameraPitch[3] = {40, 32, 26};
                    int pitch = sCameraPitch[sPlatformSettings[PLATFORM_SETTING_CAMERA_ANGLE] % 3];
                    Voxel3D_SetSettings(true, tp != NULL ? atoi(tp) : pitch, tz != NULL ? atoi(tz) : 100 * 160 / sZoomHeights[level]);
                }
                Voxel3D_SetGammaLight(sGammaWanted);
                Voxel3D_SetAntiAlias((int)sAntiAlias);
                {
                    int rate = gPlatformFrameRates[sPlatformSettings[PLATFORM_SETTING_SMOOTH] % FRAME_RATE_COUNT];
                    static int sOff = -1, sForce = -1;
                    if (sOff < 0)
                    {
                        sOff = getenv("WJPC_3D_EVERY_REFRESH_OFF") != NULL;
                        sForce = getenv("WJPC_3D_EVERY_REFRESH") != NULL;
                    }
                    s3DEveryRefresh = !sOff && !(gVoxelFxOff & VOXEL_FX_REFRESH) && (rate > 60 || sForce) && !s3DBattleFrame && !s3DTransition
                                   && sFrameInfo.mode == WIDE_VIEW_FIELD;
                    Voxel3D_SetUpdateOnly(s3DEveryRefresh);
                }
                Uint64 r0 = SDL_GetPerformanceCounter();
                s3DFrame = Voxel3D_Render(id, w, h);
                sStall3D += SDL_GetPerformanceCounter() - r0;
                if (!s3DFrame)
                    s3DEveryRefresh = false;
                // SDL caches its GL state: switching targets makes it send viewport and target again.
                STALL_GL(SDL_SetRenderTarget(sdlRenderer, sdl3DTexture));
                STALL_GL(SDL_SetRenderTarget(sdlRenderer, was));
            }
        }
    }
    {
        Uint64 now = SDL_GetPerformanceCounter();
        sVd3DBlock += now - vdMark;
        vdMark = now;
    }
    if (s3DBattleFrame)
    {
        BattleEffects(sBattleImage);
        if (!sBattleShow)
            s3DBattleFrame = false; // a move's background, BG3 off or a blank screen: the 2D battle, whole
        else
            STALL_GL(SDL_UpdateTexture(sdl3DBattleTexture, NULL, sBattleImage, DISPLAY_WIDTH * sizeof(uint32_t)));
    }
    if (sFrameInfo.mode == WIDE_VIEW_FIELD && sdlUiTexture != NULL)
    {
        if (uiOnlyField)
        {
            // only the GBA lines are shown (and drawn): converted and uploaded alone
            int y0 = sFrameInfo.gbaY, n = DISPLAY_HEIGHT;
            if (y0 < 0) y0 = 0;
            if (y0 + n > fullH) n = fullH - y0;
            for (int i = y0 * fullW; i < (y0 + n) * fullW; i++)
                uiImage[i] = (uiLayer[i] & 0x8000) ? ToArgb(uiLayer[i]) : 0;
            SDL_Rect band = {0, y0, fullW, n};
            if (n > 0)
                STALL_GL(SDL_UpdateTexture(sdlUiTexture, &band, &uiImage[y0 * fullW], fullW * sizeof(Uint32)));
        }
        else
        {
        for (int i = 0; i < fullW * fullH; i++)
            uiImage[i] = (uiLayer[i] & 0x8000) ? ToArgb(uiLayer[i]) : 0;
        STALL_GL(SDL_UpdateTexture(sdlUiTexture, &area, uiImage, fullW * sizeof(Uint32)));
        }
    }
    if (sFrameInfo.margin > 0)
    {
        for (int i = 0; i < fullW * fullH; i++)
            overlayImage[i] = (overlay[i] & 0x8000) ? ToArgb(overlay[i]) : 0;
        STALL_GL(SDL_UpdateTexture(sdlOverlayTexture, &area, overlayImage, fullW * sizeof(Uint32)));

        // How far the camera moved since the last frame; big jumps (warps, map changes) aren't slid.
        sPrevCamX = sCurCamX;
        sPrevCamY = sCurCamY;
        sCurCamX = sFrameInfo.camX;
        sCurCamY = sFrameInfo.camY;
        sCamMoveValid = prevMargin > 0 && prevW == sFrameInfo.width && prevH == sFrameInfo.height
                     && abs(sCurCamX - sPrevCamX) <= SMOOTH_M && abs(sCurCamY - sPrevCamY) <= SMOOTH_M;
    }
    else
    {
        sCamMoveValid = false;
    }
    // DLX-3: in-between frames for battles and menus (the GBA picture), like the overworld's smooth
    // motion. Sprites and BG scroll that moved a little since the last frame are drawn part of the way there;
    // DrawScreen shows them before this frame's picture.
    // Brennan 2026-11-22 12:56 ("yes you can add that and pokemon animations and move sprites at 240 fps"):
    //  - a sprite is matched to itself the frame before (sprite.c gPcOamOwner: which sprite each OAM slot holds),
    //    not to whatever sprite had its slot, so it slides even when its picture changes (a Pokémon's or a move's
    //    animation frames), up to 32 px a frame; WJPC_INTERP_SLOTS=1: matched by slot and picture, 16 px, as before.
    //  - a sprite that grows, shrinks or turns (its affine matrix) is eased between its two matrices;
    //    WJPC_INTERP_NO_AFFINE=1: stepped as before.
    //  - the healthboxes (the HP and EXP bars drain inside their pictures) fade from last frame's picture to this
    //    one's where they stand still, so a bar's edge moves a part of a pixel each refresh; WJPC_INTERP_NO_BARS=1:
    //    stepped as before.
    {
        extern bool gPcFrameHadLineEffects;
        extern void DrawFramePlain(uint16_t *pixels);
        extern u16 gPcOamOwner[128];
        extern u8 gPcOamGen[128];
        extern u8 gHealthboxSpriteIds[4];
        extern u8 gPcPixelSource[];
        extern bool8 PcInBattle(void);
        enum { SLOTS = OAM_SIZE / (int)sizeof(struct OamData) };
        static unsigned char prevOam[OAM_SIZE];
        static u16 prevOfs[8], prevOwner[SLOTS];
        static u8 prevGen[SLOTS];
        static uint16_t prevPic[DISPLAY_WIDTH * DISPLAY_HEIGHT];
        static bool havePrev;
        static int sBySlot = -1, sNoAffine, sNoBars, sDbgBoxes, sDbgBar, sDbgChanged, sDbgDiff;
        if (sBySlot < 0)
        {
            sBySlot = getenv("WJPC_INTERP_SLOTS") != NULL;
            sNoAffine = getenv("WJPC_INTERP_NO_AFFINE") != NULL;
            sNoBars = getenv("WJPC_INTERP_NO_BARS") != NULL;
        }
        volatile u16 *ofs = &REG_BG0HOFS;
        int rate = gPlatformFrameRates[sPlatformSettings[PLATFORM_SETTING_SMOOTH] % FRAME_RATE_COUNT];
        int steps = (rate + 30) / 60;
        bool gba = sFrameInfo.mode == WIDE_VIEW_GBA && sFrameInfo.width == DISPLAY_WIDTH
                && sFrameInfo.height == DISPLAY_HEIGHT && sFrameInfo.margin == 0;
        bool want = gba && (sPlatformSettings[PLATFORM_SETTING_SMOOTH] || getenv("WJPC_TEST_INTERP"))
                 && !s3DTransition && !s2DTransition && getenv("WJPC_NO_INTERP") == NULL;
        if (getenv("WJPC_TEST_INTERP"))
            steps = atoi(getenv("WJPC_TEST_INTERP"));
        if (steps > INTERP_MAX + 1)
            steps = INTERP_MAX + 1;
        sInterpCount = 0;
        sInterpShown = -1;
        u16 curOwner[SLOTS];
        u8 curGen[SLOTS];
        memcpy(curOwner, gPcOamOwner, sizeof(curOwner));
        memcpy(curGen, gPcOamGen, sizeof(curGen));
        if (want && havePrev && steps > 1 && !gPcFrameHadLineEffects)
        {
            static unsigned char curOam[OAM_SIZE];
            static uint16_t pic[DISPLAY_WIDTH * DISPLAY_HEIGHT];
            u16 curOfs[8];
            struct OamData *o = (struct OamData *)OAM, *po = (struct OamData *)prevOam;
            const struct OamData *co = (const struct OamData *)curOam;
            int from[SLOTS];        // the slot each slot's sprite had last frame, -1 none
            bool bar[SLOTS];        // a healthbox's piece standing still
            bool anyBar = false;
            memcpy(curOam, OAM, OAM_SIZE);
            for (int i = 0; i < 8; i++)
                curOfs[i] = ofs[i];
            {
                static s16 slotOf[64][16];
                u8 boxes[12];
                int nBoxes = 0;
                for (int i = 0; i < 64; i++)
                    for (int k = 0; k < 16; k++)
                        slotOf[i][k] = -1;
                for (int j = 0; j < SLOTS; j++)
                    if (prevOwner[j] != 0xFFFF && (prevOwner[j] >> 8) < 64 && (prevOwner[j] & 0xFF) < 16)
                        slotOf[prevOwner[j] >> 8][prevOwner[j] & 0xFF] = (s16)j;
                if (PcInBattle() && !sNoBars)
                    for (int bt = 0; bt < 4; bt++)
                    {
                        u8 id = gHealthboxSpriteIds[bt];
                        if (id >= 64 || !gSprites[id].inUse)
                            continue;
                        boxes[nBoxes++] = id;
                        u16 other = gSprites[id].oam.affineParam, barId = (u16)gSprites[id].data[5];
                        if (other < 64 && gSprites[other].inUse && gSprites[other].data[5] == id)
                            boxes[nBoxes++] = (u8)other;
                        if (barId < 64 && gSprites[barId].inUse && gSprites[barId].data[5] == id)
                            boxes[nBoxes++] = (u8)barId;
                    }
                sDbgBoxes = nBoxes;
                for (int i = 0; i < SLOTS; i++)
                {
                    u16 w = curOwner[i];
                    from[i] = -1;
                    bar[i] = false;
                    if (sBySlot)
                        from[i] = i;
                    else if (w != 0xFFFF && (w >> 8) < 64 && (w & 0xFF) < 16)
                        from[i] = slotOf[w >> 8][w & 0xFF];
                    if (!sBySlot && from[i] >= 0 && prevGen[from[i]] != curGen[i])
                        from[i] = -1; // a new sprite in that slot
                    if (w == 0xFFFF || from[i] < 0)
                        continue;
                    for (int n = 0; n < nBoxes; n++)
                        if ((w >> 8) == boxes[n] && po[from[i]].x == co[i].x && po[from[i]].y == co[i].y)
                            bar[i] = anyBar = true, sDbgBar++;
                }
            }
            for (int k = 1; k < steps; k++)
            {
                float f = (float)k / steps;
                bool moved = false;
                for (int i = 0; i < SLOTS; i++)
                {
                    const struct OamData *c = &co[i];
                    int j = from[i];
                    if (j < 0)
                        continue;
                    if (po[j].shape != c->shape || po[j].size != c->size || po[j].affineMode != c->affineMode)
                        continue;
                    if (sBySlot && po[j].tileNum != c->tileNum)
                        continue;
                    int lim = sBySlot ? 16 : 32;
                    int dx = (int)c->x - (int)po[j].x, dy = (int)c->y - (int)po[j].y;
                    dx = ((dx + 256) & 511) - 256;
                    dy = ((dy + 128) & 255) - 128;
                    if ((dx == 0 && dy == 0) || abs(dx) > lim || abs(dy) > lim)
                        continue;
                    if (k == 1 && getenv("WJPC_INTERP_LOG") != NULL)
                        fprintf(stderr, "SLIDE slot %d owner %04x from %d (%d,%d)->(%d,%d) tile %u->%u aff %d\n", i,
                                curOwner[i], j, po[j].x, po[j].y, c->x, c->y, po[j].tileNum, c->tileNum, c->affineMode);
                    // drawn at the earlier place plus part of the move (rounded toward where it came from)
                    o[i].x = (unsigned)((int)po[j].x + (int)(dx * f)) & 511;
                    o[i].y = (unsigned)((int)po[j].y + (int)(dy * f)) & 255;
                    moved = true;
                }
                if (!sNoAffine && !sBySlot)
                {
                    // each matrix in use this frame eased from the one its sprite used last frame
                    bool done[32] = {false};
                    for (int i = 0; i < SLOTS; i++)
                    {
                        const struct OamData *c = &co[i];
                        int j = from[i];
                        if (j < 0 || !(c->affineMode & 1) || !(po[j].affineMode & 1))
                            continue;
                        unsigned m = c->matrixNum, pm = po[j].matrixNum;
                        if (done[m])
                            continue;
                        done[m] = true;
                        s16 a[4], b[4];
                        bool ok = true, diff = false;
                        for (int e = 0; e < 4; e++)
                        {
                            a[e] = (s16)po[pm * 4 + e].affineParam;
                            b[e] = (s16)co[m * 4 + e].affineParam;
                            if (abs(b[e] - a[e]) > 0x100)
                                ok = false; // a jump, not a turn or a growth: shown as it is
                            if (a[e] != b[e])
                                diff = true;
                        }
                        if (!ok || !diff)
                            continue;
                        if (k == 1 && getenv("WJPC_INTERP_LOG") != NULL)
                            fprintf(stderr, "AFFINE slot %d owner %04x m %u<-%u (%d %d %d %d)->(%d %d %d %d)\n", i, curOwner[i],
                                    m, pm, a[0], a[1], a[2], a[3], b[0], b[1], b[2], b[3]);
                        for (int e = 0; e < 4; e++)
                            o[m * 4 + e].affineParam = (u16)(s16)lroundf(a[e] + (b[e] - a[e]) * f);
                        moved = true;
                    }
                }
                for (int i = 0; i < 8; i++)
                {
                    int d = (((int)curOfs[i] - (int)prevOfs[i] + 256) & 511) - 256;
                    if (d != 0 && abs(d) <= 16)
                    {
                        ofs[i] = (u16)(((int)prevOfs[i] + (int)(d * f)) & 511);
                        moved = true;
                    }
                }
                if (!moved && !anyBar)
                    break;
                DrawFramePlain(pic);
                if (anyBar)
                {
                    // the healthboxes: last frame's picture faded into this one's, under their standing pieces
                    bool changed = false;
                    for (int i = 0; i < SLOTS; i++)
                    {
                        if (!bar[i])
                            continue;
                        static const u8 kW[3][4] = {{8, 16, 32, 64}, {16, 32, 32, 64}, {8, 8, 16, 32}};
                        static const u8 kH[3][4] = {{8, 16, 32, 64}, {8, 8, 16, 32}, {16, 32, 32, 64}};
                        const struct OamData *c = &co[i];
                        if (c->shape > 2)
                            continue;
                        int w = kW[c->shape][c->size], h = kH[c->shape][c->size];
                        if (c->affineMode == 3)
                            w *= 2, h *= 2;
                        int x0 = c->x >= 256 ? (int)c->x - 512 : (int)c->x, y0 = c->y >= 160 ? (int)c->y - 256 : (int)c->y;
                        int xs = x0 < 0 ? 0 : x0, xe = x0 + w < DISPLAY_WIDTH ? x0 + w : DISPLAY_WIDTH;
                        for (int y = y0 < 0 ? 0 : y0; y < y0 + h && y < DISPLAY_HEIGHT; y++)
                        {
                            const uint16_t *A = &prevPic[y * DISPLAY_WIDTH], *B = &gbaImage[y * DISPLAY_WIDTH];
                            uint16_t *P = &pic[y * DISPLAY_WIDTH];
                            for (int x = xs; x < xe; x++)
                            {
                                if (A[x] == B[x])
                                    continue;
                                // a run of changed pixels: a bar's end moving along it, or a picture changing
                                int xa = x, xb = x;
                                while (xb + 1 < xe && A[xb + 1] != B[xb + 1])
                                    xb++;
                                x = xb;
                                int len = xb - xa + 1, mode = 0; // 1: emptied from its right, 2: filled from its left
                                if (len > 2 && xb + 1 < DISPLAY_WIDTH)
                                {
                                    mode = 1;
                                    for (int q = xa; q <= xb && mode; q++)
                                        if (B[q] != B[xb + 1] || A[q] != A[xa])
                                            mode = 0;
                                }
                                if (!mode && len > 2 && xa > 0)
                                {
                                    mode = 2;
                                    for (int q = xa; q <= xb && mode; q++)
                                        if (B[q] != B[xa - 1] || A[q] != A[xb])
                                            mode = 0;
                                }
                                sDbgChanged += len;
                                changed = true;
                                if (mode == 1)
                                {
                                    // the end moves left: the part already passed shows this frame's picture
                                    int split = xb + 1 - (int)lroundf(len * f);
                                    for (int q = xa; q <= xb; q++)
                                        P[q] = q >= split ? B[q] : A[q];
                                }
                                else if (mode == 2)
                                {
                                    int split = xa + (int)lroundf(len * f);
                                    for (int q = xa; q <= xb; q++)
                                        P[q] = q < split ? B[q] : A[q];
                                }
                                else
                                    for (int q = xa; q <= xb; q++)
                                    {
                                        // anything else (a short step, a colour change, the numbers): faded
                                        uint16_t a0 = A[q], b0 = B[q];
                                        int r = (a0 & 31) + (int)lroundf(((b0 & 31) - (a0 & 31)) * f);
                                        int g = ((a0 >> 5) & 31) + (int)lroundf((((b0 >> 5) & 31) - ((a0 >> 5) & 31)) * f);
                                        int bl = ((a0 >> 10) & 31) + (int)lroundf((((b0 >> 10) & 31) - ((a0 >> 10) & 31)) * f);
                                        P[q] = (uint16_t)(r | (g << 5) | (bl << 10));
                                    }
                            }
                        }
                    }
                    if (!moved && !changed)
                        break;
                }
                if (s3DBattleFrame)
                {
                    BuildBattleOverlay(pic, sInterpImage[k - 1]);
                    BattleEffects(sInterpImage[k - 1]);
                }
                else
                    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
                        sInterpImage[k - 1][i] = ToArgb(pic[i]);
                sInterpCount = k;
                memcpy(OAM, curOam, OAM_SIZE);
                for (int i = 0; i < 8; i++)
                    ofs[i] = curOfs[i];
            }
            memcpy(OAM, curOam, OAM_SIZE);
            for (int i = 0; i < 8; i++)
                ofs[i] = curOfs[i];
            if (sInterpCount > 0 && getenv("WJPC_INTERP_DUMP") != NULL && PcInBattle())
            {
                // test aid: WJPC_INTERP_DUMP=DIR writes each frame's in-between pictures and its own (PPM, 240x160)
                static int sDumped;
                if (sDumped < 1200)
                    for (int k = 0; k <= sInterpCount; k++)
                    {
                        char path[600];
                        const uint16_t *src = NULL;
                        static uint8_t rgb[DISPLAY_WIDTH * DISPLAY_HEIGHT * 3];
                        const uint32_t *img = k < sInterpCount ? sInterpImage[k] : NULL;
                        snprintf(path, sizeof(path), "%s/%05d_%d.ppm", getenv("WJPC_INTERP_DUMP"), sDumped / 4, k);
                        for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
                        {
                            uint32_t c = img != NULL ? img[i] : ToArgb(gbaImage[i]);
                            rgb[i * 3] = (c >> 16) & 255, rgb[i * 3 + 1] = (c >> 8) & 255, rgb[i * 3 + 2] = c & 255;
                        }
                        (void)src;
                        FILE *f = fopen(path, "wb");
                        if (f != NULL)
                        {
                            fprintf(f, "P6\n%d %d\n255\n", DISPLAY_WIDTH, DISPLAY_HEIGHT);
                            fwrite(rgb, 1, sizeof(rgb), f);
                            fclose(f);
                        }
                        sDumped++;
                    }
            }
            if (sInterpCount > 0)
            {
                sInterpSteps = steps;
                sInterpBattle = s3DBattleFrame;
                memcpy(sInterpCurrent, s3DBattleFrame ? sBattleImage : sLastUploaded, sizeof(sInterpCurrent));
            }
        }
        if (getenv("WJPC_INTERP_LOG") != NULL && PcInBattle())
            fprintf(stderr, "INTERP diff %d boxes %d bar %d changed %d mode %d w %d h %d m %d want %d prev %d line %d count %d\n", sDbgDiff, sDbgBoxes, sDbgBar, sDbgChanged, (int)sFrameInfo.mode,
                    sFrameInfo.width, sFrameInfo.height, sFrameInfo.margin, want, havePrev, gPcFrameHadLineEffects,
                    sInterpCount);
        havePrev = want;
        if (want)
        {
            memcpy(prevOam, OAM, OAM_SIZE);
            memcpy(prevOwner, curOwner, sizeof(prevOwner));
            memcpy(prevGen, curGen, sizeof(prevGen));
            memcpy(prevPic, gbaImage, sizeof(prevPic));
            for (int i = 0; i < 8; i++)
                prevOfs[i] = ofs[i];
        }
    }
    REG_VCOUNT = 161; // prep for being in VBlank period
    sVdEnd += SDL_GetPerformanceCounter() - vdMark;
}

int DoMain(void *data)
{
    AgbMain();
    return 0;
}

void VBlankIntrWait(void)
{
    SDL_AtomicSet(&isFrameAvailable, 1);
    SDL_SemWait(vBlankSemaphore);
}

u8 BinToBcd(u8 bin)
{
    int placeCounter = 1;
    u8 out = 0;
    do
    {
        out |= (bin % 10) * placeCounter;
        placeCounter *= 16;
    }
    while ((bin /= 10) > 0);

    return out;
}

void Platform_GetStatus(struct SiiRtcInfo *rtc)
{
    rtc->status = internalClock.status;
}

void Platform_SetStatus(struct SiiRtcInfo *rtc)
{
    internalClock.status = rtc->status;
}

static bool sClockRead; // UpdateInternalClock's reading is current (a Set... clears it: the next read is the device's)
static void UpdateInternalClock(void);
static void ResetInternalClock(void)
{
    memset(&internalClock, 0, sizeof(internalClock));
    internalClock.status = SIIRTCINFO_24HOUR;
    sClockRead = false;
    UpdateInternalClock();
}
static void UpdateInternalClock(void)
{
    /* Brennan 2026-11-21 11:01 (21g: every second LAGGY, frame 28 ms): the 3D stand-ins ask each neighbouring
     * person whether they have gone home (WallyJourney_IsNpcAway), and each asks the clock. Reading the clock
     * cost about half a millisecond on his PC (6 ms under wine). The clock is read at most every 100 ms now; the
     * game shows seconds at most. */
    static Uint32 sReadAt;
    Uint32 now = SDL_GetTicks();

    static int sNoCache = -1;

    if (sNoCache < 0)
        sNoCache = getenv("WJPC_CLOCK_CACHE_OFF") != NULL; // (as before: read every time)
    if (sClockRead && now - sReadAt < 100 && !sNoCache)
        return;
    sClockRead = true;
    sReadAt = now;
    if (getenv("WJPC_RTC_EPOCH"))
    {
        // Test (GBA vs PC comparison): a fixed clock, like the mGBA harness (local time).
        time_t fixedTime = (time_t)atoll(getenv("WJPC_RTC_EPOCH"));
        struct tm *tm = localtime(&fixedTime);
        internalClock.year = BinToBcd(tm->tm_year - 100);
        internalClock.month = BinToBcd(tm->tm_mon + 1);
        internalClock.day = BinToBcd(tm->tm_mday);
        internalClock.dayOfWeek = BinToBcd(tm->tm_wday);
        internalClock.hour = BinToBcd(tm->tm_hour);
        internalClock.minute = BinToBcd(tm->tm_min);
        internalClock.second = BinToBcd(tm->tm_sec);
        return;
    }
#ifdef _WIN32
    // Ask Windows for the local time: msvcrt's localtime() trusts a TZ variable when one is set,
    // which can be wrong (e.g. "America/Chicago" is read as UTC).
    SYSTEMTIME st;
    GetLocalTime(&st);
    internalClock.year = BinToBcd(st.wYear % 100);
    internalClock.month = BinToBcd(st.wMonth);
    internalClock.day = BinToBcd(st.wDay);
    internalClock.dayOfWeek = BinToBcd(st.wDayOfWeek);
    internalClock.hour = BinToBcd(st.wHour);
    internalClock.minute = BinToBcd(st.wMinute);
    internalClock.second = BinToBcd(st.wSecond);
    return;
#endif
    time_t rawTime = time(NULL);
    struct tm *time = localtime(&rawTime);

    internalClock.year = BinToBcd(time->tm_year - 100);
    internalClock.month = BinToBcd(time->tm_mon + 1);
    internalClock.day = BinToBcd(time->tm_mday);
    internalClock.dayOfWeek = BinToBcd(time->tm_wday);
    internalClock.hour = BinToBcd(time->tm_hour);
    internalClock.minute = BinToBcd(time->tm_min);
    internalClock.second = BinToBcd(time->tm_sec);
}

void Platform_GetDateTime(struct SiiRtcInfo *rtc)
{
    UpdateInternalClock();

    rtc->year = internalClock.year;
    rtc->month = internalClock.month;
    rtc->day = internalClock.day;
    rtc->dayOfWeek = internalClock.dayOfWeek;
    rtc->hour = internalClock.hour;
    rtc->minute = internalClock.minute;
    rtc->second = internalClock.second;
    /* (no DBGPRINTF here: the clock is read many times a frame - Brennan 2026-11-21 11:01) */
}

void Platform_SetDateTime(struct SiiRtcInfo *rtc)
{
    sClockRead = false;
    internalClock.month = rtc->month;
    internalClock.day = rtc->day;
    internalClock.dayOfWeek = rtc->dayOfWeek;
    internalClock.hour = rtc->hour;
    internalClock.minute = rtc->minute;
    internalClock.second = rtc->second;
}

void Platform_GetTime(struct SiiRtcInfo *rtc)
{
    UpdateInternalClock();

    rtc->hour = internalClock.hour;
    rtc->minute = internalClock.minute;
    rtc->second = internalClock.second;
    /* (no DBGPRINTF here: the clock is read many times a frame - Brennan 2026-11-21 11:01) */
}

void Platform_SetTime(struct SiiRtcInfo *rtc)
{
    sClockRead = false;
    internalClock.hour = rtc->hour;
    internalClock.minute = rtc->minute;
    internalClock.second = rtc->second;
}

void Platform_SetAlarm(u8 *alarmData)
{
    // TODO
}

void SoftReset(u32 resetFlags)
{
    puts("Soft Reset called. Exiting.");
    exit(0);
}

#endif
