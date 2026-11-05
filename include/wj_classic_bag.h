#ifndef GUARD_WJ_CLASSIC_BAG_H
#define GUARD_WJ_CLASSIC_BAG_H
// Emerald's bag (src/item_menu.c), reachable from Options → Bag → Classic
#include "item_menu.h"
void Classic_ResetBagScrollPositions(void);
void Classic_CB2_BagMenuFromStartMenu(void);
void Classic_CB2_BagMenuFromBattle(void);
void Classic_CB2_ChooseBerry(void);
void Classic_CB2_ChooseMulch(void);
void Classic_ChooseBerryForMachine(MainCallback exitCallback);
void Classic_CB2_ChooseBall(void);
void Classic_CB2_GoToSellMenu(void);
void Classic_CB2_GoToItemDepositMenu(void);
void Classic_ApprenticeOpenBagMenu(void);
void Classic_FavorLadyOpenBagMenu(void);
void Classic_QuizLadyOpenBagMenu(void);
void Classic_GoToBagMenu(u8 location, u8 pocket, MainCallback exitCallback);
void Classic_CB2_BagMenuRun(void);
void Classic_VBlankCB_BagMenuRun(void);
void Classic_BagDestroyPocketScrollArrowPair(void);
void Classic_Task_FadeAndCloseBagMenu(u8 taskId);
void Classic_UpdatePocketItemList(enum Pocket pocketId);
void Classic_UpdatePocketListPosition(u8 pocketId);
u16 Classic_GetItemListPosition(u8 pocketId);
void Classic_DisplayItemMessage(u8 taskId, u8 fontId, const u8 *str, TaskFunc callback);
void Classic_CloseItemMessage(u8 taskId);
void Classic_CB2_ReturnToBagMenuPocket(void);
bool8 Classic_UseRegisteredKeyItemOnField(void);
void Classic_DoWallyTutorialBagMenu(void);
void Classic_InitOldManBag(void);
void Classic_BagMenu_YesNo(u8 taskId, u8 windowType, const struct YesNoFuncTable *funcTable);
void Classic_SortItemsInBag(struct BagPocket *pocket, enum BagSortOptions type);
#endif
