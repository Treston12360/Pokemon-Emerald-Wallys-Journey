#include "global.h"
#include "caps.h"
#include "event_data.h"
#include "item.h"
#include "pokemon.h"
#include "script_pokemon_util.h"
#include "wally_journey.h"
#include "test/overworld_script.h"
#include "test/test.h"
#include "constants/items.h"

// Wally's Journey (owner request 2026-10-29): NUZLOCKE and LEVEL CAP options.

static void ClearWallyFlags(void)
{
    FlagClear(FLAG_WALLY_NUZLOCKE);
    FlagClear(FLAG_WALLY_LEVEL_CAP);
    FlagClear(FLAG_BADGE01_GET);
    FlagClear(FLAG_BADGE02_GET);
}

TEST("Wally: LEVEL CAP off means no cap")
{
    ClearWallyFlags();
    EXPECT_EQ(GetCurrentLevelCap(), MAX_LEVEL);
}

TEST("Wally: LEVEL CAP on follows the next Gym Leader's ace")
{
    ClearWallyFlags();
    FlagSet(FLAG_WALLY_LEVEL_CAP);
    EXPECT_EQ(GetCurrentLevelCap(), 15); // Roxanne
    FlagSet(FLAG_BADGE01_GET);
    EXPECT_EQ(GetCurrentLevelCap(), 19); // Brawly
    ClearWallyFlags();
}

TEST("Wally: Nuzlocke keeps a fainted Pokemon fainted when healed")
{
    u32 hp = 0;
    ClearWallyFlags();
    ZeroPlayerPartyMons();
    RUN_OVERWORLD_SCRIPT(
        givemon SPECIES_WOBBUFFET, 20;
        givemon SPECIES_WOBBUFFET, 20;
    );
    SetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_HP, &hp);
    FlagSet(FLAG_WALLY_NUZLOCKE);
    HealPlayerParty();
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_HP), 0);
    EXPECT(FlagGet(FLAG_WALLY_NUZLOCKE));
    ClearWallyFlags();
}

TEST("Wally: Nuzlocke blocks Revive on a fainted Pokemon")
{
    u32 hp = 0;
    ClearWallyFlags();
    ZeroPlayerPartyMons();
    RUN_OVERWORLD_SCRIPT(
        givemon SPECIES_WOBBUFFET, 20;
        givemon SPECIES_WOBBUFFET, 20;
    );
    SetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_HP, &hp);
    FlagSet(FLAG_WALLY_NUZLOCKE);
    // TRUE = the item had no effect
    EXPECT(ExecuteTableBasedItemEffect(&gParties[B_TRAINER_PLAYER][1], ITEM_REVIVE, 1, 0));
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_HP), 0);
    FlagClear(FLAG_WALLY_NUZLOCKE);
    EXPECT(!ExecuteTableBasedItemEffect(&gParties[B_TRAINER_PLAYER][1], ITEM_REVIVE, 1, 0));
    EXPECT_GT(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_HP), 0);
    ClearWallyFlags();
}

TEST("Wally: a whole-party faint ends the Nuzlocke run and heals normally")
{
    u32 hp = 0;
    ClearWallyFlags();
    ZeroPlayerPartyMons();
    RUN_OVERWORLD_SCRIPT(
        givemon SPECIES_WOBBUFFET, 20;
    );
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, &hp);
    FlagSet(FLAG_WALLY_NUZLOCKE);
    HealPlayerParty();
    EXPECT(!FlagGet(FLAG_WALLY_NUZLOCKE));
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP), GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MAX_HP));
    ClearWallyFlags();
}
