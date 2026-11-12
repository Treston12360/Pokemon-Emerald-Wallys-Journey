/* PC stand-in for Emerald 3DS's 3ds_video.h: only what the voxel modules use (voxel3d_pc.c). */
#ifndef WJ_VOXEL_COMPAT_3DS_VIDEO_H
#define WJ_VOXEL_COMPAT_3DS_VIDEO_H
#include <stdbool.h>
#include <stdint.h>
/* Index of texel (x, y) of a texture `width` wide in the 3DS GPU's tiled (Morton) order. The
 * voxel modules lay out every atlas this way; voxel_gl.c untiles it when uploading. */
uint32_t CtrVideo_Texel(unsigned x, unsigned y, unsigned width);
uint32_t CtrVideo_RGBA8(uint16_t color, bool opaque);
/* GBA BGR555 -> RGBA5551 (R in the top bits, alpha bit 0). */
uint16_t CtrVideo_RGBA5551(uint16_t color);
unsigned CtrVideo_ObjTile(unsigned base, unsigned x, unsigned y, unsigned width, bool color256, bool mapping1d);
/* OBJ palette `index` (16 colours) as the game holds it now. */
const uint16_t *CtrVideo_ExtPalette(unsigned index);
bool CtrVideo_InBattle(void);
#endif
