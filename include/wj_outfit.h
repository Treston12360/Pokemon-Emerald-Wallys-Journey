#ifndef GUARD_WJ_OUTFIT_H
#define GUARD_WJ_OUTFIT_H

#include "constants/wj_outfits.h"

// Wally's Journey: player outfits. Protagonist sprites (overworld, trainer card, battle back)
// come from Emerald Rogue by Pokabbie and its artists; the extra Emerald actions (bikes,
// surfing, fishing...) for protagonists Rogue doesn't draw them for are built from
// Brendan's/May's frames with the outfit's head and colours.

extern u16 gWJPlayerOwPal[16];
extern u16 gWJPlayerFrontPal[16];
extern u16 gWJPlayerBackPal[16];
extern u32 gWJPlayerFrontPicBuf[];
extern u32 gWJPlayerBackPicBuf[];

bool32 WJ_OutfitActive(void);
u32 WJ_OutfitId(void);
void WJ_OutfitRefresh(void);
void WJ_OutfitOnNewGame(void);
void WJ_OutfitOnLoad(void);
const struct ObjectEventGraphicsInfo *WJ_OutfitGraphicsInfo(u16 graphicsId);
const struct ObjectEventGraphicsInfo *WJ_GetOutfitGfxInfo(u32 outfit, u32 slot);
void WJ_OutfitMenu(void);

#endif // GUARD_WJ_OUTFIT_H
