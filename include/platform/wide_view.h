#ifndef GUARD_PLATFORM_WIDE_VIEW_H
#define GUARD_PLATFORM_WIDE_VIEW_H

// PC wide view (PC port plan step 2). The overworld is drawn on a bigger virtual
// screen; everything else (battles, menus) stays the original 240x160 picture.

#define WIDE_VIEW_MAX_W 640
#define WIDE_VIEW_MAX_H 360

enum WideViewMode
{
    WIDE_VIEW_GBA,      // normal 240x160 picture
    WIDE_VIEW_FIELD,    // wide canvas, map drawn in the margins
    WIDE_VIEW_CENTER,   // wide canvas, GBA picture in the middle, margins black (transitions)
};

struct WideViewFrame
{
    int width;
    int height;
    enum WideViewMode mode;
    // Smooth motion (see WideView_SetSmooth): when margin > 0 the buffers are
    // (width + 2*margin) x (height + 2*margin), the visible picture starting at (margin, margin).
    int margin;
    int camX, camY; // world pixel shown at the visible picture's top-left (FIELD mode)
    int gbaX, gbaY; // FIELD mode: where the GBA screen sits in the buffers (margin included)
};

// Zoom / screen shape: the display code asks for a view size (0, 0 = built-in sizes).
void WideView_SetViewSize(int w, int h);
bool WideView_IsIndoorMap(void);

// UI layer: when set (same size as the frame buffers), BG0 (text boxes, menus, the map name
// pop-up) and screen-space sprites (start menu, HM picture) go here instead of into the frame,
// so the display code can draw them at their own scale. NULL = off.
void WideView_SetUiOverlay(uint16_t *overlay);

// Smooth motion: the overworld is drawn `margin` pixels bigger on every side, and the text
// layer (BG0, text boxes, map name pop-ups) goes to `overlay` instead, so the map can be
// slid between two game frames while text stays put. overlay = NULL turns it off.
#define WIDE_VIEW_SMOOTH_MARGIN 8
void WideView_SetSmooth(uint16_t *overlay, int margin);

// Renders the current frame. `out` must hold (WIDE_VIEW_MAX_W + 2*margin) * (WIDE_VIEW_MAX_H + 2*margin) pixels
// (BGR555). Runs the per-line VCount/HBlank work exactly like DrawFrame.
void WideView_DrawFrame(uint16_t *out, struct WideViewFrame *frame);

void WideView_SetEnabled(bool enabled);
bool WideView_IsEnabled(void);

#endif // GUARD_PLATFORM_WIDE_VIEW_H
