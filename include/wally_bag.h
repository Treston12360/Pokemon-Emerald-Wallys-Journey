#ifndef GUARD_WALLY_BAG_H
#define GUARD_WALLY_BAG_H

// Wally's Journey: unlimited bag.
// Every pocket has a slot for every item that can go in it, so the bag never fills up.
// The pockets live in EWRAM. On save, the first slots of each pocket are copied into the
// normal SaveBlock1 bag (so old builds and the vanilla layout still see a valid bag), and
// the full pockets go into the spare flash sector that used to hold the recorded battle.
// A nonce stored in SaveBlock2 ties that sector to the save slot it belongs to; if they
// don't match (old save, failed write), the bag is rebuilt from the SaveBlock1 copy.

#define WJ_BAG_ITEMS_COUNT     600
#define WJ_BAG_KEYITEMS_COUNT   80
#define WJ_BAG_POKEBALLS_COUNT  30
#define WJ_BAG_TMHM_COUNT      110
#define WJ_BAG_BERRIES_COUNT    70

struct WJBagPockets
{
    struct ItemSlot items[WJ_BAG_ITEMS_COUNT];
    struct ItemSlot keyItems[WJ_BAG_KEYITEMS_COUNT];
    struct ItemSlot pokeBalls[WJ_BAG_POKEBALLS_COUNT];
    struct ItemSlot TMsHMs[WJ_BAG_TMHM_COUNT];
    struct ItemSlot berries[WJ_BAG_BERRIES_COUNT];
};

extern struct WJBagPockets *const gWJBag;

void WallyBag_Clear(void);
void WallyBag_PrepareSave(void);   // called right before any save copies SaveBlock1
void WallyBag_LoadFromSave(void);  // called right after a save slot is loaded

#endif // GUARD_WALLY_BAG_H
