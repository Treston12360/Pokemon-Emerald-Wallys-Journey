// DLX-3: in-between frames for battles, menus and every other screen that is not the walking field.
//
// The game still runs 60 updates a second. With a frame rate above 60 chosen, each game frame is drawn
// more than once: the extra pictures show the sprites, the scrolling of the backgrounds, the windows and
// the colours part of the way from the last frame to this one, so movement and fades run at the screen's
// rate. Everything else (tiles, text, which sprites exist) is this frame's.
//
// How: sdl2.c's VDraw draws the real frame, then asks for the extra ones while the game thread still
// waits. For each, PcTween_Apply writes the in-between values into OAM, the display registers and the
// palette, the normal renderer draws, and PcTween_End puts everything back as the real frame left it.
// A sprite is followed from frame to frame by the key sprite.c gives its OAM entries (sprite slot and
// piece); one that jumps further than TWEEN_MAX_STEP is not slid (it is a new sprite, or a teleport).
// Screens that run code on every line (HBlank / VCount interrupts) are left at 60.

#include "global.h"
#include "gba/io_reg.h"
#include "platform/dma.h"
#include "platform/pc_tween.h"

#define TWEEN_MAX_STEP 48       // px a frame; further is a jump
#define TWEEN_MAX_SCROLL 32     // px a frame of background scrolling
#define TWEEN_MAX_MATRIX 0x200  // change of a rotation/scaling entry a frame (8.8)
#define IO_SAVED 0xE0           // display, window, blend and DMA registers (the sound ones between are left alone)
#define IO_DISPLAY_END 0x58
#define IO_DMA_START 0xB0

// The display and DMA registers only: the sound registers in between belong to the mixer.
static void IoCopy(u8 *dst, const u8 *src)
{
    memcpy(dst, src, IO_DISPLAY_END);
    memcpy(dst + IO_DMA_START, src + IO_DMA_START, IO_SAVED - IO_DMA_START);
}

extern u16 gWjPcOamKey[128]; // sprite.c

struct TweenFrame
{
    struct OamData oam[128];
    u16 key[128];
    u8 io[IO_SAVED];
    u8 pal[PLTT_SIZE];
    bool8 valid;
};

static struct TweenFrame sPrev, sCur;
static u8 sIoAfter[IO_SAVED];
static s16 sDx[128], sDy[128];
static bool8 sMatrixMoves[32];
static bool8 sAnyOam, sAnyIo, sAnyPal, sReady;
static u8 sKeyToPrev[(129 << 7) + 128];

static const u16 sScrollRegs[] = {
    REG_OFFSET_BG0HOFS, REG_OFFSET_BG0VOFS, REG_OFFSET_BG1HOFS, REG_OFFSET_BG1VOFS,
    REG_OFFSET_BG2HOFS, REG_OFFSET_BG2VOFS, REG_OFFSET_BG3HOFS, REG_OFFSET_BG3VOFS,
};
static const u16 sWindowRegs[] = {REG_OFFSET_WIN0H, REG_OFFSET_WIN1H, REG_OFFSET_WIN0V, REG_OFFSET_WIN1V};
static const u16 sAffineRegs[] = {
    REG_OFFSET_BG2PA, REG_OFFSET_BG2PB, REG_OFFSET_BG2PC, REG_OFFSET_BG2PD,
    REG_OFFSET_BG3PA, REG_OFFSET_BG3PB, REG_OFFSET_BG3PC, REG_OFFSET_BG3PD,
};
static const u16 sAffineRefs[] = {REG_OFFSET_BG2X, REG_OFFSET_BG2Y, REG_OFFSET_BG3X, REG_OFFSET_BG3Y};

static u16 Io16(const u8 *io, u32 offset) { u16 v; memcpy(&v, io + offset, 2); return v; }
static u32 Io32(const u8 *io, u32 offset) { u32 v; memcpy(&v, io + offset, 4); return v; }
static void Put16(u32 offset, u16 v) { memcpy(REG_BASE + offset, &v, 2); }
static void Put32(u32 offset, u32 v) { memcpy(REG_BASE + offset, &v, 4); }

// Shortest way round a wrapping counter of `bits` bits.
static s32 Wrap(s32 d, u32 bits)
{
    s32 half = 1 << (bits - 1);
    return ((d + half) & ((1 << bits) - 1)) - half;
}

