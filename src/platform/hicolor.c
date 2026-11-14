// ENG-SPRITES (Project ARIA, owner 2026-11-14): sprites without the 16-colour limit.
//
// "I want them to be the same size so they don't stick out, but no limit on the colours."
//
// How it works (PC only, the game code is unchanged):
//  * A full-colour picture is listed in graphics/hicolor/list.txt. tools/pc/hicolor.py makes the normal
//    16-colour version from it (that is what the game loads, so nothing else changes: same size, same
//    sprite sheet, same palette slot, same transparency) and writes src/platform/hicolor_tiles.h: for every
//    8x8 tile of the 16-colour version, its 32 bytes, the hash of its 16-colour palette, and the 64 full-colour
//    pixels it stands for.
//  * Every frame the renderer looks at what is in sprite VRAM. A tile whose bytes match a listed tile,
//    drawn with a palette whose colours match that tile's palette, is drawn with the full-colour pixels.
//    Matching on content means it does not matter how the picture got there (compressed, copied, animated).
//    Anything that does not match exactly is drawn the normal way, so the worst case is the 16-colour look.
//  * Screen fades and flashes work by changing the palette; the full-colour pixel gets the same change
//    its 16-colour stand-in got (faded colour minus original colour, per channel).
#include "global.h"
#include "palette.h"
#include "hicolor.h"
extern char *getenv(const char *);
extern int printf(const char *, ...);

struct HiColorTile
{
    u32 palHash;
    u8 bytes[32];
    u16 pixels[64]; // RGB555, row-major in the tile
};

#include "hicolor_tiles.h" // sHiColorTiles[], HICOLOR_TILE_COUNT

#define OBJ_TILE_COUNT (OBJ_VRAM0_SIZE / 32) // the PC build has 128 KiB of sprite tiles (4096), not the GBA's 32 KiB
#define TABLE_SIZE 8192 // power of two, > 2 * listed tiles

static s16 sTable[TABLE_SIZE];      // hash -> first tile index with those bytes (-1 empty)
static bool8 sTableReady;
static s16 sVramMatch[OBJ_TILE_COUNT]; // per sprite VRAM tile: first candidate, or -1
static u32 sPalHash[16];
bool8 gHiColorActive;                 // any match this frame (renderer skips all work when FALSE)

static u32 HashBytes(const u8 *b)
{
    u32 h = 2166136261u;
    u32 i;
    for (i = 0; i < 32; i++)
        h = (h ^ b[i]) * 16777619u;
    return h;
}

u32 HiColor_PaletteHash(const u16 *colors16)
{
    u32 h = 2166136261u;
    u32 i;
    for (i = 1; i < 16; i++) // colour 0 is transparent
        h = (h ^ (colors16[i] & 0x7FFF)) * 16777619u;
    return h;
}

static void BuildTable(void)
{
    u32 i;
    for (i = 0; i < TABLE_SIZE; i++)
        sTable[i] = -1;
    for (i = 0; i < HICOLOR_TILE_COUNT; i++)
    {
        u32 slot = HashBytes(sHiColorTiles[i].bytes) & (TABLE_SIZE - 1);
        // tiles with equal bytes are next to each other in the list (the tool sorts them); keep the first
        while (sTable[slot] != -1 && memcmp(sHiColorTiles[sTable[slot]].bytes, sHiColorTiles[i].bytes, 32) != 0)
            slot = (slot + 1) & (TABLE_SIZE - 1);
        if (sTable[slot] == -1)
            sTable[slot] = i;
    }
    sTableReady = TRUE;
}

void HiColor_PrepareFrame(void)
{
    const u8 *vram = VRAM_ + 0x10000;
    u32 t, p;

    gHiColorActive = FALSE;
    if (HICOLOR_TILE_COUNT == 0)
        return;
    if (!sTableReady)
        BuildTable();
    for (t = 0; t < OBJ_TILE_COUNT; t++)
    {
        const u8 *b = vram + t * 32;
        u32 slot;
        sVramMatch[t] = -1;
        const u32 *w = (const u32 *)b;
        if ((w[0] | w[1] | w[2] | w[3] | w[4] | w[5] | w[6] | w[7]) == 0)
            continue; // empty tile
        slot = HashBytes(b) & (TABLE_SIZE - 1);
        while (sTable[slot] != -1)
        {
            if (memcmp(sHiColorTiles[sTable[slot]].bytes, b, 32) == 0)
            {
                sVramMatch[t] = sTable[slot];
                gHiColorActive = TRUE;
                break;
            }
            slot = (slot + 1) & (TABLE_SIZE - 1);
        }
    }
    if (gHiColorActive)
        for (p = 0; p < 16; p++)
            sPalHash[p] = HiColor_PaletteHash(&gPlttBufferUnfaded[0x100 + p * 16]);
    // test aid: WJPC_HICOLOR_DEBUG=1 prints, every 2 seconds, how many sprite tiles were matched
    if (getenv("WJPC_HICOLOR_DEBUG"))
    {
        static int sFrames;
        if (++sFrames % 120 == 0)
        {
            int c = 0;
            for (t = 0; t < OBJ_TILE_COUNT; t++)
                c += sVramMatch[t] >= 0;
            printf("hicolor: %d sprite tiles matched\n", c);
        }
    }
}

static inline u16 AddClamp(u16 c, s32 dr, s32 dg, s32 db)
{
    s32 r = (c & 31) + dr, g = ((c >> 5) & 31) + dg, b = ((c >> 10) & 31) + db;
    r = r < 0 ? 0 : r > 31 ? 31 : r;
    g = g < 0 ? 0 : g > 31 ? 31 : g;
    b = b < 0 ? 0 : b > 31 ? 31 : b;
    return r | (g << 5) | (b << 10);
}

bool32 HiColor_ObjPixel(u32 tileNum, u32 x, u32 y, u32 palNum, u32 index, u16 *color)
{
    s32 i;
    const struct HiColorTile *tile;
    u16 shown, original;

    i = sVramMatch[tileNum & (OBJ_TILE_COUNT - 1)];
    if (i < 0)
        return FALSE;
    for (;;)
    {
        tile = &sHiColorTiles[i];
        if (tile->palHash == sPalHash[palNum & 15])
            break;
        // another listed tile with the same bytes but another palette?
        if (++i >= HICOLOR_TILE_COUNT || memcmp(sHiColorTiles[i].bytes, tile->bytes, 32) != 0)
            return FALSE;
    }
    *color = tile->pixels[y * 8 + x];
    shown = ((const u16 *)(PLTT + 0x200))[palNum * 16 + index];
    original = gPlttBufferUnfaded[0x100 + palNum * 16 + index];
    if (shown != original)
        *color = AddClamp(*color, (s32)(shown & 31) - (original & 31), (s32)((shown >> 5) & 31) - ((original >> 5) & 31),
                          (s32)((shown >> 10) & 31) - ((original >> 10) & 31));
    return TRUE;
}
