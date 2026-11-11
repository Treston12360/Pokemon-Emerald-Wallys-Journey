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

#include "global.h"
#include "platform/null_guard.h"
#include "platform/wide_view.h"
#include "platform.h"
#include "rtc.h"
#include "gba/defines.h"
#include "gba/m4a_internal.h"
#include "cgb_audio.h"
#include "gba/flash_internal.h"
#include "platform/dma.h"
#include "platform/framedraw.h"

extern void (*const gIntrTable[])(void);

SDL_Thread *mainLoopThread;
SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_Texture *sdlTexture;
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
};

// Fast forward: the keyboard key (Space) and the controller button are both read here.
static SDL_atomic_t sFfKeyHeld, sFfPadHeld, sFfToggled;
static void ApplyFastForward(void);
static const char *const sPadSettingNames[] = {"padA", "padB", "padStart", "padSelect", "padL", "padR", "padFastForward"};
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
static u16 sTestKeys;
static int sTestHold;
static u32 sTestFrame;
static u16 *sTestLastGbaImage;
static u16 sTestGbaImage[WIDE_VIEW_MAX_W * WIDE_VIEW_MAX_H];
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

static void TestWriteShot(const char *path)
{
    FILE *f = fopen(path, "wb");
    if (f == NULL)
        return;
    fprintf(f, "P6\n%d %d\n255\n", sTestFrameInfo.width, sTestFrameInfo.height);
    for (int i = 0; i < sTestFrameInfo.width * sTestFrameInfo.height; i++)
    {
        u16 c = sTestGbaImage[i];
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
        if (!strcmp(cmd, "shot"))
        {
            TestWriteShot(arg1);
            continue;
        }
        if (!strcmp(cmd, "quit"))
            return false;
    }
    return false;
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
    sdlWindow = SDL_CreateWindow("Pokemon Wally's Journey", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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
    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_PRESENTVSYNC);
#endif
    if (sdlRenderer == NULL)
    {
        DBGPRINTF("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

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

    sdlTexture = SDL_CreateTexture(sdlRenderer,
                                   SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   WIDE_VIEW_MAX_W, WIDE_VIEW_MAX_H);
    if (sdlTexture == NULL)
    {
        DBGPRINTF("Texture could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetTextureBlendMode(sdlTexture, SDL_BLENDMODE_NONE);

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
            keyboardKeys = sTestKeys;
            WideView_DrawFrame(sTestGbaImage, &sTestFrameInfo);
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

            while (accumulator >= dt)
            {
                if (SDL_AtomicGet(&isFrameAvailable))
                {
                    VDraw(sdlTexture);
                    SDL_RenderClear(sdlRenderer);
#if defined(NATIVE_LINUX) || defined(_WIN32)
                    u8 backgroundOption = Platform_GetBorderBackground();
                    if (backgroundOption < sBorderBackgroundCount
                     && sdlBackgroundTextures[backgroundOption] != NULL)
                        SDL_RenderCopy(sdlRenderer, sdlBackgroundTextures[backgroundOption], NULL, NULL);
                    int outputWidth;
                    int outputHeight;
                    SDL_GetRendererOutputSize(sdlRenderer, &outputWidth, &outputHeight);
                    int gameHeight;
                    int gameWidth;
                    int srcW = sFrameInfo.width, srcH = sFrameInfo.height;
                    if (sPlatformSettings[PLATFORM_SETTING_INTEGER_SCALE])
                    {
                        int scale = outputWidth / srcW;
                        if (outputHeight / srcH < scale)
                            scale = outputHeight / srcH;
                        if (scale < 1)
                            scale = 1;
                        gameWidth = srcW * scale;
                        gameHeight = srcH * scale;
                    }
                    else
                    {
                        // Fill the window, keeping the picture's shape.
                        gameWidth = outputWidth;
                        if (gameWidth > outputHeight * srcW / srcH)
                            gameWidth = outputHeight * srcW / srcH;
                        gameHeight = gameWidth * srcH / srcW;
                    }
                    SDL_Rect gameViewport = {(outputWidth - gameWidth) / 2,
                                             (outputHeight - gameHeight) / 2,
                                             gameWidth, gameHeight};
                    SDL_Rect gameSource = {0, 0, srcW, srcH};
                    SDL_RenderCopy(sdlRenderer, sdlTexture, &gameSource, &gameViewport);
                    if (sFrameInfo.mode == WIDE_VIEW_GBA && sPlatformSettings[PLATFORM_SETTING_BORDER] && sdlBorderTexture != NULL)
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
#ifdef __ANDROID__
                    SDL_RenderPresent(sdlRenderer);
#endif
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
                }
            }
        }

#ifndef __ANDROID__
        SDL_RenderPresent(sdlRenderer);
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
        else if (sscanf(line, "windowScale=%u", &value) == 1 && value >= 1 && value <= 6)
            sPlatformSettings[PLATFORM_SETTING_WINDOW_SCALE] = value;
        else if (sscanf(line, "integerScale=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_INTEGER_SCALE] = value != 0;
        else if (sscanf(line, "vsync=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_VSYNC] = value != 0;
        else if (sscanf(line, "border=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_BORDER] = value != 0;
        else if (sscanf(line, "volume=%u", &value) == 1 && value <= 10)
            sPlatformSettings[PLATFORM_SETTING_VOLUME] = value;
        else if (sscanf(line, "fastForward=%u", &value) == 1 && value <= 3)
            sPlatformSettings[PLATFORM_SETTING_FF_SPEED] = value;
        else if (sscanf(line, "widescreen=%u", &value) == 1)
            WideView_SetEnabled(value != 0);
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
    fprintf(configFile, "fastForwardToggle=%u\n", sPlatformSettings[PLATFORM_SETTING_FF_TOGGLE]);
    for (int i = 0; i < 7; i++)
        fprintf(configFile, "%s=%u\n", sPadSettingNames[i], sPlatformSettings[PLATFORM_SETTING_PAD_A + i]);
    fclose(configFile);
}

static void ApplyPlatformSettings(void)
{
    SDL_RenderSetVSync(sdlRenderer, sPlatformSettings[PLATFORM_SETTING_VSYNC]);
#if defined(NATIVE_LINUX) || defined(_WIN32)
    SDL_SetWindowFullscreen(sdlWindow, sPlatformSettings[PLATFORM_SETTING_FULLSCREEN]
                                      ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    if (!sPlatformSettings[PLATFORM_SETTING_FULLSCREEN])
    {
        int scale = sPlatformSettings[PLATFORM_SETTING_WINDOW_SCALE];
        // Wide view: the window is 16:9 (1x = 320x180 ... 6x = 1920x1080).
        if (WideView_IsEnabled())
            SDL_SetWindowSize(sdlWindow, 320 * scale, 180 * scale);
        else
            SDL_SetWindowSize(sdlWindow, DISPLAY_WIDTH * scale, DISPLAY_HEIGHT * scale);
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
        // Fast-forward: drop the sound instead of piling it up. Normal speed:
        // never let more than ~1/4 s queue up, so sound stays in sync.
        if (speedUp || SDL_GetQueuedAudioSize(sdlAudioDevice) > 42060 * 2 * sizeof(float) / 4)
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

u8 Platform_GetSetting(enum PlatformSetting setting)
{
    return sPlatformSettings[setting];
}

// Called from the game thread (Options menu). Window changes must happen on
// the main thread, so they are only flagged here and applied between frames.
static SDL_atomic_t sWindowSettingsChanged;

void Platform_SetSetting(enum PlatformSetting setting, u8 value)
{
    sPlatformSettings[setting] = value;
    if (setting == PLATFORM_SETTING_FF_TOGGLE)
        SDL_AtomicSet(&sFfToggled, 0);
    if (setting == PLATFORM_SETTING_VSYNC || setting == PLATFORM_SETTING_FULLSCREEN
     || setting == PLATFORM_SETTING_WINDOW_SCALE)
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
            switch (event.key.keysym.sym)
            {
            HANDLE_KEYUP(A_BUTTON)
            HANDLE_KEYUP(B_BUTTON)
            HANDLE_KEYUP(START_BUTTON)
            HANDLE_KEYUP(SELECT_BUTTON)
            HANDLE_KEYUP(L_BUTTON)
            HANDLE_KEYUP(R_BUTTON)
            HANDLE_KEYUP(DPAD_UP)
            HANDLE_KEYUP(DPAD_DOWN)
            HANDLE_KEYUP(DPAD_LEFT)
            HANDLE_KEYUP(DPAD_RIGHT)
            case SDLK_SPACE:
                SDL_AtomicSet(&sFfKeyHeld, 0);
                break;
            }
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            HANDLE_KEYDOWN(A_BUTTON)
            HANDLE_KEYDOWN(B_BUTTON)
            HANDLE_KEYDOWN(START_BUTTON)
            HANDLE_KEYDOWN(SELECT_BUTTON)
            HANDLE_KEYDOWN(L_BUTTON)
            HANDLE_KEYDOWN(R_BUTTON)
            HANDLE_KEYDOWN(DPAD_UP)
            HANDLE_KEYDOWN(DPAD_DOWN)
            HANDLE_KEYDOWN(DPAD_LEFT)
            HANDLE_KEYDOWN(DPAD_RIGHT)
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
            case SDLK_SPACE:
                // Toggle mode: each press (not key repeat) turns fast forward on or off.
                if (sPlatformSettings[PLATFORM_SETTING_FF_TOGGLE] && !event.key.repeat)
                    SDL_AtomicSet(&sFfToggled, !SDL_AtomicGet(&sFfToggled));
                SDL_AtomicSet(&sFfKeyHeld, 1);
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
    static const double sFfScales[] = {2.0, 3.0, 5.0, 100.0};
    bool on = sPlatformSettings[PLATFORM_SETTING_FF_TOGGLE]
            ? SDL_AtomicGet(&sFfToggled) != 0
            : (SDL_AtomicGet(&sFfKeyHeld) || SDL_AtomicGet(&sFfPadHeld));
    double scale = on ? sFfScales[sPlatformSettings[PLATFORM_SETTING_FF_SPEED] % 4] : 1.0;

    if (on != speedUp || scale != timeScale)
    {
        bool wasOn = speedUp;
        speedUp = on;
        timeScale = scale;
        if (on && !wasOn)
            SDL_PauseAudioDevice(sdlAudioDevice, 1);
        else if (!on && wasOn)
        {
            SDL_ClearQueuedAudio(sdlAudioDevice);
            SDL_PauseAudioDevice(sdlAudioDevice, 0);
        }
    }
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

void VDraw(SDL_Texture *texture)
{
    static uint16_t gbaImage[WIDE_VIEW_MAX_W * WIDE_VIEW_MAX_H];
    static uint32_t image[WIDE_VIEW_MAX_W * WIDE_VIEW_MAX_H];

    sVDrawImage = gbaImage;
    if (!PcGuard_Call0(DrawFrameGuarded))
    {
        // The wide view hit a bad memory access: fall back to the normal picture for good.
        WideView_SetEnabled(FALSE);
        sVDrawImage = gbaImage;
        PcGuard_Call0(DrawFrameGuarded);
    }
    for (int i = 0; i < sFrameInfo.width * sFrameInfo.height; i++)
    {
        uint16_t color = gbaImage[i];
        uint32_t r = (color & 0x1F) * 255 / 31;
        uint32_t g = ((color >> 5) & 0x1F) * 255 / 31;
        uint32_t b = ((color >> 10) & 0x1F) * 255 / 31;
        image[i] = 0xFF000000 | (r << 16) | (g << 8) | b;
    }
    SDL_Rect area = {0, 0, sFrameInfo.width, sFrameInfo.height};
    SDL_UpdateTexture(texture, &area, image, sFrameInfo.width * sizeof(Uint32));
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