// Before the real frame is drawn: what the renderer is about to see becomes "this frame".
void PcTween_BeginFrame(void)
{
    sPrev = sCur;
    memcpy(sCur.oam, OAM, sizeof(sCur.oam));
    memcpy(sCur.key, gWjPcOamKey, sizeof(sCur.key));
    IoCopy(sCur.io, REG_BASE);
    memcpy(sCur.pal, PLTT, PLTT_SIZE);
    sCur.valid = TRUE;
    Dma_SaveState(0);
    sReady = FALSE;
}

// After the real frame is drawn: is there anything to put in between? (Also notes how the frame left
// the registers, for PcTween_End.)
bool32 PcTween_HasMotion(void)
{
    u32 i;

    IoCopy(sIoAfter, REG_BASE);
    Dma_SaveState(1);
    sAnyOam = sAnyIo = sAnyPal = FALSE;
    sReady = FALSE;
    if (!sPrev.valid)
        return FALSE;
    // Code run on every line may not be run twice a frame.
    if (Io16(sCur.io, REG_OFFSET_DISPSTAT) & (DISPSTAT_HBLANK_INTR | DISPSTAT_VCOUNT_INTR))
        return FALSE;
    // The same kind of screen as last frame (not across a mode change or the screen going blank).
    if ((Io16(sCur.io, REG_OFFSET_DISPCNT) ^ Io16(sPrev.io, REG_OFFSET_DISPCNT)) & (7 | DISPCNT_FORCED_BLANK))
        return FALSE;

    memset(sKeyToPrev, 0xFF, sizeof(sKeyToPrev));
    for (i = 0; i < 128; i++)
        if (sPrev.key[i] != 0 && sPrev.key[i] < ARRAY_COUNT(sKeyToPrev))
            sKeyToPrev[sPrev.key[i]] = i;
    for (i = 0; i < 128; i++)
    {
        const struct OamData *cur = &sCur.oam[i], *prev;
        s32 dx, dy;

        sDx[i] = sDy[i] = 0;
        if (sCur.key[i] == 0 || sCur.key[i] >= ARRAY_COUNT(sKeyToPrev) || sKeyToPrev[sCur.key[i]] == 0xFF)
            continue;
        prev = &sPrev.oam[sKeyToPrev[sCur.key[i]]];
        if (cur->affineMode == ST_OAM_AFFINE_ERASE || prev->affineMode == ST_OAM_AFFINE_ERASE
         || cur->shape != prev->shape || cur->size != prev->size || cur->affineMode != prev->affineMode)
            continue;
        dx = Wrap((s32)cur->x - (s32)prev->x, 9);
        dy = Wrap((s32)cur->y - (s32)prev->y, 8);
        if ((dx == 0 && dy == 0) || abs(dx) > TWEEN_MAX_STEP || abs(dy) > TWEEN_MAX_STEP)
            continue;
        sDx[i] = dx;
        sDy[i] = dy;
        sAnyOam = TRUE;
    }
    for (i = 0; i < 32; i++)
    {
        u32 k;

        sMatrixMoves[i] = FALSE;
        for (k = 0; k < 4; k++)
        {
            s32 d = (s16)sCur.oam[4 * i + k].affineParam - (s16)sPrev.oam[4 * i + k].affineParam;
            if (abs(d) > TWEEN_MAX_MATRIX)
                break;
            if (d != 0)
                sMatrixMoves[i] = TRUE;
        }
        if (k < 4)
            sMatrixMoves[i] = FALSE;
        if (sMatrixMoves[i])
            sAnyOam = TRUE;
    }
    for (i = 0; i < ARRAY_COUNT(sScrollRegs); i++)
        if (Io16(sCur.io, sScrollRegs[i]) != Io16(sPrev.io, sScrollRegs[i]))
            sAnyIo = TRUE;
    for (i = 0; i < ARRAY_COUNT(sWindowRegs); i++)
        if (Io16(sCur.io, sWindowRegs[i]) != Io16(sPrev.io, sWindowRegs[i]))
            sAnyIo = TRUE;
    for (i = 0; i < ARRAY_COUNT(sAffineRegs); i++)
        if (Io16(sCur.io, sAffineRegs[i]) != Io16(sPrev.io, sAffineRegs[i]))
            sAnyIo = TRUE;
    for (i = 0; i < ARRAY_COUNT(sAffineRefs); i++)
        if (Io32(sCur.io, sAffineRefs[i]) != Io32(sPrev.io, sAffineRefs[i]))
            sAnyIo = TRUE;
    sAnyPal = memcmp(sCur.pal, sPrev.pal, PLTT_SIZE) != 0;
    sReady = sAnyOam || sAnyIo || sAnyPal;
    return sReady;
}

// `back`: how much of the last step is still to go, 256ths (256 = last frame, 0 = this frame).
static s32 Part(s32 d, s32 back)
{
    return d * back / 256;
}

