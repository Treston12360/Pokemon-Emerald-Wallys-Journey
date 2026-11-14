#ifndef GUARD_HICOLOR_H
#define GUARD_HICOLOR_H

// ENG-SPRITES: full-colour sprites (see src/platform/hicolor.c)
extern bool8 gHiColorActive;
void HiColor_PrepareFrame(void);
// TRUE = draw *color (already faded like the palette) instead of the 16-colour pixel
bool32 HiColor_ObjPixel(u32 tileNum, u32 x, u32 y, u32 palNum, u32 index, u16 *color);
u32 HiColor_PaletteHash(const u16 *colors16);

#endif
