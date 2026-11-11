#ifndef GUARD_PLATFORM_WIDE_VIEW_H
#define GUARD_PLATFORM_WIDE_VIEW_H

// PC wide view (PC port plan step 2). The overworld is drawn on a bigger virtual
// screen; everything else (battles, menus) stays the original 240x160 picture.

#define WIDE_VIEW_MAX_W 384
#define WIDE_VIEW_MAX_H 216

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
};

// Renders the current frame. `out` must hold WIDE_VIEW_MAX_W * WIDE_VIEW_MAX_H pixels
// (BGR555). Runs the per-line VCount/HBlank work exactly like DrawFrame.
void WideView_DrawFrame(uint16_t *out, struct WideViewFrame *frame);

void WideView_SetEnabled(bool enabled);
bool WideView_IsEnabled(void);

#endif // GUARD_PLATFORM_WIDE_VIEW_H
