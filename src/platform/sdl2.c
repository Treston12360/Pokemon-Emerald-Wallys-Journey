#include <stdlib.h>
#ifdef PLATFORM_SDL2
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
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
#include "global.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "constants/characters.h"
#include "constants/event_objects.h"
#include "sprite.h"
#include "platform/null_guard.h"
#include "platform/wide_view.h"
#include "platform/pc_tween.h"
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
// DLX-6: 3D can be turned on and off in Settings. The renderer is OpenGL whenever the 3D data is there
// (s3DAvailable), and the 3D world loads the first time it is wanted (s3DInitTried), so no restart.
static bool s3DAvailable, s3DInitTried;
// DLX-5: 3D battle scenery (Settings "3D Battles"); s3DBattleFrame: this frame is a 3D battle.
static bool s3DBattleWanted = true, s3DBattleFrame;
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
static bool s3DTransition;
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
static unsigned int sTestFfSpeed; // test script "ff N": fast forward without a clock (0/1 = off)
static u16 sTestKeys;
static int sTestHold;
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
static char sTestTweenShot[512]; // DLX-3 test aid
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
                fclose(f);
            }
            continue;
        }
        if (!strcmp(cmd, "ff"))
        {
            // ff N (test aid): fast forward N times, as the real loop does it: only every Nth
            // frame is drawn and the sound engine runs once every N frames. "ff 0" = off.
            sTestFfSpeed = atoi(arg1);
            continue;
        }
        if (!strcmp(cmd, "tweenshot"))
        {
            SDL_snprintf(sTestTweenShot, sizeof(sTestTweenShot), "%s", arg1);
            continue;
        }
        if (!strcmp(cmd, "winshot"))
        {
            SDL_snprintf(sTestWinShot, sizeof(sTestWinShot), "%s", arg1);
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
// Each zoom level is how many world pixels tall the view should be. The map is always a whole
// number of screen pixels per game pixel (sharp) and always fills the screen edge to edge: the
// view is made as big as the screen holds at that size. Keys - and =, or the mouse wheel.
static const u16 sZoomHeights[] = {120, 144, 162, 180, 216, 240, 270, 324};
#define ZOOM_COUNT ARRAY_COUNT(sZoomHeights)
static int sFieldZ = 1, sFieldViewW = DISPLAY_WIDTH, sFieldViewH = DISPLAY_HEIGHT;

static void ComputeFieldView(int outW, int outH)
{
    int level = sPlatformSettings[PLATFORM_SETTING_ZOOM] % ZOOM_COUNT;
    int target, k;
    if (WideView_IsIndoorMap() && level > 0)
        level--; // rooms are small: one step closer
    target = sZoomHeights[level];
    k = (outH + target / 2) / target;
    if (k < 1)
        k = 1;
    while ((outW + k - 1) / k > WIDE_VIEW_MAX_W || (outH + k - 1) / k > WIDE_VIEW_MAX_H)
        k++;
    sFieldZ = k;
    sFieldViewW = (outW + k - 1) / k;
    sFieldViewH = (outH + k - 1) / k;
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

// Overworld: the map at the zoom scale, edge to edge; the UI layer (text boxes, menus) at the
// biggest whole size the GBA screen fits, bottom-centred like on the GBA; the map name pop-up
// always in the top-left corner of the screen.
// DLX-3: in-between frames for battles and menus. With a frame rate above 60, VDraw draws each game frame
// two to four times (pc_tween.c: sprites, scrolling, windows and colours part of the way from the last
// frame); the screen shows them in turn until the next game frame.
#define TWEEN_EXTRA_MAX 3
static uint32_t *sTweenImage[TWEEN_EXTRA_MAX], *sTweenBattle[TWEEN_EXTRA_MAX];
static const uint32_t *sTrueImage, *sTrueBattle;
static int sTweenSteps = 1, sTweenShown = 1, sTweenW, sTweenH;
static bool sTweenBattleOn;

static int TweenStepsWanted(void)
{
    const char *test = getenv("WJPC_TWEEN_TEST"); // test aid: steps under a test script
    int rate;

    if (test != NULL)
    {
        int n = atoi(test);
        return n < 1 ? 1 : n > TWEEN_EXTRA_MAX + 1 ? TWEEN_EXTRA_MAX + 1 : n;
    }
    if (sTestScript != NULL || speedUp)
        return 1;
    rate = gPlatformFrameRates[sPlatformSettings[PLATFORM_SETTING_SMOOTH] % FRAME_RATE_COUNT];
    return rate <= 60 ? 1 : rate <= 120 ? 2 : rate <= 180 ? 3 : 4;
}

// The picture for this moment (t: 0-1 of the way to the next game frame) into the textures.
static void TweenShow(double t)
{
    int j;

    if (sTweenSteps <= 1)
        return;
    j = (int)(t * sTweenSteps) + 1;
    if (j < 1)
        j = 1;
    if (j > sTweenSteps)
        j = sTweenSteps;
    if (j == sTweenShown)
        return;
    SDL_Rect area = {0, 0, sTweenW, sTweenH};
    SDL_UpdateTexture(sdlTexture, &area, j == sTweenSteps ? sTrueImage : sTweenImage[j - 1], sTweenW * sizeof(Uint32));
    if (sTweenBattleOn && sdl3DBattleTexture != NULL)
        SDL_UpdateTexture(sdl3DBattleTexture, NULL, j == sTweenSteps ? sTrueBattle : sTweenBattle[j - 1],
                          DISPLAY_WIDTH * sizeof(uint32_t));
    sTweenShown = j;
}

static void DrawFieldScene(double t, int outW, int outH)
{
    int z = sFieldZ, m = sFrameInfo.margin;
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
    if (s3DFrame)
    {
        // The 3D world fills the screen; the UI layer goes over it below.
        SDL_Rect whole = {0, 0, outW, outH};
        SDL_RenderCopy(sdlRenderer, sdl3DTexture, NULL, &whole);
    }
    else
    {
    SDL_SetTextureScaleMode(sdlTexture, SDL_ScaleModeNearest);
    SDL_RenderCopyF(sdlRenderer, sdlTexture, &full, &mapDest);
    if (m > 0)
    {
        SDL_SetTextureScaleMode(sdlOverlayTexture, SDL_ScaleModeNearest);
        SDL_RenderCopyF(sdlRenderer, sdlOverlayTexture, &full, &stillDest);
    }
    }
    if (sdlUiTexture != NULL)
    {
        int s = outW / DISPLAY_WIDTH;
        if (outH / DISPLAY_HEIGHT < s)
            s = outH / DISPLAY_HEIGHT;
        if (s < 1)
            s = 1;
        // Start menu open: the UI slides right so the menu is flush with the screen's right edge.
        int ux = Vol_StartMenu_IsOpen() ? outW - DISPLAY_WIDTH * s : (outW - DISPLAY_WIDTH * s) / 2, uy = outH - DISPLAY_HEIGHT * s;
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
    }
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
        SDL_RenderCopy(sdlRenderer, sdl3DTexture, NULL, &whole);
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
    TweenShow(t);
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
            if (scale < 1)
                scale = 1;
            gameWidth = srcW * scale;
            gameHeight = srcH * scale;
        }
    }
    else
    {
        gameWidth = outputWidth;
        gameHeight = outputHeight;
        if (integer)
        {
            gameWidth = srcW * (outputWidth / srcW > 0 ? outputWidth / srcW : 1);
            gameHeight = srcH * (outputHeight / srcH > 0 ? outputHeight / srcH : 1);
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

int main(int argc, char **argv)
{
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
    ReadSaveFile(sSavePath);
    ReadConfigFile();

#ifdef __ANDROID__
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif
#if defined(NATIVE_LINUX) || defined(_WIN32)
    sdlWindow = SDL_CreateWindow("Pokemon Wally's Journey", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
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
    mainLoopThread = SDL_CreateThread(DoMain, "AgbMain", NULL);

    double accumulator = 0.0;

    memset(&internalClock, 0, sizeof(internalClock));
    internalClock.status = SIIRTCINFO_24HOUR;
    UpdateInternalClock();

    while (isRunning)
    {
        ApplyChangedWindowSettings();
        if (sTestScript != NULL)
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
            if (sTestWinShot[0] != '\0' || getenv("WJPC_TEST_WINDOW"))
            {
                // Test aid: draw exactly like a normal frame (zoom, UI layer, screen options),
                // and with "winshot FILE" save the whole window.
                VDraw(sdlTexture);
                if (sTestWinShot[0] != '\0')
                {
                    int ww, wh;
                    DrawScreen(1.0);
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
                    bool lastOfBatch = accumulator - dt < dt;
                    bool outOfTime = speedUp && (SDL_GetPerformanceCounter() - batchStart) * 1000 / SDL_GetPerformanceFrequency() >= 14;
                    if (!speedUp || lastOfBatch || outOfTime)
                    {
                        VDraw(sdlTexture);
                        sDrewFrame = true;
                    }
                    else
                    {
                        WideView_SkipFrame();
                        REG_VCOUNT = 161;
                    }
                    sGameFramesCounted++;
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

#ifndef __ANDROID__
        // Draw every loop (every monitor refresh with vsync). With smooth motion the map is slid
        // part of the way between the last two game frames.
        if (sDrewFrame)
        {
            double t = paused ? 1.0 : accumulator / (fixedTimestep / timeScale);
            DrawScreen(t < 0 ? 0 : t > 1 ? 1 : t);
        }
        SDL_RenderPresent(sdlRenderer);
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
        else if (sscanf(line, "integerScale=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_INTEGER_SCALE] = value != 0;
        else if (sscanf(line, "vsync=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_VSYNC] = value != 0;
        else if (sscanf(line, "border=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_BORDER] = value != 0;
        else if (sscanf(line, "volume=%u", &value) == 1 && value <= 10)
            sPlatformSettings[PLATFORM_SETTING_VOLUME] = value;
        else if (sscanf(line, "fastForward=%u", &value) == 1 && value < FF_SPEED_COUNT)
            sPlatformSettings[PLATFORM_SETTING_FF_SPEED] = value;
        else if (sscanf(line, "world3d=%u", &value) == 1)
            s3DWanted = value != 0;
        else if (sscanf(line, "battle3d=%u", &value) == 1)
            s3DBattleWanted = value != 0;
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
        else if (sscanf(line, "bilinear=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_SMOOTHING] = value != 0;
        else if (sscanf(line, "resolution=%u", &value) == 1 && value < RESOLUTION_COUNT)
            sPlatformSettings[PLATFORM_SETTING_RESOLUTION] = value;
        else if (sscanf(line, "lockAspect=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_LOCK_ASPECT] = value != 0;
        else if (sscanf(line, "interframe=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_INTERFRAME] = value != 0;
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
    fprintf(configFile, "fastForward=%u\n", sPlatformSettings[PLATFORM_SETTING_FF_SPEED]);
    fprintf(configFile, "widescreen=%u\n", WideView_IsEnabled() ? 1 : 0);
    fprintf(configFile, "world3d=%u\n", s3DWanted ? 1 : 0);
    fprintf(configFile, "battle3d=%u\n", s3DBattleWanted ? 1 : 0);
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
    return sPlatformSettings[setting];
}

// Called from the game thread (Options menu). Window changes must happen on
// the main thread, so they are only flagged here and applied between frames.
static SDL_atomic_t sWindowSettingsChanged;

void Platform_SetSetting(enum PlatformSetting setting, u8 value)
{
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

    if (sTestScript != NULL)
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

// DLX-5: the battle's picture for over the 3D world: its scenery (BG3) and backdrop left out (only their fade kept).
static void BattleOverlay(const uint16_t *gba, uint32_t *out)
{
    extern u8 gPcPixelSource[];
    extern s8 gPcPixelFade[];

    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
    {
        u8 src = gPcPixelSource[i];
        if (src == 0 || src == 4)
        {
            int f = gPcPixelFade[i];
            out[i] = f > 0 ? ((uint32_t)(f * 255 / 16) << 24) | 0xFFFFFF
                   : f < 0 ? ((uint32_t)(-f * 255 / 16) << 24) : 0;
        }
        else
            out[i] = ToArgb(gba[i]);
    }
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
    bool smooth = sPlatformSettings[PLATFORM_SETTING_SMOOTH] && sTestScript == NULL && sdlOverlayTexture != NULL;
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
    WideView_SetSmooth(smooth ? overlay : NULL, SMOOTH_M);
    WideView_SetUiOverlay(sdlUiTexture != NULL ? uiLayer : NULL);
    PcTween_BeginFrame();
    sVDrawImage = gbaImage;
    if (!PcGuard_Call0(DrawFrameGuarded))
    {
        // The wide view hit a bad memory access: fall back to the normal picture for good.
        WideView_SetEnabled(FALSE);
        sVDrawImage = gbaImage;
        PcGuard_Call0(DrawFrameGuarded);
    }
    WideView_SetSmooth(NULL, 0);
    WideView_SetUiOverlay(NULL);

    int fullW = sFrameInfo.width + 2 * sFrameInfo.margin, fullH = sFrameInfo.height + 2 * sFrameInfo.margin;
    for (int i = 0; i < fullW * fullH; i++)
        image[i] = ToArgb(gbaImage[i]);
    if (sPlatformSettings[PLATFORM_SETTING_INTERFRAME] && lastW == fullW && lastH == fullH)
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
    lastW = fullW;
    lastH = fullH;
    SDL_Rect area = {0, 0, fullW, fullH};
    SDL_UpdateTexture(texture, &area, image, fullW * sizeof(Uint32));
    s3DFrame = false;
    s3DTransition = s3DReady && s3DWanted && sFrameInfo.mode != WIDE_VIEW_FIELD && WideView_BattleTransitionOnField();
    if (s3DTransition)
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
            SDL_UpdateTexture(sdl3DTransTexture, NULL, sTransImage, DISPLAY_WIDTH * sizeof(uint32_t));
    }
    if ((s3DWanted || s3DBattleWanted) && !s3DReady)
        Try3DInit(); // turned on in Settings
    // DLX-5: a battle over the 3D world. The battle's own picture goes on top, with its scenery (BG3) and
    // backdrop left out, so the world shows there.
    s3DBattleFrame = false;
    {
        bool battle = s3DReady && s3DBattleWanted && !s3DTransition && VoxelBattle_GameInBattle()
                   && sFrameInfo.mode != WIDE_VIEW_FIELD
                   && sFrameInfo.width == DISPLAY_WIDTH && sFrameInfo.height == DISPLAY_HEIGHT;
        Voxel3D_SetBattle(battle);
        if (battle)
        {
            static uint32_t sBattleImage[DISPLAY_WIDTH * DISPLAY_HEIGHT];
            if (sdl3DBattleTexture == NULL)
            {
                sdl3DBattleTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                                       DISPLAY_WIDTH, DISPLAY_HEIGHT);
                if (sdl3DBattleTexture != NULL)
                    SDL_SetTextureBlendMode(sdl3DBattleTexture, SDL_BLENDMODE_BLEND);
            }
            BattleOverlay(gbaImage, sBattleImage);
            sTrueBattle = sBattleImage;
            if (sdl3DBattleTexture != NULL)
                SDL_UpdateTexture(sdl3DBattleTexture, NULL, sBattleImage, DISPLAY_WIDTH * sizeof(uint32_t));
            s3DBattleFrame = sdl3DBattleTexture != NULL;
        }
    }
    // DLX-3: the in-between pictures of this frame (battles, menus; not the field, which slides its map).
    sTweenSteps = sTweenShown = 1;
    {
        bool moved = PcTween_HasMotion();
        int steps = TweenStepsWanted();

        if (moved && steps > 1 && sFrameInfo.mode != WIDE_VIEW_FIELD && !s3DTransition
         && !sPlatformSettings[PLATFORM_SETTING_INTERFRAME])
        {
            static uint16_t tweenGba[TEX_W * TEX_H];
            struct WideViewFrame real = sFrameInfo;
            bool ok = true;

            for (int j = 1; j < steps && ok; j++)
            {
                if (sTweenImage[j - 1] == NULL)
                    sTweenImage[j - 1] = malloc(TEX_W * TEX_H * sizeof(uint32_t));
                if (sTweenBattle[j - 1] == NULL)
                    sTweenBattle[j - 1] = malloc(DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint32_t));
                if (sTweenImage[j - 1] == NULL || sTweenBattle[j - 1] == NULL)
                {
                    ok = false;
                    break;
                }
                PcTween_Apply((float)j / (float)steps);
                sVDrawImage = tweenGba;
                ok = PcGuard_Call0(DrawFrameGuarded) && sFrameInfo.mode == real.mode
                  && sFrameInfo.width == real.width && sFrameInfo.height == real.height;
                if (ok)
                {
                    for (int i = 0; i < fullW * fullH; i++)
                        sTweenImage[j - 1][i] = ToArgb(tweenGba[i]);
                    if (s3DBattleFrame)
                        BattleOverlay(tweenGba, sTweenBattle[j - 1]);
                }
            }
            PcTween_End();
            sFrameInfo = real;
            if (ok)
            {
                sTweenSteps = sTweenShown = steps;
                sTweenW = fullW;
                sTweenH = fullH;
                sTrueImage = image;
                sTweenBattleOn = s3DBattleFrame;
            }
        }
    }
    if (sTestTweenShot[0] != '\0')
    {
        // Test aid "tweenshot PREFIX": this frame's pictures in order, PREFIX_1.ppm ... (the last is the real one).
        for (int j = 1; j <= sTweenSteps; j++)
        {
            char name[600];
            const uint32_t *img = j == sTweenSteps ? image : sTweenImage[j - 1];
            FILE *f;
            SDL_snprintf(name, sizeof(name), "%s_%d.ppm", sTestTweenShot, j);
            f = fopen(name, "wb");
            if (f == NULL)
                continue;
            fprintf(f, "P6\n%d %d\n255\n", fullW, fullH);
            for (int i = 0; i < fullW * fullH; i++)
            {
                u8 rgb[3] = {(u8)(img[i] >> 16), (u8)(img[i] >> 8), (u8)img[i]};
                fwrite(rgb, 1, 3, f);
            }
            fclose(f);
        }
        printf("tweenshot %s: %d pictures at frame %u\n", sTestTweenShot, sTweenSteps, sTestFrame);
        fflush(stdout);
        sTestTweenShot[0] = '\0';
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
            SDL_RenderFlush(sdlRenderer);
            id = Voxel3D_TextureId(sdl3DTexture);
            if (id != 0)
            {
                SDL_Texture *was = SDL_GetRenderTarget(sdlRenderer);
                {
                    // The zoom setting moves the 3D camera: the 2D view's height in GBA pixels
                    // against the 3DS's ~160 at its 100%.
                    int level = sPlatformSettings[PLATFORM_SETTING_ZOOM] % ZOOM_COUNT;
                    if (WideView_IsIndoorMap() && level > 0)
                        level--;
                    Voxel3D_SetSettings(true, 0, 100 * 160 / sZoomHeights[level]);
                }
                s3DFrame = Voxel3D_Render(id, w, h);
                // SDL caches its GL state: switching targets makes it send viewport and target again.
                SDL_SetRenderTarget(sdlRenderer, sdl3DTexture);
                SDL_SetRenderTarget(sdlRenderer, was);
            }
        }
    }
    if (sFrameInfo.mode == WIDE_VIEW_FIELD && sdlUiTexture != NULL)
    {
        for (int i = 0; i < fullW * fullH; i++)
            uiImage[i] = (uiLayer[i] & 0x8000) ? ToArgb(uiLayer[i]) : 0;
        SDL_UpdateTexture(sdlUiTexture, &area, uiImage, fullW * sizeof(Uint32));
    }
    if (sFrameInfo.margin > 0)
    {
        for (int i = 0; i < fullW * fullH; i++)
            overlayImage[i] = (overlay[i] & 0x8000) ? ToArgb(overlay[i]) : 0;
        SDL_UpdateTexture(sdlOverlayTexture, &area, overlayImage, fullW * sizeof(Uint32));

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
    REG_VCOUNT = 161; // prep for being in VBlank period
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

static void UpdateInternalClock(void)
{
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
    DBGPRINTF("GetDateTime: %d-%02d-%02d %02d:%02d:%02d\n", ConvertBcdToBinary(rtc->year),
                                                         ConvertBcdToBinary(rtc->month),
                                                         ConvertBcdToBinary(rtc->day),
                                                         ConvertBcdToBinary(rtc->hour),
                                                         ConvertBcdToBinary(rtc->minute),
                                                         ConvertBcdToBinary(rtc->second));
}

void Platform_SetDateTime(struct SiiRtcInfo *rtc)
{
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
    DBGPRINTF("GetTime: %02d:%02d:%02d\n", ConvertBcdToBinary(rtc->hour),
                                        ConvertBcdToBinary(rtc->minute),
                                        ConvertBcdToBinary(rtc->second));
}

void Platform_SetTime(struct SiiRtcInfo *rtc)
{
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
