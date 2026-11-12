/* PC stand-in for Emerald 3DS's port_platform.h (voxel3d_pc.c). On PC every asset pointer is real. */
#ifndef WJ_VOXEL_COMPAT_PORT_PLATFORM_H
#define WJ_VOXEL_COMPAT_PORT_PLATFORM_H
#include "global.h"
#define Port_ResolveAssetPointer(p) (p)
#define Port_ResolveAssetPointerSized(p, n) (p)
u32 Port_GetAssetSizeExact(const void *p);
const struct MapLayout *Port_GetMapLayoutById(u16 id);
u32 Port_GetSpriteFrameSize(const void *data, u32 size);
const u8 *Port_PeekSpriteFramePointer(const void *data, u32 size, u32 offset);
u32 GetMapClosedEdges(void);
const u16 *MapGridGetCameraBorderAt(int x, int y, u8 *index);
struct Sprite;
u8 GetSpriteExtPalette(const struct Sprite *sprite);
#endif
