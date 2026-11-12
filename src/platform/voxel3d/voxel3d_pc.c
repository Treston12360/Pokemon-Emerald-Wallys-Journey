// Wally's Journey PC: what Emerald 3DS's voxel modules expect from their 3DS port, done the PC way.
// The modules themselves are Emerald 3DS's (CC BY-NC-SA 4.0, see NOTICE.md); this file is ours.
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "fieldmap.h"
#include "sprite.h"
#include "palette.h"
#include "3ds_video.h"
#include "port_platform.h"
#include "3ds_field_view.h"
#include "3ds_platform.h"

extern const struct MapLayout *const gMapLayouts[];
u16 WideView_NearestBorderMetatile(s32 gx, s32 gy);

void Voxel3D_Log(const char *fmt, ...)
{
    static int on = -1;
    va_list ap;

    if (on < 0)
        on = getenv("WJPC_VOXEL_LOG") != NULL;
    if (!on)
        return;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    if (fmt[0] != '\0' && fmt[strlen(fmt) - 1] != '\n')
        fputc('\n', stderr);
}

uint32_t CtrVideo_Texel(unsigned x, unsigned y, unsigned width)
{
    unsigned morton = (x & 1) | ((y & 1) << 1) | ((x & 2) << 1)
                    | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3);
    return ((y / 8) * (width / 8) + x / 8) * 64 + morton;
}

uint32_t CtrVideo_RGBA8(uint16_t color, bool opaque)
{
    unsigned r = color & 31, g = (color >> 5) & 31, b = (color >> 10) & 31;
    r = (r << 3) | (r >> 2);
    g = (g << 3) | (g >> 2);
    b = (b << 3) | (b >> 2);
    return (r << 24) | (g << 16) | (b << 8) | (opaque ? 255 : 0);
}

uint16_t CtrVideo_RGBA5551(uint16_t color)
{
    return ((color & 31) << 11) | ((color & 0x3e0) << 1) | ((color >> 9) & 0x3e) | 1;
}

unsigned CtrVideo_ObjTile(unsigned base, unsigned x, unsigned y, unsigned width, bool color256, bool mapping1d)
{
    unsigned units = color256 ? 2 : 1;
    if (color256)
        base &= ~1u;
    return (base + y * (mapping1d ? width / 8 * units : 32) + x * units) & 1023;
}

const uint16_t *CtrVideo_ExtPalette(unsigned index)
{
    // OBJ palettes as shown (fades included): the second half of palette RAM.
    return (const uint16_t *)(PLTT + 0x200) + 16 * (index & 15);
}

bool CtrVideo_InBattle(void)
{
    return FALSE;
}

// The PC doesn't keep asset sizes: "big enough". A compressed asset says its own size in its
// header; an uncompressed tileset is clamped to the tile area it fills (voxel_world.c).
u32 Port_GetAssetSizeExact(const void *p)
{
    return p != NULL ? 0x01000000u : 0;
}

const struct MapLayout *Port_GetMapLayoutById(u16 id)
{
    return id == 0 ? NULL : gMapLayouts[id - 1];
}

u32 Port_GetSpriteFrameSize(const void *data, u32 size)
{
    return size;
}

const u8 *Port_PeekSpriteFramePointer(const void *data, u32 size, u32 offset)
{
    return data == NULL ? NULL : (const u8 *)data + offset;
}

u32 GetMapClosedEdges(void)
{
    // The PC draws the maps around the current one (and their filler), so no edge stops the camera.
    return 0;
}

const u16 *MapGridGetCameraBorderAt(int x, int y, u8 *index)
{
    static u16 sBorder;

    sBorder = WideView_NearestBorderMetatile(x + MAP_OFFSET, y + MAP_OFFSET);
    *index = 0;
    return &sBorder;
}

u8 GetSpriteExtPalette(const struct Sprite *sprite)
{
    return sprite->oam.paletteNum;
}

float CtrFieldView_Limit(float at, float before, float after, float low, float high, unsigned closed)
{
    float least = low + before, most = high - after;

    if ((closed & CTR_FIELD_EDGE_LOW) && (closed & CTR_FIELD_EDGE_HIGH) && least > most)
        return (low + high + before - after) * 0.5f;
    if ((closed & CTR_FIELD_EDGE_LOW) && at < least)
        return least;
    if ((closed & CTR_FIELD_EDGE_HIGH) && at > most)
        return most;
    return at;
}

void CtrFieldView_SetVoxelShift(int x, int y)
{
}

static int sVoxelPitch = 40, sVoxelZoom = 100;
static bool sVoxelOn;

int CtrSettings_VoxelPitch(void)
{
    return sVoxelPitch;
}

int CtrSettings_VoxelZoom(void)
{
    return sVoxelZoom;
}

bool CtrSettings_Voxel(void)
{
    return sVoxelOn;
}

void Voxel3D_SetSettings(bool on, int pitch, int zoom)
{
    sVoxelOn = on;
    if (pitch > 0)
        sVoxelPitch = pitch;
    if (zoom > 0)
        sVoxelZoom = zoom;
}

// Weather sprites are drawn over the view, not as cards in the world: on PC the screen-space ones.
bool8 CtrSprite_IsVoxelWeather(const struct Sprite *sprite)
{
    return !sprite->coordOffsetEnabled;
}

bool8 CtrSprite_IsVoxelCloud(const struct Sprite *sprite)
{
    return FALSE;
}

// The opening door's frame (the 3D view has no door animation yet).
bool8 PortDoor_Get(s16 *x, s16 *y, u8 *width, const u8 **tiles, const u8 **palettes)
{
    return FALSE;
}

bool8 PortDoor_GetTwin(s16 *x, s16 *y)
{
    return FALSE;
}
