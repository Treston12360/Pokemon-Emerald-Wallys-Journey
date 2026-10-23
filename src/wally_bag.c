#include "global.h"
#include "save.h"
#include "wally_bag.h"

// Wally's Journey: unlimited bag (see wally_bag.h).

#define WJ_BAG_EXT_MAGIC 0x57424147 // 'WBAG'

struct WJBagSector
{
    u32 magic;
    u32 nonce;
    struct WJBagPockets pockets;
};

// TryRead/WriteSpecialSaveSector copy SECTOR_COUNTER_OFFSET bytes, so the buffer is that big.
union WJBagSectorBuffer
{
    struct WJBagSector s;
    u8 raw[SECTOR_COUNTER_OFFSET];
};

STATIC_ASSERT(sizeof(struct WJBagSector) <= SECTOR_COUNTER_OFFSET - 4, WallyBagFitsInSector);

static EWRAM_DATA union WJBagSectorBuffer sBagSector = {0};

struct WJBagPockets *const gWJBag = &sBagSector.s.pockets;

void WallyBag_Clear(void)
{
    memset(gWJBag, 0, sizeof(*gWJBag));
}

static void CopyPrefixToSaveBlock1(void)
{
    struct Bag *bag = &gSaveBlock1Ptr->bag;
    memcpy(bag->items, gWJBag->items, sizeof(bag->items));
    memcpy(bag->keyItems, gWJBag->keyItems, sizeof(bag->keyItems));
    memcpy(bag->pokeBalls, gWJBag->pokeBalls, sizeof(bag->pokeBalls));
    memcpy(bag->TMsHMs, gWJBag->TMsHMs, sizeof(bag->TMsHMs));
    memcpy(bag->berries, gWJBag->berries, sizeof(bag->berries));
}

static void CopyFromSaveBlock1(void)
{
    struct Bag *bag = &gSaveBlock1Ptr->bag;
    WallyBag_Clear();
    memcpy(gWJBag->items, bag->items, sizeof(bag->items));
    memcpy(gWJBag->keyItems, bag->keyItems, sizeof(bag->keyItems));
    memcpy(gWJBag->pokeBalls, bag->pokeBalls, sizeof(bag->pokeBalls));
    memcpy(gWJBag->TMsHMs, bag->TMsHMs, sizeof(bag->TMsHMs));
    memcpy(gWJBag->berries, bag->berries, sizeof(bag->berries));
}

void WallyBag_PrepareSave(void)
{
    struct Pokedex *dex = &gSaveBlock2Ptr->pokedex;

    // The vanilla-sized bag in SaveBlock1 always holds the first slots of each pocket.
    CopyPrefixToSaveBlock1();

    if (dex->wjBagExtMagic != WJ_BAG_EXT_MAGIC)
    {
        dex->wjBagExtMagic = WJ_BAG_EXT_MAGIC;
        dex->wjBagExtNonce = 0;
    }
    dex->wjBagExtNonce++;

    // The full pockets go to the spare sector, tagged with the nonce this save will carry.
    // If this write fails, the nonces won't match on load and the SaveBlock1 copy is used.
    sBagSector.s.magic = WJ_BAG_EXT_MAGIC;
    sBagSector.s.nonce = dex->wjBagExtNonce;
    TryWriteSpecialSaveSector(SECTOR_ID_RECORDED_BATTLE, sBagSector.raw);
}

void WallyBag_LoadFromSave(void)
{
    struct Pokedex *dex = &gSaveBlock2Ptr->pokedex;

    if (dex->wjBagExtMagic == WJ_BAG_EXT_MAGIC
     && TryReadSpecialSaveSector(SECTOR_ID_RECORDED_BATTLE, sBagSector.raw) == SAVE_STATUS_OK
     && sBagSector.s.magic == WJ_BAG_EXT_MAGIC
     && sBagSector.s.nonce == dex->wjBagExtNonce)
        return; // full pockets loaded straight into place

    // Old save (or the sector belongs to another save): start from the SaveBlock1 bag.
    CopyFromSaveBlock1();
}