// Set the screen up as it was `t` of the way from the last frame to this one (0 < t < 1).
void PcTween_Apply(float t)
{
    s32 back = (s32)((1.0f - t) * 256.0f + 0.5f);
    struct OamData *oam = (struct OamData *)OAM;
    u32 i;

    if (!sReady)
        return;
    // As the real frame found things: registers and line-by-line copies start over.
    IoCopy(REG_BASE, sCur.io);
    Dma_RestoreState(0);
    memcpy(OAM, sCur.oam, sizeof(sCur.oam));

    for (i = 0; i < 128; i++)
    {
        if (sDx[i] == 0 && sDy[i] == 0)
            continue;
        oam[i].x = (sCur.oam[i].x - Part(sDx[i], back)) & 0x1FF;
        oam[i].y = (sCur.oam[i].y - Part(sDy[i], back)) & 0xFF;
    }
    for (i = 0; i < 32; i++)
    {
        u32 k;

        if (!sMatrixMoves[i])
            continue;
        for (k = 0; k < 4; k++)
        {
            s32 cur = (s16)sCur.oam[4 * i + k].affineParam, prev = (s16)sPrev.oam[4 * i + k].affineParam;
            oam[4 * i + k].affineParam = (u16)(cur - Part(cur - prev, back));
        }
    }

    for (i = 0; i < ARRAY_COUNT(sScrollRegs); i++)
    {
        s32 cur = Io16(sCur.io, sScrollRegs[i]) & 0x1FF;
        s32 d = Wrap(cur - (Io16(sPrev.io, sScrollRegs[i]) & 0x1FF), 9);
        if (d != 0 && abs(d) <= TWEEN_MAX_SCROLL)
            Put16(sScrollRegs[i], (cur - Part(d, back)) & 0x1FF);
    }
    for (i = 0; i < ARRAY_COUNT(sWindowRegs); i++)
    {
        // Two edges a register (low byte: right/bottom, high byte: left/top).
        u16 cur = Io16(sCur.io, sWindowRegs[i]), prev = Io16(sPrev.io, sWindowRegs[i]);
        s32 dLo = (cur & 0xFF) - (prev & 0xFF), dHi = (cur >> 8) - (prev >> 8);
        if (cur != prev && abs(dLo) <= TWEEN_MAX_STEP && abs(dHi) <= TWEEN_MAX_STEP)
            Put16(sWindowRegs[i], (u16)((((cur >> 8) - Part(dHi, back)) << 8) | (((cur & 0xFF) - Part(dLo, back)) & 0xFF)));
    }
    for (i = 0; i < ARRAY_COUNT(sAffineRegs); i++)
    {
        s32 cur = (s16)Io16(sCur.io, sAffineRegs[i]), d = cur - (s16)Io16(sPrev.io, sAffineRegs[i]);
        if (d != 0 && abs(d) <= TWEEN_MAX_MATRIX)
            Put16(sAffineRegs[i], (u16)(cur - Part(d, back)));
    }
    for (i = 0; i < ARRAY_COUNT(sAffineRefs); i++)
    {
        // 28-bit signed, 8 fraction bits.
        s32 cur = (s32)(Io32(sCur.io, sAffineRefs[i]) << 4) >> 4;
        s32 d = cur - ((s32)(Io32(sPrev.io, sAffineRefs[i]) << 4) >> 4);
        if (d != 0 && abs(d) <= (TWEEN_MAX_SCROLL << 8))
            Put32(sAffineRefs[i], (u32)(cur - (s32)((s64)d * back / 256)) & 0x0FFFFFFF);
    }

    if (sAnyPal)
    {
        const u16 *cur = (const u16 *)sCur.pal, *prev = (const u16 *)sPrev.pal;
        u16 *out = (u16 *)PLTT;

        for (i = 0; i < PLTT_SIZE / 2; i++)
        {
            u16 c = cur[i], p = prev[i];
            if (c != p)
            {
                s32 r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
                r -= Part(r - (p & 31), back);
                g -= Part(g - ((p >> 5) & 31), back);
                b -= Part(b - ((p >> 10) & 31), back);
                out[i] = (u16)(r | (g << 5) | (b << 10) | (c & 0x8000));
            }
        }
    }
}

// Everything back as the real frame left it.
void PcTween_End(void)
{
    if (!sReady)
        return;
    memcpy(OAM, sCur.oam, sizeof(sCur.oam));
    memcpy(PLTT, sCur.pal, PLTT_SIZE);
    IoCopy(REG_BASE, sIoAfter);
    Dma_RestoreState(1);
}
