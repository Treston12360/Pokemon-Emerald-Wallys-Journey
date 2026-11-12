#ifndef GUARD_PLATFORM_H
#define GUARD_PLATFORM_H

#include "global.h"
#include "siirtc.h"

void Platform_StoreSaveFile(void);
void Platform_ReadFlash(u16 sectorNum, u32 offset, u8 *dest, u32 size);
void Platform_QueueAudio(float *audioBuffer, s32 samplesPerFrame);
u16 Platform_GetKeyInput(void);
u8 Platform_GetBorderBackgroundCount(void);
u8 Platform_GetBorderBackground(void);
void Platform_SetBorderBackground(u8 selection);

#define FF_SPEED_COUNT 10

// Frame rate choices (frames drawn per second; the game itself always runs at 60).
#define FRAME_RATE_COUNT 6
extern const u16 gPlatformFrameRates[FRAME_RATE_COUNT];

enum PlatformSetting
{
    PLATFORM_SETTING_FULLSCREEN,
    PLATFORM_SETTING_WINDOW_SCALE,
    PLATFORM_SETTING_INTEGER_SCALE, // mGBA "Force integer scaling"
    PLATFORM_SETTING_VSYNC,
    PLATFORM_SETTING_BORDER,
    PLATFORM_SETTING_VOLUME,
    PLATFORM_SETTING_FF_SPEED, // 0 = 2x, 1 = 4x, ... 9 = 20x (FF_SPEED_COUNT steps of 2x)
    PLATFORM_SETTING_FF_TOGGLE, // 0 = hold the button, 1 = press once to turn on/off
    // Controller button for each game button (enum PadButton).
    PLATFORM_SETTING_PAD_A,
    PLATFORM_SETTING_PAD_B,
    PLATFORM_SETTING_PAD_START,
    PLATFORM_SETTING_PAD_SELECT,
    PLATFORM_SETTING_PAD_L,
    PLATFORM_SETTING_PAD_R,
    PLATFORM_SETTING_PAD_FF,
    PLATFORM_SETTING_WIDESCREEN, // kept by the wide view (platform/wide_view.c), not in the array
    PLATFORM_SETTING_SMOOTH,     // frames drawn per second: index into gPlatformFrameRates (0 = 60, the original)
    PLATFORM_SETTING_FILTER,     // screen filter: 0 = off, 1 = scanlines, 2 = LCD grid
    PLATFORM_SETTING_ZOOM,       // overworld zoom level, index into the view heights in sdl2.c (3 = normal)
    PLATFORM_SETTING_SMOOTHING,  // mGBA "Bilinear filtering"
    PLATFORM_SETTING_LOCK_ASPECT, // mGBA "Lock aspect ratio" (off = stretch to the whole screen)
    PLATFORM_SETTING_INTERFRAME, // mGBA "Interframe blending" (mix each frame with the one before)
    PLATFORM_SETTING_LOCK_FRAME, // mGBA "Lock frame size" (window can't be resized by dragging)
    PLATFORM_SETTING_RESOLUTION, // 0 = monitor/window, 1 = 720p, 2 = 1080p, 3 = 1440p, 4 = 4K
    PLATFORM_SETTING_COUNT,
};

// Controller buttons (Xbox names) a game button can be put on.
enum PadButton
{
    PAD_BUTTON_A,
    PAD_BUTTON_B,
    PAD_BUTTON_X,
    PAD_BUTTON_Y,
    PAD_BUTTON_LB,
    PAD_BUTTON_RB,
    PAD_BUTTON_LT,
    PAD_BUTTON_RT,
    PAD_BUTTON_BACK,
    PAD_BUTTON_START,
    PAD_BUTTON_LSTICK,
    PAD_BUTTON_RSTICK,
    PAD_BUTTON_NONE,
    PAD_BUTTON_COUNT,
};

// Keyboard keys for each game button (PC Settings > Keyboard).
enum KeyBind
{
    KEYBIND_UP,
    KEYBIND_DOWN,
    KEYBIND_LEFT,
    KEYBIND_RIGHT,
    KEYBIND_A,
    KEYBIND_B,
    KEYBIND_START,
    KEYBIND_SELECT,
    KEYBIND_L,
    KEYBIND_R,
    KEYBIND_FAST_FORWARD,
    KEYBIND_COUNT,
};

// Settings menu: the next key pressed is given to `bind` (Esc cancels). A key already used by
// another button swaps with it.
void Platform_StartKeyCapture(u8 bind);
bool8 Platform_IsCapturingKey(void);
// Name of the key on `bind`, in the game's text encoding (EOS-terminated, at most size-1 chars).
void Platform_GetKeyBindName(u8 bind, u8 *dst, u32 size);

u8 Platform_GetSetting(enum PlatformSetting setting);
void Platform_SetSetting(enum PlatformSetting setting, u8 value);
void Platform_GetStatus(struct SiiRtcInfo *rtc);
void Platform_SetStatus(struct SiiRtcInfo *rtc);
static void UpdateInternalClock(void);
void Platform_GetDateTime(struct SiiRtcInfo *rtc);
void Platform_SetDateTime(struct SiiRtcInfo *rtc);
void Platform_GetTime(struct SiiRtcInfo *rtc);
void Platform_SetTime(struct SiiRtcInfo *rtc);
void Platform_SetAlarm(u8 *alarmData);

#endif
